<?php
/**
 * @file
 * Stream wrapper fro Brightcove videos.
 */

class MediaBrightcoveStreamWrapper extends MediaReadOnlyStreamWrapper {
  public function getTarget() {
    return FALSE;
  }

  public static function getMimeType($uri, $mapping = NULL) {
    return 'video/brightcove';
  }

  public function interpolateUrl() {
    if ($video_id = $this->getVideoID()) {
      // @todo find out how to detect this url in a nice way, it might break any time
      return url('http://c.brightcove.com/services/viewer/federated_f9', array(
          'query' => array(
            'playerID' => variable_get('brightcove_player'),
            'playerKey' => variable_get('brightcove_player_key'),
            'isVid' => 'true',
            'isUI' => 'true',
            '@videoPlayer' => $video_id,
          ),
        ));
    }
    return NULL;
  }

  public function getVideoID() {
    if ($url = parse_url($this->uri)) {
      if ($url['scheme'] == 'brightcove' && is_numeric($url['host'])) {
        return $url['host'];
      }
    }

    return NULL;
  }

  public function setUri($uri) {
    $this->uri = $uri;
  }

  function getOriginalThumbnailPath() {
    if ($video_id = $this->getVideoID()) {
      if ($video = brightcove_video_load($video_id)) {
        return isset($video['thumbnailURL']) ? $video['thumbnailURL'] : NULL;
      }
    }

    return NULL;
  }
}
