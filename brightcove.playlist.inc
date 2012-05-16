<?php
/**
 * @file
 * This file contains function over the brightcove playlist feature.
 */


/**
 * Renders the playlist overview page.
 *
 * @return string
 *   Rendered overivew page.
 */
function brightcove_playlist_overview_page() {
  $table = array('header' => array('Title', 'Operations'));

  $playlists = brightcove_get_playlists();
  if ($playlists) {
    foreach ($playlists as $playlist) {
      $table['rows'][] = _brightcove_playlist_overview_row($playlist);
    }
  }
  else {
    $table['rows'][] = array(
      array(
        'data' => t('No playlist found.'),
        'colspan' => count($table['header']),
      ),
    );
  }

  return theme('table', $table);
}

/**
 * Renders a table row on the Brightcove playlist overview page.
 *
 * @param stdClass $playlist
 *   Playlist object.
 *
 * @return array
 *   Table row.
 *   @see theme_table()
 */
function _brightcove_playlist_overview_row(stdClass $playlist) {
  $operations = array(
    l(t('edit'), "admin/config/media/brightcove/playlist/{$playlist->id}/edit"),
    l(t('delete'), "admin/config/media/brightcove/playlist/{$playlist->id}/delete"),
  );

  return array(
    $playlist->name,
    implode(', ', $operations),
  );
}

/**
 * Get the playlists from Brightcove.
 *
 * This function is a cached wrapper around BCMAPI::find('allplaylists').
 *
 * @param bool $cache_reset
 *   (optional) True to reset the cache.
 *
 * @return bool|array
 *   Array of playlists or FALSE if none found.
 *   @see http://support.brightcove.com/en/docs/media-api-objects-reference#Playlist
 */
function brightcove_get_playlists($cache_reset = FALSE) {

  $cache = cache_get('bc:playlist:list', 'cache');
  if (!$cache_reset && $cache && !empty($cache->data)) {
    return $cache->data;
  }

  $playlists = FALSE;
  $bc = brightcove_initialize();
  try {
    $playlists = $bc ? $bc->find('allplaylists') : NULL;
  }
  catch (Exception $e) {
    watchdog('brightcove', 'Loading brightcove playlists failed.', array(), WATCHDOG_ERROR);
  }

  if ($playlists) {
    cache_set('bc:playlist:list', $playlists, 'cache', REQUEST_TIME + 300);
  }

  return $playlists;
}

/**
 *
 *
 * @param $playlist
 * @return array|mixed|null|string
 */
function brightcove_playlist_edit_page($playlist) {
  if (!$playlist) {
    return t('Playlist not found');
  }

  return drupal_get_form('brightcove_playlist_edit_form');
}

/**
 *
 *
 * @param $form
 * @param $form_state
 * @param null $playlist
 * @return array
 */
function brightcove_playlist_edit_form($form, &$form_state, $playlist = NULL) {
  $form = array();

  $form['referenceId'] = array(
    '#type' => 'hidden',
    '#value' => isset($form_state['values']['referenceId']) ? $form_state['values']['referenceId'] : '',
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
    '#default_value' => isset($form_state['values']['name']) ? $form_state['values']['name'] : '',
  );

  $form['shortDescription'] = array(
    '#type' => 'textarea',
    '#title' => 'Short description',
    '#default_value' => isset($form_state['values']['shortDescription']) ? $form_state['values']['shortDescription'] : '',
  );

  $form['videos'] = array(
    '#type' => 'textfield',
    '#title' => 'Videos',
    '#default_value' => isset($form_state['values']['name']) ? $form_state['values']['name'] : '',
    '#autocomplete_path' => 'brightcove/autocomplete/videos',
  );

  $form['playlistType'] = array(
    '#type' => 'select',
    '#title' => t('Playlist ordering'),
    '#options' => array(
      'OLDEST_TO_NEWEST' => t('Oldest to newest (by activation date)'),
      'NEWEST_TO_OLDEST' => t('Newest to oldest (by activation date)'),
      'START_DATE_OLDEST_TO_NEWEST' => t('Oldest to newest'),
      'START_DATE_NEWEST_TO_OLDEST' => t('Newest to Oldest'),
      'ALPHABETICAL' => t('Alphabetical (by video name)'),
      'PLAYS_TOTAL' => t('Total plays'),
      'PLAYS_TRAILING_WEEK' => t('Plays trailing week'),
    ),
    '#default_value' => isset($form_state['values']['playlistType']),
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Add'));

  return $form;
}

/**
 *
 *
 * @param $form
 * @param $form_state
 */
function brightcove_playlist_edit_form_submit($form, &$form_state) {
  $metadata = array();
  $keys_to_send = array('name', 'playlistType', 'shortDescription');

  foreach ($keys_to_send as $key) {
    $metadata[$key] = $form_state['values'][$key];
  }

  $videos = drupal_explode_tags($form_state['values']['videos']);
  foreach ($videos as $video) {
    preg_match('/[0-9]+/', $video, $video_id);
    $metadata['videoIds'][] = end($video_id);
  }
  brightcove_add_playlist($metadata);
}

/**
 *
 *
 * @param $metadata
 * @return bool|string
 */
function brightcove_add_playlist($metadata) {
  $bc = brightcove_initialize();

  try {
    $playlist = $bc->createPlaylist('video', $metadata);
  }
  catch(Exception $error) {
    drupal_set_message(t('Playlist upload to Brightcove failed. Error: @error', array('@error' => $error)), 'error');
    return FALSE;
  }

  // Invalidate playlist cache.
  cache_set('bc:playlist:list', FALSE);

  return $playlist;
}

/**
 *
 *
 * @param $form
 * @param $form_state
 * @param $playlist
 * @return array
 */
function brightcove_playlist_delete_form($form, &$form_state, $playlist) {
  $form = array();

  if (!$playlist) {
    drupal_set_message(t('Playlist not found!', 'error'));
    return $form;
  }

  $form['playlist_id'] = array(
    '#type' => 'hidden',
    '#value' => $playlist->id,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete playlist "%name"', array('%name' => $playlist->name)),
    'admin/config/media/brightcove/playlist',
    t('This action annot be undone'),
    t('Delete'),
    t('Cancel')
  );
}


