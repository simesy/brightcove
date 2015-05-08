<?php
/**
 * @file
 * This file contains function over the brightcove playlist feature.
 */

/**
 * Entity controller class for Brightcove client.
 */
class BrightcovePlaylistEntityController extends EntityAPIController {

  /**
   * Overwrites EntityAPIController::save().
   */
  public function save($playlist, DatabaseTransaction $transaction = NULL) {
    if (!isset($playlist->client)) {
      watchdog('brightcove', 'Saving playlist failed due to missing client property.', [], WATCHDOG_ERROR);
      return;
    }
    try {
      list($cms, ) = _brightcove_create_classes($playlist->client);
      $cms->updatePlaylist($playlist->playlist);
      $name = $playlist->getName();
      watchdog('brightcove', 'Saving brightcove playlist @name was successful.', ['@name' => $name], WATCHDOG_INFO);
    }
    catch (BrightcoveAPIException $e) {
      watchdog('brightcove', 'Saving brightcove playlist failed.', [], WATCHDOG_ERROR);
    }
  }

  /**
   * Overwrites DrupalDefaultEntityController::load().
   *
   * @param array ids
   *  Array of ids. The format for an id is "1234-5" where 1234 is
   *  a BrightcovePlaylist->getId() and 5 is a BrightcoveClient->bcid.
   * @param $conditions
   *
   * @return An array of BrightcovePlaylist objects, keyed by the playlist id.
   */
  public function load($ids = array(), $conditions = array()) {
    $playlists = [];
    $entity_info = entity_get_info('brightcove_playlist');
    foreach ($ids as $id) {
      $playlist_ids = explode('-', $id);
      if (count($playlist_ids) != 2) {
        continue;
      }
      $playlist_id = $playlist_ids[0];
      $client_id = $playlist_ids[1];
      $playlist = brightcove_load_playlist($playlist_id, $client_id);

      if ($playlist) {
        $values = [
          $entity_info['entity keys']['id'] => $id,
          'playlist_id' => $playlist_id,
          $entity_info['entity keys']['label'] => $playlist->getName(),
          $entity_info['entity keys']['bundle'] => ($playlist->getType() == 'EXPLICIT') ? BRIGHTCOVE_PLAYLIST_TYPE_MANUAL : BRIGHTCOVE_PLAYLIST_TYPE_SMART,
          'playlist' => $playlist,
          'client' => brightcove_client_load($client_id),
        ];
        $playlist_entity = entity_create('brightcove_playlist', $values);
        $playlists[$id] = $playlist_entity;
      }

    }
    return $playlists;
  }
}

/**
 * Entity UI Controller class for brightcove playlists.
 */
class BrightcovePlaylistEntityUIController extends EntityBundleableUIController {

  public function hook_menu() {
    $items = parent::hook_menu();
    $items['admin/config/media/brightcove/playlist'] = array(
      'type' => MENU_LOCAL_TASK,
      'title' => 'Playlists',
      'page callback' => 'drupal_get_form',
      'page arguments' => array($this->entityType . '_overview_form', $this->entityType),
      'description' => 'Manage ' . $plural_label . '.',
      'access callback' => 'entity_access',
      'access arguments' => array('view', $this->entityType),
      'file' => 'includes/entity.ui.inc',
    );
    return $items;
  }

  /**
   * Overrides EntityDefaultUIController::overviewForm()
   */
  public function overviewForm($form, &$form_state) {
    module_load_include('inc', 'brightcove', 'brightcove.client');
    $form['client_id'] = brightcove_client_select_element();
    $wrapper_id = 'bc-playlists';

    switch ($form['client_id']['#type']) {
      case 'select' :
        $form['client_id']['#ajax'] = [
          'wrapper' => $wrapper_id,
          'callback' => 'brightcove_playlist_tables_ajax_callback'
        ];
        $client_id = isset($form_state['values']['client_id']) ? $form_state['values']['client_id'] : NULL;
        break;

      case 'value' :
        $client_id = $form['client_id']['#value'];
        break;

      default :
        return $form;
    }
    $form['playlists'] = [
      '#prefix' => '<div id="' . $wrapper_id . '">',
      '#suffix' => '</div>'
    ];
    if (!empty($client_id) && ($client_id != BRIGHTCOVE_CLIENT_ID_NONE)) {
      $client = brightcove_client_load($client_id);
      $form['playlists']['#markup'] = $this->overviewTable(['client' => $client]);
    }
    return $form;
  }

