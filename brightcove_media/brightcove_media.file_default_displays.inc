<?php

/**
 * Implements hook_file_default_displays().
 */
function brightcove_media_file_default_displays() {
  return [
    'video__default__brightcove_media_video' => (object) [
      'api_version' => 1,
      'name' => 'video__default__brightcove_media_video',
      'weight' => 0,
      'status' => TRUE,
      'settings' => [],
    ],
  ];
}
