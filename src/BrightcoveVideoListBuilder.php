<?php

/**
 * @file
 * Contains \Drupal\brightcove\BrightcoveVideoListBuilder.
 */

namespace Drupal\brightcove;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityListBuilder;
use Drupal\Core\Routing\LinkGeneratorTrait;
use Drupal\Core\Url;

/**
 * Defines a class to build a listing of Brightcove Videos.
 *
 * @ingroup brightcove
 */
class BrightcoveVideoListBuilder extends EntityListBuilder {
  use LinkGeneratorTrait;
  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header['id'] = $this->t('Brightcove Video ID');
    $header['name'] = $this->t('Name');
    return $header + parent::buildHeader();
  }

  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    /* @var $entity \Drupal\brightcove\Entity\BrightcoveVideo */
    $row['id'] = $entity->id();
    $row['name'] = $this->l(
      $entity->label(),
      new Url(
        'entity.brightcove_video.canonical', array(
          'brightcove_video' => $entity->id(),
        )
      )
    );
    return $row + parent::buildRow($entity);
  }

}
