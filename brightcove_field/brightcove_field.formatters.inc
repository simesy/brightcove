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

  if (isset($variables['element']['video_id'])) {
    $vidcount = &drupal_static(__FUNCTION__, 1);
    if ($vidcount == 1) {
      drupal_add_js('http://admin.brightcove.com/js/BrightcoveExperiences.js');
    }

    $params['id'] = 'myExperience' . $vidcount;
    $player = theme('brightcove_field_embed', array(
      'type' => 'video',
      'video_id' => $variables['element']['video_id'],
      'params' => $params,
      'player' => brightcove_field_get_value($variables['instance'], $variables['element']['player']),
      'width' => $variables['width'],
      'height' => $variables['height'],
    ));

    $vidcount++;

    $output = theme('brightcove_field_player', array(
      'player' => $player,
      'video_id' => $variables['element']['video_id'],
    ));
  }

  return $output;
}

/**
 * Theme function to render brightcove images.
 *
 * @param $variables
 */
function theme_brightcove_field_image($variables) {
  $output = NULL;
  $elements = $variables['elements'];
  $dialog_width = BRIGHTCOVE_DEFAULT_VIDEO_WIDTH;
  $dialog_height = BRIGHTCOVE_DEFAULT_VIDEO_HEIGHT;
  $entity_type = (isset($elements['#entity_type'])) ? $elements['#entity_type'] : NULL;
  $delta = (isset($elements['#delta'])) ? $elements['#delta'] : NULL;
  $field = (isset($elements['#field'])) ? $elements['#field'] : NULL;
  $entity = (isset($elements['#entity'])) ? $elements['#entity'] : NULL;
  $entity_info = (isset($elements['#entity_type'])) ? entity_get_info($entity_type) : NULL;
  $entity_id = is_null($entity) ? NULL : $entity->{$entity_info['entity keys']['id']};

  if ($elements['#video']) {
    if (empty($elements['#video']->{$elements['#brightcove_image_type']})) {
      $path = brightcove_get_default_image();
      $styled_path = image_style_path($elements['#brightcove_image_style'], 'default-image.png');
      $style = image_style_load($elements['#brightcove_image_style']);
      image_style_create_derivative($style, $path, $styled_path);
      $image = theme('image', array('path' => $styled_path));
    }
    else {
      $remote_file = brightcove_remote_image($elements['#video']->{$elements['#brightcove_image_type']});
      $image = theme('image_style', array(
        'style_name' => $elements['#brightcove_image_style'],
        'path' => $remote_file,
      ));
    }

    if ($elements['#brightcove_image_link'] == 'none') {
      return $image;
    }

    if ($elements['#brightcove_image_link'] == 'dialog') {
      $destination = "brightcove_dialog/nojs/{$dialog_width}/{$dialog_height}/{$entity_type}/{$entity_id}/{$field['field_name']}/{$delta}";
      $output = l($image, $destination, array(
        'attributes' => array(
          'rel' => $field['field_name'],
          'class' => array($field['field_name'], 'use-ajax'),
          'title' => $elements['#video'] ? check_plain($elements['#video']->name) : '',
        ),
        'html' => TRUE,
      ));
    }

    if ($elements['#brightcove_image_link'] == 'entity' && !is_null($entity_info)) {
      $uri = $entity_info['uri callback']($entity);
      $output = l($image, $uri['path'], array(
        'attributes' => array(
          'class' => array($field['field_name']),
          'title' => $elements['#video'] ? check_plain($elements['#video']->name) : '',
        ),
        'html' => TRUE,
      ));
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

  if ($elements['#video']) {
    $text = check_plain($elements['#video']->{$elements['#key']});
    $output = "<span><strong>{$elements['#label']}:</strong> $text</span>";
  }

  return $output;
}
