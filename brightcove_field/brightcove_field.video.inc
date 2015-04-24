<?php

/**
 * @file
 * Holds all of the video field related methods.
 */

function _brightcove_field_video_instance_settings_form(&$form, $field, $instance) {
  // Only allow Upload if this site has Write API keys.
  if (brightcove_write_api_access()) {
    $form['allow_upload'] = [
      '#type' => 'checkbox',
      '#title' => t('Allow upload'),
      '#description' => t('Whether to allow uploading new videos to ' .
        'Brightcove Studio from this field. Requires Write API keys with at least a ' .
        '!link-to-editions-and-pricing', [
          '!link-to-editions-and-pricing' => l(t('Professional account'), 'http://www.brightcove.com/en/video-platform/editions-and-pricing'),
        ]
      ),
      '#default_value' => isset($instance['settings']['allow_upload']) ? $instance['settings']['allow_upload'] : 0,
    ];
  }
  else {
    $form['allow_upload'] = [
      '#type' => 'value',
      '#value' => isset($instance['settings']['allow_upload']) ?
        $instance['settings']['allow_upload'] : 0,
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

function _brightcove_field_video_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  $entity_type = $element['#entity_type'];
  $entity_info = entity_get_info($entity_type);
  $eid = isset($form[$entity_info['entity keys']['id']]['#value']) ? $form[$entity_info['entity keys']['id']]['#value'] : NULL;
  $bundle = isset($form[$entity_info['entity keys']['bundle']]['#value']) ? $form[$entity_info['entity keys']['bundle']]['#value'] : NULL;
  // It needs to be able to work with field collection.
  $parent_delta = array_pop($element['#field_parents']);
  // We need this mixed variable in access checking.
  $entity_id_or_bundle = !empty($eid) ? $eid : $bundle;
  // Pass on the selected client.
  $id = 'bc-video-' . str_replace('_', '-', $field['field_name']) . '-' . $element['#delta'];
  $element['#prefix'] = '<div id="' . $id . '">';
  $element['#suffix'] = '</div>';
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
      '#markup' => t('To add videos you need to have access to some bricghtcove clients first.'),
    ];
    return $element;
  }
  $client = brightcove_client_load($client_id);

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
  ];

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
    '#default_value' => isset($items[$delta]['player']) ?
      $items[$delta]['player'] : NULL,
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

function _brightcove_field_video_widget_client_ajax($form, $form_state) {
  return $form;
}
/**
 *
 * @param Entity $client
 *   The brightcove client entity object.
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
        $params = ['tags' => $_SESSION['brightcove_field_filter']['keywords']];
        $search = "tags:{$_SESSION['brightcove_field_filter']['keywords']}";
        break;

      case 'everything':
      default:
        $params = ['text' => join(' ', explode(',', $_SESSION['brightcove_field_filter']['keywords']))];
        $search = $_SESSION['brightcove_field_filter']['keywords'];
        break;
    }
  }
  $params['page_size'] = $items_per_page;
  $params['page_number'] = $page;
  $params['get_item_count'] = 'TRUE';

  $result = NULL;

  // Try to load the data from cache.
  $cid = 'brightcove:video:list';
  foreach ($params as $key => $param) {
    $cid .= ":{$key}:{$param}";
  }
  $cid .= ":$client->bcid";
  $content = brightcove_cache_get($cid);

  // If no cached data is available.
  if (!$content) {
    list($cms,) = _brightcove_create_classes($client);
    try {
      $result = $cms->listVideos($search, NULL, $items_per_page, $items_per_page * $page);
    }
    catch (Exception $error) {
      watchdog('brightcove', 'Finding videos in browse failed.', [], WATCHDOG_ERROR);
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
