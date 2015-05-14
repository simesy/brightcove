<?php

/**
 * @file
 * Holds all of the video field related methods.
 */

function _brightcove_field_video_instance_settings_form(&$form, $field, $instance) {
  $form['allow_upload'] = [
    '#type' => 'checkbox',
    '#title' => t('Allow upload if client has the Brightcove permission for it.'),
    '#description' => t('Whether to allow uploading new videos to Brightcove from this field. Requires exposure of the "Video Read/Write" API at the Brightcove API Authentication page.'),
    '#default_value' => isset($instance['settings']['allow_upload']) ? $instance['settings']['allow_upload'] : 0,
  ];

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
 * Helper function to return the video widget form.
 */
function _brightcove_field_video_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  $entity_type = $element['#entity_type'];
  $entity_info = entity_get_info($entity_type);
  $eid = isset($form['#entity']->{$entity_info['entity keys']['id']}) ? $form['#entity']->{$entity_info['entity keys']['id']} : NULL;
  $bundle = $element['#bundle'];
  // It needs to be able to work with field collection.
  $parent_delta = array_pop($element['#field_parents']);
  // We need this mixed variable in access checking.
  $entity_id_or_bundle = !empty($eid) ? $eid : $bundle;
  // Select the client.
  $element['#tree'] = TRUE;
  $wrapper_id = _brightcove_field_get_wrapper_name($field['field_name'], $delta);

  // Playlists already determine the client.
  if ($entity_type == 'brightcove_playlist') {
    // Editing a playlist.
    if (!empty($form['#entity']->client && ($form['#entity']->client instanceof Entity))) {
      $bcid = $form['#entity']->client->bcid;
    }
    // Adding a playlist.
    else {
      $bcid = isset($form_state['values']['bcid']) ? $form_state['values']['bcid'] : $form_state['input']['bcid'];
    }
    $element['bcid'] = [
      '#type' => 'value',
      '#value' => $bcid,
    ];
  }
  // This is not a playlist entity.
  else {
    module_load_include('inc', 'brightcove', 'brightcove.client');
    $element['bcid'] = brightcove_client_select_element();
    switch ($element['bcid']['#type']) {
      case 'select':
        $element['bcid']['#ajax'] = [
          'wrapper' => $wrapper_id,
          'callback' => 'brightcove_field_video_client_ajax_callback',
        ];
        $bcid = $element['bcid']['#default_value'];
        if (isset($form_state['values'][$field['field_name']][$langcode][$delta]['bcid'])) {
          $bcid = $form_state['values'][$field['field_name']][$langcode][$delta]['bcid'];
        }
        elseif (isset($form_state['input'][$field['field_name']][$langcode][$delta]['bcid'])) {
          $bcid = $form_state['input'][$field['field_name']][$langcode][$delta]['bcid'];
        }
        elseif (isset($items[$delta]['bcid'])) {
          $bcid = $items[$delta]['bcid'];
        }
        $element['bcid']['#default_value'] = $bcid;
        break;

      case 'value':
        $bcid = $element['bcid']['#value'];
        break;

      default:
        return;
    }
  }

  if (empty($brightcove_field_settings[$element['#field_name']])) {
    $brightcove_field_settings[$element['#field_name']] = [
      'brightcove_field' => [
        $element['#field_name'] => [
          'entity_type' => $entity_type,
          'field_name' => $element['#field_name'],
          'entity_id' => is_null($eid) ? '0' : $eid,
        ],
      ],
    ];
  }

  // Since the actions buttons depend on the client selection,
  // we wrap them to update them via AJAX.
  $element['#prefix'] = "<div id=\"{$wrapper_id}\">";
  $element['#suffix'] = '</div>';

  $element['actions'] = [
    '#type' => 'container',
    '#weight' => 99,
  ];

  $client = brightcove_client_load($bcid);
  if (!empty($client)) {
    $element['brightcove_id'] = [
      '#type' => 'textfield',
      '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : NULL,
      '#title' => $element['#title'],
      '#maxlength' => 256,
      '#value_callback' => 'brightcove_field_video_browser_value',
      '#attributes' => [
        'rel' => [$element['#field_name'] . '-' . $parent_delta . '-' . $delta],
        'class' => ['brightcove-video-field', $element['#field_name'] . '-' . $parent_delta . '-' . $delta],
        'data-field-name' => $element['#field_name'],
      ],
      '#element_validate' => [
        'brightcove_field_browser_video_validate',
      ],
      '#field_name' => $element['#field_name'],
      '#description' => $element['#description'],
      '#attached' => [
        'library' => [
          ['system', 'ui.dialog'],
        ],
        'css' => [
          drupal_get_path('module', 'brightcove_field') . '/styles/brightcove.css',
        ],
        'js' => [
          drupal_get_path('module', 'brightcove_field') . '/js/brightcove.js',
          [
            'data' => $brightcove_field_settings[$element['#field_name']],
            'type' => 'setting',
          ],
        ],
      ],
    ];

    $element['player'] = [
      '#type' => $instance['settings']['per_content_player'] ? 'select' : 'value',
      '#title' => t('Player'),
      '#options' => brightcove_player_list($instance),
      '#default_value' => isset($items[$delta]['player']) ? $items[$delta]['player'] : NULL,
    ];

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

    if (!isset($element['#default_value'])) {
      $element['actions']['remove']['#attributes']['disabled'] = 'disabled';
    }

    if (brightcove_field_browse_access('browse videos', $entity_type, $field['field_name'], $entity_id_or_bundle, $client)) {
      // Button to browse videos.
      $element['actions']['browse'] = [
        '#type' => 'button',
        '#attributes' => [
          'class' => ['brightcove-field-browse-video-button'],
          'rel' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
          'data-entity-type' => $entity_type,
          'data-bundle' => $element['#bundle'],
          'data-field-name' => $element['#field_name'],
          'data-entity-id' => is_null($eid) ? '0' : $eid,
          'data-client-id' => $bcid,
        ],
        '#executes_submit_callback' => FALSE,
        '#limit_validation_errors' => [],
        '#default_value' => t('Browse'),
        '#value_callback' => 'brightcove_field_button_value_callback',
        '#ajax' => [
          'callback' => 'ajax_browse_video_dialog_callback',
        ],
        '#name' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
      ];
    }

    if (brightcove_field_browse_access('upload videos', $entity_type, $field['field_name'], $entity_id_or_bundle, $client) &&
      $instance['settings']['allow_upload']
    ) {
      $element['actions']['upload'] = [
        '#type' => 'button',
        '#attributes' => [
          'class' => ['brightcove-field-upload-button'],
          'rel' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
          'data-entity-type' => $entity_type,
          'data-bundle' => $element['#bundle'],
          'data-field-name' => $element['#field_name'],
          'data-entity-id' => is_null($eid) ? '0' : $eid,
          'data-client-id' => $bcid,
        ],
        '#limit_validation_errors' => [],
        '#default_value' => t('Upload'),
        '#value_callback' => 'brightcove_field_button_value_callback',
        '#ajax' => [
          'callback' => 'ajax_upload_video_dialog_callback',
        ],
        '#name' => $element['#field_name'] . '-' . $parent_delta . '-' . $delta,
      ];
    }
  }
}

