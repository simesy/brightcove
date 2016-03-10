<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\StatusOverviewForm.
 */

namespace Drupal\brightcove\Form;

use Drupal\Core\Database\Connection;
use Drupal\Core\Entity\EntityTypeManager;
use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Queue\QueueWorkerInterface;
use Drupal\Core\Queue\SuspendQueueException;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\Core\Queue\QueueFactory;

/**
 * Class StatusOverviewForm.
 *
 * @package Drupal\brightcove\Form
 */
class StatusOverviewForm extends FormBase {

  /**
   * The queue factory.
   *
   * @var \Drupal\Core\Queue\QueueFactory
   */
  protected $queueFactory;

  /**
   * The database connection.
   *
   * @var \Drupal\Core\Database\Connection
   */
  protected $connection;

  /**
   * The entity type manager.
   *
   * @var \Drupal\Core\Entity\EntityTypeManager
   */
  protected $entityTypeManager;

  /**
   * Constructs a StatusOverviewForm object.
   *
   * @param \Drupal\Core\Queue\QueueFactory $queue
   *   The queue factory.
   */
  public function __construct(QueueFactory $queueFactory, Connection $connection, EntityTypeManager $entityTypeManager) {
    $this->queueFactory = $queueFactory;
    $this->connection = $connection;
    $this->entityTypeManager = $entityTypeManager;
  }

  /**
   * @inheritdoc
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('queue'),
      $container->get('database'),
      $container->get('entity_type.manager')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'queue_overview_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $counts = [
      'client' => $this->entityTypeManager->getStorage('brightcove_api_client')->getQuery()->count()->execute(),
      'video' => $this->entityTypeManager->getStorage('brightcove_video')->getQuery()->count()->execute(),
      'playlist' => $this->entityTypeManager->getStorage('brightcove_playlist')->getQuery()->count()->execute(),
    ];
    $queues = [
      'client' => $this->t('Client'),
      'video' => $this->t('Video'),
      'playlist' => $this->t('Playlist'),
    ];

    // There is no form element (ie. widget) in the table, so it's safe to
    // return a render array for a table as a part of the form build array.
    $form['queues'] = [
      '#type' => 'table',
      '#header' => [
        $this->t('Name'),
        $this->t('Number of entities'),
        $this->t('Item(s) in queue'),
      ],
      '#rows' => [],
    ];
    foreach ($queues as $queue => $title) {
      $form['queues']['#rows'][$queue] = [
        $title,
        $counts[$queue],
        $this->queueFactory->get("brightcove_{$queue}_queue_worker")->numberOfItems(),
      ];
    }

    $form['sync'] = array(
      '#name' => 'sync',
      '#type' => 'submit',
      '#value' => $this->t('Sync all'),
    );
    $form['run'] = array(
      '#name' => 'run',
      '#type' => 'submit',
      '#value' => $this->t('Run all queues'),
    );
    $form['clear'] = array(
      '#name' => 'clear',
      '#type' => 'submit',
      '#value' => $this->t('Clear all queues'),
      '#description' => $this->t('Remove all items from all queues'),
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {
    if ($triggering_element = $form_state->getTriggeringElement()) {
      $batch_operations = [];
      switch ($triggering_element['#name']) {
        case 'sync':
          $batch_operations[] = ['_brightcove_initiate_sync', []];
          // There is intentionally no break here.
        case 'run':
          $batch_operations[] = [[self::class, 'runQueue'], ['brightcove_client_queue_worker']];
          $batch_operations[] = [[self::class, 'runQueue'], ['brightcove_video_page_queue_worker']];
          $batch_operations[] = [[self::class, 'runQueue'], ['brightcove_video_queue_worker']];
          $batch_operations[] = [[self::class, 'runQueue'], ['brightcove_playlist_page_queue_worker']];
          $batch_operations[] = [[self::class, 'runQueue'], ['brightcove_playlist_queue_worker']];
          break;

        case 'clear':
          $batch_operations[] = [[self::class, 'clearQueue'], ['brightcove_client_queue_worker']];
          $batch_operations[] = [[self::class, 'clearQueue'], ['brightcove_video_page_queue_worker']];
          $batch_operations[] = [[self::class, 'clearQueue'], ['brightcove_video_queue_worker']];
          $batch_operations[] = [[self::class, 'clearQueue'], ['brightcove_playlist_page_queue_worker']];
          $batch_operations[] = [[self::class, 'clearQueue'], ['brightcove_playlist_queue_worker']];
          break;
      }
      if ($batch_operations) {
        // Reset expired items in the default queue implementation table. If
        // that's not used, this will simply be a no-op.
        // @see system_cron()
        $this->connection->update('queue')
          ->fields(array(
            'expire' => 0,
          ))
          ->condition('expire', 0, '<>')
          ->condition('expire', REQUEST_TIME, '<')
          ->condition('name', 'brightcove_%', 'LIKE')
          ->execute();

        batch_set([
          'operations' => $batch_operations,
        ]);
      }
    }
  }

  /**
   * Clears a queue.
   *
   * @param $queue
   *   The queue name to clear.
   */
  public static function clearQueue($queue) {
    // This is a static function called by Batch API, so it's not possible to
    // use dependency injection here.
    \Drupal::queue($queue)->deleteQueue();
  }

  /**
   * Runs a queue.
   *
   * @param $queue
   *   The queue name to clear.
   * @param &$context
   *   The Batch API context.
   */
  public static function runQueue($queue, &$context) {
    // This is a static function called by Batch API, so it's not possible to
    // use dependency injection here.
    /** @var QueueWorkerInterface $queue_worker */
    $queue_worker = \Drupal::getContainer()->get('plugin.manager.queue_worker')->createInstance($queue);
    $queue = \Drupal::queue($queue);

    // Let's process ALL the items in the queue, 5 by 5, to avoid PHP timeouts.
    // If there's any problem with processing any of those 5 items, stop sooner.
    $limit = 5;
    $handled_all = TRUE;
    while (($limit > 0) && ($item = $queue->claimItem(5))) {
      try {
        $queue_worker->processItem($item->data);
        $queue->deleteItem($item);
      }
      catch (SuspendQueueException $e) {
        $queue->releaseItem($item);
        $handled_all = FALSE;
        break;
      }
      catch (\Exception $e) {
        watchdog_exception(self::class, $e);
        \Drupal::logger('brightcove')->notice($e->getMessage());
        $handled_all = FALSE;
      }
      $limit--;
    }

    // As this batch may be run synchronously with the queue's cron processor,
    // we can't be sure about the number of items left for the batch as long as
    // there is any. So let's just inform the user about the number of remaining
    // items, as we don't really care if they are processed by this batch
    // processor or the cron one.
    $remaining = $queue->numberOfItems();
    $context['message'] = t('@count item(s) left in current queue', ['@count' => $remaining]);
    $context['finished'] = $handled_all && ($remaining == 0);
  }

}
