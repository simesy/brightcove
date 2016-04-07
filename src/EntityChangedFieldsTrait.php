<?php
/**
 * @file
 * Contains \Drupal\brightcove\EntityChangedFieldsTrait.
 */

namespace Drupal\brightcove;

use Drupal\Core\Entity\Entity;
use Drupal\Core\Entity\EntityStorageInterface;

/**
 * Class EntityChangedFieldsTrait
 *
 * @package Drupal\brightcove
 */
trait EntityChangedFieldsTrait {
  /**
   * Changed fields.
   *
   * @var bool[]
   */
  protected $changedFields;

  /**
   * Has changed field or not.
   *
   * @var bool
   */
  protected $hasChangedField = FALSE;

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
    // Indicate that there is at least one changed field.
    if (!$this->changedFields) {
      $this->changedFields = TRUE;
    }

    return !empty($this->changedFields[$name]);
  }

  /**
   * Checked if the Entity has a changed field or not.
   *
   * @return bool
   */
  public function hasChangedField() {
    return $this->hasChangedField;
  }

  /**
   * Check for updated fields, ideally it should be called from the entity's
   * preSave() method before the parent's preSave() call.
   *
   * @param \Drupal\Core\Entity\EntityStorageInterface $storage
   */
  public function checkUpdatedFields(EntityStorageInterface $storage) {
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
  }
}