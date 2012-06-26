<?php
/**
 * @file
 * Theme funciton for the module.
 */

/**
 * Theme callback for a Brightcove browse button.
 * Currently it's just a thin wrapper around the theme_button()
 * function which only returns a button of type submit. The themed
 * representation is just adapted to achieve an input html tag
 * of the type button.
 *
 * @see nodereference_explorer.module
 */
function theme_brightcove_field_browse_button($element) {
  drupal_add_js(drupal_get_path('module', 'brightcove_field') . '/js/brightcove.js');
  $element['#button_type'] = 'button';
  // TODO: review after the field part is stable
  $button = theme('button', array('element' => $element));
  return str_ireplace('type="submit"', 'type="button"', $button);
}

/**
 * Theme callback for Brightcove browse table item.
 *
 * @param item
 *   Video item.
 *
 * @return
 *   Themed item form.
 */
function theme_brightcove_field_browse_item($variables) {
  $form = drupal_get_form('brightcove_field_browser_form' . $variables['item']['brightcove_id'], $variables['item']);
  return drupal_render($form);
}

/**
 * Theme callback for Brightcove browse table.
 *
 * @param $variables
 *   Array of video items.
 *
 * @return
 *   Themed browse table.
 */
function theme_brightcove_field_browse_items($variables) {
  $rowcount = 0;
  $activerow = 0;
  $rows = array();
  foreach ($variables['items'] as $item) {
    // TODO test these after field port is stable
    $themed = theme('brightcove_field_browse_item', array('item' => $item));
    $rowcount++;
    if ($rowcount == 4) {
      $activerow++;
      $rowcount = 1;
    }
    $rows[$activerow][] = $themed;
  }
  // TODO test these after field port is stable
  return theme('table', array('header' => array(), 'rows' => $rows));
}

/**
 * The brightcove_field_embed theme function preprocess function.
 *
 * @param $variables
 */
function brightcove_field_preprocess_brightcove_field_embed(&$variables) {
  if (!isset($variables['player'])) {
    watchdog('brightcove', 'Brightcove player is missing.', array(), WATCHDOG_ERROR);
  }

  $flashvars = array();
  $player = brightcove_player_load($variables['player']);

  $vars['id'] = 'myExperience';
  $vars['bgcolor'] = '#FFFFFF';
  $vars['width'] = isset($variables['width']) ? $variables['width'] : BRIGHTCOVE_DEFAULT_VIDEO_WIDTH;
  $vars['height'] = isset($variables['height']) ? $variables['height'] : BRIGHTCOVE_DEFAULT_VIDEO_HEIGHT;

  foreach ($vars as $key => $value) {
    $variables[$key] = isset($variables['params'][$key]) ? $variables['params'][$key] : $vars[$key];
  }
  if (isset($variables['brightcove_id'])) {
    $flashvars['domain'] = 'embed';
    $flashvars['dynamicStreaming'] = 'true';
    $flashvars['playerID'] = isset($player->player_id) ? $player->player_id : '';
    $flashvars['playerKey'] = isset($player->player_key) ? $player->player_key : '';

    if (drupal_strtolower($variables['type']) == 'brightcove') {
      $flashvars['@videoPlayer'] = $variables['brightcove_id'];
    }
    elseif (drupal_strtolower($variables['type']) == 'brightcove-player') {
      $flashvars['@playlistTabs'] = $variables['brightcove_id'];
      $flashvars['@playlistCombo'] = $variables['brightcove_id'];
    }
  }

  array_walk($flashvars, create_function('&$item, $key', '$item = urlencode($key) . "=" . urlencode($item);'));
  $variables['flashvars'] = join($flashvars, '&');
}
