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
  $wrapper_id = "bc-playlist-{$field['field_name']}-{$delta}";
  module_load_include('inc', 'brightcove', 'brightcove.client');
  $element['bcid'] = brightcove_client_select_element();
  switch ($element['bcid']['#type']) {
    case 'select':
      $element['bcid']['#default_value'] = isset($items[$delta]['bcid']) ? $items[$delta]['bcid'] : $element['bcid']['#default_value'];
      $element['bcid']['#ajax'] = [
        'wrapper' => $wrapper_id,
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
  $client = brightcove_client_load($bcid);
  $playlist_options = [BRIGHTCOVE_BCID_NONE => t('- Select a playlist -')];
  if ($bcid != BRIGHTCOVE_BCID_NONE) {
    module_load_include('inc', 'brightcove', 'brightcove.playlist');
    $playlists = brightcove_get_playlists($client);
    if (!empty($playlists)) {
      foreach ($playlists as $playlist) {
        $id = $playlist->getId();
        $playlist_options[$id] = $playlist->getName() . " ($id)";
      }
    }
  }

  $element['brightcove_id'] = [
    '#type' => 'select',
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : BRIGHTCOVE_BCID_NONE,
    '#title' => $element['#title'],
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
    '#options' => $playlist_options,
  ];

  $element['player'] = [
    '#type' => $instance['settings']['per_content_player'] ? 'select' : 'value',
    '#title' => t('Player'),
    '#default_value' => isset($items[$delta]['player']) ? $items[$delta]['player'] : '',
    '#options' => brightcove_player_list(),
  ];
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
