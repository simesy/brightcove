<?php

/**
 * Define this Export UI plugin.
 *
 * @see ctools_export_ui_process()
 */
$plugin = array(
  'schema' => 'brightcove_player',  // As defined in hook_schema().
  'access' =>  'administer brightcove settings',  // Define a permission users must have to access these pages.

  // Define the menu item.
  'menu' => array(
    'menu prefix' => 'admin/config/media/brightcove',
    'menu item' => 'players',
    'menu title' => 'Players',
  ),

  // Define user interface texts.
  'title singular' => t('player preset'),
  'title plural' => t('player presets'),
  'title singular proper' => t('Brightcove Player preset'),
  'title plural proper' => t('Brightcove Player presets'),

  // UI Handler class.
  'handler' => array(
    'class' => 'brightcove_player_ui',
    'parent' => 'ctools_export_ui',
  ),

  // Adding a "Set Default" op.
  'allowed operations' => array(
    'set-default' => array(
      'title' => t('Set default'),
    ),
  ),
);
