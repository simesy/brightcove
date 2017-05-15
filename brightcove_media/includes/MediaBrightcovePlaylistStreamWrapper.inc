<?php
/**
 * @file
 * Stream wrapper for Brightcove playlists.
 */

class MediaBrightcovePlaylistStreamWrapper extends MediaBrightcoveVideoStreamWrapper {

  protected function urlTargetType() {
    return BRIGHTCOVE_EMBED_TYPE_PLAYLIST;
  }

  protected function streamPrefix() {
    return 'brightcove-playlist';
  }

}
