<?php
/**
 * @file
 * Entity controller classes definition for brightcove entities.
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
