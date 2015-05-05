<?php

/**
 * @file
 * Holds all of the video field related methods.
 */

/**
 * Additional form settings for playlist and player fields.
 */
function _brightcove_field_playlist_instance_settings_form(&$form, $field, $instance) {
  $form['brightcove_player'] = [
    '#type' => 'select',
    '#title' => t('Brightcove Player'),
    '#default_value' => isset($instance['settings']['brightcove_player']) ? $instance['settings']['brightcove_player'] : NULL,
    '#options' => brightcove_player_list(),
    '#description' => t('Leave value on "Default" to use global settings.'),
  ];

  $form['per_content_player'] = [
    '#type' => 'checkbox',
    '#title' => t('Allow setting player per content'),
    '#default_value' => isset($instance['settings']['per_content_player']) ? $instance['settings']['per_content_player'] : NULL,
  ];
}

/**
 * Helper function to return the playlist widget form.
 */
function _brightcove_field_playlist_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  // Select the client.
  //$element['#tree'] = TRUE;
  $wrapper_id = "bc-playlist-{$field['field_name']}-{$delta}";
  module_load_include('inc', 'brightcove', 'brightcove.client');
  $element['client_id'] = brightcove_client_select_element();
  switch ($element['client_id']['#type']) {
    case 'select':
      $element['client_id']['#default_value'] = isset($items[$delta]['client_id']) ? $items[$delta]['client_id'] : BRIGHTCOVE_CLIENT_ID_NONE;
      $element['client_id']['#ajax'] = [
        'wrapper' => $wrapper_id,
        'callback' => 'brightcove_field_playlist_client_ajax_callback',
      ];
      $client_id = isset($form_state['values'][$field['field_name']][$langcode][$delta]['client_id']) ? $form_state['values'][$field['field_name']][$langcode][$delta]['client_id'] : $element['client_id']['#default_value'];
      break;

    case 'value':
      $client_id = $element['client_id']['#value'];
      break;

    default :
      return $element;
  }
  $playlist_options = [BRIGHTCOVE_CLIENT_ID_NONE => t('- Select a playlist -')];
  if ($client_id != BRIGHTCOVE_CLIENT_ID_NONE) {
    $cid = 'brightcove:playlist:list:' . $client_id;
    $playlist_options = brightcove_cache_get($cid);
    if (!$playlist_options) {
      $client = brightcove_client_load($client_id);
      try {
        list ($cms, ) = _brightcove_create_classes($client);
        $playlists = $cms->listPlaylists();
      }
      catch ( Exception $error ) {
        watchdog('brightcove', 'Finding playlists has failed.', [], WATCHDOG_ERROR);
      }
      if (!empty($playlists)) {
        foreach ($playlists as $playlist) {
          $id = $playlist->getId();
          $playlist_options[$id] = $playlist->getName() . " ($id)";
        }
      }
      brightcove_cache_set($cid, $playlist_options);
    }
  }

  $element['brightcove_id'] = [
    '#type' => 'select',
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : BRIGHTCOVE_CLIENT_ID_NONE,
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