/**
 * AJAX callback for playlist select update on client selection and Attach
 * button.
 */
function brightcove_field_video_client_ajax_callback($form, $form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  // This callback is used by both the client selection and the attach button
  // so we are not sure about the depth of the array. We want to get until sg
  // like ['field_video']['und'][1] and we do this by checking if the popped
  // array element is numeric or not.
  $popped = NULL;
  while (!is_numeric($popped)) {
    $popped = array_pop($parents);
  }
  $parents[] = $popped;
  $return = drupal_array_get_nested_value($form, $parents);
  unset($return['_weight']);
  return $return;
}

/**
 * Video browser form.
 *
 * @param Entity $client
 *   The brightcove client entity object.
 *
 * @return An array rendered by brightcove_field_browse().
 */
function _brightcove_field_video_browse($client) {
  global $pager_total, $pager_page_array;
  $items_per_page = 20;
  $page = isset($_GET['page']) ? intval($_GET['page']) : '0';

  $params = [];
  $search = '';
  if (isset($_SESSION['brightcove_field_filter']['search'])) {
    switch ($_SESSION['brightcove_field_filter']['search']) {
      case 'tags':
        $search = "tags:{$_SESSION['brightcove_field_filter']['keywords']}";
        break;

      case 'everything':
      default:
        $search = "text:{$_SESSION['brightcove_field_filter']['keywords']}";
        break;
    }
  }
  $params['page_size'] = $items_per_page;
  $params['page_number'] = $page;

  $result = NULL;

  // Try to load the data from cache.
  $cid = "brightcove:video:list:search:$search";
  foreach ($params as $key => $param) {
    $cid .= ":{$key}:{$param}";
  }
  $cid .= ":client:$client->bcid";
  $content = brightcove_cache_get($cid);

  // If no cached data is available.
  if (!$content) {
    list($cms,) = _brightcove_create_classes($client);
    try {
      $result = $cms->listVideos($search, NULL, $items_per_page, $items_per_page * $page);
    }
    catch (Exception $error) {
      watchdog('brightcove', 'Finding videos in browse failed.', [], WATCHDOG_ERROR);
      drupal_set_message(t('Finding videos in browse failed.'), 'error');
    }

    $pager_page_array = explode(',', $page);
    $pager_total[0] = ceil($cms->countVideos($search) / $items_per_page);
    $pager_page_array[0] = max(0, min(
      (int)$pager_page_array[0],
      ((int)$pager_total[0]) - 1)
    );
    $items = [];
    if (count($result)) {
      foreach ($result as $video) {
        $item = [];
        $item['title'] = check_plain($video->getName());
        $item['brightcove_id'] = $video->getId();
        $video_images = $video->getImages();
        if (!empty($video_images)) {
          $image_vars = ['path' => $video_images['thumbnail']->getSrc(), 'alt' => '', 'title' => '', 'attributes' => ''];
          $item['thumbnail'] = theme('image', $image_vars);
        }
        else {
          $image_vars = ['path' => brightcove_get_default_image(), 'alt' => '', 'title' => '', 'attributes' => '','width' => '120', 'height' => '90'];
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

    $content['pager'] = theme('pager', $pager_vars);
    $content['items'] = $items;

    // Save the output to the cache record(not only the video objects),
    // because we also need data for pagination.
    brightcove_cache_set($cid, $content);
  }

  // The filter form should not be cached,
  // because it wouldn't invoke the submit handler in this case.
  $content['filter_form'] = drupal_get_form('brightcove_field_filter_form', $client);
  return $content;
}
