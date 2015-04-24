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
  $form['account']['brightcove_custom_field_number'] = [
    '#type' => 'textfield',
    '#title' => t('Custom field number'),
    '#default_value' => variable_get('brightcove_custom_field_number', 10),
    '#description' => t('Basically Brightcove provides 10 fields as a default, but it supports more custom fields than 10. If you have more fields than 10 you can set it here.'),
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

  // Custom fields
  $num_customfields = empty($form_state['brightcove_custom_fields']) ?
    $form_state['brightcove_custom_fields'] =
      variable_get('brightcove_custom_fields', 0) :
    $form_state['brightcove_custom_fields'];

  $form['upload_customfields'] = [
    '#type' => 'fieldset',
    '#title' => t('Custom fields'),
    '#collapsible' => TRUE,
    '#collapsed' => !((bool) $num_customfields),
    '#prefix' => '<div id="brightcove-admin-customfield">',
    '#suffix' => '</div>',
  ];

  $form['upload_customfields']['brightcove_custom_fields'] = [
    '#type' => 'value',
    '#value' => $num_customfields,
  ];

  for ($i = 0; $i < $num_customfields; ++$i) {
    $form['upload_customfields'][] = [
      '#type' => 'fieldset',
      "brightcove_custom_fields_{$i}_key" => [
        '#type' => 'textfield',
        '#title' => t('Field key'),
        '#default_value' => variable_get("brightcove_custom_fields_{$i}_key"),
      ],
      "brightcove_custom_fields_{$i}_label" => [
        '#type' => 'textfield',
        '#title' => t('Label'),
        '#default_value' => variable_get("brightcove_custom_fields_{$i}_label"),
      ],
      "brightcove_custom_fields_{$i}_type" => [
        '#type' => 'radios',
        '#title' => t('Type'),
        '#default_value' =>
          variable_get("brightcove_custom_fields_{$i}_type", 'text'),
        '#options' => [
          'text' => t('Text'),
          'list' => t('List'),
        ],
      ],
      "brightcove_custom_fields_{$i}_values" => [
        '#type' => 'textarea',
        '#title' => t('Values'),
        '#description' => t('Put each values into a new line.'),
        '#default_value' =>
          variable_get("brightcove_custom_fields_{$i}_values"),
        '#states' => [
          'visible' => [
            ":input[name=brightcove_custom_fields_{$i}_type]" => [
              'value' => 'list',
            ],
          ],
        ],
      ],
      "brightcove_custom_fields_{$i}_required" => [
        '#type' => 'checkbox',
        '#title' => t('Required'),
        '#default_value' =>
          variable_get("brightcove_custom_fields_{$i}_required"),
      ],
    ];
  }

  $form['upload_customfields']['add'] = [
    '#type' => 'submit',
    '#value' => t('Add more'),
    '#submit' => [
      'brightcove_admin_add_customfield_submit',
    ],
    '#ajax' => [
      'callback' => 'brightcove_admin_customfield_ajax_callback',
      'wrapper' => 'brightcove-admin-customfield'
    ],
  ];

  if ($num_customfields > 0) {
    $form['upload_customfields']['remove'] = [
      '#type' => 'submit',
      '#value' => t('Remove last'),
      '#submit' => [
        'brightcove_admin_remove_customfield_submit',
      ],
      '#ajax' => [
        'callback' => 'brightcove_admin_customfield_ajax_callback',
        'wrapper' => 'brightcove-admin-customfield',
      ],
    ];
  }

  $form = system_settings_form($form);
  $form['#validate'][] = 'brightcove_admin_settings_validate';

  return $form;
}

function brightcove_admin_customfield_ajax_callback($form) {
  return $form['upload_customfields'];
}

function brightcove_admin_add_customfield_submit($form, &$form_state) {
  $settings_custom_field = variable_get('brightcove_custom_field_number', 10);
  if ($form_state['brightcove_custom_fields'] < $settings_custom_field) {
    $form_state['brightcove_custom_fields']++;
  }
  $form_state['rebuild'] = TRUE;
}

function brightcove_admin_remove_customfield_submit($form, &$form_state) {
  if ($form_state['brightcove_custom_fields'] > 0) {
    $form_state['brightcove_custom_fields']--;
    $field_number = $form_state['brightcove_custom_fields'];
    variable_del("brightcove_custom_fields_{$field_number}_key");
    variable_del("brightcove_custom_fields_{$field_number}_label");
    variable_del("brightcove_custom_fields_{$field_number}_values");
    variable_del("brightcove_custom_fields_{$field_number}_required");
    variable_del("brightcove_custom_fields_{$field_number}_type");
    variable_set('brightcove_custom_fields', $form_state['brightcove_custom_fields']);
  }
  $form_state['rebuild'] = TRUE;
}

function brightcove_admin_settings_validate($form, &$form_state) {
  if ($form_state['values']['brightcove_cache_enabled'] && $form_state['values']['brightcove_cache_type'] === 'file') {
    system_check_directory($form['cache']['brightcove_cache_file']['path']);
  }
}

