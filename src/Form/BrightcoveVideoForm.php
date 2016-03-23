<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcoveVideoForm.
 */

namespace Drupal\brightcove\Form;

use Drupal\brightcove\Entity\BrightcoveCustomField;
use Drupal\brightcove\Entity\BrightcoveVideo;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Render\Element;

/**
 * Form controller for Brightcove Video edit forms.
 *
 * @ingroup brightcove
 */
class BrightcoveVideoForm extends BrightcoveVideoPlaylistForm {
  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildForm($form, $form_state);

    /** @var $entity \Drupal\brightcove\Entity\BrightcoveVideo */
    $entity = $this->entity;

    // Set default profile.
    if (!$form['profile']['widget']['#default_value']) {
      $profile_allowed_values = BrightcoveVideo::getProfileAllowedValues();
      $form['profile']['widget']['#default_value'] = reset($profile_allowed_values);
    }

    /** @var \Drupal\brightcove\Entity\BrightcoveCustomField[] $custom_fields */
    $custom_fields = BrightcoveCustomField::loadMultipleByAPIClient($entity->getAPIClient());

    // Show custom fields.
    if (count($custom_fields) > 0) {
      $form['custom_fields'] = [
        '#type' => 'details',
        '#title' => $this->t('Custom fields'),
      ];

      $has_required = FALSE;
      $custom_field_values = $entity->getCustomFieldValues();
      foreach ($custom_fields as $custom_field) {
        // Indicate whether that the custom fields has required field(s) or
        // not.
        if (!$has_required && $custom_field->isRequired()) {
          $has_required = TRUE;
        }

        switch ($custom_field_type = $custom_field->getType()) {
          case $custom_field::TYPE_STRING:
            $type = 'textfield';
            break;

          case $custom_field::TYPE_ENUM:
            $type = 'select';
            break;

          default:
            continue 2;
        }

        // Assemble form field for the custom field.
        $form['custom_fields'][$custom_field_id = $custom_field->getCustomFieldId()] = [
          '#type' => $type,
          '#title' => $custom_field->getName(),
          '#description' => $custom_field->getDescription(),
          '#required' => $custom_field->isRequired(),
        ];

        // Set custom field value if it is set.
        if (isset($custom_field_values[$custom_field_id])) {
          $form['custom_fields'][$custom_field_id]['#default_value'] = $custom_field_values[$custom_field_id];
        }

        // Add options for enum types.
        if ($custom_field_type == $custom_field::TYPE_ENUM) {
          $options = [];
          foreach ($custom_field->getEnumValues() as $enum) {
            $options[$enum['value']] = $enum['value'];
          }
          $form['custom_fields'][$custom_field_id]['#options'] = $options;
        }
      }

      // Show custom field group opened if it has at least one required field.
      if ($has_required) {
        $form['custom_fields']['#open'] = TRUE;
      }
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {
    /** @var $entity \Drupal\brightcove\Entity\BrightcoveVideo */
    $entity = $this->entity;

    // Save custom field values.
    $custom_field_values = [];
    foreach (Element::children($form['custom_fields']) as $field_name) {
      $custom_field_values[$field_name] = $form_state->getValue($field_name);
    }
    $entity->setCustomFieldValues($custom_field_values);

    $status = $entity->save(TRUE);

    switch ($status) {
      case SAVED_NEW:
        drupal_set_message($this->t('Created the %label Brightcove Video.', [
          '%label' => $entity->label(),
        ]));
        break;

      default:
        drupal_set_message($this->t('Saved the %label Brightcove Video.', [
          '%label' => $entity->label(),
        ]));
    }
    $form_state->setRedirect('entity.brightcove_video.canonical', ['brightcove_video' => $entity->id()]);
  }
}
