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
function brightcove_admin_settings($form, &$form_state) {
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

  $num_customfields = empty($form_state['brightcove_custom_fields']) ?
    $form_state['brightcove_custom_fields'] =
      variable_get('brightcove_custom_fields', 0) :
    $form_state['brightcove_custom_fields'];

  $form['upload_customfields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Custom fields'),
    '#collapsible' => TRUE,
    '#collapsed' => !((bool) $num_customfields),
    '#prefix' => '<div id="brightcove-admin-customfield">',
    '#suffix' => '</div>',
  );

  $form['upload_customfields']['brightcove_custom_fields'] = array(
    '#type' => 'value',
    '#value' => $num_customfields,
  );

  for ($i = 0; $i < $num_customfields; ++$i) {
    $form['upload_customfields'][] = array(
      '#type' => 'fieldset',
      "brightcove_custom_fields_{$i}_key" => array(
        '#type' => 'textfield',
        '#title' => t('Field key'),
        '#default_value' => variable_get("brightcove_custom_fields_{$i}_key"),
      ),
      "brightcove_custom_fields_{$i}_label" => array(
        '#type' => 'textfield',
        '#title' => t('Label'),
        '#default_value' => variable_get("brightcove_custom_fields_{$i}_label"),
      ),
      "brightcove_custom_fields_{$i}_type" => array(
        '#type' => 'radios',
        '#title' => t('Type'),
        '#default_value' =>
          variable_get("brightcove_custom_fields_{$i}_type", 'text'),
        '#options' => array(
          'text' => t('Text'),
          'list' => t('List'),
        ),
      ),
      "brightcove_custom_fields_{$i}_values" => array(
        '#type' => 'textarea',
        '#title' => t('Values'),
        '#description' => t('Put each values into a new line.'),
        '#default_value' =>
          variable_get("brightcove_custom_fields_{$i}_values"),
        '#states' => array(
          'visible' => array(
            ":input[name=brightcove_custom_fields_{$i}_type]" => array(
              'value' => 'list',
            ),
          ),
        ),
      ),
      "brightcove_custom_fields_{$i}_required" => array(
        '#type' => 'checkbox',
        '#title' => t('Required'),
        '#default_value' =>
          variable_get("brightcove_custom_fields_{$i}_required"),
      ),
    );
  }

  $form['upload_customfields']['add'] = array(
    '#type' => 'submit',
    '#value' => t('Add more'),
    '#submit' => array(
      'brightcove_admin_add_customfield_submit',
    ),
    '#ajax' => array(
      'callback' => 'brightcove_admin_customfield_ajax_callback',
      'wrapper' => 'brightcove-admin-customfield'
    ),
  );

  if ($num_customfields > 0) {
    $form['upload_customfields']['remove'] = array(
      '#type' => 'submit',
      '#value' => t('Remove last'),
      '#submit' => array(
        'brightcove_admin_remove_customfield_submit',
      ),
      '#ajax' => array(
        'callback' => 'brightcove_admin_customfield_ajax_callback',
        'wrapper' => 'brightcove-admin-customfield',
      ),
    );
  }

  $form = system_settings_form($form);
  $form['#validate'][] = 'brightcove_admin_settings_validate';

  return $form;
}

function brightcove_admin_customfield_ajax_callback($form) {
  return $form['upload_customfields'];
}

function brightcove_admin_add_customfield_submit($form, &$form_state) {
  if ($form_state['brightcove_custom_fields'] < 10) {
    $form_state['brightcove_custom_fields']++;
  }
  $form_state['rebuild'] = TRUE;
}

function brightcove_admin_remove_customfield_submit($form, &$form_state) {
  if ($form_state['brightcove_custom_fields'] > 0) {
    $form_state['brightcove_custom_fields']--;
  }
  $form_state['rebuild'] = TRUE;
}

function brightcove_admin_settings_validate($form, &$form_state) {
  list($can_read, $can_write) = brightcove_verify_tokens(
    $form_state['values']['brightcove_read_api_key'],
    $form_state['values']['brightcove_write_api_key']
  );

  if (!$can_read) {
    form_set_error('brightcove_read_api_key', t('Invalid read key'));
  }
  if ($form_state['values']['brightcove_write_api_key'] && !$can_write) {
    form_set_error('brightcove_write_api_key', t('Invalid write key'));
  }
}

function brightcove_admin_players() {
  $players = brightcove_player_load_all();

  $header = array(
    t('Name'),
    t('Player ID'),
    t('Player Key'),
    array(
      'data' => t('Operations'),
      'colspan' => 3,
    ),
  );

  $rows = array();
  foreach ($players as $player) {
    $default = _brightcove_player_is_default($player);
    $rows[] = array(
      check_plain($player->name),
      check_plain($player->player_id),
      check_plain($player->player_key),
      $default ?
        t('Default') : l(t('Set default'), 'admin/config/media/brightcove/players/' . $player->pid . '/setdefault'),
      l(t('Edit'), 'admin/config/media/brightcove/players/' . $player->pid . '/edit'),
      $default ? '' : l(t('Delete'), 'admin/config/media/brightcove/players/' . $player->pid .  '/delete'),
    );
  }

  return array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );
}

function _brightcove_player_is_default($player) {
  $ret = 1;
  $ret &= (int) (variable_get('brightcove_player_pid') == $player->pid);

  return (bool) $ret;
}

function brightcove_player_editor_form($form, &$form_state, $player = NULL) {
  if (!$player) {
    $player = new stdClass();
  }
  else {
    $form['pid'] = array(
      '#type' => 'hidden',
      '#value' => $player->pid,
    );
  }

  $titles = array(
    'name' => 'Name',
    'player_id' => 'Player ID',
    'player_key' => 'Player Key',
  );

  $required = array(
    'name' => TRUE,
    'player_id' => TRUE,
    'player_key' => FALSE,
  );

  foreach (array('name', 'player_id', 'player_key') as $item) {
    $form[$item] = array(
      '#title' => check_plain(t($titles[$item])),
      '#type' => 'textfield',
      '#default_value' => isset($player->$item) ?
        $player->$item : NULL,
      '#required' => $required[$item],
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );

  return $form;
}

function brightcove_player_editor_form_submit($form, &$form_state) {
  brightcove_player_save((object) $form_state['values']);
  $form_state['redirect'] = 'admin/config/media/brightcove/players';
}

function brightcove_player_delete_form($form, &$form_state, $player) {
  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $player->pid,
  );

  return confirm_form($form,
    t('Are you sure that you want to delete "@name" player?', array(
        '@name' => $player->name,
      )),
    'admin/config/media/brightcove/players');
}

function brightcove_player_delete_form_submit($form, &$form_state) {
  brightcove_player_delete($form_state['values']['pid']);
  $form_state['redirect'] = 'admin/config/media/brightcove/players';
}

function brightcove_player_setdefault_form($form, &$form_state, $player) {
  $form['pid'] = array(
    '#type' => 'value',
    '#value' => $player->pid,
  );

  return confirm_form($form,
    t('Are you sure that you want to set "@name" player default?', array(
        '@name' => $player->name,
      )),
    'admin/config/media/brightcove/players');
}

function brightcove_player_setdefault_form_submit($form, &$form_state) {
  variable_set('brightcove_player_pid', $form_state['values']['pid']);
  $form_state['redirect'] = 'admin/config/media/brightcove/players';
}
