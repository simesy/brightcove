<?php

/**
 * @file
 * Contains \Drupal\brightcove\BrightcovePlaylistListBuilder.
 */

namespace Drupal\brightcove;

use Drupal\Core\Entity\EntityInterface;
use Drupal\Core\Entity\EntityListBuilder;
use Drupal\Core\Routing\LinkGeneratorTrait;
use Drupal\Core\Url;

/**
 * Defines a class to build a listing of Brightcove Playlists.
 *
 * @ingroup brightcove
 */
class BrightcovePlaylistListBuilder extends EntityListBuilder {
  use LinkGeneratorTrait;
  /**
   * {@inheritdoc}
   */
  public function buildHeader() {
    $header['id'] = $this->t('Brightcove Playlist ID');
    $header['name'] = $this->t('Name');
    return $header + parent::buildHeader();
  }

  /**
   * {@inheritdoc}
   */
  public function buildRow(EntityInterface $entity) {
    /* @var $entity \Drupal\brightcove\Entity\BrightcovePlaylist */
    $row['id'] = $entity->id();
    $row['name'] = $this->l(
      $entity->label(),
      new Url(
        'entity.brightcove_playlist.canonical', array(
          'brightcove_playlist' => $entity->id(),
        )
      )
    );
    return $row + parent::buildRow($entity);
  }

}
