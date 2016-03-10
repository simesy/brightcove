<?php

/**
 * @file
 * Contains \Drupal\brightcove\Plugin\QueueWorker\BrightcoveClientQueueWorker.
 */

namespace Drupal\brightcove\Plugin\QueueWorker;

use Drupal\brightcove\BrightcoveUtil;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Queue\QueueInterface;
use Drupal\Core\Queue\QueueWorkerBase;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Processes Entity Update Tasks for My Module.
 *
 * @QueueWorker(
 *   id = "brightcove_client_queue_worker",
 *   title = @Translation("Brightcove client queue worker."),
 *   cron = { "time" = 30 }
 * )
 */
class BrightcoveClientQueueWorker extends QueueWorkerBase implements ContainerFactoryPluginInterface {
  /**
   * The video page queue object.
   *
   * @var \Drupal\Core\Queue\QueueInterface
   */
  protected $video_page_queue;

  /**
   * The playlist page queue object.
   *
   * @var \Drupal\Core\Queue\QueueInterface
   */
  protected $playlist_page_queue;

  /**
   * Constructs a new BrightcoveClientQueueWorker object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Queue\QueueInterface $video_page_queue
   *   The video page queue object.
   * @param \Drupal\Core\Queue\QueueInterface $playlist_page_queue
   *   The video page queue object.
   */
  public function __construct(array $configuration, $plugin_id, array $plugin_definition, QueueInterface $video_page_queue, QueueInterface $playlist_page_queue) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->video_page_queue = $video_page_queue;
    $this->playlist_page_queue = $playlist_page_queue;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('queue')->get('brightcove_video_page_queue_worker'),
      $container->get('queue')->get('brightcove_playlist_page_queue_worker')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function processItem($data) {
    $cms = BrightcoveUtil::getCMSAPI($data);
    $items_per_page = 100;

    // Create queue items for each video page.
    $video_count = $cms->countVideos();
    $page = 0;
    while ($page * $items_per_page < $video_count) {
      $this->video_page_queue->createItem(array(
        'api_client_id' => $data,
        'page' => $page,
        'items_per_page' => $items_per_page,
      ));
      $page++;
    }

    // Create queue items for each playlist page.
    $playlist_count = $cms->countPlaylists();
    $page = 0;
    while ($page * $items_per_page < $playlist_count) {
      $this->playlist_page_queue->createItem(array(
        'api_client_id' => $data,
        'page' => $page,
        'items_per_page' => $items_per_page,
      ));
      $page++;
    }
  }
}
