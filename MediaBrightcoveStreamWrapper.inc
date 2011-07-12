<?php

class MediaBrightcoveStreamWrapper extends MediaReadOnlyStreamWrapper {
  public function getTarget() {
    return FALSE;
  }

  public static function getMimeType() {
    return 'video/brightcove';
  }

  public function interpolateUrl() {
    if ($url = parse_url($this->uri)) {
      if ($url['scheme'] == 'brightcove' && is_numeric($url['host'])) {
        $bc = brightcove_video_load($url['host']);
      }
    }
    return NULL;
  }

  public function setUri($uri) {
    $this->uri = $uri;
  }
}
