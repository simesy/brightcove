<?php

/**
 * @file
 * Holds all of the playlist field related methods.
 */

function _brightcove_field_playlist_instance_settings_form(&$form, $field, $instance) {
  // Only allow Upload if this site has Write API keys.
  if (brightcove_write_api_access()) {
    $form['allow_create'] = array(
      '#type' => 'checkbox',
      '#title' => t('Create playlists'),
      '#description' => t('Whether to allow create new playlists to ' .
                          'Brightcove Studio from this field. ' .
                          'Requires Write API keys with at least a ' .
                          '!link-to-editions-and-pricing',
                          array(
                            '!link-to-editions-and-pricing' => l(
                              t('Professional account'),
              'http://www.brightcove.com/en/video-platform/editions-and-pricing'
                            ),
                          )),
      '#default_value' => isset($instance['settings']['allow_create']) ?
        $instance['settings']['allow_create'] : 0,
    );
  }
  else {
    $form['allow_create'] = array(
      '#type' => 'value',
      '#value' => isset($instance['settings']['allow_create']) ?
        $instance['settings']['allow_create'] : 0,
    );
  }

  $form['brightcove_player'] = array(
    '#type' => 'select',
    '#title' => t('Brightcove Player'),
    '#default_value' => isset($instance['settings']['brightcove_player']) ?
      $instance['settings']['brightcove_player'] : NULL,
    '#options' => brightcove_player_list(),
    '#description' => t('Leave empty if you don\'t want to override the global settings.'),
  );

  $form['per_content_player'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow setting player per content'),
    '#default_value' => isset($instance['settings']['per_content_player']) ?
      $instance['settings']['per_content_player'] : NULL,
  );
}

function _brightcove_field_playlist_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, &$element) {
  $entity_type = $element['#entity_type'];
  $entity_info = entity_get_info($entity_type);
  $eid = isset($form[$entity_info['entity keys']['id']]['#value']) ? $form[$entity_info['entity keys']['id']]['#value'] : NULL;
  $bundle = isset($form[$entity_info['entity keys']['bundle']]['#value']) ? $form[$entity_info['entity keys']['bundle']]['#value'] : NULL;
  $element['brightcove_id'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($items[$delta]['brightcove_id']) ? $items[$delta]['brightcove_id'] : NULL,
    '#title' => $element['#title'],
    '#value_callback' => 'brightcove_field_playlist_browser_value',
    '#autocomplete_path' => 'brightcove_field/autocomplete/playlist/' .
      $element['#entity_type'] . '/' . $element['#field_name'] . '/' .
      ($eid ? $eid : ($bundle ? $bundle : 0)),
    '#attributes' => array(
      'rel' => array($element['#field_name'] . '-' . $delta),
      'class' => array('brightcove-playlist-field', $element['#field_name'] . '-' . $delta),
      'data-field-name' => $element['#field_name'],
    ),
    '#element_validate' => array(
      'brightcove_field_browser_playlist_validate',
    ),
    '#field_name' => $element['#field_name'],
  );

  if (user_access('browse playlists')) {
    // Button to browse playlists.
    $element['actions']['browse'] = array(
      '#type' => 'button',
      '#attributes' => array(
        'class' => array('brightcove-field-browse-button'),
        'rel' => $element['#field_name'] . '-' . $delta,
        'data-entity-type' => $entity_type,
        'data-bundle' => $element['#bundle'],
        'data-field-name' => $element['#field_name'],
        'data-entity-id' => is_null($eid) ? '0' : $eid,
      ),
      '#executes_submit_callback' => FALSE,
      '#limit_validation_errors' => array(),
      '#default_value' => t('Browse'),
      '#value_callback' => 'brightcove_field_button_value_callback',
      '#ajax' => array(
        'callback' => 'ajax_browse_playlist_dialog_callback',
      ),
      '#name' => $element['#field_name'] . '-' . $delta,
    );
  }

  if ((user_access('administer brightcove playlists')) &&
      (!empty($instance['settings']['allow_create']))) {
    $element['actions']['create'] = array(
      '#type' => 'button',
      '#attributes' => array(
        'class' => array('brightcove-field-create-button'),
        'rel' => $element['#field_name'] . '-' . $delta,
        'data-entity-type' => $entity_type,
        'data-bundle' => $element['#bundle'],
        'data-field-name' => $element['#field_name'],
        'data-entity-id' => is_null($eid) ? '0' : $eid,
      ),
      '#limit_validation_errors' => array(),
      '#default_value' => t('Create'),
      '#value_callback' => 'brightcove_field_button_value_callback',
      '#ajax' => array(
        'callback' => 'ajax_create_playlist_dialog_callback',
      ),
      '#name' => $element['#field_name'] . '-' . $delta,
    );
  }

  $element['actions']['remove'] = array(
    '#type' => 'button',
    '#attributes' => array(
      'class' => array('brightcove-field-remove-button'),
      'rel' => $element['#field_name'] . '-' . $delta,
      'data-entity-type' => $entity_type,
      'data-field-name' => $element['#field_name'],
      'data-entity-id' => is_null($eid) ? '0' : $eid,
    ),
    '#default_value' => t('Remove'),
    '#value_callback' => 'brightcove_field_button_value_callback',
    '#name' => $element['#field_name'] . '-' . $delta,
  );

  $element['player'] = array(
    '#type' => $instance['settings']['per_content_player'] ?
      'select' : 'value',
    '#title' => t('Player'),
    '#options' => brightcove_player_list(),
    '#default_value' => isset($items[$delta]['player']) ?
      $items[$delta]['player'] : NULL,
  );

  if (!isset($element['#default_value'])) {
    $element['actions']['remove']['#attributes']['disabled'] = 'disabled';
  }

  if (empty($brightcove_field_settings[$element['#field_name']])) {
    $brightcove_field_settings[$element['#field_name']] = array(
      'brightcove_field' => array($element['#field_name'] => array(
        'entity_type' => $entity_type,
        'field_name' => $element['#field_name'],
        'entity_id' => is_null($eid) ? '0' : $eid,
      )),
    );
  }

  $element['brightcove_id']['#attached']['library'] = array(
    array('system', 'ui.dialog')
  );

  $element['brightcove_id']['#attached']['css'] = array(
    drupal_get_path('module', 'brightcove_field') . '/styles/brightcove.css',
  );

  $element['brightcove_id']['#attached']['js'] = array(
    drupal_get_path('module', 'brightcove_field') . '/js/brightcove.js',
  );

  $element['brightcove_id']['#attached']['js'][] = array(
    'data' => $brightcove_field_settings[$element['#field_name']],
    'type' => 'setting',
  );
}