  /**
   * Renders the playlist overview table.
   *
   * @return string
   *   Rendered overivew page.
   */
  public function overviewTable($conditions = array()) {
    $client = $conditions['client'];
    $table = [
      'header' => [
        t('ID'),
        t('Title'),
        [
          'colspan' => 2,
          'data' => t('Operations')
        ]
      ]
    ];
    module_load_include('inc', 'brightcove', 'brightcove.playlist');

    $playlists = brightcove_get_playlists($client);
    if ($playlists) {
      foreach ( $playlists as $playlist ) {
        $playlist_entity_id = $playlist->getId() . '-' . $client->bcid;
        $playlist_entity = brightcove_playlist_load($playlist_entity_id);
        $table['rows'][] = $this->overviewRow($playlist_entity);
      }
    }
    else {
      $table['rows'][] = [
        [
          'data' => t('No playlist found.'),
          'colspan' => count($table['header'])
        ]
      ];
    }
    return theme('table', $table);
  }

  /**
   * Renders a table row on the Brightcove playlist overview page.
   *
   * @param Entity $client
   *  The client entity object.
   * @param BrightcovePlaylist $playlist
   *   Playlist object.
   *
   * @return array
   *   Table row.
   *   @see theme_table()
   */
  protected function overviewRow(Entity $playlist) {
    return [
      $playlist->playlist->getId(),
      $playlist->playlist->getName(),
      l(t('edit'), "admin/config/media/brightcove/playlist/{$playlist->bpid}/edit"),
      l(t('delete'), "admin/config/media/brightcove/playlist/{$playlist->bpid}/delete"),
    ];
  }
}

/**
 * AJAX callback to return the playlists table.
 */
