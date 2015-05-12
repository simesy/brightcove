<?php

/**
 * @file
 * Client related code.
 */

/**
 * Admin UI controller for 'brightcove_client' entity type.
 */
class BrightcoveClientEntityUIController extends EntityDefaultUIController {

  public function hook_menu() {
    $items = parent::hook_menu();
    $items['admin/config/media/brightcove/client']['type'] = MENU_LOCAL_TASK;
    $items['admin/config/media/brightcove/client']['weight'] = 0.1;
    $items['admin/config/media/brightcove/client']['title'] = 'API';
    $items['admin/config/media/brightcove/client/add']['title'] = 'Add Brightcove API Authentication';
    $items['admin/config/media/brightcove/client/add']['title callback'] = 't';
    return $items;
  }

  protected function overviewTableRow($conditions, $id, $entity, $additional_cols = []) {
    if (variable_get('brightcove_client_default') !== $id) {
      $additional_cols[] = l(t('set default'), "{$this->path}/manage/{$id}/setdefault");
    }
    return parent::overviewTableRow($conditions, $id, $entity, $additional_cols);
  }

  public function operationForm($form, &$form_state, $entity, $op) {
    switch ($op) {
      case 'setdefault':
        $label = entity_label($this->entityType, $entity);
        $confirm_question = t('Are you sure you want to set default the %entity %label?', [
          '%entity' => $this->entityInfo['label'],
          '%label' => $label
        ]);
        return confirm_form($form, $confirm_question, $this->path, '');
      default:
        return parent::operationForm($form, $form_state, $entity, $op);
    }
  }

  protected function operationCount() {
    return parent::operationCount() + 1;
  }

  public function applyOperation($op, $entity) {
    $label = entity_label($this->entityType, $entity);
    $vars = ['%entity' => $this->entityInfo['label'], '%label' => $label];
    switch ($op) {
      case 'setdefault':
        variable_set('brightcove_client_default', $entity->bcid);
        return t('%entity %label is now the default.', $vars);
      default:
        return parent::applyOperation($op, $entity);
    }
  }
}

/**
 * Entity controller class for Brightcove client.
 */
class BrightcoveClientEntityController extends EntityAPIController {

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

    $client->revision_timestamp = REQUEST_TIME;
    $client->revision_uid = $user->uid;

    // Determine if we will be inserting a new client.
    $client->is_new = empty($client->bcid);

    if ($client->is_new || !empty($client->revision)) {
      // When inserting either a new client or revision, $entity->log must be set
      // because {brightcove_client_revision}.log is a text column and therefore
      // cannot have a default value. However, it might not be set at this
      // point, so we ensure that it is at least an empty string in that case.
      if (!isset($client->log)) {
        $client->log = '';
      }
    }
    elseif (empty($client->log)) {
      // If we are updating an existing client without adding a new revision,
      // we need to make sure $entity->log is unset whenever it is empty. As
      // long as $entity->log is unset, drupal_write_record() will not attempt
      // to update the existing database column when re-saving the revision.
      unset($client->log);
    }

    return parent::save($client, $transaction);
  }
}

/**
 * Returns a form element to select a brightcove client.
 *
 * This function should be used in a hook_form_alter() whenever a client
 * select is needed on a form.
 */
function brightcove_client_select_element() {
  $client_options = [BRIGHTCOVE_CLIENT_ID_NONE => t('- Select a client -')];
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

  // When updating a client, do not collapse the Change History fieldset.
  $form['change_history'] = [
    '#type' => 'fieldset',
    '#title' => t('Change history'),
    '#collapsible' => TRUE,
    '#collapsed' => empty($client->bcid),
    '#weight' => 350,
  ];
  if (!empty($client->bcid)) {
    $form['change_history']['revision'] = [
      '#type' => 'checkbox',
      '#title' => t('Create new revision on update'),
      '#description' => t('If an update log message is entered, a revision will be created even if this is unchecked.'),
    ];
  }
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
  try {
    $client = BrightcoveClient::authorize($client_id, $client_secret);
    if (!$client->isAuthorized()) {
      form_error($form, t('Failed to authorize.'));
    }
  } catch (BrightcoveAuthenticationException $ex) {
    form_error($form, t('Failed to authorize.'));
    watchdog_exception('brightcove', $ex, 'Brightcove client validation');
  }

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
