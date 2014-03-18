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
    $this->registerTypes(array('brightcove_field'));
  }

  public function fields($type, $instance, $migration = NULL) {
    $fields['brightcove_id'] =  t('Subfield: The brightcove asset id');
    $fields['player'] =  t('Subfield: The brightcove player machine name');
    return $fields;
  }

  public function prepare($entity, array $field_info, array $instance, array $values) {
    $arguments = array();
    if (isset($values['arguments'])) {
      $arguments = array_filter($values['arguments']);
      unset($values['arguments']);
    }
    $language = $this->getFieldLanguage($entity, $field_info, $arguments);

    // Setup the standard Field API array for saving.
    $delta = 0;
    foreach ($values as $value) {
      if (isset($arguments['brightcove_id'])) {
        if (is_array($arguments['brightcove_id'])) {
          $item['brightcove_id'] = $arguments['brightcove_id'][$delta];
        }
        else {
          $item['brightcove_id'] = $arguments['brightcove_id'];
        }
      }
      else {
        $item['brightcove_id'] = 0;
      }
      if (isset($arguments['player'])) {
        if (is_array($arguments['player'])) {
          $item['player'] = $arguments['player'][$delta];
        }
        else {
          $item['player'] = $arguments['player'];
        }
      }
      else {
        $item['player'] = 0;
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
