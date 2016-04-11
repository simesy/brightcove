<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcoveVideoForm.
 */

namespace Drupal\brightcove\Form;

use Brightcove\API\Exception\APIException;
use Drupal\brightcove\Entity\BrightcoveCustomField;
use Drupal\brightcove\Entity\BrightcoveVideo;
use Drupal\Core\Ajax\ReplaceCommand;
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

    // Get api client from the form settings.
    $api_client = NULL;
    if (!empty($form_state->getValue('api_client'))) {
      $api_client = $form_state->getValue('api_client')[0]['target_id'];
    }
    else {
      $api_client = $form['api_client']['widget']['#default_value'];
    }

    // Remove _none value to make sure the first item is selected.
    if (isset($form['profile']['widget']['#options']['_none'])) {
      unset($form['profile']['widget']['#options']['_none']);
    }

    // Get the correct profiles' list for the selected api client.
    if ($entity->isNew()) {
      // Class this class's update form method instead of the supper class's.
      $form['api_client']['widget']['#ajax']['callback'] = [
        self::class, 'apiClientUpdateForm',
      ];

      // Add ajax wrapper for profile.
      $form['profile']['widget']['#ajax_id'] = 'ajax-profile-wrapper';
      $form['profile']['widget']['#prefix'] = '<div id="' . $form['profile']['widget']['#ajax_id'] . '">';
      $form['profile']['widget']['#suffix'] = '</div>';

      // Update allowed values for profile.
      $form['profile']['widget']['#options'] = BrightcoveVideo::getProfileAllowedValues($api_client);
    }

    // Set default profile.
    if (!$form['profile']['widget']['#default_value']) {
      $profile_keys = array_keys($form['profile']['widget']['#options']);
      $form['profile']['widget']['#default_value'] = reset($profile_keys);
    }

    /** @var \Drupal\brightcove\Entity\BrightcoveCustomField[] $custom_fields */
    $custom_fields = BrightcoveCustomField::loadMultipleByAPIClient($api_client);

    // Add ajax wrapper for custom fields.
    if ($entity->isNew()) {
      $form['custom_fields']['#ajax_id'] = 'ajax-custom-fields-wrapper';
      $form['custom_fields']['#prefix'] = '<div id="' . $form['custom_fields']['#ajax_id'] . '">';
      $form['custom_fields']['#suffix'] = '</div>';
    }

    // Show custom fields.
    if (count($custom_fields) > 0) {
      $form['custom_fields']['#type'] = 'details';
      $form['custom_fields']['#title'] = $this->t('Custom fields');

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

          // Add none option if the field is not required.
          if (!$form['custom_fields'][$custom_field_id]['#required']) {
            $options[''] = $this->t(' - None -');
          }

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

    try {
      // Save custom field values.
      $custom_field_values = [];
      if (!empty($form['custom_fields'])) {
        foreach (Element::children($form['custom_fields']) as $field_name) {
          $custom_field_values[$field_name] = $form_state->getValue($field_name);
        }
        $entity->setCustomFieldValues($custom_field_values);
      }

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
    catch (APIException $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
  }

  /**
   * Ajax callback to update the profile and player options list.
   *
   * @param $form
   * @param \Drupal\Core\Form\FormStateInterface $form_state
   *
   * @return \Drupal\Core\Ajax\AjaxResponse
   */
  public static function apiClientUpdateForm($form, FormStateInterface $form_state) {
    $response = parent::apiClientUpdateForm($form, $form_state);

    // Update profile field.
    $response->addCommand(new ReplaceCommand(
      '#' . $form['profile']['widget']['#ajax_id'],
      $form['profile']
    ));

    // Update custom fields.
    $response->addCommand(new ReplaceCommand(
      '#' . $form['custom_fields']['#ajax_id'],
      $form['custom_fields']
    ));

    return $response;
  }
}
