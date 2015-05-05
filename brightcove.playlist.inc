<?php
/**
 * @file
 * This file contains function over the brightcove playlist feature.
 */

/**
 * Form function for the playlists page.
 */
function brightcove_playlist_overview_form($form, $form_state) {
  module_load_include('inc', 'brightcove', 'brightcove.client');
  $form['client_id'] = brightcove_client_select_element();
  $wrapper_id = 'bc-playlists';

  switch ($form['client_id']['#type']) {
    case 'select':
      $form['client_id']['#ajax'] = [
        'wrapper' => $wrapper_id,
        'callback' => 'brightcove_playlist_tables_ajax_callback',
      ];
      $client_id = isset($form_state['values']['client_id']) ? $form_state['values']['client_id'] : NULL;
      break;

    case 'value':
      $client_id = $form['client_id']['#value'];
      break;

    default :
      return $form;
  }
  $form['playlists'] = [
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
  ];
  if (!empty($client_id) && ($client_id != BRIGHTCOVE_CLIENT_ID_NONE)) {
    $client = brightcove_client_load($client_id);
    $form['playlists']['#markup'] = brightcove_playlist_overview_table($client);
  }
  return $form;
}

/**
 * AJAX callback to return the playlists table.
 */
function brightcove_playlist_tables_ajax_callback($form, $form_state) {
  return $form['playlists'];
}

/**
 * Renders the playlist overview page.
 *
 * @return string
 *   Rendered overivew page.
 */
function brightcove_playlist_overview_table($client) {
  $table = ['header' => [t('ID'), t('Title'), ['colspan' => 2, 'data' => t('Operations')]]];

  $playlists = brightcove_get_playlists($client);
  if ($playlists) {
    foreach ($playlists as $playlist) {
      $table['rows'][] = _brightcove_playlist_overview_row($playlist);
    }
  }
  else {
    $table['rows'][] = [
      [
        'data' => t('No playlist found.'),
        'colspan' => count($table['header']),
      ],
    ];
  }

  return theme('table', $table);
}

/**
 * Renders a table row on the Brightcove playlist overview page.
 *
 * @param BrightcovePlaylist $playlist
 *   Playlist object.
 *
 * @return array
 *   Table row.
 *   @see theme_table()
 */
function _brightcove_playlist_overview_row(BrightcovePlaylist $playlist) {
  return [
    $playlist->getId(),
    $playlist->getName(),
    l(t('edit'), "admin/config/media/brightcove/playlist/{$playlist->getId()}/edit"),
    l(t('delete'), "admin/config/media/brightcove/playlist/{$playlist->getId()}/delete"),
  ];
}

/**
 * Get the playlists from Brightcove.
 *
 * This function is a cached wrapper around BCMAPI::find('allplaylists').
 *
 * @param Entity $client
 *   The brightcove_client entity object.
 * @param bool $cache_reset
 *   (optional) True to reset the cache.
 *
 * @return bool|array
 *   Array of playlists or FALSE if none found.
 *   @see http://support.brightcove.com/en/docs/media-api-objects-reference#Playlist
 */
