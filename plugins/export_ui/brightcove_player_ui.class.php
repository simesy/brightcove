<?php

/**
 * @file
 * Class brightcove_player_ui
 */

class brightcove_player_ui extends ctools_export_ui {

  /**
   * Fake constructor.
   * @see ctools_export_ui::init()
   */
  function init($plugin) {
    // Adding a menu op for setting default player.
    $prefix_count = count(explode('/', $plugin['menu']['menu prefix']));
    $plugin['menu']['items']['set-default'] = [
      'path' => 'list/%ctools_export_ui/set-default',
      'title' => 'Set default',
      'page callback' => 'ctools_export_ui_switcher_page',
      'page arguments' => [$plugin['name'], 'set_default', $prefix_count + 2],
      'load arguments' => [$plugin['name']],
      'access callback' => 'ctools_export_ui_task_access',
      'access arguments' => [$plugin['name'], 'set_default', $prefix_count + 2],
      'type' => MENU_CALLBACK,
    ];

    $plugin['menu']['items']['list callback']['type'] = MENU_LOCAL_TASK;

    parent::init($plugin);

    module_load_include('inc', 'brightcove', 'brightcove.admin');
  }

  /**
   * Enhancing the list form.
   */
  function list_build_row($item, &$form_state, $operations) {
    $op = $operations['set-default'];
    unset($operations['set-default']);
    $operations['set-default'] = $op;

    parent::list_build_row($item, $form_state, $operations);

    $name = $item->{$this->plugin['export']['key']};

    $this->rows[$name]['data'][0]['data'] = (empty($item->display_name)) ? $item->name : $item->display_name ;

    if (_brightcove_player_is_default($item)) {
      $this->rows[$name]['data'][0]['data'] .= ' ' . t('(Default)');
    }

    if (!empty($item->responsive)) {
      $this->rows[$name]['data'][0]['data'] .= ' ' . t('(Responsive)');
    }
  }

  /**
   * Edit for for Brightcove Player preset.
   */
  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);
    unset($form['info']);

    $form['display_name'] = [
      '#title' => t('Name'),
      '#description' => t('Example: My Player') . ' (' . t('Do not begin name with numbers.') . ')',
      '#type' => 'textfield',
      '#default_value' => !empty($form_state['item']->display_name) ? $form_state['item']->display_name : '',
      '#required' => TRUE,
    ];

    $form['name'] = [
      '#type' => 'machine_name',
      '#title' => t('Machine-readable name'),
      '#description' => t('Example: my_player') . '<br/>' . t('May only contain lowercase letters, numbers and underscores. <strong>Try to avoid conflicts with the names of existing Drupal projects.</strong>'),
      '#required' => TRUE,
      '#default_value' => !empty($form_state['item']->name) ? $form_state['item']->name : '',
      '#disabled' => !empty($form_state['item']->name) ? TRUE : FALSE,
      '#machine_name' => [
        'exists' => 'brightcove_player_form_validate_field',
        'source' => ['display_name'],
      ],
    ];

    $form['player_id'] = [
      '#title' => t('Player ID'),
      '#type' => 'textfield',
      '#default_value' => isset($form_state['item']->player_id) ? $form_state['item']->player_id : '',
      '#required' => TRUE,
    ];

    $form['player_key'] = [
      '#title' => t('Player Key'),
      '#type' => 'textfield',
      '#default_value' => isset($form_state['item']->player_key) ? $form_state['item']->player_key : '',
      '#required' => FALSE,
    ];

    $form['responsive'] = [
      '#title' => t('Responsive'),
      '#type' => 'checkbox',
      '#default_value' => isset($form_state['item']->responsive) ? $form_state['item']->responsive : 0,
      '#required' => FALSE,
      '#description' => t('Make the player responsive. Please note that the player will use a different template in this case. This setting can be overwritten by the global player setting.')
    ];
  }

  /**
   * Page callback for "set default" op.
   */
  function set_default_page($plugin_name, $op, $player) {
    module_load_include('inc', 'brightcove', 'brightcove.admin');
    return drupal_get_form('brightcove_player_setdefault_form', $player);
  }

  /**
   * Submit callback.
   */
  function edit_form_submit(&$form, &$form_state) {
    parent::edit_form_submit($form, $form_state);

    // Check if a default player is already set and if not set the currently
    // submitted as default.
    if (variable_get('brightcove_player_default', NULL) == NULL) {
      variable_set('brightcove_player_default', $form_state['values']['name']);
    }
  }

  /**
   * Delete page submit callback.
   */
  function delete_form_submit(&$form_state) {
    parent::delete_form_submit($form_state);

    // Change the default player to a newly selected one.
    if (isset($form_state['values']['default_player_delete_confirm'])) {
      variable_set('brightcove_player_default', $form_state['values']['default_player_delete_confirm']);
    }
  }
}
