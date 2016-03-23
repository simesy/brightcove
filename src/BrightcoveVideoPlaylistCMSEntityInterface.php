<?php
/**
 * @file
 * Contains \Drupal\brightcove\Entity\BrightcoveCMSEntityInterface.
 */

namespace Drupal\brightcove;

use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityChangedInterface;
use Drupal\user\EntityOwnerInterface;

interface BrightcoveVideoPlaylistCMSEntityInterface extends BrightcoveCMSEntityInterface {
  /**
   * Returns the player.
   *
   * @return int
   *   Target ID of the Brightcove Player.
   */
  public function getPlayer();

  /**
   * Sets the player.
   *
   * @param string $player
   *   The player's Brightcove ID.
   *
   * @return \Drupal\brightcove\BrightcoveVideoPlaylistCMSEntityInterface
   *   The called Brightcove Video or Playlist.
   */
  public function setPlayer($player);

  /**
   * Returns the reference ID.
   *
   * @return string
   *   Reference ID.
   */
  public function getReferenceID();

  /**
   * Sets the reference ID.
   *
   * @param string $reference_id
   *   The reference ID.
   *
   * @return \Drupal\brightcove\BrightcoveVideoPlaylistCMSEntityInterface
   *   The called Brightcove Video or Playlist.
   */
  public function setReferenceID($reference_id);

  /**
   * Returns the entity published status indicator.
   *
   * Unpublished entities are only visible to restricted users.
   *
   * @return bool
   *   TRUE if the entity is published.
   */
  public function isPublished();

  /**
   * Sets the published status of the entity.
   *
   * @param bool $published
   *   TRUE to set this entity to published, FALSE to set it to unpublished.
   *
   * @return \Drupal\brightcove\BrightcoveVideoPlaylistCMSEntityInterface
   *   The called Brightcove Video or Playlist.
   */
  public function setPublished($published);
}