<?php
/**
 * @file
 * Stream wrapper for Brightcove videos.
 */


class MediaBrightcoveVideoStreamWrapper extends MediaReadOnlyStreamWrapper {

  /**
   * Get target.
   *
   * @return bool
   */
  public function getTarget() {
    return FALSE;
  }

  /**
   * Get MIME type.
   *
   * @static
   * @param $uri
   * @param null $mapping
   * @return string
   */
  public static function getMimeType($uri, $mapping = NULL) {
    return 'video/brightcove';
  }

  protected function urlTargetType() {
    return BRIGHTCOVE_EMBED_TYPE_VIDEO;
  }

  protected function streamPrefix() {
    return 'brightcove';
  }

  public function interpolateUrl() {
    $values = $this->brightcoveValues(TRUE);

    return ($values['id'] && $values['player']) ?
      brightcove_embed_iframe_url(
        $values['account'],
        $values['player'],
        $values['embed'],
        $values['id'],
        $this->urlTargetType()
      ) :
      NULL;
  }

  public function getVideoID() {
    if (($url = parse_url($this->uri))) {
      if ($url['scheme'] === $this->streamPrefix() && is_numeric($url['host'])) {
        return $url['host'];
      }
    }

    return NULL;
  }

  /**
   * Extracts values from a brightcove:// or a brightcove-playlist:// uri.
   *
   * @param bool $ensure
   *   If set, the values will be filled up with defaults instead of NULLs, when
   *   a value is missing from the URL, or the simplified URL is used.
   *
   * @return array
   *   An associative array with keys 'id', 'account' and 'player'.
   */
  public function brightcoveValues($ensure = FALSE) {
    $parameters = (array) $this->get_parameters();

    $values = [
      'id' => isset($parameters['v']) ? $parameters['v'] : $this->getVideoID(),
      'account' => isset($parameters['a']) ? $parameters['a'] : NULL,
      'player' => isset($parameters['p']) ? $parameters['p'] : NULL,
      'embed' => isset($parameters['e']) ? $parameters['e'] : NULL,
    ];

    if ($ensure) {
      if (!$values['account']) {
        $client_entity = brightcove_client_load_or_default();
        $values['account'] = $client_entity->account_id;
      }

      if (!$values['embed']) {
        $values['embed'] = 'default';
      }

      if (!$values['player']) {
        $client_entities = brightcove_get_clients_by_account_id($values['account']);
        if ($client_entities) {
          $client_entity = reset($client_entities);
          $values['player'] = brightcove_get_default_player($client_entity->bcid);
        }
        else {
          $values['player'] = 'default';
        }
      }
    }

    return $values;
  }

}
