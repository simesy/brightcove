<?php

/**
 * @file
 * Contains \Drupal\brightcove\BrightcoveVideoListBuilder.
 */

namespace Drupal\brightcove;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityListBuilder;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Entity\EntityTypeInterface;
use Drupal\Core\Routing\LinkGeneratorTrait;
use Drupal\Core\Session\AccountProxy;
use Drupal\Core\Url;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Defines a class to build a listing of Brightcove Videos.
 *
 * @ingroup brightcove
 */
class BrightcoveVideoListBuilder extends EntityListBuilder {
  use LinkGeneratorTrait;

  /**
   * Account proxy.
   *
   * @var \Drupal\Core\Session\AccountProxy
   */
  protected $accountProxy;

  /**
   * Constructor.
   *
   * @param \Drupal\Core\Entity\EntityTypeInterface $entity_type
   * @param \Drupal\Core\Entity\EntityStorageInterface $storage
   * @param \Drupal\Core\Session\AccountProxy $account_proxy
   */
  public function __construct(EntityTypeInterface $entity_type, EntityStorageInterface $storage, AccountProxy $account_proxy) {
    parent::__construct($entity_type, $storage);
    $this->accountProxy = $account_proxy;
  }

  /**
   * {@inheritdoc}
   */
  public static function createInstance(ContainerInterface $container, EntityTypeInterface $entity_type) {
    return new static(
      $entity_type,
      $container->get('entity_type.manager')->getStorage($entity_type->id()),
      $container->get('current_user')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    // Assemble header columns.
    $header = [
      'id' => $this->t('Brightcove Video ID'),
      'name' => $this->t('Name'),
    ];

    // Add operations header column only if the user has access.
    if ($this->accountProxy->hasPermission('edit brightcove videos') || $this->accountProxy->hasPermission('delete brightcove videos')) {
      $header += parent::buildHeader();
    }

    return $header;
  }

  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    /* @var $entity \Drupal\brightcove\Entity\BrightcoveVideo */
    if (($entity->isPublished() && $this->accountProxy->hasPermission('view published brightcove videos')) || (!$entity->isPublished() && $this->accountProxy->hasPermission('view unpublished brightcove videos'))) {
      $name = $this->l(
        $entity->label(),
        new Url(
          'entity.brightcove_video.canonical', array(
            'brightcove_video' => $entity->id(),
          )
        )
      );
    }
    else {
      $name = $entity->label();
    }

    // Assemble row.
    $row = [
      'id' => $entity->id(),
      'name' => $name,
    ];

    // Add operations column only if the user has access.
    if ($this->accountProxy->hasPermission('edit brightcove videos') || $this->accountProxy->hasPermission('delete brightcove videos')) {
      $row += parent::buildRow($entity);
    }

    return $row;
  }
}
