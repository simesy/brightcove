<?php

/**
 * @file
 * Holds all of the playlist field related methods.
 */

function _brightcove_field_playlist_instance_settings_form(&$form, $field, $instance) {
  // Only allow Upload if this site has Write API keys.
  if (brightcove_write_api_access()) {
    $form['allow_create'] = [
      '#type' => 'checkbox',
      '#title' => t('Create playlists'),
      '#description' => t('Whether to allow create new playlists to ' .
        'Brightcove Studio from this field. Requires Write API keys with at least a ' .
        '!link-to-editions-and-pricing', [
          '!link-to-editions-and-pricing' => l(t('Professional account'), 'http://www.brightcove.com/en/video-platform/editions-and-pricing'),
        ]
      ),
      '#default_value' => isset($instance['settings']['allow_create']) ? $instance['settings']['allow_create'] : 0,
    ];
  }
  else {
    $form['allow_create'] = [
      '#type' => 'value',
      '#value' => isset($instance['settings']['allow_create']) ?
        $instance['settings']['allow_create'] : 0,
    ];
  }

  $form['brightcove_player'] = [
    '#type' => 'select',
    '#title' => t('Brightcove Player'),
    '#default_value' => isset($instance['settings']['brightcove_player']) ?
      $instance['settings']['brightcove_player'] : NULL,
    '#options' => brightcove_player_list(),
    '#description' => t('Leave value on "Default" to use global settings.'),
  ];

  $form['per_content_player'] = [
    '#type' => 'checkbox',
    '#title' => t('Allow setting player per content'),
    '#default_value' => isset($instance['settings']['per_content_player']) ?
      $instance['settings']['per_content_player'] : NULL,
  ];
}

/**
 * Helper function to return the playlist widget form.
 */
function _brightcove_field_playlist_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  $entity_type = $element['#entity_type'];
  $entity_info = entity_get_info($entity_type);
  $eid = isset($form[$entity_info['entity keys']['id']]['#value']) ? $form[$entity_info['entity keys']['id']]['#value'] : NULL;
  $bundle = isset($form[$entity_info['entity keys']['bundle']]['#value']) ? $form[$entity_info['entity keys']['bundle']]['#value'] : NULL;
  // It needs to be able to work with field collection.
  $parent_delta = array_pop($element['#field_parents']);
  // We need this mixed variable in access checking.
  $entity_id_or_bundle = !empty($eid) ? $eid : $bundle;
  // Select the client.
  $element['#tree'] = TRUE;
  $client_options =  _brightcove_my_client_select_options();
  if (count($client_options) > 1) {
    $element['client_id'] = [
      '#type' => 'select',
      '#options' => $client_options,
      '#title' => t('Client'),
      '#default_value' => isset($items[$delta]['client_id']) ? $items[$delta]['client_id'] : array_keys($client_options)[0],
    ];
    $client_id = isset($form_state['values'][$field['field_name']]['client_id']) ? $form_state['values'][$field['field_name']]['client_id'] : $element['client_id']['#default_value'];
  }
  elseif (count($client_options) == 1) {
    $client_id = array_keys($client_options)[0];
    $element['client_id'] = [
      '#type' => 'value',
      '#value' => $client_id,
      '#default_value' => $client_id,
    ];
  }
  else {
    $element['warning'] = [
      '#markup' => t('To add playlists you need to have access to some brightcove clients first.'),
    ];
    return $element;
  }
  $client = brightcove_client_load($client_id);

  $element['brightcove_id'] = [
    '#type' => 'textfield',
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : NULL,
    '#title' => $element['#title'],
    '#maxlength' => 256,
    '#value_callback' => 'brightcove_field_playlist_browser_value',
    '#autocomplete_path' => 'brightcove_field/autocomplete/playlist/' .
      $element['#entity_type'] . '/' . $element['#field_name'] . '/' .
      ($eid ? $eid : ($bundle ? $bundle : 0)),
    '#attributes' => [
      'rel' => [$element['#field_name'] . '-' . $parent_delta . '-' . $delta],
      'class' => ['brightcove-playlist-field', $element['#field_name'] . '-' . $parent_delta . '-' . $delta],
      'data-field-name' => $element['#field_name'],
    ],
    '#element_validate' => [
      'brightcove_field_browser_playlist_validate',
    ],
    '#field_name' => $element['#field_name'],
  ];

  if (brightcove_field_browse_access('browse playlists', $entity_type, $field['field_name'], $entity_id_or_bundle, $client)) {
    // Button to browse playlists.
    $element['actions']['browse'] = [
      '#type' => 'button',
      '#attributes' => [
        'class' => ['brightcove-field-browse-button'],
        'rel' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
        'data-entity-type' => $entity_type,
        'data-bundle' => $element['#bundle'],
        'data-field-name' => $element['#field_name'],
        'data-entity-id' => is_null($eid) ? '0' : $eid,
      ],
      '#executes_submit_callback' => FALSE,
      '#limit_validation_errors' => [],
      '#default_value' => t('Browse'),
      '#value_callback' => 'brightcove_field_button_value_callback',
      '#ajax' => [
        'callback' => 'ajax_browse_playlist_dialog_callback',
      ],
      '#name' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
    ];
  }

  if (brightcove_field_browse_access('administer brightcove playlists', $entity_type, $field['field_name'], $entity_id_or_bundle, $client) &&
      (!empty($instance['settings']['allow_create']))) {
    $element['actions']['create'] = [
      '#type' => 'button',
      '#attributes' => [
        'class' => ['brightcove-field-create-button'],
        'rel' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
        'data-entity-type' => $entity_type,
        'data-bundle' => $element['#bundle'],
        'data-field-name' => $element['#field_name'],
        'data-entity-id' => is_null($eid) ? '0' : $eid,
      ],
      '#limit_validation_errors' => [],
      '#default_value' => t('Create'),
      '#value_callback' => 'brightcove_field_button_value_callback',
      '#ajax' => [
        'callback' => 'ajax_create_playlist_dialog_callback',
      ],
      '#name' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
    ];
  }

  $element['actions']['remove'] = [
    '#type' => 'button',
    '#attributes' => [
      'class' => ['brightcove-field-remove-button'],
      'rel' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
      'data-entity-type' => $entity_type,
      'data-field-name' => $element['#field_name'],
      'data-entity-id' => is_null($eid) ? '0' : $eid,
    ],
    '#default_value' => t('Remove'),
    '#value_callback' => 'brightcove_field_button_value_callback',
    '#name' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
  ];

  $element['player'] = [
    '#type' => $instance['settings']['per_content_player'] ?
      'select' : 'value',
    '#title' => t('Player'),
    '#options' => brightcove_player_list($instance),
    '#default_value' => isset($items[$delta]['player']) ? $items[$delta]['player'] : NULL,
  ];

  if (!isset($element['#default_value'])) {
    $element['actions']['remove']['#attributes']['disabled'] = 'disabled';
  }

  if (empty($brightcove_field_settings[$element['#field_name']])) {
    $brightcove_field_settings[$element['#field_name']] = [
      'brightcove_field' => [
        $element['#field_name'] => [
        'entity_type' => $entity_type,
        'field_name' => $element['#field_name'],
        'entity_id' => is_null($eid) ? '0' : $eid,
        ]
      ],
    ];
  }

  $element['brightcove_id']['#attached']['library'] = [
    ['system', 'ui.dialog']
  ];

  $element['brightcove_id']['#attached']['css'] = [
    drupal_get_path('module', 'brightcove_field') . '/styles/brightcove.css',
  ];

  $element['brightcove_id']['#attached']['js'] = [
    drupal_get_path('module', 'brightcove_field') . '/js/brightcove.js',
  ];

  $element['brightcove_id']['#attached']['js'][] = [
    'data' => $brightcove_field_settings[$element['#field_name']],
    'type' => 'setting',
  ];
}

