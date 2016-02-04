<?php

/**
 * @file
 * Client related code.
 */

/**
 * Admin UI controller for 'brightcove_client' entity type.
 */
class BrightcoveClientEntityUIController extends EntityDefaultUIController {

  /**
   * @inheritdoc
   */
  public function hook_menu() {
    $items = parent::hook_menu();
    $items['admin/config/media/brightcove/client']['type'] = MENU_LOCAL_TASK;
    $items['admin/config/media/brightcove/client']['weight'] = 0.1;
    $items['admin/config/media/brightcove/client']['title'] = 'API Clients';
    $items['admin/config/media/brightcove/client/add']['title'] = 'Add API Client';
    $items['admin/config/media/brightcove/client/add']['title callback'] = 't';

    $items["{$this->path}/manage/%brightcove_client/setdefault"] = [
      'title' => 'Set client default',
      'page callback' => 'brightcove_client_set_default',
      'page arguments' => [6],
      'access callback' => 'brightcove_client_set_default_access',
      'access arguments' => [6],
      'file path' => drupal_get_path('module', 'brightcove'),
      'file' => 'brightcove.client.inc',
    ];
    return $items;
  }

  /**
   * @inheritdoc
   */
  public function overviewForm($form, &$form_state) {
    $form['#attached']['library'][] = ['system', 'drupal.ajax'];
    return parent::overviewForm($form, $form_state);
  }

  /**
   * @inheritdoc
   */
  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = []) {
    $wrapper_id = _brightcove_client_default_link_id($entity->bcid);
    $link = "<div id=\"$wrapper_id\">";
    $link .= _brightcove_client_default_link($entity);
    $link .= '</div>';
    $additional_cols[] = $link;
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

  /**
   * @inheritdoc
   */
  public function operationFormValidate($form, &$form_state) {
    parent::operationFormValidate($form, $form_state);
    if ($form_state['op'] === 'import') {
      $import = $form_state['values']['import'];
      $json = json_decode($import, TRUE);
      if ($json) {
        brightcove_try(function() use($json) {
          $client_id = isset($json['client_id']) ? $json['client_id'] : NULL;
          $client_secret = isset($json['client_secret']) ? $json['client_secret'] : NULL;
          \Brightcove\API\Client::authorize($client_id, $client_secret);
        }, function() {
          form_set_error('import', t("Invalid credentials."));
        });
      }
    }
  }

  /**
   * @inheritdoc
   */
  protected function overviewTableHeaders($conditions, $rows, $additional_header = []) {
    $header = $additional_header;
    $header[] = t('Default');

    return parent::overviewTableHeaders($conditions, $rows, $header);
  }
}

/**
 * Entity controller class for Brightcove client.
 */
class BrightcoveClientEntityController extends EntityAPIControllerExportable {

  /**
   * Overrides EntityAPIController::save()
   */
  public function save($client, DatabaseTransaction $transaction = NULL) {
    global $user;

    // Hardcode the changed time.
    $client->changed = REQUEST_TIME;

    if (empty($client->{$this->idKey}) || !empty($client->is_new)) {
      // Set the creation timestamp if not set, for new entities.
      if (empty($client->created)) {
        $client->created = REQUEST_TIME;
      }
    }
    else {
      // Otherwise if the client is not new but comes from an entity_create()
      // or similar function call that initializes the created timestamp and uid
      // value to empty strings, unset them to prevent destroying existing data
      // in those properties on update.
      if ($client->created === '') {
        unset($client->created);
      }
      if ($client->uid === '') {
        unset($client->uid);
      }
    }

    // Determine if we will be inserting a new client.
    $client->is_new = empty($client->bcid);

    $return = parent::save($client, $transaction);

    if (!(($default_client = variable_get('brightcove_client_default')) && (bool) brightcove_client_load($default_client))) {
      variable_set('brightcove_client_default', $client->bcid);
    }

    return $return;
  }

  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    parent::delete($ids, $transaction);

