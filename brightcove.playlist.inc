<?php
/**
 * @file
 * Brightcove playlist related functions.
 */

/**
 * Entity controller class for Brightcove client.
 */
class BrightcovePlaylistEntityController extends EntityAPIController {

  /**
   * Overwrites EntityAPIController::create().
   */
  public function create(array $values = []) {
    // Add is_new property if it is not set.
    $values += ['is_new' => TRUE];
    brightcove_load_lib();
    $playlist_entity = new Entity($values, $this->entityType);
    if (empty($playlist_entity->client) || !($playlist_entity->client instanceof Entity)) {
      $playlist_entity->client = entity_create('brightcove_client', []);
    }
    if (empty($playlist_entity->playlist) || !($playlist_entity->playlist instanceof \Brightcove\Object\Playlist)) {
      $playlist_entity->playlist = new \Brightcove\Object\Playlist();
    }
    return $playlist_entity;
  }

  /**
   * Overwrites EntityAPIController::save().
   */
  public function save($playlist_entity, DatabaseTransaction $transaction = NULL) {
    if (!isset($playlist_entity->client)) {
      watchdog('brightcove', 'Saving playlist failed due to missing client property.', [], WATCHDOG_ERROR);
      drupal_set_message(t('Saving playlist failed due to missing client property.'), 'error');
      return;
    }
    $bcid = $playlist_entity->client->bcid;
    brightcove_try(function() use($playlist_entity, $bcid) {
      list($cms,) = _brightcove_create_classes($playlist_entity->client);
      if (!empty($playlist_entity->bpid)) {
        field_attach_update('brightcove_playlist', $playlist_entity);
        $cms->updatePlaylist($playlist_entity->playlist);
        brightcove_invalidate_cache("brightcove:playlist:{$playlist_entity->playlist->getId()}:$bcid");
      }
      else {
        field_attach_insert('brightcove_playlist', $playlist_entity);
        $cms->createPlaylist($playlist_entity->playlist);
      }
      $name = $playlist_entity->playlist->getName();
      watchdog('brightcove', 'Saving brightcove playlist "@name" was successful.', ['@name' => $name], WATCHDOG_INFO);
      drupal_set_message(t('Saving brightcove playlist "@name" was successful.', ['@name' => $name]), 'status');
    }, function (\Brightcove\API\Exception\APIException $ex) {
      throw $ex;
    });
    brightcove_invalidate_cache('brightcove:playlist:list', TRUE);
  }

  /**
   * Overwrites DrupalDefaultEntityController::load().
   *
   * @param array $ids
   *  Array of ids. The format for an id is "1234-5" where 1234 is
   *  a Playlist->getId() and 5 is a Client->bcid.
   * @param $conditions
   *
   * @return \Brightcove\Object\Playlist[] An array of Playlist objects, keyed by the playlist id.
   */
  public function load($ids = [], $conditions = []) {
    $playlists = [];
    $entity_info = entity_get_info('brightcove_playlist');
    foreach ($ids as $id) {
      $playlist_ids = explode('-', $id);
      if (count($playlist_ids) != 2) {
        continue;
      }
      $playlist_id = $playlist_ids[0];
      $bcid = $playlist_ids[1];
      $playlist = brightcove_load_playlist($playlist_id, $bcid);

      if ($playlist) {
        $values = [
          $entity_info['entity keys']['id'] => $id,
          'playlist_id' => $playlist_id,
          $entity_info['entity keys']['label'] => $playlist->getName(),
          $entity_info['entity keys']['bundle'] => ($playlist->getType() == 'EXPLICIT') ? BRIGHTCOVE_PLAYLIST_TYPE_MANUAL : BRIGHTCOVE_PLAYLIST_TYPE_SMART,
          'playlist' => $playlist,
          'client' => brightcove_client_load($bcid),
        ];
        $playlist_entity = entity_create('brightcove_playlist', $values);
        $playlists[$id] = $playlist_entity;
      }
    }
    field_attach_load('brightcove_playlist', $playlists);
    return $playlists;
  }