function _brightcove_field_playlist_browse() {
  global $pager_from_array, $pager_total, $pager_page_array;
  $items_per_page = 20;
  $page = isset($_GET['page']) ? intval($_GET['page']) : '0';

  // Issue a BC request to list videos
  // For all videos, get a thumbnail URL
  // Theme a table with videos and a button to attach.
  $bc = brightcove_initialize();

  $params = array();
  $method = 'find_all_playlists';
  $result = NULL;

  try {
    $params['page_size'] = $items_per_page;
    $params['page_number'] = $page;

    $result = $bc->find($method, $params);
  } catch (Exception $error) {
    watchdog('brightcove', 'Finding playlists in browse failed.', array(), WATCHDOG_ERROR);
  }

  $pager_page_array = explode(',', $page);
  $pager_total[0] = ceil($bc->total_count / $items_per_page);
  $pager_page_array[0] =
    max(0, min(
      (int)$pager_page_array[0],
      ((int)$pager_total[0]) - 1)
    );

  $items = array();
  if (count($result)) {
    foreach ($result as $playlist) {
      $item = array();
      $item['title'] = check_plain($playlist->name);
      $item['brightcove_id'] = $playlist->id;
      if (!empty($playlist->thumbnailURL)) {
          $image_vars = array('path' => $playlist->thumbnailURL, 'alt' => '', 'title' => '', 'attributes' => '');
          $item['thumbnail'] = theme('image', $image_vars);
      }
      else {
        $image_vars = array('path' => brightcove_get_default_image(), 'alt' => '', 'title' => '', 'attributes' => '', 'width' => '120', 'height' => '');
        $item['thumbnail'] = theme('image', $image_vars);
      }
      $items[] = $item;
    }
  }

  $pager_vars = array(
    'tags' => NULL,
    'element' => 0,
    'parameters' => array(),
    'quantity' => $items_per_page,
  );
  // No possibility to search any parameter of the playlist.(Only ID).
  $content['filter_form'] = '';
  $content['items'] = $items;
  $content['pager'] = theme_pager($pager_vars);

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
  $result = array();
  if (count($playlists)) {
    foreach ($playlists as $playlist) {
      if (strpos($playlist->name, $string) !== FALSE) {
        $result[] = $playlist;
      }
    }
  }
  return $result;
}