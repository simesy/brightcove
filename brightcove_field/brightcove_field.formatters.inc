<?php

/**
 * @file
 * Formatters for video field.
 */

/**
  * Theme callback for dialog with image player.
  */
function theme_brightcove_field_dialog_image($variables) {
  $element = $variables['element'];
  $style_data = str_replace('dialog_image_', '', $element['#formatter']);
  list($style, $field) = explode('__', $style_data);

  if (isset($element['#item']['video_id'])) {
    $video = brightcove_video_load($element['#item']['video_id']);
    $image = theme('image_style', array(
      'style_name' => $style,
      'path' => empty($video->thumbnailURL) ?
        brightcove_get_default_image() :
        brightcove_remote_image($video->{$field}),
    ));
    return theme("brightcove_field_formatter_dialog_player_{$field}", array(
      'element' => $element,
      'image' => $image,
      'style' => $style,
    ));
  }

  return '';
}

/**
 * Theme callback for Default thumbnail -> dialog player formatter.
 */
function theme_brightcove_field_dialog_player($variables) {
  $element = $variables['element'];
  $image = $variables['image'];
  $style = $variables['style'];
  $entity_type = $variables['entity_type'];
  $entity = $variables['entity'];
  $field = $variables['field'];
  $display = $variables['display'];
  $delta = $variables['delta'];

  $entity_info = $entity_type ? entity_get_info($entity_type) : NULL;

  if (!empty($element['video_id'])) {
    $formatter_name = str_replace('dialog_player_', '', $display['type']);
    $video = brightcove_video_load($element['video_id']);
    $video_name = $video ? check_plain($video->name) : '';

    if (!$image) {
      $image = theme('image', empty($video->{$formatter_name}) ?
        array(
          'path' => brightcove_get_default_image(),
          'alt' => $video_name,
          'title' => $video_name,
          'width' => 120,
          'height' => 120,
        ) :
        array(
          'path' => $video->{$formatter_name},
        )
      );
    }
    $entity_id = $entity && $entity_info ?
      $entity->{$entity_info['entity keys']['id']} : 0;

    // loading theme.inc in order to make sure that the constants are loaded
    module_load_include('inc', 'brightcove_field', 'theme');

    $video_width = BRIGHTCOVE_VIDEO_DEFAULT_WIDTH;
    $video_height = BRIGHTCOVE_VIDEO_DEFAULT_HEIGHT;
    $dialog_width = $video_width;
    $dialog_height = $video_height;

    $destination = "brightcove_dialog/nojs/{$dialog_width}/{$dialog_height}/{$entity_type}/{$entity_id}/{$field['field_name']}/{$delta}";

    $output = l($image, $destination,
      array(
        'attributes' => array(
          'rel' => $field['field_name'],
          'class' => array($field['field_name'], 'use-ajax'),
          'title' => $video ? check_plain($video->name) : '',
        ),
        'html' => TRUE,
      )
    );

    return theme('brightcove_field_dialog', array(
      'output' => $output,
      'video_id' => $element['video_id'],
      'video_width' => $video_width,
      'video_height' => $video_height,
      'dialog_width' => $dialog_width,
      'dialog_height' => $dialog_height,
      'destination' => $destination,
      'image_field' => $formatter_name,
      'field_name' => $field['field_name'],
      'entity_type' => $entity_type,
      'style' => $style,
    ));
  }

  return '';
}

/**
  * Template preprocess for brightcove_field_lightbox2 theme file.
  * Suggest templates based on a field and type name.
  */
function template_preprocess_brightcove_field_dialog(&$vars) {
//  $vars['template_files'] = array(
//    'brightcove-field-dialog--' . $vars['image_field'],
//    'brightcove-field-dialog--' . $vars['field_name'] . '--' .
//      $vars['image_field'],
//    'brightcove-field-dialog--' . $vars['type_name'] . '--' .
//      $vars['field_name'] . '--' . $vars['image_field'],
//  );
}

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
  $player = '';

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
        'player' =>
          brightcove_field_get_value(
            $variables['instance'], $variables['element']['player']),
        'width' => $variables['width'],
        'height' => $variables['height'],
    ));
    $vidcount++;
  }
  return $player ?
    theme('brightcove_field_player', array(
      'player' => $player,
      'video_id' => $variables['element']['video_id'],
    )) :
    '';
}


/**
 * Theme callback for Default thumbnail linked to entity formatter.
 */
function theme_brightcove_field_entity_link($variables) {
  $video_id = isset($variables['element']['video_id']) ?
    $variables['element']['video_id'] : NULL;
  $formatter_name = $variables['display']['type'];
  $entity_info = entity_get_info($variables['entity_type']);
  $entity = $variables['entity'];
  if (!empty($video_id)) {

    $field = str_replace('entity_link_', '', $formatter_name);

    $video = brightcove_video_load($video_id);

    $image = $variables['image'] ? $variables['image'] :
      theme('image', empty($video->{$field}) ?
        array(
          'path' => brightcove_get_default_image(),
          'width' => 120,
          'height' => 120,
        ) :
        array(
          'path' => $video->{$field},
        ));

    // TODO make some checks here
    $uri = $entity_info['uri callback']($entity);
    $destination = $uri['path'];

    $output = l($image, $destination, array(
      'attributes' => array(
        'class' => array($variables['field']['field_name']),
      ),
      'html' => TRUE,
    ));

    return theme('brightcove_field_entity_link', array(
      'output' => $output,
    ));
  }

  return '';
}