function brightcove_playlist_tables_ajax_callback($form, $form_state) {
  return $form['playlists'];
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
 * @param array $form
 * @param array $form_state
 * @param BrightcovePlaylistEntity|string $playlist
 *  The BrightcovePlaylistEntity to edit or the bundle string.
 */
function _brightcove_playlist_form(&$form, $form_state, $playlist_entity) {
  if (is_object($playlist_entity)) {
    $client = $playlist_entity->client;
    $playlist = $playlist_entity->playlist;
  }

  $wrapper_id = 'bc-playlist-data-wrapper';
  if (!empty($client)) {
    $form['client_id'] = [
      '#type' => 'hidden',
      '#value' => $client->bcid,
    ];
  }
  else {
    module_load_include('inc', 'brightcove', 'brightcove.client');
    $form['client_id'] = brightcove_client_select_element();
  }
  switch ($form['client_id']['#type']) {
    case 'select':
      $form['client_id']['#ajax'] = [
        'wrapper' => $wrapper_id,
        'callback' => 'brightcove_playlist_client_ajax'
      ];
      $client_id = isset($form_state['values']['client_id']) ? $form_state['values']['client_id'] : $form['client_id']['#default_value'];
      break;

    case 'value':
    case 'hidden' :
      $client_id = $form['client_id']['#value'];
      break;
  }
  if (empty($client)) {
    $client = brightcove_client_load($client_id);
  }

  $type_default = BRIGHTCOVE_PLAYLIST_TYPE_MANUAL;
  if (!is_null($playlist) && $playlist->getType() != 'EXPLICIT') {
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

  $form['playlist_data'] = [
    '#type' => 'container',
    '#id' => $wrapper_id,
  ];
  if ($client_id != '_none') {
  $form['playlist_data']['type'] = [
    '#title' => t('Playlist type'),
    '#type' => 'select',
    '#default_value' => $type_default,
    '#ajax' => [
      'callback' => 'brightcove_playlist_form_ajax_callback',
      'wrapper' => 'brightcove-playlist-settings',
    ],
    '#options' => [
      BRIGHTCOVE_PLAYLIST_TYPE_MANUAL => t('Manual'),
      BRIGHTCOVE_PLAYLIST_TYPE_SMART => t('Smart'),
    ],
  ];

  $form['playlist_data']['name'] = [
    '#type' => 'textfield',
    '#title' => 'Name',
    '#default_value' => empty($playlist) ? '' : $playlist->getName(),
    '#required' => TRUE,
  ];

  $form['playlist_data']['description'] = [
    '#type' => 'textarea',
    '#title' => 'Short description',
    '#default_value' => empty($playlist) ? '' : $playlist->getDescription(),
  ];

  $form['playlist_data']['settings'] = [
    '#type' => 'container',
    '#id' => 'brightcove-playlist-settings',
  ];

  if ($active_type == BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
    $videos = NULL;
    if (!empty ($playlist)) {
      foreach ($playlist->getVideoIds() as $video_id) {
        $video = brightcove_video_load($video_id, $client);
        $videos .= check_plain($video->getName()) . ' [id:' . $video_id . '], ';
      }
      // Get rid of the trailing ", ".
      $videos = substr($videos, 0, -2);
    }
    $form['playlist_data']['settings']['videos'] = [
      '#type' => 'textfield',
      '#title' => 'Videos',
      '#default_value' => !is_null($videos) ? $videos : '',
      '#autocomplete_path' => 'brightcove/autocomplete/videos/' . $client->bcid,
    ];
  }
  else if ($active_type == BRIGHTCOVE_PLAYLIST_TYPE_SMART) {
    $form['playlist_data']['settings']['tagInclusionRule'] = [
      '#type' => 'select',
      '#title' => t('Smart playlist settings'),
      '#field_suffix' => t('of the following'),
      '#default_value' => 'OR',
      '#options' => [
        'OR' => t('Contain one or more'),
        'AND' => t('Contain all'),
      ],
    ];

    $form['playlist_data']['settings']['filterTags'] = [
      '#type' => 'textfield',
      '#title' => t('Tags'),
      '#default_value' => (isset($playlist->filterTags)) ? drupal_implode_tags($playlist->filterTags) : '',
      '#description' => t('Videos will be automatically added to this Playlist
                           based on the following settings.'),
    ];

    $form['playlist_data']['settings']['playlistType'] = [
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
}

/**
 * AJAX callback for playlist form.
 */
function brightcove_playlist_client_ajax($form, $form_state) {
  return $form['playlist_data'];
}
/**
 * Playlist edit form.
 *
 * @param $form
 * @param $form_state
 * @param Entity $client
 *  The client entity object to use for editing the playlist.
 * @param string $playlist_id
 *  The id of the playlist to edit.
 * @return array
 */
function brightcove_playlist_form($form, &$form_state, $playlist) {
  _brightcove_playlist_form($form, $form_state, $playlist);

  $form['actions'] = ['#type' => 'actions'];

  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save'),
    '#submit' => ['brightcove_playlist_form_submit'],
  ];

  if (!empty($playlist->bpid)) {
    $form['actions']['delete'] = [
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#submit' => ['brightcove_playlist_form_delete'],
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
function brightcove_playlist_form_ajax_callback($form, &$form_state) {
  return $form['settings'];
}

/**
 * brightcove_playlist_form submit handler.
 *
 * @param $form
 * @param $form_state
 */
function brightcove_playlist_form_submit($form, &$form_state) {
  $client = brightcove_client_load($form_state['values']['client_id']);
  list($cms,) = _brightcove_create_classes($client);

  // Manual playlists.
  if ($form_state['values']['type'] == BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
    // Editing a playlist.
    if (isset($form_state['playlist'])) {
      try {
        $playlist = $cms->getPlaylist($form_state['playlist']->getId());
      }
      catch (BrightcoveAPIException $e) {
        watchdog('brightcove', 'Getting playlist to save failed.', [], WATCHDOG_ERROR);
      }
    }
    // Adding a playlist.
    else {
      $playlist = new BrightcovePlaylist();
    }
    $videos = drupal_explode_tags($form_state['values']['videos']);
    $video_ids = [];
    foreach ($videos as $video) {
      // Parse the video id.
      preg_match('/\[id:(?P<videoid>\d+)\]/', $video, $matches);
      $video_ids[] = $matches['videoid'];
    }
    $playlist->setVideoIds($video_ids);
    $playlist->setName($form_state['values']['name']);
    $playlist->setDescription($form_state['values']['description']);
    $playlist->setType('EXPLICIT');
  }
  // Smart playlists.
  else {
    $metadata['tagInclusionRule'] = $form_state['values']['tagInclusionRule'];
    $metadata['playlistType'] = $form_state['values']['playlistType'];
    $metadata['filterTags'] = drupal_explode_tags($form_state['values']['filterTags']);
  }

  if (isset($form_state['playlist'])) {
    try {
      $cms->updatePlaylist($playlist);
    }
    catch (BrightcoveAPIException $e) {
      watchdog('brightcove', 'Updating playlist has failed with message @message.', ['@message' => $e->getMessage()], WATCHDOG_ERROR);
      drupal_set_message(t('Updating the playlist has failed. Probably this client does not have permission to edit playlists.'), 'error');
    }
    brightcove_invalidate_cache('brightcove:playlist:' . $playlist->getId());
    brightcove_invalidate_cache('brightcove:playlist:list:' . $client->bcid, TRUE);
  }
  else {
    try {
      $cms->createPlaylist($playlist);
    }
    catch ( BrightcoveAPIException $e ) {
      watchdog('brightcove', 'Creating playlist has failed with message @message.', ['@message' => $e->getMessage()], WATCHDOG_ERROR);
      drupal_set_message(t('Creating the playlist has failed. This client might not have permission to create playlists.'), 'error');
    }
    brightcove_invalidate_cache('brightcove:playlist:list:' . $client->bcid, TRUE);
  }

  // Redirect the user to the playlist listing page.
  $form_state['redirect'] = ['admin/config/media/brightcove/playlist'];
}

/**
 * brightcove_playlist_form delete button callback.
 *
 * @param $form
 * @param $form_state
 */
function brightcove_playlist_form_delete($form, &$form_state) {
  $form_state['redirect'] = ["admin/config/media/brightcove/playlist/" . $form_state['playlist']->id . "/delete"];
}

/**
 * Delete playlist confirm form.
 *
 * @param $form
 * @param $form_state
 * @param Entity $client
 *   The brightcove client Entity object.
 * @param string $playlist_id
 *   The BrightcovePlaylist id.
 * @return array
 */
function brightcove_playlist_delete_form($form, &$form_state, $client, $playlist_id) {
  $form = [];

  $form['client_id'] = [
    '#type' => 'hidden',
    '#value' => $client->bcid,
  ];

  try {
    list($cms,) = _brightcove_create_classes($client);
    $playlist = $cms->getPlaylist($playlist_id);
  }
  catch (BrightcoveAPIException $e) {
    watchdog('brightcove', 'This playlist is not accessible by this client.', [], WATCHDOG_ERROR);
  }

  $form['playlist_id'] = [
    '#type' => 'hidden',
    '#value' => $playlist_id,
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

function _brightcove_configure_playlist_entity() {
  $field_type = 'brightcove_field';
  $field_name = 'brightcove_playlist_videos';
  $entity_type = 'brightcove_playlist';
  $field = field_info_field($field_name);
  $instance = field_info_instance($entity_type, $field_name, BRIGHTCOVE_PLAYLIST_TYPE_MANUAL);

  if (empty($field)) {
    $field = array(
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => -1,
      'entity_types' => array($entity_type),
      'translatable' => FALSE,
      'locked' => TRUE,
      'storage' => [
        'type' => 'brightcove_playlist_video_storage',
      ],
    );
    $field = field_create_field($field);
  }

  // Set field to active via a query because field_update_field() does
  // not work on inactive fields.
  db_update('field_config')
    ->fields(array('active' => 1))
    ->condition('field_name', $field_name, '=')
    ->condition('deleted', 0, '=')
    ->execute();
  field_cache_clear();

  if (empty($instance)) {
    $instance = array(
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => BRIGHTCOVE_PLAYLIST_TYPE_MANUAL,
      'label' => t('Videos'),
      'settings' => array(),
      'widget' => array(
        'type' => BRIGHTCOVE_VIDEO_WIDGET,
      ),
      'display' => array(),
    );

    field_create_instance($instance);
  }
}

/**
 * @param $playlist_id
 * @param bool $reset
 * @return bool|object
 */
function brightcove_load_playlist($playlist_id, $client_id, $reset = FALSE) {
  brightcove_load_lib();
  $cid = "brightcove:playlist:$playlist_id:$client_id";
  $cache = brightcove_cache_get($cid);
  if (!$reset && $cache) {
    return (object)$cache;
  }
  $playlist = FALSE;
  $client = brightcove_client_load($client_id);

  try {
    list($cms, ) = _brightcove_create_classes($client);
    $playlist = $cms->getPlaylist($playlist_id);
  }
  catch (Exception $e) {
    watchdog('brightcove', 'Loading brightcove playlists failed.', [], WATCHDOG_ERROR);
  }

  if ($playlist) {
    brightcove_cache_set($cid, $playlist);
  }

  return $playlist;
}

function brightcove_playlist_load($id) {
  $playlists = entity_get_controller('brightcove_playlist')->load([$id]);
  return reset($playlists);
}