    $default_client = variable_get('brightcove_client_default');
    if (!in_array($default_client, $ids)) {
      $default_client = db_query('SELECT bcid FROM {brightcove_client} ORDER BY bcid ASC LIMIT 1')->fetchField();
      if ($default_client) {
        variable_set('brightcove_client_default', $default_client);
      }
    }
  }

  /**
   * Overridden.
   */
  public function export($entity, $prefix = '') {
    $vars = get_object_vars($entity);
    unset($vars[$this->statusKey], $vars[$this->moduleKey], $vars['is_new'], $vars['changed']);
    if ($this->nameKey != $this->idKey) {
      unset($vars[$this->idKey]);
    }
    return entity_var_json_export($vars, $prefix);
  }
}

/**
 * Returns a form element to select a brightcove client.
 *
 * This function should be used in a hook_form_alter() whenever a client
 * select is needed on a form.
 */
function brightcove_client_select_element() {
  $client_options = [BRIGHTCOVE_BCID_NONE => t('- Select a client -')];
  $client_options += _brightcove_my_client_select_options();
  if (count($client_options) > 2) {
    $default_value = variable_get('brightcove_client_default');
    if (!array_key_exists($default_value, $client_options)) {
      $default_value = BRIGHTCOVE_BCID_NONE;
    }
    $element = [
      '#type' => 'select',
      '#options' => $client_options,
      '#title' => t('Client'),
      '#default_value' => $default_value,
      '#disable_video_validation' => TRUE,
    ];
  }
  elseif (count($client_options) == 2) {
    $bcid = array_keys($client_options)[1];
    $element = [
      '#type' => 'value',
      '#value' => $bcid,
      '#default_value' => $bcid,
    ];
  }
  else {
    $element = [
      '#markup' => t('Sorry, it seems you don\'t have access to any brightcove clients. You can add clients <a href="@admin-page">here</a>.', [
        '@admin-page' => url('admin/config/media/brightcove/client/add'),
      ]),
      '#type' => 'markup',
    ];
  }
  return $element;
}

/**
 * Form callback: create or edit a brightcove client.
 *
 * @param $client
 *   The client object to edit or for a create form NULL.
 */
function brightcove_client_form($form, &$form_state, $client = NULL) {
  $form['label'] = [
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($client->label) ? $client->label : '',
    '#maxlength' => 128,
    '#required' => TRUE,
    '#weight' => -10,
  ];

  $form['account_id'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove Account id'),
    '#default_value' => isset($client->account_id) ? $client->account_id : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => -5,
  ];

  $form['client_id'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove API Client id'),
    '#default_value' => isset($client->client_id) ? $client->client_id : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => 0,
  ];

  $form['client_secret'] = [
    '#type' => 'textfield',
    '#title' => t('Brightcove API Client Secret Key'),
    '#default_value' => isset($client->client_secret) ? $client->client_secret : '',
    '#maxlength' => 255,
    '#required' => TRUE,
    '#weight' => 5,
  ];

  if (empty($client->is_new)) {
    $players = [];
    brightcove_try(function() use(&$players, $client) {
      foreach (brightcove_player_load_all($client->bcid) as $player) {
        $players[$player->getId()] = $player->getName();
      }
    });

    $form['default_player'] = [
      '#type' => 'select',
      '#required' => TRUE,
      '#default_value' => brightcove_get_default_player($client->bcid),
      '#title' => t('Default player'),
      '#options' => $players,
    ];
  }

  // When updating a client, do not collapse the Change History fieldset.
  $form['change_history'] = [
    '#type' => 'fieldset',
    '#title' => t('Change history'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($client->bcid),
    '#weight' => 350,
  ];
  $form['change_history']['log'] = [
    '#type' => 'textarea',
    '#title' => !empty($client->bcid) ? t('Update log message') : t('Creation log message'),
    '#rows' => 4,
    '#description' => t('Provide an explanation of the changes you are making. This will provide a meaningful history of changes to this client.'),
  ];

  $form['actions'] = [
    '#type' => 'actions',
    '#weight' => 400,
  ];

  // We add the form's #submit array to this button along with the actual submit
  // handler to preserve any submit handlers added by a form callback_wrapper.
  $submit = [];

  if (!empty($form['#submit'])) {
    $submit += $form['#submit'];
  }

  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => t('Save client'),
    '#submit' => array_merge($submit, ['brightcove_client_form_submit']),
  ];

  // We append the validate handler to #validate in case a form callback_wrapper
  // is used to add validate handlers earlier.
  $form['#validate'][] = 'brightcove_client_form_validate';

  return $form;
}

