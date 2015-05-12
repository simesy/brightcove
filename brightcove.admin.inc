<?php

/**
 * @file
 * Admin settings for Brightcove module.
 */

/**
 * Form builder.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function brightcove_admin_settings($form, &$form_state) {
  // Account
  $form['account'] = [
    '#type' => 'fieldset',
    '#title' => t('Account settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['account']['brightcove_user_field'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove Drupal User Custom Field'),
    '#default_value' => variable_get('brightcove_user_field', ''),
    '#description' => t('A Brightcove Custom Field to store the Drupal username of the user who uploaded a video - useful to determine which Drupal user uploaded a given video in BC Studio. This field must be created in BC Studio first. Read more about !link. <strong>Requires at least a Professional account</strong>', ['!link' =>  l(t('Brightcove custom metadata'), 'http://support.brightcove.com/en/video-cloud/docs/creating-custom-metadata-fields')]),
  ];
  $form['account']['brightcove_link_field'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove Drupal Entity Link Custom Field'),
    '#default_value' => variable_get('brightcove_link_field', ''),
    '#description' => t('A Brightcove Custom Field to store the Drupal link of the entity where video belongs. This field must be created in BC Studio first. Read more about !link. <strong>Requires at least a Professional account</strong>', ['!link' =>  l(t('Brightcove custom metadata'), 'http://support.brightcove.com/en/video-cloud/docs/creating-custom-metadata-fields')]),
  ];

  // Caching.
  $form['cache'] = [
    '#type' => 'fieldset',
    '#title' => t('Caching'),
    '#collapsible' => TRUE,
  ];
  $form['cache']['brightcove_cache_enabled'] = [
    '#type' => 'checkbox',
    '#title' => t('Cache enabled'),
    '#description' => t('Will temporarily cache results retrieved from Brightcove.'),
    '#default_value' => variable_get('brightcove_cache_enabled', TRUE),
  ];
  $form['cache']['brightcove_cache_type'] = [
    '#type' => 'select',
    '#title' => t('Cache type'),
    '#options' => [
      'db' => t('Database'),
      'file' => t('File'),
      'memcached' => t('Memcached'),
    ],
    '#default_value' => variable_get('brightcove_cache_type', 'database'),
    '#states' => [
      'visible' => [
        ':input[name="brightcove_cache_enabled"]' => ['checked' => TRUE],
      ],
    ],
  ];
  // Database
  $cache_settings = variable_get('brightcove_cache_db', []);
  $form['cache']['brightcove_cache_db'] = [
    '#type' => 'fieldset',
    '#title' => t('Database cache configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="brightcove_cache_enabled"]' => ['checked' => TRUE],
        ':input[name="brightcove_cache_type"]' => ['value' => 'db'],
      ],
    ],
  ];
  $form['cache']['brightcove_cache_db']['cache_time'] = [
    '#type' => 'textfield',
    '#title' => t('Max cache age'),
    '#field_suffix' => ' ' . t('seconds'),
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => isset($cache_settings['cache_time']) ? $cache_settings['cache_time'] : BRIGHTCOVE_CACHE_LIFETIME,
  ];
  // File
  $cache_settings = variable_get('brightcove_cache_file', []);
  $form['cache']['brightcove_cache_file'] = [
    '#type' => 'fieldset',
    '#title' => t('File-based cache configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="brightcove_cache_enabled"]' => ['checked' => TRUE],
        ':input[name="brightcove_cache_type"]' => ['value' => 'file'],
      ],
    ],
  ];
  $form['cache']['brightcove_cache_file']['cache_time'] = [
    '#type' => 'textfield',
    '#title' => t('Max cache age'),
    '#field_suffix' => ' ' . t('seconds'),
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => isset($cache_settings['cache_time']) ? $cache_settings['cache_time'] : BRIGHTCOVE_CACHE_LIFETIME,
  ];
  $form['cache']['brightcove_cache_file']['path'] = [
    '#type' => 'textfield',
    '#title' => t('Path to cache directory'),
    '#maxlength' => 255,
    '#description' => t('A local file system path where the file cache will be stored. This directory must exist and be writable by Drupal. This directory must be an absolute path or relative to the Drupal installation directory.'),
    '#default_value' => isset($cache_settings['path']) ? $cache_settings['path'] : BRIGHTCOVE_CACHE_FILE_PATH,
  ];
  $form['cache']['brightcove_cache_file']['ext'] = [
    '#type' => 'textfield',
    '#title' => t('File extension'),
    '#field_prefix' => '.',
    '#default_value' => isset($cache_settings['ext']) ? $cache_settings['ext'] : BRIGHTCOVE_CACHE_FILE_EXT,
  ];
  // Memcached
  $cache_settings = variable_get('brightcove_cache_memcached', []);
  $form['cache']['brightcove_cache_memcached'] = [
    '#type' => 'fieldset',
    '#title' => t('Memcached configuration'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
    '#tree' => TRUE,
    '#states' => [
      'visible' => [
        ':input[name="brightcove_cache_enabled"]' => ['checked' => TRUE],
        ':input[name="brightcove_cache_type"]' => ['value' => 'memcached'],
      ],
    ],
  ];
  $form['cache']['brightcove_cache_memcached']['cache_time'] = [
    '#type' => 'textfield',
    '#title' => t('Max cache age'),
    '#field_suffix' => ' ' . t('seconds'),
    '#element_validate' => ['element_validate_integer_positive'],
    '#default_value' => isset($cache_settings['cache_time']) ? $cache_settings['cache_time'] : BRIGHTCOVE_CACHE_LIFETIME,
  ];
  $form['cache']['brightcove_cache_memcached']['host'] = [
    '#type' => 'textfield',
    '#title' => t('Host'),
    '#default_value' => isset($cache_settings['host']) ? $cache_settings['host'] : BRIGHTCOVE_CACHE_MEMCACHE_PATH,
  ];
  $form['cache']['brightcove_cache_memcached']['port'] = [
    '#type' => 'textfield',
    '#title' => t('Port'),
    '#default_value' => isset($cache_settings['port']) ? $cache_settings['port'] : BRIGHTCOVE_CACHE_MEMCACHE_PORT,
  ];

  $form['player'] = [
    '#type' => 'fieldset',
    '#title' => t('Player settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['player']['brightcove_default_image'] = [
    '#type' => 'textfield',
    '#title' => t('Default image'),
    '#description' => t("In case a video doesn't have a thumbnail or still image, display an image from this path"),
    '#default_value' => variable_get('brightcove_default_thumbnail', brightcove_get_default_image()),
  ];
  $form['player']['brightcove_player_responsive'] = [
    '#type' => 'checkbox',
    '#title' => t('Make all players responsive'),
    '#description' => t("Global setting to make all players responsive. Please note that this will change the template of the player."),
    '#default_value' => variable_get('brightcove_player_responsive', FALSE),
  ];
  $form['player']['brightcove_player_full_api'] = [
    '#type' => 'checkbox',
    '#title' => t('Load full JavaScript API'),
    '#description' => t("Optionally load the <a href=\"!url\">full Brightcove JavaScript API</a> rather than just the bare player logic.", ['!url' => 'http://support.brightcove.com/en/video-cloud/docs/using-javascript-flash-only-player-api']),
    '#default_value' => variable_get('brightcove_player_full_api', FALSE),
    '#states' => [
      'disabled' => [
        ':input[name="brightcove_player_smart_api"]' => ['checked' => TRUE],
      ],
    ]
  ];
  $form['player']['brightcove_player_smart_api'] = [
    '#type' => 'checkbox',
    '#title' => t('Use Smart Palyer API'),
    '#description' => t("Optionally use the <a href=\"!url\">Brightcove Smart Player API</a>. You can only use it, if you avoid to load the full JavaScript API. Smart Player API doesn't support playlist players.", ['!url' => 'http://support.brightcove.com/en/video-cloud/docs/using-smart-player-api']),
    '#default_value' => variable_get('brightcove_player_smart_api', FALSE),
    '#states' => [
      'disabled' => [
        ':input[name="brightcove_player_full_api"]' => ['checked' => TRUE],
      ],
    ],
  ];

  //Upload
  $form['upload'] = [
    '#type' => 'fieldset',
    '#title' => t('Upload settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t("Note that these settings apply only to non-FLV videos (MP4, AVI, QT, etc.)."),
  ];
  $form['upload']['brightcove_track_ingestion'] = [
    '#type' => 'checkbox',
    '#title' => t('Track video ingestion'),
    '#default_value' => variable_get('brightcove_track_ingestion'),
  ];
  $form['upload']['brightcove_auto_callback'] = [
    '#type' => 'checkbox',
    '#title' => t('Allow Drupal to automatically determine the callback'),
    '#description' => t('The current callback is: %callback', ['%callback' => _brightcove_get_ingest_callback('')]),
    '#default_value' => variable_get('brightcove_auto_callback', TRUE),
    '#states' => [
      'visible' => [
        ':input[name=brightcove_track_ingestion]' => ['checked' => TRUE],
      ],
    ],
  ];
  $form['upload']['brightcove_callback'] = [
    '#type' => 'textfield',
    '#title' => t('Ingestion service callback'),
    '#default_value' => variable_get('brightcove_callback'),
    '#states' => [
      'visible' => [
        ':input[name=brightcove_auto_callback]' => ['checked' => FALSE],
      ],
    ],
  ];

  // Status
  $form['status'] = [
    '#type' => 'fieldset',
    '#title' => t('Status settings'),
    '#description' => t("Settings relating to the availability status of a video."),
    '#collapsible' => TRUE,
  ];
  $form['status']['brightcove_check_for_unavailable'] = [
    '#type' => 'checkbox',
    '#title' => t('Check for unavailable videos'),
    '#description' => t("If checked, then the message below will be displayed if a recently uploaded video is not yet available."),
    '#default_value' => variable_get('brightcove_check_for_unavailable', TRUE),
  ];
  $form['status']['brightcove_status_display_unavailable'] = [
    '#type' => 'textarea',
    '#title' => t('Unavailable video message'),
    '#description' => t("If the checkbox above is checked, and you have a message below (which may contain HTML), it will be displayed if a video is not yet available for viewing."),
    '#default_value' => variable_get('brightcove_status_display_unavailable', 'This video is unavailable for the moment.'),
  ];

  $form = system_settings_form($form);
  $form['#validate'][] = 'brightcove_admin_settings_validate';

  return $form;
}

function brightcove_admin_settings_validate($form, &$form_state) {
  if ($form_state['values']['brightcove_cache_enabled'] && $form_state['values']['brightcove_cache_type'] === 'file') {
    system_check_directory($form['cache']['brightcove_cache_file']['path']);
  }
}

function _brightcove_player_is_default($player) {
  return (bool) (variable_get('brightcove_player_default') == $player->name);
}

function brightcove_player_setdefault_form($form, &$form_state, $player) {
  $form['name'] = [
    '#type' => 'value',
    '#value' => $player->name,
  ];

  return confirm_form($form,
    t('Are you sure that you want to set "@name" player default?', [
        '@name' => $player->display_name,
    ]),
    'admin/config/media/brightcove/players');
}

function brightcove_player_setdefault_form_submit($form, &$form_state) {
  variable_set('brightcove_player_default', $form_state['values']['name']);
  $form_state['redirect'] = 'admin/config/media/brightcove/players';
}

function brightcove_player_form_validate_field($name) {
  ctools_include('export');
  $result = ctools_export_load_object('brightcove_player', 'names', [$name]);
  return isset($result[$name]);
}

/**
 * Implements hook_form_FORM_ID_alter().
 */
function brightcove_form_ctools_export_ui_delete_confirm_form_alter(&$form, $form_state) {
  // Check whether the default player is going to be deleted or not.
  if ($form_state['item']->name == variable_get('brightcove_player_default', NULL)) {
    $players = brightcove_player_load_all();

    // If there is more than one players added, show an option for the user
    // to select a new default player.
    if (count($players) > 1) {
      $available_players = array();
      foreach ($players as $player) {
        if ($player->name != $form_state['item']->name) {
          $available_players[$player->name] = $player->display_name;
        }
      }

      $form['default_player_delete_confirm'] = array(
        '#type' => 'select',
        '#title' => t('You are about to delete the default player, please select another one to continue') . ':',
        '#options' => $available_players,
        '#required' => TRUE,
      );
    }
    // Otherwise forbid the deletion and redirect.
    else {
      $_SESSION['player_delete_redirect'] = 'admin/config/media/brightcove/players';
      drupal_set_message('The default player cannot be deleted!', 'error');
      drupal_exit();
    }
  }
}