function brightcove_mapi_settings($form, &$form_state) {
  $form['account'] = [
    '#type' => 'fieldset',
    '#title' => t('Account settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['account']['brightcove_read_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove Read API key'),
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('brightcove_read_api_key', ''),
    '#description' => t('A Brightcove Read API key from your account at Brightcove Studio. Please check at !link page.', ['!link' => l(t('Brightcove studio'), 'http://my.brightcove.com')]),
  ];
  $form['account']['brightcove_write_api_key'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove Write API key'),
    '#maxlength' => 255,
    '#default_value' => variable_get('brightcove_write_api_key', ''),
    '#description' => t('A Brightcove Write API key from your account at Brightcove Studio. Please check at !link page. <strong>Requires at least a Professional account</strong>.', ['!link' => l(t('Brightcove studio'), 'http://my.brightcove.com')]),
  ];

  $form = system_settings_form($form);
  $form['#validate'][] = 'brightcove_mapi_settings_validate';

  return $form;
}

function brightcove_mapi_settings_validate($form, &$form_state) {
  list($can_read, $can_write) = brightcove_verify_tokens(
    $form_state['values']['brightcove_read_api_key'],
    $form_state['values']['brightcove_write_api_key']
  );

  if (!$can_read) {
    form_set_error('brightcove_read_api_key', t('Invalid read key'));
  }
  if ($form_state['values']['brightcove_write_api_key'] && !$can_write) {
    form_set_error('brightcove_write_api_key', t('Invalid write key'));
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
 * Form callback: create or edit a brightcove client.
 *
 * @param $client
 *   The client object to edit or for a create form NULL.
 */
function brightcove_client_form($form, &$form_state, $client = NULL) {
  $form['label'] = [
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($client->label) ? $client->label : '',
    '#maxlength' => 128,
    '#required' => TRUE,
    '#weight' => -10,
  ];

  $form['account_id'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove Account id'),
    '#default_value' => isset($client->account_id) ? $client->account_id : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -5,
  ];

  $form['client_id'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove API Client id'),
    '#default_value' => isset($client->client_id) ? $client->client_id : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => 0,
  ];

  $form['client_secret'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove API Client Secret Key'),
    '#default_value' => isset($client->client_secret) ? $client->client_secret : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => 5,
  ];

  // When updating a client, do not collapse the Change History fieldset.
  $form['change_history'] = [
    '#type' => 'fieldset',
    '#title' => t('Change history'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($client->bcid),
    '#weight' => 350,
  ];
  if (!empty($client->bcid)) {
    $form['change_history']['revision'] = [
      '#type' => 'checkbox',
      '#title' => t('Create new revision on update'),
      '#description' => t('If an update log message is entered, a revision will be created even if this is unchecked.'),
    ];
  }
  $form['change_history']['log'] = [
    '#type' => 'textarea',
    '#title' => !empty($client->bcid) ? t('Update log message') : t('Creation log message'),
    '#rows' => 4,
    '#description' => t('Provide an explanation of the changes you are making. This will provide a meaningful history of changes to this client.'),
  ];

  $form['actions'] = [
    '#type' => 'actions',
    '#weight' => 400,
  ];

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = [];

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save client'),
    '#submit' => array_merge($submit, ['brightcove_client_form_submit']),
  ];

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'brightcove_client_form_validate';

  return $form;
}

/**
 * Validation callback for brightcove_client_form().
 */
function brightcove_client_form_validate($form, &$form_state) {
  $client = $form_state['brightcove_client'];
  $bcid = empty($client->bcid) ? NULL : $client->bcid;
  $client_id = $form_state['values']['client_id'];
  $client_secret = $form_state['values']['client_secret'];

  libraries_load('brightcove');
  try {
    $client = BrightcoveClient::authorize($client_id, $client_secret);
    if (!$client->isAuthorized()) {
      form_error($form, t('Failed to authorize.'));
    }
  } catch (BrightcoveAuthenticationException $ex) {
    form_error($form, t('Failed to authorize.'));
    watchdog_exception('brightcove', $ex, 'Brightcove client validation');
  }

  // Ensure the proposed client id is unique or reused only during client
  // updates.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'brightcove_client')
    ->propertyCondition('client_id', $client_id);
  if (!empty($bcid)) {
    $query->entityCondition('entity_id', $bcid, '!=');
  }

  $result = $query->execute();
  if (!empty($result)) {
    $bcid = key($result['brightcove_client']);
    if (empty($client->bcid) || $bcid != $client->bcid) {
      form_set_error('client_id', t('This client id is <a href="!url">already in use</a> and must be unique. Please supply another value.', [
          '!url' => url('admin/config/media/brightcove/brightcove_client/manage/' . $bcid)
      ]));
    }
  }
  // Trim leading and trailing whitespace from the client_id.
  form_set_value($form['client_id'], trim($client_id), $form_state);
}

/**
 * Submit callback for brightcove_client_form().
 */
function brightcove_client_form_submit($form, &$form_state) {
  global $user;

  $client = &$form_state['brightcove_client'];

  $properties = [
    'label',
    'client_id',
    'client_secret',
    'account_id',
  ];
  foreach ($properties as $property) {
    $client->{$property} = $form_state['values'][$property];
  }

  // Set the client uid if it's being created at this time.
  if (empty($client->uid)) {
    $client->uid = $user->uid;
  }

  // Trigger a new revision if the checkbox was enabled or a log message supplied.
  if (!empty($form_state['values']['revision']) || !empty($form['change_history']['revision']['#default_value']) ||
    !empty($form_state['values']['log'])) {
    $client->revision = TRUE;
    $client->log = $form_state['values']['log'];
  }

  // Save the client.
  brightcove_client_save($client);
  drupal_set_message(t('Client saved.'));
}
