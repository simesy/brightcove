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
    $plugin['menu']['items']['set-default'] = array(
      'path' => 'list/%ctools_export_ui/set-default',
      'title' => 'Set default',
      'page callback' => 'ctools_export_ui_switcher_page',
      'page arguments' => array($plugin['name'], 'set_default', $prefix_count + 2),
      'load arguments' => array($plugin['name']),
      'access callback' => 'ctools_export_ui_task_access',
      'access arguments' => array($plugin['name'], 'set_default', $prefix_count + 2),
      'type' => MENU_CALLBACK,
    );

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
  }

  /**
   * Edit for for Brightcove Player preset.
   */
  function edit_form(&$form, &$form_state) {
    parent::edit_form($form, $form_state);
    unset($form['info']);

    $form['display_name'] = array(
      '#title' => t('Name'),
      '#description' => t('Example: My Player') . ' (' . t('Do not begin name with numbers.') . ')',
      '#type' => 'textfield',
      '#default_value' => !empty($form_state['item']->display_name) ? $form_state['item']->display_name : '',
      '#required' => TRUE,
    );

    $form['name'] = array(
      '#type' => 'machine_name',
      '#title' => t('Machine-readable name'),
      '#description' => t('Example: my_player') . '<br/>' . t('May only contain lowercase letters, numbers and underscores. <strong>Try to avoid conflicts with the names of existing Drupal projects.</strong>'),
      '#required' => TRUE,
      '#default_value' => !empty($form_state['item']->name) ? $form_state['item']->name : '',
      '#disabled' => !empty($form_state['item']->name) ? TRUE : FALSE,
      '#machine_name' => array(
        'exists' => 'brightcove_player_form_validate_field',
        'source' => array('display_name'),
      ),
    );

    $form['player_id'] = array(
      '#title' => t('Player ID'),
      '#type' => 'textfield',
      '#default_value' => isset($form_state['item']->player_id) ? $form_state['item']->player_id : '',
      '#required' => TRUE,
    );

    $form['player_key'] = array(
      '#title' => t('Player Key'),
      '#type' => 'textfield',
      '#default_value' => isset($form_state['item']->player_key) ? $form_state['item']->player_key : '',
      '#required' => FALSE,
    );
  }

  /**
   * Page callback for "set default" op.
   */
  function set_default_page($plugin_name, $op, $player) {
    module_load_include('inc', 'brightcove', 'brightcove.admin');
    return drupal_get_form('brightcove_player_setdefault_form', $player);
  }
}
