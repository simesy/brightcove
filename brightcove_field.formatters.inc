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
    $client = brightcove_client_load_or_default($variables['element']['bcid']);
    $output = theme('brightcove_field_embed', [
      'type' => $variables['type'],
      'brightcove_id' => $variables['element']['brightcove_id'],
      'player_id' => $variables['element']['player'] ?: brightcove_get_default_player($client->bcid),
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
    $images = $type === 'video' ? $elements["#{$type}"]->getImages() : [];
    if (empty($images[$elements['#brightcove_image_type']])) {
      $path = brightcove_get_default_image();
      $image_variables = [
        'path' => $path,
      ];
      if (!empty($elements['#brightcove_image_style'])) {
        $styled_path = $elements['#brightcove_image_style'] ? image_style_path($elements['#brightcove_image_style'], $path) : $path;
        $style = image_style_load($elements['#brightcove_image_style']);
        image_style_create_derivative($style, $path, $styled_path);
        $image_variables['path'] = image_style_path($elements['#brightcove_image_style'], $path);
      }
      else {
        $image_variables['width'] = 222;
        $image_variables['height'] = 222;
      }
      $image = theme('image', $image_variables);
    }
    else {
      $cid = "brightcove:video:{$elements["#{$type}"]->getId()}:{$elements['#element']['bcid']}:{$elements['#brightcove_image_type']}";
      $cache = brightcove_cache_get($cid);
      if (!empty($cache)) {
        $remote_file = $cache;
      }
      else {
        $image_src = $images[$elements['#brightcove_image_type']]->getSrc();
        $parse = parse_url($image_src);
        $path = pathinfo($parse['path']);
        $remote_file = system_retrieve_file($image_src, file_build_uri('brightcove_thumbnail/' . $path['basename']), FALSE, FILE_EXISTS_REPLACE);
        if ($remote_file) {
          brightcove_cache_set($cid, $remote_file);
        }
      }

      if ($elements['#brightcove_image_style']) {
        $image = theme('image_style', [
          'style_name' => $elements['#brightcove_image_style'],
          'path' => $remote_file,
        ]);
      }
      else {
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
    if ($elements['#key'] === 'all') {
      $keys = _brightcove_field_get_object_formatter_keys($type);
      unset($keys['all']);
      $val = [];
      foreach ($keys as $key => $label) {
        $v = $elements["#{$type}"]->{'get' . ucfirst($key)}();
        if (is_array($v)) {
          if (array_values($v) === $v) {
            $val[$label] = implode(', ', $v); // this will go trough check_plain() later
          }
          else {
            $val += $v;
          }
        }
        else {
          $val[$label] = $v;
        }
      }
    }
    else {
      $val = $elements["#{$type}"]->{'get' . ucfirst($elements['#key'])}();
    }
    if (is_array($val)) {
      if (array_values($val) === $val) {
        $text = implode(', ', array_map('check_plain', $val));
        $output = "<p><strong>{$elements['#label']}</strong> {$text}</p>";
      }
      else {
        $rows = '';
        foreach ($val as $k => $v) {
          $kp = check_plain($k);
          $vp = $v instanceof \Brightcove\Object\Video\Link ? l(t($v->getText()), $v->getUrl()) : check_plain($v);
          $rows .= "<tr><td><strong>{$kp}</strong></td><td>{$vp}</td></tr>";
        }

        $output = "<table><th colspan=\"2\">{$elements['#label']}</th>{$rows}</table>";
      }
    }
    else {
      if ($val instanceof \Brightcove\Object\Video\Link) {
        $text = l(t($val->getText()), $val->getUrl());
      } else {
        $text = check_plain($val);
      }
      $output = "<p><strong>{$elements['#label']}:</strong> {$text}</p>";
    }
  }

  return $output;
}
