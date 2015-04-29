<?php

/**
 * Helper function to return the playlist widget form.
 */
function _brightcove_field_playlist_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  // Select the client.
  //$element['#tree'] = TRUE;
  $wrapper_id = "bc-playlist-{$field['field_name']}-$delta";
  module_load_include('inc', 'brightcove', 'brightcove.client');
  $element['client_id'] = brightcove_client_select_element();
  switch ($element['client_id']['#type']) {
    case 'select':
      $element['client_id']['#default_value'] = isset($items[$delta]['client_id']) ? $items[$delta]['client_id'] : '_none';
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

  $playlists = [];
  if ($client_id != '_none') {
    $cid = 'brightcove:playlist:list:' . $client_id;
    $playlists = brightcove_cache_get($cid);
    if (!$playlists) {
      $client = brightcove_client_load($client_id);
      try {
        list ($cms, ) = _brightcove_create_classes($client);
        $playlists = $cms->listPlaylists();
      }
      catch ( Exception $error ) {
        watchdog('brightcove', 'Finding playlists has failed.', [], WATCHDOG_ERROR);
      }
      brightcove_cache_set($cid, $playlists);
    }
  }
  $playlist_options = ['_none' => t('- Select a playlist -')];
  foreach ($playlists as $playlist) {
    $id = $playlist->getId();
    $playlist_options[$id] = $playlist->getName() . " ($id)";
  }
  $element['brightcove_id'] = [
    '#type' => 'select',
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : '_none',
    '#title' => $element['#title'],
    '#prefix' => '<div id="' . $wrapper_id . '">',
    '#suffix' => '</div>',
    '#options' => $playlist_options,
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
