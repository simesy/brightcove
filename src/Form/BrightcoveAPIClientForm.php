<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcoveAPIClientForm.
 */

namespace Drupal\brightcove\Form;

use Brightcove\API\Exception\APIException;
use Drupal\Core\Config\Config;
use Drupal\Core\Entity\EntityForm;
use Drupal\Core\Form\FormStateInterface;
use Symfony\Component\DependencyInjection\ContainerInterface;
use Drupal\brightcove\Entity\BrightcoveAPIClient;
use Brightcove\API\Exception\AuthenticationException;
use Brightcove\API\Client;
use Brightcove\API\CMS;

/**
 * Class BrightcoveAPIClientForm.
 *
 * @package Drupal\brightcove\Form
 */
class BrightcoveAPIClientForm extends EntityForm {

  /**
   * The config for brightcove.settings.
   *
   * @var \Drupal\Core\Config\Config
   */
  protected $config;

  /**
   * Constructs a new BrightcoveAPIClientForm.
   *
   * @param \Drupal\Core\Config\Config $config
   *   The config for brightcove.settings.
   */
  public function __construct(Config $config) {
    $this->config = $config;
  }

  /**
   * @inheritdoc
   */
  public static function create(ContainerInterface $containerInterface) {
    return new static(
      $containerInterface->get('config.factory')->getEditable('brightcove.settings')
    );
  }

  /**
   * {@inheritdoc}
   */
  public function form(array $form, FormStateInterface $form_state) {
    $form = parent::form($form, $form_state);

    /** @var \Drupal\brightcove\Entity\BrightcoveAPIClient $brightcove_api_client */
    $brightcove_api_client = $this->entity;
    $brightcove_api_client->authorizeClient();

    // Don't even try reporting the status/error message of a new client.
    if (!$brightcove_api_client->isNew()) {
      $form['status'] = array(
        '#type' => 'item',
        '#title' => t('Status'),
        '#markup' => $brightcove_api_client->getClientStatus() ? $this->t('OK') : $this->t('Error'),
      );

      if ($brightcove_api_client->getClientStatus() == 0) {
        $form['status_message'] = array(
          '#type' => 'item',
          '#title' => t('Error message'),
          '#markup' => $brightcove_api_client->getClientStatusMessage(),
        );
      }
    }

    $form['label'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Label'),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->label(),
      '#description' => $this->t('Label for the Brightcove API Client.'),
      '#required' => TRUE,
    );

    $form['id'] = array(
      '#type' => 'machine_name',
      '#default_value' => $brightcove_api_client->id(),
      '#machine_name' => array(
        'exists' => '\Drupal\brightcove\Entity\BrightcoveAPIClient::load',
      ),
      '#disabled' => !$brightcove_api_client->isNew(),
    );

    $form['account_id'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Account ID'),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->getAccountID(),
      '#required' => TRUE,
    );

    $form['client_id'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Client ID'),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->getClientID(),
      '#required' => TRUE,
    );

    $form['default_player'] = array(
      '#type' => 'select',
      '#title' => $this->t('Default player'),
      '#options' => [
        // Other players will be implemented later (as configuration entities).
        $brightcove_api_client::DEFAULT_PLAYER => $this->t('Brightcove Default Player'),
      ],
      '#default_value' => $brightcove_api_client->getDefaultPlayer() ? $brightcove_api_client->getDefaultPlayer() : 'default',
      '#required' => TRUE,
    );

    $form['secret_key'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Secret key'),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->getSecretKey(),
      '#required' => TRUE,
    );

    $form['default_client'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Default API Client'),
      '#description' => $this->t('Enable this to make this API Client the default.'),
      '#default_value' => ($this->config->get('defaultAPIClient') == $brightcove_api_client->id()),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    try {
      // Try to authorize client and save values on success.
      $json = BrightcoveAPIClient::authorize($form_state->getValue('client_id'), $form_state->getValue('secret_key'));

      // Test account ID.
      $client = new Client($json['access_token']);
      $cms = new CMS($client, $form_state->getValue('account_id'));
      $cms->countVideos();

      $form_state->setValue('access_token', $json['access_token']);
      $form_state->setValue('access_token_expire_date', REQUEST_TIME + intval($json['expires_in']));
    }
    catch (AuthenticationException $e) {
      $form_state->setErrorByName('client_id', $e->getMessage());
      $form_state->setErrorByName('secret_key');
    }
    catch (APIException $e) {
      $form_state->setErrorByName('account_id', $e->getMessage());
    }
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {
    $brightcove_api_client = $this->entity;
    $status = $brightcove_api_client->save();

    switch ($status) {
      case SAVED_NEW:
        drupal_set_message($this->t('Created the %label Brightcove API Client.', [
          '%label' => $brightcove_api_client->label(),
        ]));
        break;

      default:
        drupal_set_message($this->t('Saved the %label Brightcove API Client.', [
          '%label' => $brightcove_api_client->label(),
        ]));
    }

    if ($form_state->getValue('default_client')) {
      $this->config->set('defaultAPIClient', $brightcove_api_client->id())->save();
    }

    $form_state->setRedirectUrl($brightcove_api_client->urlInfo('collection'));
  }

}
