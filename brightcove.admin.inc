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
  // Account.
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
  // Database.
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
  // File.
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
  // Memcached.
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

  // Upload.
  $form['upload'] = [
    '#type' => 'fieldset',
    '#title' => t('Upload settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['upload']['brightcove_track_ingestion'] = [
    '#type' => 'checkbox',
    '#title' => t('Track video ingestion'),
    '#default_value' => variable_get('brightcove_track_ingestion'),
  ];
  $form['upload']['brightcove_auto_callback'] = [
    '#type' => 'checkbox',
    '#title' => t('Allow Drupal to automatically determine the callback'),
    '#description' => t('The current callback is: %callback', ['%callback' => _brightcove_get_ingest_callback('', TRUE)]),
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

  // Other.
  $form['other'] = [
    '#type' => 'fieldset',
    '#title' => t('Other'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  ];
  $form['other']['brightcove_pager_max_items'] = [
    '#type' => 'textfield',
    '#title' => t('Playlists pager max items'),
    '#default_value' => variable_get('brightcove_pager_max_items', 20),
    '#description' => t('If you change this value you have to clear cache.'),
  ];

  $form['other']['brightcove_3rdparty'] = [
    '#type' => 'checkbox',
    '#title' => t('Where applicable, show 3rd party integration help text.'),
    '#default_value' => variable_get('brightcove_3rdparty', FALSE),
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

function brightcove_integrations_page() {
  $integrations = module_invoke_all('brightcove_integration_info');
  $page = [
    '#attached' => [
      'css' => [
        drupal_get_path('module', 'brightcove') . '/styles/integrations.css',
      ],
    ],
  ];

  if ($integrations) {
    foreach ($integrations as $integration) {
      $page[] = [
        '#theme' => 'brightcove_integration',
        '#name' => $integration['name'],
        '#description' => $integration['description'],
        '#project' => $integration['project'],
        '#image' => $integration['image'],
      ];
    }
  }
  else {
    $page[] = [
      '#markup' => '<p>' . t('No integrations has been installed. Visit the !link to learn more.', [
          '!link' => l(t('Brightcove Video Connect project page'), 'https://drupal.org/project/brightcove'),
        ]) . '</p>',
    ];
  }

  return $page;
}
