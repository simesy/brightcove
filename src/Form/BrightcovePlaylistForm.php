<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcovePlaylistForm.
 */

namespace Drupal\brightcove\Form;

use Drupal\brightcove\BrightcoveUtil;
use Drupal\brightcove\Entity\BrightcovePlaylist;
use Drupal\Core\Entity\ContentEntityForm;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Form\FormStateInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Form controller for Brightcove Playlist edit forms.
 *
 * @ingroup brightcove
 */
class BrightcovePlaylistForm extends ContentEntityForm {

  /**
   * The default API Client.
   *
   * @var string
   */
  protected $defaultAPIClient;

  /**
   * Constructs a ContentEntityForm object.
   *
   * @param \Drupal\Core\Entity\EntityManagerInterface $entity_manager
   *   The entity manager.
   * @param string $defaultAPIClient
   *   The default API Client.
   */
  public function __construct(EntityManagerInterface $entity_manager, $defaultAPIClient) {
    parent::__construct($entity_manager);
    $this->defaultAPIClient = $defaultAPIClient;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity.manager'),
      $container->get('config.factory')->get('brightcove.settings')->get('defaultAPIClient')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    /* @var $entity \Drupal\brightcove\Entity\BrightcovePlaylist */
    $form = parent::buildForm($form, $form_state);
    $entity = $this->entity;

    // Check for an updated version of the Playlist.
    if ($entity->id()) {
      BrightcoveUtil::checkUpdatedVersion($entity);
    }

    if (!$form['api_client']['widget']['#default_value']) {
      $form['api_client']['widget']['#default_value'] = $this->defaultAPIClient;
    }

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