/**
 * Validation callback for brightcove_client_form().
 */
function brightcove_client_form_validate($form, &$form_state) {
  $client = $form_state['brightcove_client'];
  $bcid = empty($client->bcid) ? NULL : $client->bcid;
  $client_id = $form_state['values']['client_id'];
  $client_secret = $form_state['values']['client_secret'];

  brightcove_load_lib();
  $client = NULL;
  brightcove_try(function () use (&$form, &$client, $client_id, $client_secret) {
    $client = \Brightcove\API\Client::authorize($client_id, $client_secret);
    if (!$client->isAuthorized()) {
      form_error($form, t('Failed to authorize.'));
    }
  }, function (Exception $ex) use (&$form) {
    form_error($form, t('Failed to authorize.'));
  });

  // Ensure the proposed client id is unique or reused only during client
  // updates.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'brightcove_client')
    ->propertyCondition('client_id', $client_id);
  if (!empty($bcid)) {
    $query->entityCondition('entity_id', $bcid, '!=');
  }

  $result = $query->execute();
  if (!empty($result)) {
    $bcid = key($result['brightcove_client']);
    if (empty($client->bcid) || $bcid != $client->bcid) {
      form_set_error('client_id', t('This client id is <a href="!url">already in use</a> and must be unique. Please supply another value.', [
        '!url' => url('admin/config/media/brightcove/brightcove_client/manage/' . $bcid)
      ]));
    }
  }
  // Trim leading and trailing whitespace from the client_id.
  form_set_value($form['client_id'], trim($client_id), $form_state);
}

/**
 * Submit callback for brightcove_client_form().
 */
function brightcove_client_form_submit($form, &$form_state) {
  global $user;

  $client = &$form_state['brightcove_client'];

  $properties = [
    'label',
    'client_id',
    'client_secret',
    'account_id',
  ];
  foreach ($properties as $property) {
    $client->{$property} = $form_state['values'][$property];
  }

  // Set the client uid if it's being created at this time.
  if (empty($client->uid)) {
    $client->uid = $user->uid;
  }

  if (!empty($form_state['values']['default_player'])) {
    $client->data['default_player'] = $form_state['values']['default_player'];
  }

  // Trigger a new revision if the checkbox was enabled or a log message supplied.
  if (!empty($form_state['values']['revision']) || !empty($form['change_history']['revision']['#default_value']) ||
    !empty($form_state['values']['log'])) {
    $client->revision = TRUE;
    $client->log = $form_state['values']['log'];
  }

  // Save the client.
  brightcove_client_save($client);
  drupal_set_message(t('Client saved.'));
}

/**
 * Access callback.
 *
 * @param Entity $client
 */
function brightcove_client_set_default_access($client) {
  $value = _brightcove_client_default_link_id($client->bcid);
  if (drupal_valid_token($_GET['token'], $value) && brightcove_client_access('use', $client)) {
    return TRUE;
  }
  return FALSE;
}

/**
 * AJAX menu callback to set a client default.
 *
 * @param Entity $client
 * @param string $type
 *  Set internally, either 'nojs' or 'ajax'.
 */
function brightcove_client_set_default($client, $type = 'ajax') {
  $old_default_client_id = variable_get('brightcove_client_default');
  $old_default_client_entity = entity_load_single('brightcove_client', $old_default_client_id);
  variable_set('brightcove_client_default', $client->bcid);
  drupal_set_message(t('Client "@client" has been set to default.', ['@client' => $client->label]));
  if (!empty($old_default_client_id)) {
    drupal_set_message(t('The default client before was "@client".', ['@client' => $old_default_client_entity->label]));
  }
  if ($type == 'ajax') {
    $commands = [];
    $commands[] = ajax_command_html('#' . _brightcove_client_default_link_id($client->bcid), _brightcove_client_default_link($client));
    if (!empty($old_default_client_id)) {
      $commands[] = ajax_command_html('#' . _brightcove_client_default_link_id($old_default_client_id), _brightcove_client_default_link($old_default_client_entity));
    }
    $commands[] = ajax_command_prepend('#content', theme('status_messages'));
    ajax_deliver(['#type' => 'ajax', '#commands' => $commands]);
  }
  else {
    drupal_goto('/admin/config/media/brightcove/client');
  }

}

