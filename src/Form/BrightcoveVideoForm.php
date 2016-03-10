<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcoveVideoForm.
 */

namespace Drupal\brightcove\Form;

use Drupal\brightcove\BrightcoveUtil;
use Drupal\brightcove\Entity\BrightcoveVideo;
use Drupal\Core\Entity\ContentEntityForm;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\Entity\EntityManagerInterface;
use Drupal\Core\Url;
use Drupal\Core\Utility\LinkGenerator;
use Symfony\Component\DependencyInjection\ContainerInterface;

/**
 * Form controller for Brightcove Video edit forms.
 *
 * @ingroup brightcove
 */
class BrightcoveVideoForm extends ContentEntityForm {
  /**
   * The default API Client.
   *
   * @var string
   */
  protected $defaultAPIClient;

  /**
   * The link generator.
   *
   * @var \Drupal\Core\Utility\LinkGenerator
   */
  protected $linkGenerator;

  /**
   * Constructs a ContentEntityForm object.
   *
   * @param \Drupal\Core\Entity\EntityManagerInterface $entity_manager
   *   The entity manager.
   * @param string $defaultAPIClient
   *   The default API Client.
   * @param \Drupal\Core\Utility\LinkGenerator $linkGenerator
   *   The LinkGenerator.
   */
  public function __construct(EntityManagerInterface $entity_manager, $defaultAPIClient, LinkGenerator $linkGenerator) {
    parent::__construct($entity_manager);
    $this->defaultAPIClient = $defaultAPIClient;
    $this->linkGenerator = $linkGenerator;
  }

  /**
   * {@inheritdoc}
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('entity.manager'),
      $container->get('config.factory')->get('brightcove.settings')->get('defaultAPIClient'),
      $container->get('link_generator')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {
    /** @var $entity \Drupal\brightcove\Entity\BrightcoveVideo */
    $form = parent::buildForm($form, $form_state);
    $entity = $this->entity;

    // Check for an updated version of the Video.
    if ($entity->id()) {
      BrightcoveUtil::checkUpdatedVersion($entity);
    }

    // Set default api client.
    if (!$form['api_client']['widget']['#default_value']) {
      $form['api_client']['widget']['#default_value'] = $this->defaultAPIClient;
    }

    // Set default profile.
    if (!$form['profile']['widget']['#default_value']) {
      $profile_allowed_values = BrightcoveVideo::getProfileAllowedValues();
      $form['profile']['widget']['#default_value'] = reset($profile_allowed_values);
    }

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {
    /** @var $entity \Drupal\brightcove\Entity\BrightcoveVideo */
    $entity = $this->entity;
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
