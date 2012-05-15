<?php
/**
 * @file
 * Stream wrapper for Brightcove playlists.
 */

class MediaBrightcovePlaylistStreamWrapper extends MediaReadOnlyStreamWrapper {
  /**
   * Get MIME type.
   *
   * @static
   * @param string $uri
   * @param null $mapping
   *
   * @return string
   */
  public static function getMimeType($uri, $mapping = NULL) {
    return 'video/brightcove_playlist';
  }

}
