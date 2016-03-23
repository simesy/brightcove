<?php
/**
 * @file
 * Contains \Drupal\brightcove\Plugin\QueueWorker\BrightcoveCustomFieldDeleteQueueWorker.
 */

namespace Drupal\brightcove\Plugin\QueueWorker;

use Drupal\brightcove\Entity\BrightcoveCustomField;
use Drupal\Core\Plugin\ContainerFactoryPluginInterface;
use Drupal\Core\Queue\QueueWorkerBase;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Processes Entity Delete Tasks for Custom Fields.
 *
 * @QueueWorker(
 *   id = "brightcove_custom_field_delete_queue_worker",
 *   title = @Translation("Brightcove custom field delete queue worker."),
 *   cron = { "time" = 30 }
 * )
 */
class BrightcoveCustomFieldDeleteQueueWorker extends QueueWorkerBase implements ContainerFactoryPluginInterface {
  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container, array $configuration, $plugin_id, $plugin_definition) {
    return new static(
      $configuration,
      $plugin_id,
      $plugin_definition
    );
  }

  /**
   * {@inheritdoc}
   */
  public function processItem($data) {
    $custom_field = $data['custom_field_entity'];

    // Delete custom field.
    if ($custom_field instanceof BrightcoveCustomField) {
      $custom_field->delete();
    }
  }
}
