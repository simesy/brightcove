<?php
/**
 * @file
 * Contains \Drupal\brightcove\Entity\BrightcoveCMSEntityInterface.
 */

namespace Drupal\brightcove;

use Drupal\Core\Entity\ContentEntityInterface;
use Drupal\Core\Entity\EntityChangedInterface;
use Drupal\user\EntityOwnerInterface;

interface BrightcoveCMSEntityInterface extends ContentEntityInterface, EntityChangedInterface, EntityOwnerInterface {
  /**
   * Gets the Brightcove Playlist name.
   *
   * @return string
   *   Name of the Brightcove Playlist.
   */
  public function getName();

  /**
   * Sets the Brightcove Playlist name.
   *
   * @param string $name
   *   The Brightcove Playlist name.
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   */
  public function setName($name);

  /**
   * Returns the Brightcove Client API target ID.
   *
   * return int
   *   Target ID of the Brightcove Client API.
   */
  public function getAPIClient();

  /**
   * Sets the Brightcove Client API target ID.
   *
   * @param int $api_client
   *   Target ID of the Brightcove Client API.
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   */
  public function setAPIClient($api_client);

  /**
   * Returns the reference ID of the video.
   *
   * @return string
   *   Reference ID.
   */
  public function getReferenceID();

  /**
   * Sets the video's reference ID.
   *
   * @param string $reference_id
   *   The reference ID of the video.
   *
   * @return \Drupal\brightcove\BrightcoveVideoInterface
   *   The called Brightcove Video.
   */
  public function setReferenceID($reference_id);

  /**
   * Returns the description.
   *
   * @return string
   *   The description of the playlist.
   */
  public function getDescription();

  /**
   * Sets the playlist's description.
   *
   * @param string $description
   *   The description of the playlist.
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   */
  public function setDescription($description);

  /**
   * Returns the Brightcove Video published status indicator.
   *
   * Unpublished Brightcove Video are only visible to restricted users.
   *
   * @return bool
   *   TRUE if the Brightcove Video is published.
   */
  public function isPublished();

  /**
   * Sets the published status of a Brightcove Video.
   *
   * @param bool $published
   *   TRUE to set this Brightcove Video to published, FALSE to set it to unpublished.
   *
   * @return \Drupal\brightcove\BrightcoveVideoInterface
   *   The called Brightcove Video.
   */
  public function setPublished($published);

  /**
   * Gets the Brightcove Video creation timestamp.
   *
   * @return int
   *   Creation timestamp of the Brightcove Video.
   */
  public function getCreatedTime();

  /**
   * Sets the Brightcove Video creation timestamp.
   *
   * @param int $timestamp
   *   The Brightcove Video creation timestamp.
   *
   * @return \Drupal\brightcove\BrightcoveVideoInterface
   *   The called Brightcove Video.
   */
  public function setCreatedTime($timestamp);
}