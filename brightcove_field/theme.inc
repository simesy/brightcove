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
  $form = drupal_get_form('brightcove_field_browser_form' . $variables['item']['video_id'], $variables['item']);
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

function theme_brightcove_field_embed($variables) {
  if (!isset($variables['player'])) {
    watchdog('brightcove', 'Video Player is missing.', array(), WATCHDOG_ERROR);
  }

  $player = brightcove_player_load($variables['player']);

  $values = array(
    'id' => 'myExperience',
    'bgcolor' => 'FFFFFF',
    'width' => $variables['width'],
    'height' => $variables['height'],
  );

  foreach ($values as $key => $value) {
    if (isset($variables['params'][$key])) {
      $values[$key] = $variables['params'][$key];
    }
  }

  $asset_code = '';

  if (isset($variables['video_id'])) {
    if (is_array($variables['video_id'])) {
      if (drupal_strtolower($variables['type']) == 'video') {
        $asset_code = '<param name="@videoPlayer" value="';
      }
      else {
        // TODO: Add different types than video.
      }

      foreach ($variables['video_id'] as $asset_td) {
        $asset_code .= $asset_td . ',';
      }

      $asset_code = drupal_substr($asset_code, 0, -1);
      $asset_code .= '" />';
    }
    else {
      if (drupal_strtolower($variables['type']) == 'video') {
        $asset_code = '<param name="@videoPlayer" value="' . $variables['video_id'] . '" />';
      }
      else {
        // TODO: Add different types than video.
      }
    }
  }

  $code = '
    <object id="' . $values['id'] . '" class="BrightcoveExperience">
    <param name="bgcolor" value="#' . $values['bgcolor'] . '" />
    <param name="width" value="' . $values['width'] . '" />
    <param name="height" value="' . $values['height'] . '" />
    <param name="playerID" value="' . $player->player_id . '" />'
    . $asset_code . '
    <param name="isVid" value="true" />
    <param name="isUI" value="true" />
    <param name="playerKey" value="' . $player->player_key . '" />
    </object>';

  return $code;
}
