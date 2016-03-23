<?php
/**
 * @file
 * Contains \Drupal\brightcove\Entity\BrightcoveCMSEntity.
 */

namespace Drupal\brightcove\Entity;

use Drupal\brightcove\BrightcoveCMSEntityInterface;
use Drupal\Core\Entity\ContentEntityBase;
use Drupal\Core\Entity\EntityChangedTrait;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\user\UserInterface;

/**
 * Common base class for CMS entities like Video and Playlist.
 */
abstract class BrightcoveCMSEntity extends ContentEntityBase implements BrightcoveCMSEntityInterface {
  use EntityChangedTrait;

  /**
   * Changed fields.
   *
   * @var bool[]
   */
  protected $changedFields;

  /**
   * Returns whether the field is changed or not.
   *
   * @param $name
   *   The name of the field on the entity.
   *
   * @return bool
   *   The changed status of the field, TRUE if changed, FALSE otherwise.
   */
  public function isFieldChanged($name) {
    return !empty($this->changedFields[$name]);
  }

  /**
   * {@inheritdoc}
   */
  public function getName() {
    return $this->get('name')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function setName($name) {
    $this->set('name', $name);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getAPIClient() {
    return $this->get('api_client')->target_id;
  }

  /**
   * {@inheritdoc}
   */
  public function setAPIClient($api_client) {
    $this->set('api_client', $api_client);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getDescription() {
    return $this->get('description')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function setDescription($description) {
    $this->set('description', $description);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getOwner() {
    return $this->get('uid')->entity;
  }

  /**
   * {@inheritdoc}
   */
  public function setOwner(UserInterface $account) {
    $this->set('uid', $account->id());
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getOwnerId() {
    return $this->get('uid')->target_id;
  }

  /**
   * {@inheritdoc}
   */
  public function setOwnerId($uid) {
    $this->set('uid', $uid);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getCreatedTime() {
    return $this->get('created')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function setCreatedTime($timestamp) {
    $this->set('created', $timestamp);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function preSave(EntityStorageInterface $storage) {
    // Collect object getters.
    $methods = [];
    foreach (get_class_methods($this) as $key => $method) {
      // Create a matchable key for the get methods.
      if (preg_match('/get[\w\d_]+/i', $method)) {
        $methods[strtolower($method)] = $method;
      }
    }

    // Check fields if they were updated and mark them if changed.
    if (!empty($this->id())) {
      /** @var \Drupal\brightcove\Entity\BrightcoveVideo $original_entity */
      $original_entity = $storage->loadUnchanged($this->id());

      if ($original_entity->getChangedTime() != $this->getChangedTime()) {
        /**
         * @var string $name
         * @var \Drupal\Core\Field\FieldItemList $field
         */
        foreach ($this->getFields() as $name => $field) {
          // Acquire getter method name.
          $getter_name = 'get' . str_replace('_', '', $name);
          $getter = isset($methods[$getter_name]) ? $methods[$getter_name] : NULL;

          // If the getter is available for the field then compare the two
          // field and if changed mark it.
          if (!is_null($getter) && $this->$getter() != $original_entity->$getter()) {
            $this->changedFields[$name] = TRUE;
          }
        }
      }
    }
    // If there is no original entity, mark all fields modified, because in
    // this case the entity is being created.
    else {
      foreach ($this->getFields() as $name => $field) {
        // Acquire getter method name.
        $getter_name = 'get' . str_replace('_', '', $name);
        $getter = isset($methods[$getter_name]) ? $methods[$getter_name] : NULL;

        if (!is_null($getter)) {
          $this->changedFields[$name] = TRUE;
        }
      }
    }

    return parent::preSave($storage);
  }

  /**
   * Default value callback for 'uid' base field definition.
   *
   * @see ::baseFieldDefinitions()
   *
   * @return array
   *   An array of default values.
   */
  public static function getCurrentUserId() {
    return [
      \Drupal::currentUser()->id(),
    ];
  }

  /**
   * {@inheritdoc}
   */
  public static function preCreate(EntityStorageInterface $storage_controller, array &$values) {
    parent::preCreate($storage_controller, $values);
    $values += [
      'uid' => \Drupal::currentUser()->id(),
    ];
  }
}