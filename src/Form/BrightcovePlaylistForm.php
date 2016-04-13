<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcovePlaylistForm.
 */

namespace Drupal\brightcove\Form;

use Brightcove\API\Exception\APIException;
use Drupal\brightcove\Entity\BrightcovePlaylist;
use Drupal\Core\Ajax\ReplaceCommand;
use Drupal\Core\Form\FormStateInterface;

/**
 * Form controller for Brightcove Playlist edit forms.
 *
 * @ingroup brightcove
 */
class BrightcovePlaylistForm extends BrightcoveVideoPlaylistForm {
  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    $form = parent::buildForm($form, $form_state);

    /* @var $entity \Drupal\brightcove\Entity\BrightcovePlaylist */
    $entity = $this->entity;

    // Get api client from the form settings.
    if (!empty($form_state->getValue('api_client'))) {
      $api_client = $form_state->getValue('api_client')[0]['target_id'];
    }
    else {
      $api_client = $form['api_client']['widget']['#default_value'];

      if (is_array($api_client)) {
        $api_client = reset($api_client);
      }
    }

    $form['#attached']['library'][] = 'brightcove/brightcove.chosen';

    // Manual playlist: no search, only videos.
    $manual_type = array_keys(BrightcovePlaylist::getTypes(BrightcovePlaylist::TYPE_MANUAL));
    $form['videos']['#states'] = [
      'visible' => [
        ':input[name="type"]' => ['value' => reset($manual_type)],
      ],
    ];

    // Remove none value.
    if (isset($form['videos']['widget']['#options']['_none'])) {
      unset($form['videos']['widget']['#options']['_none']);
    }

    // Smart playlist: no videos, only search.
    $smart_types = [];
    foreach (array_keys(BrightcovePlaylist::getTypes(BrightcovePlaylist::TYPE_SMART)) as $smart_type) {
      $smart_types[] = ['value' => $smart_type];
    }

    $form['tags_search_condition']['#states'] = [
      'visible' => [
        ':input[name="type"]' => $smart_types,
      ],
    ];

    $form['tags']['#states'] = [
      'visible' => [
        ':input[name="type"]' => $smart_types,
      ],
    ];

    // Get tags for the given api client.
    $form['tags']['widget']['#options'] = BrightcovePlaylist::getTagsAllowedValues($api_client);

    // Ajax wrapper to be able to update the tags list on api client change.
    if ($entity->isNew()) {
      $form['api_client']['widget']['#ajax']['callback'] = [
        self::class, 'apiClientUpdateForm',
      ];

      $form['tags']['widget']['#ajax_id'] = 'ajax-tags-wrapper';
      $form['tags']['widget']['#prefix'] = '<div id="' . $form['tags']['widget']['#ajax_id'] . '">';
      $form['tags']['widget']['#suffix'] = '</div>';
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {
    /** @var $entity \Drupal\brightcove\Entity\BrightcovePlaylist */
    $entity = $this->entity;

    try {
      $status = $entity->save(TRUE);

      switch ($status) {
        case SAVED_NEW:
          drupal_set_message($this->t('Created the %label Brightcove Playlist.', [
            '%label' => $entity->label(),
          ]));
          break;

        default:
          drupal_set_message($this->t('Saved the %label Brightcove Playlist.', [
            '%label' => $entity->label(),
          ]));
      }
      $form_state->setRedirect('entity.brightcove_playlist.canonical', ['brightcove_playlist' => $entity->id()]);
    }
    catch (APIException $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
  }

  /**
   * Ajax callback to update the tags list.
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
      '#' . $form['tags']['widget']['#ajax_id'],
      $form['tags']
    ));

    return $response;
  }
}
