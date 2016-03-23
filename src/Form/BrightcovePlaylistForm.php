<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcovePlaylistForm.
 */

namespace Drupal\brightcove\Form;

use Drupal\brightcove\Entity\BrightcovePlaylist;
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
    /* @var $entity \Drupal\brightcove\Entity\BrightcovePlaylist */
    $form = parent::buildForm($form, $form_state);

    // Manual playlist: no search, only videos.
    $form['videos']['#states'] = [
      'visible' => [
        'input[name="type"]' => ['value' => BrightcovePlaylist::TYPE_MANUAL],
      ],
    ];
    // Smart playlist: no videos, only search.
    $form['search']['#states'] = [
      'visible' => [
        'input[name="type"]' => ['value' => BrightcovePlaylist::TYPE_SMART],
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
}