/**
  * Template preprocess for brightcove_field_entity_link theme file.
  * Suggest templates based on a field and type name.
  */
function template_preprocess_brightcove_field_entity_link(&$vars) {
//  $vars['template_files'] = array(
//      'brightcove-field-entity-link--' . $vars['image_field'],
//      'brightcove-field-entity-link--' . $vars['field_name'] . '--' . $vars['image_field'],
//      'brightcove-field-entity-link--' . $vars['type_name'] . '--' . $vars['field_name'] . '--' . $vars['image_field'],
//      );
}

/**
 * Theme callback for image linking to entity.
 */
function theme_brightcove_field_entity_link_dialog($variables) {
  $video_id = isset($variables['element']['video_id']) ?
    $variables['element']['video_id'] : NULL;
  list($style, $field) = explode('__',
    str_replace('entity_link_image_', '', $variables['display']['type']));

  if (isset($video_id)) {
    $video = brightcove_video_load($video_id);

    $image = theme('image_style', array(
      'style_name' => $style,
      'path' => empty($video->{$field}) ?
        brightcove_get_default_image() :
        brightcove_remote_image($video->{$field}),
    ));

    return theme("brightcove_field_formatter_entity_link_{$field}", array(
      'image' => $image,
      'style' => $style,
    ) + $variables);
  }

  return '';
}

/**
 * Theme callback for Default thumbnail.
 */
function theme_brightcove_field_entity_image($variables) {
  module_load_include('inc', 'brightcove_field', 'theme');
  $video_id = isset($variables['element']['video_id']) ?
    $variables['element']['video_id'] : NULL;
  $entity_info = entity_get_info($variables['entity_type']);
  if (!empty($video_id)) {

    $field = str_replace('entity_image_', '', $variables['display']['type']);

    $video = brightcove_video_load($video_id);

    $image = $variables['image'] ? $variables['image'] :
      theme('image', empty($video->{$field}) ?
          array(
            'path' => brightcove_get_default_image(),
            'width' => BRIGHTCOVE_DEFAULT_VIDEO_WIDTH,
            'height' => BRIGHTCOVE_DEFAULT_VIDEO_HEIGHT,
          ) :
          array(
            'path' => $video->{$field},
          )
      );

    $entity_id = isset($entity_info['entity keys']['id']) &&
      isset($variables['entity']->{$entity_info['entity keys']['id']}) ?
        $variables['entity']->{$entity_info['entity keys']['id']} : NULL;

    return theme('brightcove_field_entity_image', array(
      'output' => $image,
      'video_id' => $video_id,
      'entity_id' => $entity_id,
      'image_field' => $field,
      'field_name' => $variables['field']['field_name'],
      'entity_type' => $variables['entity_type'],
    ));
  }

  return '';
}

/**
  * Template preprocess for brightcove_field_node_image theme file.
  * Suggest templates based on a field and type name.
  */
function template_preprocess_brightcove_field_entity_image(&$vars) {
//  $vars['template_files'] = array(
//    'brightcove-field-entity-image--' . $vars['image_field'],
//    'brightcove-field-entity-image--' . $vars['field_name'] . '--' . $vars['image_field'],
//    'brightcove-field-entity-image--' . $vars['type_name'] . '--' . $vars['field_name'] . '--' . $vars['image_field'],
//  );
}

/**
 * Theme callback for imagecache image
 *
 * @param $element
 *   Element with the Video ID.
 * @return
 *   Link HTML code.
 */
function theme_brightcove_field_entity_image_style($variables) {
  $video_id = isset($variables['element']['video_id']) ?
    $variables['element']['video_id'] : NULL;
  list($style, $field) = explode('__',
    str_replace('entity_link_image_', '', $variables['display']['type']));

  if (isset($video_id)) {
    $video = brightcove_video_load($video_id);

    $image = theme('image_style', array(
      'style_name' => $style,
      'path' => empty($video->{$field}) ?
        brightcove_get_default_image() :
        brightcove_remote_image($video->{$field}),
    ));

    return theme("brightcove_field_formatter_entity_image_{$field}", array(
      'element' => $variables['element'],
      'image' => $image,
    ));
  }

  return '';
}

function theme_brightcove_field_formatter_metadata($variables) {
  if ($variables['video']) {
    $text = check_plain($variables['video']->{$variables['key']});
    return "<span><strong>{$variables['label']}:</strong> $text</span>";
  }

  return '';
}