/**
 * The html id for the default client links on the client overcview page.
 *
 * @param string $id
 *  The client entity id.
 * @return string
 *  The HTML id.
 */
function _brightcove_client_default_link_id($id) {
  return 'bc-default-client-' . $id;
}

/**
 *  Generates the 'set default' AJAX link for clients.
 *
 * @param Entity $bcid
 *   The client entity.
 * @return string
 *   Either the link to set the client default or a text saying 'default'.
 */
function _brightcove_client_default_link(Entity $client) {
  if (variable_get('brightcove_client_default') !== $client->bcid) {
    $wrapper_id = _brightcove_client_default_link_id($client->bcid);
    $token = drupal_get_token($wrapper_id);
    $options = [
      'query' => [
        'token' => $token
      ],
      'attributes' => [
        'class' => [
          'use-ajax',
        ],
      ],
    ];
    return l(t('set default'), "admin/config/media/brightcove/client/manage/{$client->bcid}/setdefault", $options);
  }
  else {
    return t('default');
  }
}

/**
 * Access callback for brightcove_client entity.
 *
 * @param $op
 *   Can be 'edit' or 'use'.
 * @param Entity|string $entity
 *   Brightcove client entity or client ID.
 * @param $account
 *   Loaded user account.
 * @return bool
 */
function brightcove_client_access($op, $entity = NULL, $account = NULL) {
  // If we got ID try to load the entity.
  if (is_numeric($entity)) {
    $entity = brightcove_client_load($entity);
  }

  // If account is not set use the currently logged in user.
  if (is_null($account)) {
    global $user;
    $account = $user;
  }
  return _brightcove_client_access($op, empty($entity) ? NULL : $entity->bcid, $account);
}

/**
 * Helper function for brightcove_client_access().
 *
 * @param $op
 * @param string $bcid
 *   The Brightcove Client bcid.
 * @param object $account
 *  The drupal user object.
 *
 * @return boolean
 */
function _brightcove_client_access($op, $bcid, $account) {
  switch ($op) {
    case 'use':
      return user_access('use brightcove client ' . $bcid, $account) || user_access('administer brightcove settings', $account);
    default:
      return user_access('administer brightcove settings', $account);
  }
}


/**
 * Saves a brightcove_client.
 */
function brightcove_client_save($client) {
  entity_get_controller('brightcove_client')->save($client);
}

/**
 * Load a brightcove client entity.
 *
 * @param string $bcid
 *  The client id
 * @return Entity
 *  The client entity object.
 */
function brightcove_client_load($bcid) {
  return entity_load_single('brightcove_client', $bcid);
}

/**
 * Load a client or if none given, the default client.
 *
 * @param string $bcid
 *  The client entity id.
 *
 * @return Entity|boolean
 *  The client entity object or FALSE.
 */
function brightcove_client_load_or_default($bcid = NULL) {
  if ($bcid) {
    $client = brightcove_client_load($bcid);
    if ($client) {
      return $client;
    }
  }

  if (($default_bcid = variable_get('brightcove_client_default'))) {
    $client = brightcove_client_load($default_bcid);
    if ($client) {
      return $client;
    }
  }

  return FALSE;
}

/**
 * Loads all brightcove clients for a given brightcove account a drupal user
 * has 'Use X client' permission to.
 *
 * @param string $account_id
 *  The brightcove client account id.
 * @param object $account
 *  The drupal user object.
 */
function brightcove_get_clients_by_account_id($account_id, $account = NULL) {
  if (is_null($account)) {
    global $user;
    $account = $user;
  }
  // Get all client bcids belonging to this $account_id.
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'brightcove_client')
    ->propertyCondition('account_id', $account_id)
    ->addMetaData('account', user_load(1));
  $result = $query->execute();

  // Only keep the clients this $account has 'use' access to.
  $bcids = [];
  if (isset($result['brightcove_client'])) {
    foreach ($result['brightcove_client'] as $bcid => $client_result) {
      if (_brightcove_client_access('use', $bcid, $account)) {
        $bcids[] = $bcid;
      }
    }
  }
  return entity_load('brightcove_client', $bcids);
}
