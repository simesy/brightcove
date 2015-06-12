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
  //$element['#tree'] = TRUE;
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
      $bcid = isset($form_state['values'][$field['field_name']][$langcode][$delta]['bcid']) ? $form_state['values'][$field['field_name']][$langcode][$delta]['bcid'] : $element['bcid']['#default_value'];
      break;

    case 'value':
      $bcid = $element['bcid']['#value'];
      break;

    default :
      return $element;
  }

  // Load client.
  $client = brightcove_client_load($bcid);

  // Set field title.
  $element['brightcove_id_title'] = [
    '#type' => 'item',
    '#title' => $element['#title'],
  ];

  $items_total = _brightcove_playlist_items_total($client);

  // Get the current page.
  if (isset($form_state['input'][$field['field_name']][$langcode][$delta]['brightcove_id_current_page'])) {
    $page = $form_state['input'][$field['field_name']][$langcode][$delta]['brightcove_id_current_page'];
  }
  else {
    $page = 0;
  }

  // Page back.
  if (isset($form_state['triggering_element']['#button_id']) && $form_state['triggering_element']['#button_id'] == 'bc_pager_prev') {
    $page--;
  }

  // Page forward.
  if (isset($form_state['triggering_element']['#button_id']) && $form_state['triggering_element']['#button_id'] == 'bc_pager_next') {
    $page++;
  }

  if (isset($form['#node']->{$field['field_name']}) && isset($form['#node_edit_form']) && $form['#node_edit_form'] && !isset($form_state['triggering_element']['#button_id'])) {
    // Find the set value.
    $brightcove_id = $form['#node']->{$field['field_name']}[$langcode][$delta]['brightcove_id'];
    // Have to page through the list.
    do {
      $playlist_options = _brightcove_field_get_playlists($client, $page);
      $page++;
    }
    while (!in_array($brightcove_id, array_keys($playlist_options)) && !empty($playlist_options));

    if (empty($playlist_options)) {
      // Reset page number if the value is not found.
      $page = 0;
    }
    else {
      // The be on the current page we have to remove one because the do ... while
      // counts +1;
      $page--;
    }
  }
  else {
    $playlist_options = _brightcove_field_get_playlists($client, $page);
  }

  $offset = BRIGHTCOVE_PLAYLIST_MAX_ITEMS * $page;

  $element['brightcove_id_current_page'] = [
    '#type' => 'hidden',
    '#value' => $page,
  ];

  $element['brightcove_id'] = [
    '#type' => 'tableselect',
    '#header' => array(
      'id' => t('ID'),
      'title' =>t('Title')
    ),
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : BRIGHTCOVE_BCID_NONE,
    '#options' => $playlist_options,
    '#multiple' => FALSE,
    '#bc_current_page' => $page,
  ];

  $element['brightcove_id_pager'] = [
    '#type' => 'container',
  ];

  // Previous button.
  $element['brightcove_id_pager']['pager']['previous_button'] = [
    '#value' => t('Previous'),
    '#button_id' => 'bc_pager_prev',
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
    '#button_id' => 'bc_pager_next',
  ];
  if ($offset + BRIGHTCOVE_PLAYLIST_MAX_ITEMS > $items_total) {
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
  }

  return $element;
}

/**
 * AJAX callback to update playlist table.
 */
function brightcove_field_playlist_pager($form, &$form_state) {
  $field = $form_state['triggering_element']['#field_name'];
  return $form[$field];
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
  $parents[] = 'brightcove_id';
  $return = drupal_array_get_nested_value($form, $parents);
  return $return;
}
