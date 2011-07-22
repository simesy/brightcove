<?php

/**
 * @file
 * Admin settings for Brightcove module.
 */

/**
 * Form builder.
 *
 * @ingroup forms
 * @see system_settings_form()
 */
function brightcove_admin_settings() {
  $form['account'] = array(
    '#type' => 'fieldset',
    '#title' => t('Account settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['account']['brightcove_read_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Brightcove Read API key'),
    '#maxlength' => 255,
    '#required' => TRUE,
    '#default_value' => variable_get('brightcove_read_api_key', ''),
    '#description' => t('A Brightcove Read API key from your account at Brightcove Studio. Please check at !link page.', array('!link' => l(t('Brightcove studio'), 'http://my.brightcove.com'))),
  );
  $form['account']['brightcove_write_api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Brightcove Write API key'),
    '#maxlength' => 255,
    '#default_value' => variable_get('brightcove_write_api_key', ''),
    '#description' => t('A Brightcove Write API key from your account at Brightcove Studio. Please check at !link page. <strong>Requires at least a Professional account</strong>.', array('!link' =>  l(t('Brightcove studio'), 'http://my.brightcove.com'))),
  );
  $form['account']['brightcove_user_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Brightcove Drupal User Custom Field'),
    '#default_value' => variable_get('brightcove_user_field', ''),
    '#description' => t('A Brightcove Custom Field to store the Drupal username of the user who uploaded a video - useful to determine which Drupal user uploaded a given video in BC Studio. This field must be created in BC Studio first. Read more about !link. <strong>Requires at least a Professional account</strong>', array('!link' =>  l(t('Brightcove custom metadata'), 'http://support.brightcove.com/en/docs/setting-custom-metadata'))),
  );
  $form['account']['brightcove_link_field'] = array(
    '#type' => 'textfield',
    '#title' => t('Brightcove Drupal Entity Link Custom Field'),
    '#default_value' => variable_get('brightcove_link_field', ''),
    '#description' => t('A Brightcove Custom Field to store the Drupal link of the entity where video belongs. This field must be created in BC Studio first. Read more about !link. <strong>Requires at least a Professional account</strong>', array('!link' =>  l(t('Brightcove custom metadata'), 'http://support.brightcove.com/en/docs/setting-custom-metadata'))),
  );
  $form['player'] = array(
    '#type' => 'fieldset',
    '#title' => t('Player settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $form['player']['brightcove_player'] = array(
    '#type' => 'textfield',
    '#title' => t('Brightcove Player ID'),
    '#required' => TRUE,
    '#default_value' => variable_get('brightcove_player', ''),
    '#description' => t('A Brightcove Player ID from BC Studio. Will be used to play all videos. Please check at !link.', array('!link' => l(t('Brightcove integration handbook on Drupal.org'), 'http://drupal.org/node/807598'))),
  );
  $form['player']['brightcove_player_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Brightcove Player KEY'),
    '#required' => TRUE,
    '#default_value' => variable_get('brightcove_player_key', ''),
    '#description' => t('A Brightcove Player KEY from BC Studio. Will be used to play all videos. Please check at !link.', array('!link' => l(t('Brightcove integration handbook on Drupal.org'), 'http://drupal.org/node/807598'))),
  );
  $form['player']['brightcove_default_image'] = array(
    '#type' => 'textfield',
    '#title' => t('Default image'),
    '#description' => t("In case a video doesn't have a thumbnail or still image, display an image from this path"),
    '#default_value' => variable_get('brightcove_default_thumbnail', brightcove_get_default_image()),
  );
  //see: http://support.brightcove.com/en/docs/creating-videos-multi-bitrate-streaming-media-api
  $form['upload'] = array(
    '#type' => 'fieldset',
    '#title' => t('Upload settings for non-FLV videos'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t("Note that these settings apply only to non-FLV videos (MP4, AVI, QT, etc.)."),
  );
  $form['upload']['renditions_open'] = array(
    '#type' => 'markup',
    '#value' => '<div class="container-inline">',
  );
  $form['upload']['brightcove_create_multiple_renditions'] = array(
    '#type' => 'select',
    '#default_value' => variable_get('brightcove_create_multiple_renditions', TRUE),
    '#options' => array(
      TRUE => t('multiple renditions'),
      FALSE => t('a single rendition'),
    ),
    '#prefix' => 'Automatically create ',
    '#suffix' => 'for uploading videos from Drupal.',
  );
  $form['upload']['renditions_close'] = array(
    '#type' => 'markup',
    '#value' => '</div>',
  );
  $form['upload']['brightcove_encode_to'] = array(
    '#type' => 'radios',
    '#title' => t('Output rendition encoding'),
    '#default_value' => variable_get('brightcove_encode_to', 'MP4'),
    '#options' => array(
      'MP4' => t('H.264 (MP4)'),
      'FLV' => t('VP6 (FLV)')
    ),
    '#description' => t(''),
  );
  $form['upload']['brightcove_preserve_source_rendition'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add H.264 source as a rendition.'),
    '#return_value' => 1,
    '#default_value' => variable_get('brightcove_preserve_source_rendition', 0),
    '#disabled' => !variable_get('brightcove_create_multiple_renditions', TRUE)
  );
  $form['status'] = array(
    '#type' => 'fieldset',
    '#title' => t('Status settings'),
    '#description' => t("Settings relating to the availability status of a video."),
    '#collapsible' => TRUE,
  );
  $form['status']['brightcove_check_for_unavailable'] = array(
    '#type' => 'checkbox',
    '#title' => t('Check for unavailable videos'),
    '#description' => t("If checked, then the message below will be displayed if a recently uploaded video is not yet available."),
    '#default_value' => variable_get('brightcove_check_for_unavailable', TRUE),
  );
  $form['status']['brightcove_status_display_unavailable'] = array(
    '#type' => 'textarea',
    '#title' => t('Unavailable video message'),
    '#description' => t("If the checkbox above is checked, and you have a message below (which may contain HTML), it will be displayed if a video is not yet available for viewing."),
    '#default_value' => variable_get('brightcove_status_display_unavailable', 'This video is unavailable for the moment.'),
  );


  return system_settings_form($form);
}
