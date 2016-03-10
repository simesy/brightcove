<?php

/**
 * @file
 * Contains \Drupal\brightcove\Plugin\QueueWorker\BrightcoveVideoQueueWorker.
 */

namespace Drupal\brightcove\Plugin\QueueWorker;

use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Queue\QueueWorkerBase;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\brightcove\Entity\BrightcoveVideo;

/**
 * Processes Entity Update Tasks for My Module.
 *
 * @QueueWorker(
 *   id = "brightcove_video_queue_worker",
 *   title = @Translation("Brightcove video queue worker."),
 *   cron = { "time" = 30 }
 * )
 */
class BrightcoveVideoQueueWorker extends QueueWorkerBase implements ContainerFactoryPluginInterface {
  /**
   * The brightcove_video storage.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $storage;

  /**
   * Constructs a new BrightcoveVideoQueueWorker object.
   *
   * @param array $configuration
   *   A configuration array containing information about the plugin instance.
   * @param string $plugin_id
   *   The plugin_id for the plugin instance.
   * @param array $plugin_definition
   *   The plugin implementation definition.
   * @param \Drupal\Core\Entity\EntityStorageInterface $storage
   *   The storage object.
   */
  public function __construct(array $configuration, $plugin_id, $plugin_definition, EntityStorageInterface $storage) {
    parent::__construct($configuration, $plugin_id, $plugin_definition);
    $this->storage = $storage;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition,
      $container->get('entity_type.manager')->getStorage('brightcove_video')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function processItem($data) {
    /** @var \Brightcove\Object\Video\Video $video */
    $video = $data['video'];

    BrightcoveVideo::createOrUpdate($video, $this->storage, $data['api_client_id']);
  }
}