  /**
   * Overwrites DrupalDefaultEntityController::delete().
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $entities = $ids ? $this->load($ids) : FALSE;
    if (!$entities) {
      // Do nothing in case invalid or no ids have been passed.
      return;
    }
    foreach ($entities as $playlist_entity) {
      $playlist = $playlist_entity->playlist;
      brightcove_try(function () use($playlist, $playlist_entity) {
        list($cms,) = _brightcove_create_classes($playlist_entity->client);
        $cms->deletePlaylist($playlist->getId());

        // deletePlaylist() fails silently in case the client has no permission to
        // delete the playlist so we check here if it has been really removed or
        // not.
        try {
          $cms->getPlaylist($playlist->getId());
          watchdog('brightcove', 'Deleting the playlist @playlist has failed. Maybe the brightcove client has not enough permissions.', ['@playlist' => $playlist->getId()], WATCHDOG_ERROR);
          drupal_set_message(t('Deleting the playlist @playlist has failed. Maybe the brightcove client has not enough permissions.', ['@playlist' => $playlist->getId()]), 'error');
        }
        catch (Exception $e) {
           // Invalidate playlist cache.
          brightcove_invalidate_cache('brightcove:playlist', TRUE);
        }
      });
    }
  }
}

/**
 * Entity UI Controller class for brightcove playlists.
 */
class BrightcovePlaylistEntityUIController extends EntityBundleableUIController {
  private $form_state;

  public function hook_menu() {
    $items = parent::hook_menu();
    $items['admin/config/media/brightcove/playlist'] = [
      'type' => MENU_LOCAL_TASK,
      'title' => 'Playlists',
      'page callback' => 'drupal_get_form',
      'page arguments' => [$this->entityType . '_overview_form', $this->entityType],
      'description' => 'Manage playlists.',
      'access callback' => 'entity_access',
      'access arguments' => ['view', $this->entityType],
      'file' => 'includes/entity.ui.inc',
    ];
    return $items;
  }