function brightcove_get_playlists(Entity $client, $cache_reset = FALSE) {
  brightcove_load_lib();
  $cid = 'brightcove:playlist:list:' . $client->bcid;
  $cache = brightcove_cache_get($cid);
  if (!$cache_reset && $cache && !empty($cache->data)) {
    return $cache->data;
  }

  $playlists = FALSE;
  list($cms, ) = _brightcove_create_classes($client);
  try {
    $playlists = $cms->listPlaylists();
  }
  catch (Exception $error) {
    watchdog('brightcove', 'Fetching playlists failed.', [], WATCHDOG_ERROR);
  }

  if ($playlists) {
    brightcove_cache_set($cid, $playlists);
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
  $form['referenceId'] = [
    '#type' => 'hidden',
    '#value' => isset($form_state['values']['referenceId']) ? $form_state['values']['referenceId'] : '',
  ];

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

  $form['type'] = [
    '#title' => t('Playlist type'),
    '#type' => 'select',
    '#default_value' => $type_default,
    '#ajax' => [
      'callback' => 'brightcove_playlist_edit_form_ajax_callback',
      'wrapper' => 'brightcove-playlist-settings',
    ],
    '#options' => [
      BRIGHTCOVE_PLAYLIST_TYPE_MANUAL => t('Manual'),
      BRIGHTCOVE_PLAYLIST_TYPE_SMART => t('Smart'),
    ],
  ];

  $form['name'] = [
    '#type' => 'textfield',
    '#title' => 'Name',
    '#default_value' => isset($playlist->name) ? $playlist->name : '',
  ];

  $form['shortDescription'] = [
    '#type' => 'textarea',
    '#title' => 'Short description',
    '#default_value' => isset($playlist->shortDescription) ? $playlist->shortDescription : '',
  ];

  $form['settings'] = [
    '#type' => 'container',
    '#id' => 'brightcove-playlist-settings',
  ];

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
    $form['settings']['videos'] = [
      '#type' => 'textfield',
      '#title' => 'Videos',
      '#default_value' => !is_null($videos) ? $videos : '',
      '#autocomplete_path' => 'brightcove/autocomplete/videos',
    ];
  }
  else if ($active_type == BRIGHTCOVE_PLAYLIST_TYPE_SMART) {
    $form['settings']['tagInclusionRule'] = [
      '#type' => 'select',
      '#title' => t('Smart playlist settings'),
      '#field_suffix' => t('of the following'),
      '#default_value' => 'OR',
      '#options' => [
        'OR' => t('Contain one or more'),
        'AND' => t('Contain all'),
      ],
    ];

    $form['settings']['filterTags'] = [
      '#type' => 'textfield',
      '#title' => t('Tags'),
      '#default_value' => (isset($playlist->filterTags)) ? drupal_implode_tags($playlist->filterTags) : '',
      '#description' => t('Videos will be automatically added to this Playlist
                           based on the following settings.'),
    ];

    $form['settings']['playlistType'] = [
      '#type' => 'select',
      '#title' => t('Playlist ordering'),
      '#options' => [
        'OLDEST_TO_NEWEST' => t('Oldest to newest (by activation date)'),
        'NEWEST_TO_OLDEST' => t('Newest to oldest (by activation date)'),
        'START_DATE_OLDEST_TO_NEWEST' => t('Oldest to newest'),
        'START_DATE_NEWEST_TO_OLDEST' => t('Newest to Oldest'),
        'ALPHABETICAL' => t('Alphabetical (by video name)'),
        'PLAYS_TOTAL' => t('Total plays'),
        'PLAYS_TRAILING_WEEK' => t('Plays trailing week'),
      ],
      '#default_value' => isset($playlist->playlistType) ? $playlist->playlistType : '',
    ];
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

  $form['actions'] = ['#type' => 'actions'];

  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => ['brightcove_playlist_edit_form_submit'],
  ];

  if (!is_null($playlist)) {
    $form['actions']['delete'] = [
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => ['brightcove_playlist_edit_form_delete'],
    ];
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
  $metadata = [];
  $keys_to_send = ['name', 'shortDescription'];
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
    $metadata['id'] = $form_state['playlist']->id;
    brightcove_update_playlist($metadata);
  }
  else {
    brightcove_add_playlist($metadata);
  }
  // Redirect the user to the playlist listing page.
  $form_state['redirect'] = ['admin/config/media/brightcove/playlist'];
}

/**
 * brightcove_playlist_edit_form delete button callback.
 *
 * @param $form
 * @param $form_state
 */
function brightcove_playlist_edit_form_delete($form, &$form_state) {
  $form_state['redirect'] = ["admin/config/media/brightcove/playlist/" . $form_state['playlist']->id . "/delete"];
}

/**
 * Delete playlist confirm form.
 *
 * @param $form
 * @param $form_state
 * @param $playlist
 * @return array
 */
function brightcove_playlist_delete_form($form, &$form_state, $client, $playlist) {
  $form = [];

  if (!$playlist) {
    drupal_set_message(t('Playlist not found!', 'error'));
    return $form;
  }

  $form['client_id'] = [
    '#type' => 'hidden',
    '#value' => $client->bcid,
  ];

  $form['playlist_id'] = [
    '#type' => 'hidden',
    '#value' => $playlist->getId(),
  ];

  return confirm_form(
    $form,
    t('Are you sure you want to delete playlist "%name"', ['%name' => $playlist->getName()]),
    'admin/config/media/brightcove/playlist',
    t('This action annot be undone'),
    t('Delete'),
    t('Cancel')
  );
}


