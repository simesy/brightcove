<?php

/**
 * @file
 * Holds all of the video field related methods.
 */

/**
 * Helper function to return the playlist widget form.
 */
function _brightcove_field_playlist_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  // Select the client.
  $ajax_wrapper_id = "bc-playlist-{$field['field_name']}-{$delta}-ajax-wrapper";

  $element['#prefix'] = "<div id='{$ajax_wrapper_id}'>";
  $element['#suffix'] = '</div>';

  module_load_include('inc', 'brightcove', 'brightcove.client');
  $element['bcid'] = brightcove_client_select_element();
  switch ($element['bcid']['#type']) {
    case 'select':
      $element['bcid']['#default_value'] = isset($items[$delta]['bcid']) ? $items[$delta]['bcid'] : $element['bcid']['#default_value'];
      $element['bcid']['#ajax'] = [
        'wrapper' => $ajax_wrapper_id,
        'callback' => 'brightcove_field_playlist_client_ajax_callback',
      ];
      $element['bcid']['#ajax_id'] = 'bcid';
      $bcid = $element['bcid']['#default_value'];
      if (isset($form_state['values'][$field['field_name']][$langcode][$delta]['bcid'])) {
        $bcid = $form_state['values'][$field['field_name']][$langcode][$delta]['bcid'];
      }
      elseif (isset($form_state['input'][$field['field_name']][$langcode][$delta]['bcid'])) {
        $bcid = $form_state['input'][$field['field_name']][$langcode][$delta]['bcid'];
      }
      elseif (isset($items[$delta]['bcid'])) {
        $bcid = $items[$delta]['bcid'];
        if ($bcid === "0" && !empty($items[$delta]['brightcove_id'])) {
          $bcid = variable_get('brightcove_client_default');
        }
      }
      $element['bcid']['#default_value'] = $bcid;
      break;

    case 'value':
      $bcid = $element['bcid']['#value'];
      break;

    default :
      return $element;
  }

  // Load client.
  $client = brightcove_client_load($bcid);

  if (!$client) {
    return $element;
  }

  $element['previous_client'] = [
    '#type' => 'value',
    '#value' => $bcid,
  ];

  if (isset($form_state['values'][$field['field_name']][$langcode][$delta]['previous_client']) && $form_state['values'][$field['field_name']][$langcode][$delta]['previous_client'] !== $bcid) {
    $form_state['input'][$field['field_name']][$langcode][$delta]['brightcove_id'] = '';
    $form_state['values'][$field['field_name']][$langcode][$delta]['brightcove_id'] = '';
  }

  // Set field title.
  $element['brightcove_id_title'] = [
    '#type' => 'item',
    '#title' => $element['#title'],
  ];

  $items_total = _brightcove_playlist_items_total($client);

  if (isset($form_state['triggering_element']['#ajax_id']) && $form_state['triggering_element']['#ajax_id'] == 'bcid') {
    $form_state['input'][$field['field_name']][$langcode][$delta]['brightcove_id_current_page'] = 0;
  }

  // Get the current page.
  if (isset($form_state['input'][$field['field_name']][$langcode][$delta]['brightcove_id_current_page'])) {
    $page = $form_state['input'][$field['field_name']][$langcode][$delta]['brightcove_id_current_page'];
  }
  else {
    $page = 0;
  }

  // Page back.
  if (isset($form_state['triggering_element']['#ajax_id']) && $form_state['triggering_element']['#ajax_id'] == 'bc_pager_prev' && $form_state['triggering_element']['#delta'] == $delta) {
    $page = $page > 0 ? $page - 1 : 0;
  }

  // Page forward.
  if (isset($form_state['triggering_element']['#ajax_id']) && $form_state['triggering_element']['#ajax_id'] == 'bc_pager_next' && $form_state['triggering_element']['#delta'] == $delta) {
    $page++;
  }

  if (isset($form['#node']->{$field['field_name']}[$langcode][$delta]) && isset($form['#node_edit_form']) && $form['#node_edit_form'] && !isset($form_state['triggering_element'])) {
    // Find the and set value.
    $brightcove_id = $form['#node']->{$field['field_name']}[$langcode][$delta]['brightcove_id'];
    // Have to page through the list.
    do {
      $playlist_options = _brightcove_field_get_playlists($client, $page);
      $page++;
    }
    while (!in_array($brightcove_id, array_keys($playlist_options)) && !empty($playlist_options));

    if (empty($playlist_options)) {
      // Give back the first page if value not found.
      $page = 0;
      $playlist_options = _brightcove_field_get_playlists($client, $page);
    }
    else {
      // The be on the current page we have to remove one because the do ... while
      // counts +1;
      $page = $page > 0 ? $page - 1 : 0;
    }
  }
  else {
    $playlist_options = _brightcove_field_get_playlists($client, $page);
  }

  $player_list = [];
  brightcove_try(function() use(&$player_list, $client) {
    $player_list = brightcove_player_list($client->bcid);
  });
  $element['player'] = [
    '#type' => 'select',
    '#title' => t('Player'),
    '#options' => $player_list,
    '#default_value' => isset($items[$delta]['player']) ? $items[$delta]['player'] : brightcove_get_default_player($client->bcid),
  ];

  $max_items = variable_get('brightcove_pager_max_items', 20);
  $offset = $max_items * $page;

  $element['brightcove_id_current_page'] = [
    '#type' => 'hidden',
    '#value' => $page,
  ];

  $element['brightcove_id'] = [
    '#type' => 'tableselect',
    '#header' => [
      'id' => t('ID'),
      'title' =>t('Title')
    ],
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : BRIGHTCOVE_BCID_NONE,
    '#options' => $playlist_options,
    '#multiple' => FALSE,
  ];

  $element['brightcove_id_pager'] = [
    '#type' => 'container',
  ];

  // Previous button.
  $element['brightcove_id_pager']['pager']['previous_button'] = [
    '#value' => t('Previous'),
    '#ajax_id' => 'bc_pager_prev',
  ];
  if ($page == 0) {
    $element['brightcove_id_pager']['pager']['previous_button']['#disabled'] = TRUE;
  }
  else {
    $element['brightcove_id_pager']['pager']['previous_button']['#disabled'] = FALSE;
  }

  // Next button.
  $element['brightcove_id_pager']['pager']['next_button'] = [
    '#value' => t('Next'),
    '#ajax_id' => 'bc_pager_next',
  ];
  if ($offset + $max_items > $items_total) {
    $element['brightcove_id_pager']['pager']['next_button']['#disabled'] = TRUE;
  }
  else {
    $element['brightcove_id_pager']['pager']['next_button']['#disabled'] = FALSE;
  }

  foreach ($element['brightcove_id_pager']['pager'] as &$item) {
    $item['#type'] = 'button';
    $item['#limit_validation_errors'] = [];
    $item['#ajax'] = [
      'callback' => 'brightcove_field_playlist_pager',
      'event' => 'click',
      'method' => 'replace',
      'wrapper' => $ajax_wrapper_id,
    ];
    $item['#field_name'] = $field['field_name'];
    $item['#langcode'] = $langcode;
    $item['#delta'] = $delta;
    $item['#name'] = "{$item['#ajax_id']}_{$field['field_name']}_{$delta}";
  }

  return $element;
}

/**
 * AJAX callback to update playlist table.
 */
function brightcove_field_playlist_pager($form, &$form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  // Pop some parents as we have to update the whole field.
  array_pop($parents);
  array_pop($parents);
  array_pop($parents);
  $return = drupal_array_get_nested_value($form, $parents);
  unset($return['_weight']);
  return $return;
}

function _brightcove_field_get_playlists(Entity $client, $page) {
  $playlist_options = [];
  if ($client->bcid != BRIGHTCOVE_BCID_NONE) {
    module_load_include('inc', 'brightcove', 'brightcove.playlist');
    $playlists = brightcove_get_playlists($client, 'name', $page);
    if (!empty($playlists)) {
      foreach ($playlists as $playlist) {
        $id = $playlist->getId();
        $playlist_options[$id] = [
          'id' => $id,
          'title' => $playlist->getName(),
        ];
      }
    }
  }
  return $playlist_options;
}

/**
 * AJAX callback for playlist select update on client selection.
 */
function brightcove_field_playlist_client_ajax_callback($form, $form_state) {
  $parents = $form_state['triggering_element']['#parents'];
  array_pop($parents);
  $return = drupal_array_get_nested_value($form, $parents);
  unset($return['_weight']);
  return $return;
}
