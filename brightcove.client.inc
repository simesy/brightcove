<?php

/**
 * @file
 * Client related code.
 */

/**
 * Returns a form element to select a brightcove client.
 *
 * This function should be used in a hook_form_alter() whenever a client
 * select is needed on a form.
 */
function brightcove_client_select_element() {
  $client_options[BRIGHTCOVE_CLIENT_ID_NONE] = t('- Select a client -');
  $client_options +=  _brightcove_my_client_select_options();
  if (count($client_options) > 2) {
    $element = [
      '#type' => 'select',
      '#options' => $client_options,
      '#title' => t('Client'),
      '#default_value' => BRIGHTCOVE_CLIENT_ID_NONE,
    ];
  }
  elseif (count($client_options) == 2) {
    $client_id = array_keys($client_options)[0];
    $element = [
      '#type' => 'value',
      '#value' => $client_id,
      '#default_value' => $client_id,
    ];
  }
  else {
    $element = [
      '#markup' => t('To add playlists you need to have access to some brightcove clients first.'),
    ];
  }
  return $element;
}
