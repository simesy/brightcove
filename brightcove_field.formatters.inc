<?php

/**
 * @file
 * Formatters for video field.
 */

/**
 * Theme callback for Brightcove field formatter.
 * Returns an embedded player with default site player.
 *
 * @param $element
 *   Element with the Video ID.
 * @return
 *   Player HTML code.
 */
function theme_brightcove_field_formatter_default($variables) {
  $output = NULL;

  if (isset($variables['element']['brightcove_id'])) {
    $vidcount = &drupal_static(__FUNCTION__, 1);
    if ($vidcount == 1) {
      $path = brightcove_get_experiences_js_url();
      drupal_add_js($path, ['type' => 'external', 'defer' => FALSE, 'async' => TRUE]);
    }
    $client = brightcove_client_load($variables['element']['bcid']);
    $output = theme('brightcove_field_embed', [
      'type' => $variables['type'],
      'video_id' => $variables['element']['brightcove_id'],
      'player_id' => $variables['element']['player'],
      'account_id' => $client->account_id,
      'width' => $variables['width'],
      'height' => $variables['height'],
    ]);

    $vidcount++;
  }

  return $output;
}

/**
 * Theme function to render brightcove images.
 *
 * @param $variables
 * @return null|string
 */
function theme_brightcove_field_image($variables) {
  $output = NULL;
  $elements = $variables['elements'];
  $dialog_width = isset($elements['#width']) ? $elements['#width'] : BRIGHTCOVE_DEFAULT_VIDEO_WIDTH;
  $dialog_height = isset($elements['#height']) ? $elements['#height'] : BRIGHTCOVE_DEFAULT_VIDEO_HEIGHT;

  $entity_type = (isset($elements['#entity_type'])) ? $elements['#entity_type'] : NULL;
  $delta = (isset($elements['#delta'])) ? $elements['#delta'] : NULL;
  $field = (isset($elements['#field'])) ? $elements['#field'] : NULL;
  $entity = (isset($elements['#entity'])) ? $elements['#entity'] : NULL;
  $entity_info = (isset($elements['#entity_type'])) ? entity_get_info($entity_type) : NULL;
  $entity_id = is_null($entity) ? NULL : $entity->{$entity_info['entity keys']['id']};

  $type = ($elements['#brightcove_widget_type'] == BRIGHTCOVE_VIDEO_WIDGET) ? 'video' : 'playlist';

  if ($elements["#{$type}"]) {
    if (empty($elements["#{$type}"]->{$elements['#brightcove_image_type']})) {
      $path = brightcove_get_default_image();
      $styled_path = image_style_path($elements['#brightcove_image_style'], 'default-image.png');
      $style = image_style_load($elements['#brightcove_image_style']);
      image_style_create_derivative($style, $path, $styled_path);
      $image = theme('image', ['path' => $styled_path]);
    }
    else {
      $remote_file = brightcove_remote_image($elements["#{$type}"]->{$elements['#brightcove_image_type']});
      if ($elements['#brightcove_image_style']){
        $image = theme('image_style', [
          'style_name' => $elements['#brightcove_image_style'],
          'path' => $remote_file,
        ]);
      }else{
        $image = theme('image', ['path' => $remote_file]);
      }
    }

    if (($elements['#brightcove_image_link'] == 'none') || (empty($elements['#brightcove_image_link']))) {
      return $image;
    }

    elseif ($elements['#brightcove_image_link'] == 'dialog') {
      $destination = "brightcove_dialog/nojs/{$type}/{$dialog_width}/{$dialog_height}/{$entity_type}/{$entity_id}/{$field['field_name']}/{$delta}";
      $output = l($image, $destination, [
        'attributes' => [
          'rel' => $field['field_name'],
          'class' => [$field['field_name'], 'use-ajax'],
          'title' => $elements["#{$type}"] ? check_plain(!empty($elements["#{$type}"]) ? $elements["#{$type}"]->getName() : '') : '',
        ],
        'html' => TRUE,
      ]);
    }

    elseif ($elements['#brightcove_image_link'] == 'entity' && !is_null($entity_info)) {
      $uri = $entity_info['uri callback']($entity);
      $output = l($image, $uri['path'], [
        'attributes' => [
          'class' => [$field['field_name']],
          'title' => $elements["#{$type}"] ? check_plain(!empty($elements["#{$type}"]) ? $elements["#{$type}"]->getName() : '') : '',
        ],
        'html' => TRUE,
      ]);
    }
  }
  return $output;
}

/**
 * Theme function to render brightcove metadata.
 *
 * @param $variables
 * @return null|string
 */
function theme_brightcove_field_metadata($variables) {
  $output = NULL;
  $elements = $variables['elements'];
  $type = ($elements['#brightcove_widget_type'] == BRIGHTCOVE_VIDEO_WIDGET) ? 'video' : 'playlist';

  if ($elements["#{$type}"]) {
    $val = $elements["#{$type}"]->{'get' . ucfirst($elements['#key'])}();
    if (is_array($val)) {
      $rows = '';
      foreach ($val as $k => $v) {
        $kp = check_plain($k);
        $vp = check_plain($v);
        $rows .= "<tr><td><strong>{$kp}</strong></td><td>{$vp}</td></tr>";
      }

      $output = "<table><th colspan=\"2\">{$elements['#label']}</th>{$rows}</table>";
    }
    else {
      $text = check_plain($val);
      $output = "<p><strong>{$elements['#label']}:</strong> {$text}</p>";
    }
  }

  return $output;
}