function _brightcove_field_playlist_browse() {
  global $pager_total, $pager_page_array;
  $items_per_page = 20;
  $page = isset($_GET['page']) ? intval($_GET['page']) : '0';

  $params = [];
  $method = 'find_all_playlists';
  $result = NULL;

  // Add parameters.
  $params['page_size'] = $items_per_page;
  $params['page_number'] = $page;

  // Try to load the data from cache.
  $cid = 'brightcove:playlist:list';
  foreach ($params as $key => $param) {
    $cid .= ":{$key}:{$param}";
  }
  $content = brightcove_cache_get($cid);

  if (!$content) {
    try {
      $bc = brightcove_initialize();
      $result = $bc->find($method, $params);
    } catch (Exception $error) {
      watchdog('brightcove', 'Finding playlists in browse failed.', [], WATCHDOG_ERROR);
    }

    $pager_page_array = explode(',', $page);
    $pager_total[0] = ceil($bc->total_count / $items_per_page);
    $pager_page_array[0] =
      max(0, min(
          (int) $pager_page_array[0],
          ((int) $pager_total[0]) - 1)
      );

    $items = [];
    if (count($result)) {
      foreach ($result as $playlist) {
        $item = [];
        $item['title'] = check_plain($playlist->name);
        $item['brightcove_id'] = $playlist->id;
        if (!empty($playlist->thumbnailURL)) {
          $image_vars = ['path' => $playlist->thumbnailURL, 'alt' => '', 'title' => '', 'attributes' => ''];
          $item['thumbnail'] = theme('image', $image_vars);
        }
        else {
          $image_vars = [
            'path' => brightcove_get_default_image(),
            'alt' => '',
            'title' => '',
            'attributes' => '',
            'width' => '120',
            'height' => ''
          ];
          $item['thumbnail'] = theme('image', $image_vars);
        }
        $items[] = $item;
      }
    }

    $pager_vars = [
      'tags' => NULL,
      'element' => 0,
      'parameters' => [],
      'quantity' => $items_per_page,
    ];
    // No possibility to search any parameter of the playlist.(Only ID).
    $content['filter_form'] = [];
    $content['items'] = $items;
    $content['pager'] = theme('pager', $pager_vars);

    // Save the output to the cache record(not only the video objects),
    // because we also need data for pagination.
    brightcove_cache_set($cid, $content);
  }

  return $content;
}

/**
 * Return the playlists which contain the given string.
 *
 * @param string $string
 *   String to search for - will match playlists by this text.
 * @param boolean $reset
 *   (Optional) If TRUE, then reset the cached playlists.
 * @return array
 *   The matching playlists.
 */
function brightcove_field_get_matched_playlists($string, $reset = FALSE) {
  module_load_include('inc', 'brightcove', 'brightcove.playlist');
  $playlists = brightcove_get_playlists($reset);
  // Search for the matching elements.
  $result = [];
  if (count($playlists)) {
    foreach ($playlists as $playlist) {
      if (strpos($playlist->name, $string) !== FALSE) {
        $result[] = $playlist;
      }
    }
  }
  return $result;
}
