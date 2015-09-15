<?php

/**
 * Primary value passed to this field is the entity id.
 *
 * Subfields are used to specify all the other values:
 *   'brightcove id'  - the brightcove asset id
 *   'player' - the brightcove player machine name
 *
 * @code
 *   $this->addFieldMapping('field_brightcove_field:brightcove_id', '');
 *   $this->addFieldMapping('field_brightcove_field:player', '')
 * @endcode
 */
class MigrateBrightcoveFieldHandler extends MigrateFieldHandler {
  public function __construct() {
    $this->registerTypes(['brightcove_field']);
  }

  public function fields($type, $instance, $migration = NULL) {
    $fields['brightcove_id'] =  t('Subfield: The brightcove asset id');
    $fields['player'] =  t('Subfield: The brightcove player machine name');
    return $fields;
  }

  public function prepare($entity, array $field_info, array $instance, array $values) {
    $arguments = [];

    $language = $this->getFieldLanguage($entity, $field_info, $arguments);

    // Setup the standard Field API array for saving.
    $delta = 0;
    $return = array();
    foreach ($values as $value) {
      if (isset($value['brightcove_id'])) {
        if (is_array($value['brightcove_id'])) {
          $item['brightcove_id'] = $value['brightcove_id'][$delta];
        }
        else {
          $item['brightcove_id'] = $value['brightcove_id'];
        }
      }
      else {
        $item['brightcove_id'] = 0;
      }

      if (isset($value['player'])) {
        if (is_array($value['player'])) {
          $item['player'] = $value['player'][$delta];
        }
        else {
          $item['player'] = $value['player'];
        }
      }
      else {
        $item['player'] = 0;
      }

      if (isset($value['bcid'])) {
        if (is_array($value['bcid'])) {
          $item['bcid'] = $value['bcid'][$delta];
        }
        else {
          $item['bcid'] = $value['bcid'];
        }
      }
      else {
        $item['bcid'] = 0;
      }

      $item['choice'] = $value;
      if (is_array($language)) {
        $current_language = $language[$delta];
      }
      else {
        $current_language = $language;
      }
      $return[$current_language][] = $item;
      $delta++;
    }
    return isset($return) ? $return : NULL;
  }

}
