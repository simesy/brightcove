<?php

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
  drupal_add_js(drupal_get_path('module', 'brightcove_field') . '/js/browse.js');
  $element['#button_type'] = 'button';
  // TODO: review after the field part is stable
  $button = theme('button', array('element' => $element));
  return str_ireplace('type="submit"', 'type="button"', $button);
}

/**
  * Theme callback for Brightcove browse table item.
  *
  * @param item
  * Video item.
  *
  * @return
  * Themed item form.
  */
function theme_brightcove_field_browse_item($item) {
  return drupal_get_form('brightcove_field_browser_form' . $item['video_id'], $item);
}

/**
  * Theme callback for Brightcove browse table.
  *
  * @param $items
  * Array of video items.
  *
  * @return
  * Themed browse table.
  */
function theme_brightcove_field_browse_items($items) {
  $rowcount = 0;
  $activerow = 0;
  $rows = array();
  foreach($items as $item) {
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

function theme_brightcove_field_embed($type, $playerId, $playerKey, $assetIds, $params) {
  if(!isset($playerId)) {
    watchdog('brightcove', 'Video Player ID is missing.', array(), WATCHDOG_ERROR);
  }

    if(!isset($playerKey)) {
    watchdog('brightcove', 'Video Player KEY is missing.', array(), WATCHDOG_ERROR);
  }

  $values = array('id' => 'myExperience', 'bgcolor' => 'FFFFFF', 'width' => 486, 'height' => 412);

  foreach($values as $key => $value) {
    if(isset($params[$key])) {
      $values[$key] = $params[$key];
    }
  }

  $assetCode = '';

  if(isset($assetIds)) {
    if(is_array($assetIds)) {
      if(strtolower($type) == 'video') {
        $assetCode = '<param name="@videoPlayer" value="';
      } else {
        // TODO: Add different types than video.
      }

      foreach($assetIds as $assetId) {
        $assetCode .= $assetId . ',';
      }

      $assetCode = substr($assetCode, 0, -1);
      $assetCode .= '" />';
    } else {
      if(strtolower($type) == 'video') {
        $assetCode = '<param name="@videoPlayer" value="' . $assetIds . '" />';
      } else {
        // TODO: Add different types than video.
      }
    }
  }

  $code = '
    <object id="' . $values['id'] . '" class="BrightcoveExperience">
    <param name="bgcolor" value="#' . $values['bgcolor'] . '" />
    <param name="width" value="' . $values['width'] . '" />
    <param name="height" value="' . $values['height'] . '" />
    <param name="playerID" value="' . $playerId . '" />'
    . $assetCode . '
    <param name="isVid" value="true" />
    <param name="isUI" value="true" />
    <param name="playerKey" value="' . $playerKey . '" />
    </object>';

  return $code;
}