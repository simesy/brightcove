<?php

/**
 * @file
 * Contains \Drupal\brightcove\BrightcoveAPIClientListBuilder.
 */

namespace Drupal\brightcove;

use Drupal\Core\Config\Entity\ConfigEntityListBuilder;
use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityTypeInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Provides a listing of Brightcove API Client entities.
 */
class BrightcoveAPIClientListBuilder extends ConfigEntityListBuilder {

  /**
   * The default API Client.
   *
   * @var string
   */
  protected static $defaultAPIClient;

  /**
   * {@inheritdoc}
   */
  public static function createInstance(ContainerInterface $container, EntityTypeInterface $entityTypeInterface) {
    self::$defaultAPIClient = $container->get('config.factory')->get('brightcove.settings')->get('defaultAPIClient');
    return parent::createInstance($container, $entityTypeInterface);
  }

  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header['label'] = $this->t('Brightcove API Client');
    $header['id'] = $this->t('Machine name');
    $header['account_id'] = $this->t('Account ID');
    $header['client_id'] = $this->t('Client ID');
    $header['default_player'] = $this->t('Default player');
    $header['secret_key'] = $this->t('Secret key');
    $header['client_status'] = $this->t('Status');
    return $header + parent::buildHeader();
  }

  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    /** @var \Drupal\brightcove\Entity\BrightcoveAPIClient $entity */
    // Authorize client to get client status.
    $entity->authorizeClient();

    $row['label'] = $entity->label();
    $row['id'] = $entity->id();
    if (self::$defaultAPIClient == $entity->id()) {
      $row['id'] .= $this->t(' (default)');
    }
    $row['account_id'] = $entity->getAccountID();
    $row['client_id'] = $entity->getClientID();
    $row['default_player'] = $entity->getDefaultPlayer();
    $row['secret_key'] = $entity->getSecretKey();
    $row['client_status'] = $entity->getClientStatus() ? $this->t('OK') : $this->t('Error');
    return $row + parent::buildRow($entity);
  }

}
