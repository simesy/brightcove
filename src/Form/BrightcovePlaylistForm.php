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

    // Manual playlist: no search, only videos.
    $manual_type = array_keys(BrightcovePlaylist::getTypes(BrightcovePlaylist::TYPE_MANUAL));
    $form['videos']['#states'] = [
      'visible' => [
        ':input[name="type"]' => ['value' => reset($manual_type)],
      ],
    ];

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
}