  /**
   * Overrides EntityDefaultUIController::overviewForm()
   */
  public function overviewForm($form, &$form_state) {
    $this->form_state = $form_state;

    module_load_include('inc', 'brightcove', 'brightcove.client');
    $form['bcid'] = brightcove_client_select_element();
    $wrapper_id = 'bc-playlists';

    if (!isset($form['bcid']['#type'])) {
      return $form;
    }
    switch ($form['bcid']['#type']) {
      case 'select' :
        $form['bcid']['#ajax'] = [
          'wrapper' => $wrapper_id,
          'callback' => 'brightcove_playlist_tables_ajax_callback'
        ];
        $form['bcid']['#ajax_id'] = 'bc_client_select';

        $client_id = filter_input(INPUT_GET, 'client', FILTER_SANITIZE_NUMBER_INT);
        if (!empty($client_id) && (!isset($form_state['triggering_element']['#ajax_id']) || isset($form_state['triggering_element']['#ajax_id']) && $form_state['triggering_element']['#ajax_id'] != 'bc_client_select')) {
          $bcid = $client_id;
        }
        else {
          $bcid = isset($form_state['values']['bcid']) ? $form_state['values']['bcid'] : $form['bcid']['#default_value'];
        }

        if (!empty($client_id)) {
          $form['bcid']['#default_value'] = $bcid;
        }
        break;

      case 'value' :
        $bcid = $form['bcid']['#value'];
        break;

      default :
        return $form;
    }
    $form['playlists'] = [
      '#prefix' => '<div id="' . $wrapper_id . '">',
      '#suffix' => '</div>'
    ];
    if (!empty($bcid) && ($bcid != BRIGHTCOVE_BCID_NONE)) {
      $client = brightcove_client_load($bcid);
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
  public function overviewTable($conditions = []) {
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

    $page = filter_input(INPUT_GET, 'page', FILTER_SANITIZE_NUMBER_INT);
    if (empty($page)) {
      $page = 0;
    }

    $playlists = brightcove_get_playlists($client, 'name', $page);
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

    // Restore original url for the pager.
    if (isset($this->form_state['complete form']['#action'])) {
      $action = $this->form_state['complete form']['#action'];
      $qmark_pos = strripos($action, '?');
      $_GET['q'] = $qmark_pos !== FALSE ? substr($action, 0, $qmark_pos) : substr($action, 0);
    }
    pager_default_initialize(_brightcove_playlist_items_total($client), variable_get('brightcove_pager_max_items', 20), 0);
    return theme('table', $table) . theme('pager', ['parameters' => ['client' => $client->bcid]]);
  }

  /**
   * Renders a table row on the Brightcove playlist overview page.
   *
   * @param Entity $playlist
   *  The playlist entity object.
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

  /**
   * Override operationFormSubmit().
   *
   * @param $form
   * @param $form_state
   */
  public function operationFormSubmit($form, &$form_state) {
    parent::operationFormSubmit($form, $form_state);

    // Redirect back to the playlist after submit.
    $form_state['redirect'] = 'admin/config/media/brightcove/playlist';
    $form_state['rebuild'] = FALSE;
  }
}

/**
 * Access callback for playlist entities.
 */
function brightcove_playlist_access($op, $entity, $account = NULL) {
  // Viewing the playlist should be restricted by the content access the
  // playlist is embedded into.
  // Playlist creation depends on the client anyway so let's grant access and
  // notify user on failure later.
  if ($op == 'view' || $op == 'add' || $op == 'create') {
    return TRUE;
  }
  return brightcove_client_access('use', $entity->client);
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
 * It only loads the first 100 playlists.
 *
 * @param Entity $client
 *   The brightcove_client entity object.
 * @param string $sort
 *   (optional) Field to sort results on (use -field for descending order).
 * @param int $page
 *   (optional) The page to get.
 * @param bool $cache_reset
 *   (optional) True to reset the cache.
 *
 * @return bool|array
 *   Array of playlists or FALSE if none found.
 *   @see http://support.brightcove.com/en/docs/media-api-objects-reference#Playlist
 */
function brightcove_get_playlists(Entity $client, $sort = NULL, $page = 0, $cache_reset = FALSE) {
  brightcove_load_lib();
  $cid = 'brightcove:playlist:list:' . $client->bcid . ':' . $page;
  $cache = brightcove_cache_get($cid);
  if (!$cache_reset && !empty($cache)) {
    return $cache;
  }

  $playlists = FALSE;
  brightcove_try(function() use(&$playlists, $client, $sort, $page) {
    list($cms, ) = _brightcove_create_classes($client);
    $max_items = variable_get('brightcove_pager_max_items', 20);
    $offset = $max_items * $page;

    $playlists = $cms->listPlaylists($sort, $max_items, $offset);
  });

  if ($playlists) {
    brightcove_cache_set($cid, $playlists);
  }

  return $playlists;
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
 * @param Entity $playlist_entity
 *  The playlist entity to edit or create. In case of creation the
 *  only property it should have is 'type'.
 */
function brightcove_playlist_form($form, &$form_state, $playlist_entity) {
  brightcove_load_lib();
  // On AJAX the playlist_entity->playlist might not be properly populated.
  if (isset($playlist_entity->playlist) && !($playlist_entity->playlist instanceof \Brightcove\Object\Playlist) && !empty($playlist_entity->client) && isset($playlist_entity->playlist_id)) {
    $playlist_entity->playlist = brightcove_load_playlist($playlist_entity->playlist_id, $playlist_entity->client->bcid);
  }

  $form['#entity'] = $playlist_entity;
  $form['#entity_type'] = 'brightcove_playlist';
  $entity_info = entity_get_info('brightcove_playlist');
  $form['#bundle'] = $playlist_entity->{$entity_info['entity keys']['bundle']};

  $form['#tree'] = TRUE;
  // Adding a playlist.
  $playlist = NULL;
  if (!empty($playlist_entity->bpid)) {
    $playlist = $playlist_entity->playlist;
    $form['bcid'] = [
      '#type' => 'hidden',
      '#value' => $playlist_entity->client->bcid,
    ];
  }
  // Editing a playlist.
  else {
    module_load_include('inc', 'brightcove', 'brightcove.client');
    $form['bcid'] = brightcove_client_select_element();
  }
  $wrapper_id = 'bc-playlist-data-wrapper';

  $bcid = isset($form['bcid']['#default_value']) ? $form['bcid']['#default_value'] : $form['bcid']['#value'];
  switch ($form['bcid']['#type']) {
    case 'select':
      $form['bcid']['#ajax'] = [
        'wrapper' => $wrapper_id,
        'callback' => 'brightcove_playlist_client_ajax'
      ];
      if (!empty($form_state['values']['bcid'])) {
        $bcid = $form_state['values']['bcid'];
      }
      else if(!empty($form_state['input']['bcid'])) {
        $bcid = $form_state['input']['bcid'];
      }
      break;

    case 'value':
    case 'hidden' :
      $bcid = $form['bcid']['#value'];
      break;
  }
  $client = brightcove_client_load($bcid);
  $playlist_entity->client = $client;
  // If we have just changed the client select from '_none' to something, the
  // $playlist_entity->playlist is not a Playlist.
  if (empty($playlist_entity->playlist) || !($playlist_entity->playlist instanceof \Brightcove\Object\Playlist)) {
    $playlist_entity->playlist = new \Brightcove\Object\Playlist();
  }
  $form['playlist_data'] = [
    '#type' => 'container',
    '#id' => $wrapper_id,
  ];
  if ($bcid != BRIGHTCOVE_BCID_NONE) {
    $form['playlist_data']['name'] = [
      '#type' => 'textfield',
      '#title' => 'Name',
      '#default_value' => empty($playlist) ? '' : $playlist->getName(),
      '#required' => TRUE,
      '#weight' => -10,
    ];

    $form['playlist_data']['description'] = [
      '#type' => 'textarea',
      '#title' => 'Short description',
      '#default_value' => empty($playlist) ? '' : $playlist->getDescription(),
      '#weight' => -9,
    ];

    $form['playlist_data']['settings'] = [
      '#type' => 'container',
      '#id' => 'brightcove-playlist-settings',
    ];

    field_attach_form('brightcove_playlist', $playlist_entity, $form['playlist_data'], $form_state);
    if ($playlist_entity->type == BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
      $playlist_entity->playlist->setType('EXPLICIT');
    }
    elseif ($playlist_entity->type == BRIGHTCOVE_PLAYLIST_TYPE_SMART) {
      $form['playlist_data']['settings']['search'] = [
        '#type' => 'textfield',
        '#title' => t('Search string'),
        '#default_value' => $playlist_entity->playlist->getSearch(),
        '#description' => t('The playlist will contain the videos resulting from the search. Currently only tags are supported. E.g. "+tags:bird". For further information click <a href="https://docs.brightcove.com/en/video-cloud/cms-api/guides/search-videos.html#search">here</a>.'),
      ];

      $form['playlist_data']['settings']['playlistType'] = [
        '#type' => 'select',
        '#title' => t('Playlist ordering'),
        '#options' => [
          'ACTIVATED_OLDEST_TO_NEWEST' => t('Oldest to newest (by activation date)'),
          'ACTIVATED_NEWEST_TO_OLDEST' => t('Newest to oldest (by activation date)'),
          'START_DATE_OLDEST_TO_NEWEST' => t('Oldest to newest'),
          'START_DATE_NEWEST_TO_OLDEST' => t('Newest to Oldest'),
          'ALPHABETICAL' => t('Alphabetical (by video name)'),
          'PLAYS_TOTAL' => t('Total plays'),
          'PLAYS_TRAILING_WEEK' => t('Plays trailing week'),
        ],
        '#default_value' => isset($playlist->playlistType) ? $playlist->playlistType : '',
      ];
    }
    $form['playlist_data']['actions'] = ['#type' => 'actions'];

    $form['playlist_data']['actions']['submit'] = [
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => ['brightcove_playlist_form_submit'],
    ];

    if (!empty($playlist_entity->bpid)) {
      $form['playlist_data']['actions']['delete'] = [
        '#type' => 'submit',
        '#value' => t('Delete'),
        '#submit' => ['brightcove_playlist_form_delete'],
      ];
    }
  }
  return $form;
}

/**
 * Ajax callback for playlist edit form.
 */
function brightcove_playlist_form_ajax_callback($form, &$form_state) {
  return $form['settings'];
}

/**
 * brightcove_playlist_form submit handler.
 */
function brightcove_playlist_form_submit($form, &$form_state) {
  $playlist_entity = $form['#entity'];
  $playlist = $playlist_entity->playlist;
  $playlist->setName($form_state['values']['name']);
  $playlist->setDescription($form_state['values']['description']);
  if ($playlist_entity->type != BRIGHTCOVE_PLAYLIST_TYPE_MANUAL) {
    $playlist->setType($form_state['values']['settings']['playlistType']);
    $playlist->setSearch($form_state['values']['settings']['search']);
  }
  else {
    $playlist->setType('EXPLICIT');
  }
  field_attach_submit('brightcove_playlist', $playlist_entity, $form, $form_state);
  try {
    entity_get_controller('brightcove_playlist')->save($playlist_entity);
  }
  catch (Exception $e) {
    // In case of save failure keep form changes.
    $form_state['rebuild'] = TRUE;
  }
  return;
}

/**
 * brightcove_playlist_form delete button callback.
 */
function brightcove_playlist_form_delete($form, &$form_state) {
  $entity_info = entity_get_info('brightcove_playlist');
  $id_key = $entity_info['entity keys']['id'];
  $form_state['redirect'] = ["admin/config/media/brightcove/playlist/" . $form['#entity']->{$id_key} . "/delete"];
}

/**
 * Delete playlist confirm form.
 *
 * @param $form
 * @param $form_state
 * @param Entity $client
 *   The brightcove client Entity object.
 * @param string $playlist_id
 *   The Playlist id.
 * @return array
 */
function brightcove_playlist_delete_form($form, &$form_state, $client, $playlist_id) {
  $form = [];

  $form['bcid'] = [
    '#type' => 'hidden',
    '#value' => $client->bcid,
  ];

  $playlist = NULL;
  brightcove_try(function () use($client, &$playlist, $playlist_id) {
    list($cms,) = _brightcove_create_classes($client);
    $playlist = $cms->getPlaylist($playlist_id);
  });

  $form['playlist_id'] = [
    '#type' => 'hidden',
    '#value' => $playlist_id,
  ];

  return confirm_form(
    $form,
    t('Are you sure you want to delete playlist "%name"', ['%name' => $playlist ? $playlist->getName() : '']),
    'admin/config/media/brightcove/playlist',
    t('This action cannot be undone'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Playlist delete confirm form submit handler.
 */
function brightcove_playlist_delete_form_submit($form, &$form_state) {
  $entity_info = entity_get_info('brightcove_playlist');
  entity_get_controller('brightcove_playlist')->delete([$form['#entity']->{$entity_info['entity keys']['id']}]);
  // Redirect the user the playlists listing page.
  $form_state['redirect'] = ['admin/config/media/brightcove/playlist'];
}

/**
 * Create fields on the playlist entity when enabling or upgrading the module.
 */
function _brightcove_configure_playlist_entity() {
  // Clear field types from cache
  _field_info_collate_types(TRUE);

  // Add the brightcove_field field type.
  $field_type = 'brightcove_field';
  $field_name = 'brightcove_playlist_videos';
  $entity_type = 'brightcove_playlist';
  $field = field_info_field($field_name);
  $instance = field_info_instance($entity_type, $field_name, BRIGHTCOVE_PLAYLIST_TYPE_MANUAL);

  if (empty($field)) {
    $field = [
      'field_name' => $field_name,
      'type' => $field_type,
      'cardinality' => -1,
      'entity_types' => [$entity_type],
      'translatable' => FALSE,
      'locked' => TRUE,
      'storage' => [
        'type' => 'brightcove_playlist_video_storage',
      ],
    ];
    $field = field_create_field($field);
  }

  // Set field to active via a query because field_update_field() does
  // not work on inactive fields.
  db_update('field_config')
    ->fields(['active' => 1])
    ->condition('field_name', $field_name, '=')
    ->condition('deleted', 0, '=')
    ->execute();
  field_cache_clear();

  if (empty($instance)) {
    $instance = [
      'field_name' => $field_name,
      'entity_type' => $entity_type,
      'bundle' => BRIGHTCOVE_PLAYLIST_TYPE_MANUAL,
      'label' => t('Videos'),
      'settings' => [],
      'widget' => [
        'type' => BRIGHTCOVE_VIDEO_WIDGET,
      ],
      'display' => [],
    ];

    field_create_instance($instance);
  }
}

/**
 * Loads a Playlist object.
 *
 * @param $playlist_id
 *  The Playlist->getId() id.
 * @param $bcid
 *  The Client->bcid to use for fetching the playlist.
 * @param bool $reset
 *  Set to TRUE if you do not want to use the cache.
 *
 * @return bool|\Brightcove\Object\Playlist
 *  The playlist object or FALSE on failure.
 */
function brightcove_load_playlist($playlist_id, $bcid, $reset = FALSE) {
  if (!$bcid) {
    return FALSE;
  }

  brightcove_load_lib();
  $cid = "brightcove:playlist:$playlist_id:$bcid";
  $cache = brightcove_cache_get($cid);
  if (!$reset && $cache) {
    return (object)$cache;
  }
  $playlist = FALSE;
  $client = brightcove_client_load($bcid);

  brightcove_try(function() use($client, &$playlist, $playlist_id) {
    list($cms, ) = _brightcove_create_classes($client);
    $playlist = $cms->getPlaylist($playlist_id);
  });

  if ($playlist) {
    brightcove_cache_set($cid, $playlist);
  }

  return $playlist;
}

/**
 * Loads a brightcove playlist entity object by id.
 *
 * @param string $id
 *  An id in the form of "12345-6" where 12345 is Playlist->getId()
 *  and 6 is the Client->bcid.
 * @return mixed
 */
function brightcove_playlist_load($id) {
  brightcove_load_lib();
  $playlists = entity_get_controller('brightcove_playlist')->load([$id]);
  return reset($playlists);
}
