<?php

/**
 * @file
 * Contains \Drupal\brightcove\BrightcovePlaylistInterface.
 */

namespace Drupal\brightcove;

/**
 * Provides an interface for defining Brightcove Playlists.
 *
 * @ingroup brightcove
 */
interface BrightcovePlaylistInterface {

  /**
   * Returns the playlist type.
   *
   * @return string
   *   The type of the playlist.
   */
  public function getType();

  /**
   * Sets the playlist's type.
   *
   * @param string $type
   *   The type of the playlist, either BrightcovePlaylist::TYPE_MANUAL or
   *   BrightcovePlaylist::SMART.
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   *
   * @throws \InvalidArgumentException
   *   If the value input is inappropriate.
   */
  public function setType($type);

  /**
   * Returns the Brightcove Playlist favorite indicator.
   *
   * Favorite Brightcove Playlists are displayed in the sidebar.
   *
   * @return bool
   *   TRUE if the Brightcove Playlist is favorite.
   */
  public function isFavorite();

  /**
   * Returns the Brightcove Playlist ID.
   *
   * @return int
   *   The Brightcove Playlist ID (not the entity's).
   */
  public function getPlaylistId();

  /**
   * Sets The Brightcove Playlist ID.
   *
   * @param int $playlist_id
   *   The Brightcove Playlist ID (not the entity's).
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   */
  public function setPlaylistId($playlist_id);

  /**
   * Returns the search string.
   *
   * @return string
   *   The search string of the playlist.
   */
  public function getSearch();

  /**
   * Sets the playlist's search string.
   *
   * @param string $search
   *   The search string of the playlist.
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   */
  public function setSearch($search);

  /**
   * Returns the list of videos on the playlist.
   *
   * @return int[]
   *   The videos on the playlist.
   */
  public function getVideos();

  /**
   * Sets the playlist's videos.
   *
   * @param ['target_id' => int][] $videos
   *   The videos on the playlist.
   *
   * @return \Drupal\brightcove\BrightcovePlaylistInterface
   *   The called Brightcove Playlist.
   */
  public function setVideos($videos);
}
