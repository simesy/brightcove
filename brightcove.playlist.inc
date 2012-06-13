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
  $table = array('header' => array(t('Title'), array('colspan' => 2, 'data' => t('Operations'))));

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
  return array(
    $playlist->name,
    l(t('edit'), "admin/config/media/brightcove/playlist/{$playlist->id}/edit"),
    l(t('delete'), "admin/config/media/brightcove/playlist/{$playlist->id}/delete"),
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

  return drupal_get_form('brightcove_playlist_edit_form', $playlist);
}

function _brightcove_playlist_form(&$form, $form_state, $playlist = NULL) {
  $form['referenceId'] = array(
    '#type' => 'hidden',
    '#value' => isset($form_state['values']['referenceId']) ? $form_state['values']['referenceId'] : '',
  );

  $type_default = BRIGHTCOVE_PLAYLIST_TYPE_MANUAL;
  if (!is_null($playlist) && $playlist->playlistType != 'EXPLICIT') {
    $type_default = BRIGHTCOVE_PLAYLIST_TYPE_SMART;
  }

  $active_type = $type_default;
  if (!empty($form_state['values']['type'])) {
    if ($form_state['values']['type'] == BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
      $active_type = BRIGHTCOVE_PLAYLIST_TYPE_MANUAL;
    }
    else if ($form_state['values']['type'] == BRIGHTCOVE_PLAYLIST_TYPE_SMART) {
      $active_type = BRIGHTCOVE_PLAYLIST_TYPE_SMART;
    }
  }

  $form['type'] = array(
    '#title' => t('Playlist type'),
    '#type' => 'select',
    '#default_value' => $type_default,
    '#ajax' => array(
      'callback' => 'brightcove_playlist_edit_form_ajax_callback',
      'wrapper' => 'brightcove-playlist-settings',
    ),
    '#options' => array(
      BRIGHTCOVE_PLAYLIST_TYPE_MANUAL => t('Manual'),
      BRIGHTCOVE_PLAYLIST_TYPE_SMART => t('Smart'),
    ),
  );

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Name',
    '#default_value' => isset($playlist->name) ? $playlist->name : '',
  );

  $form['shortDescription'] = array(
    '#type' => 'textarea',
    '#title' => 'Short description',
    '#default_value' => isset($playlist->shortDescription) ? $playlist->shortDescription : '',
  );

  $form['settings'] = array(
    '#type' => 'container',
    '#id' => 'brightcove-playlist-settings',
  );

  if ($active_type == BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
    $videos = NULL;
    if (!empty ($playlist)) {
      foreach ($playlist->videos as $video) {
        $videos .= check_plain($video->name) . ' [id:' . $video->id . ']';
        if (end($playlist->videos)->id != $video->id) {
          $videos .= ', ';
        }
      }
    }
    $form['settings']['videos'] = array(
      '#type' => 'textfield',
      '#title' => 'Videos',
      '#default_value' => !is_null($videos) ? $videos : '',
      '#autocomplete_path' => 'brightcove/autocomplete/videos',
    );
  }
  else if ($active_type == BRIGHTCOVE_PLAYLIST_TYPE_SMART) {
    $form['settings']['tagInclusionRule'] = array(
      '#type' => 'select',
      '#title' => t('Smart playlist settings'),
      '#field_suffix' => t('of the following'),
      '#default_value' => 'OR',
      '#options' => array(
        'OR' => t('Contain one or more'),
        'AND' => t('Contain all'),
      ),
    );

    $form['settings']['filterTags'] = array(
      '#type' => 'textfield',
      '#title' => t('Tags'),
      '#default_value' => (isset($playlist->filterTags)) ? drupal_implode_tags($playlist->filterTags) : '',
      '#description' => t('Videos will be automatically added to this Playlist
                           based on the following settings.'),
    );

    $form['settings']['playlistType'] = array(
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
      '#default_value' => isset($playlist->playlistType) ? $playlist->playlistType : '',
    );
  }
}

/**
 * Playlist edit form.
 *
 * @param $form
 * @param $form_state
 * @param null $playlist
 * @return array
 */
function brightcove_playlist_edit_form($form, &$form_state, $playlist = NULL) {
  if (!is_null($playlist) && !isset($form_state['playlist'])) {
    $form_state['playlist'] = $playlist;
  }
  _brightcove_playlist_form($form, $form_state, $playlist);

  $form['actions'] = array('#type' => 'actions');

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => array('brightcove_playlist_edit_form_submit'),
  );

  if (!is_null($playlist)) {
    $form['actions']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => array('brightcove_playlist_edit_form_delete'),
    );
  }

  return $form;
}

/**
 * Ajax callback for playlist edit form.
 *
 * @param $form
 * @param $form_state
 * @return mixed
 */
function brightcove_playlist_edit_form_ajax_callback($form, &$form_state) {
  return $form['settings'];
}

/**
 * brightcove_playlist_edit_form submit handler.
 *
 * @param $form
 * @param $form_state
 */
function brightcove_playlist_edit_form_submit($form, &$form_state) {
  $metadata = array();
  $keys_to_send = array('name', 'shortDescription');
  foreach ($keys_to_send as $key) {
    $metadata[$key] = $form_state['values'][$key];
  }

  if ($form_state['values']['type'] == BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
    $metadata['playlistType'] = 'explicit';
    $videos = drupal_explode_tags($form_state['values']['videos']);
    foreach ($videos as $video) {
      // Parse the video id.
      preg_match('/\[id:(?P<videoid>\d+)\]/', $video, $matches);
      $metadata['videoIds'][] = $matches['videoid'];
    }
  }
  else {
    $metadata['tagInclusionRule'] = $form_state['values']['tagInclusionRule'];
    $metadata['playlistType'] = $form_state['values']['playlistType'];
    $metadata['filterTags'] = drupal_explode_tags($form_state['values']['filterTags']);
  }

  if (isset($form_state['playlist'])) {
    brightcove_update_playlist($metadata);
  }
  else {
    brightcove_add_playlist($metadata);
  }
  // Invalidate playlist cache.
  cache_set('bc:playlist:list', FALSE);
  // Redirect the user to the playlist listing page.
  $form_state['redirect'] = array('admin/config/media/brightcove/playlist');
}

/**
 * brightcove_playlist_edit_form delete button callback.
 *
 * @param $form
 * @param $form_state
 */
function brightcove_playlist_edit_form_delete($form, &$form_state) {
  $form_state['redirect'] = array("admin/config/media/brightcove/playlist/" . $form_state['playlist']->id . "/delete");
}

/**
 * Delete playlist confirm form.
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


