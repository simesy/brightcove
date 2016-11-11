<?php

/**
 * @file
 * Contains \Drupal\brightcove\Form\BrightcoveAPIClientForm.
 */

namespace Drupal\brightcove\Form;

use Brightcove\API\Exception\APIException;
use Brightcove\API\PM;
use Drupal\brightcove\BrightcoveUtil;
use Drupal\brightcove\Entity\BrightcovePlayer;
use Drupal\Core\Config\Config;
use Drupal\Core\Entity\Query\QueryFactory;
use Drupal\Core\Entity\EntityForm;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Form\FormStateInterface;
use Drupal\Core\KeyValueStore\KeyValueStoreExpirableInterface;
use Drupal\Core\Queue\QueueInterface;
use Drupal\key\KeyRepositoryInterface;
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
   * The brightcove_player storage.
   *
   * @var \Drupal\Core\Entity\EntityStorageInterface
   */
  protected $player_storage;

  /**
   * The player queue object.
   *
   * @var \Drupal\Core\Queue\QueueInterface
   */
  protected $player_queue;

  /**
   * The custom field queue object.
   *
   * @var \Drupal\Core\Queue\QueueInterface
   */
  protected $custom_field_queue;

  /**
   * Query factory.
   *
   * @var \Drupal\Core\Entity\Query\QueryFactory
   */
  protected $query_factory;

  /**
   * Key/Value expirable store.
   *
   * @var \Drupal\Core\KeyValueStore\KeyValueStoreExpirableInterface
   */
  protected $key_value_expirable_store;

  /**
   * Key repository.
   *
   * @var \Drupal\key\KeyRepositoryInterface
   */
  protected $key_repository;

  /**
   * Constructs a new BrightcoveAPIClientForm.
   *
   * @param \Drupal\Core\Config\Config $config
   *   The config for brightcove.settings.
   * @param \Drupal\Core\Entity\EntityStorageInterface $player_storage
   *   Player entity storage.
   * @param \Drupal\Core\Queue\QueueInterface $player_queue
   *   Player queue.
   * @param \Drupal\Core\Queue\QueueInterface $custom_field_queue
   *   Custom field queue.
   * @param \Drupal\Core\Entity\Query\QueryFactory $query_factory
   *   Query factory.
   * @param \Drupal\Core\KeyValueStore\KeyValueStoreExpirableInterface $key_value_expirable_store
   *   Key/Value expirable store for "brightcove_access_token".
   * @param \Drupal\key\KeyRepositoryInterface
   *   Key repository.
   */
  public function __construct(Config $config, EntityStorageInterface $player_storage, QueueInterface $player_queue, QueueInterface $custom_field_queue, QueryFactory $query_factory, KeyValueStoreExpirableInterface $key_value_expirable_store, KeyRepositoryInterface $key_repository) {
    $this->config = $config;
    $this->player_storage = $player_storage;
    $this->player_queue = $player_queue;
    $this->query_factory = $query_factory;
    $this->custom_field_queue = $custom_field_queue;
    $this->key_value_expirable_store = $key_value_expirable_store;
    $this->key_repository = $key_repository;
  }

  /**
   * @inheritdoc
   */
  public static function create(ContainerInterface $container) {
    return new static(
      $container->get('config.factory')->getEditable('brightcove.settings'),
      $container->get('entity_type.manager')->getStorage('brightcove_player'),
      $container->get('queue')->get('brightcove_player_queue_worker'),
      $container->get('queue')->get('brightcove_custom_field_queue_worker'),
      $container->get('entity.query'),
      $container->get('keyvalue.expirable')->get('brightcove_access_token'),
      $container->get('key.repository')
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
      '#description' => $this->t('A label to identify the API Client (authentication credentials).'),
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

    $form['client_id'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Brightcove API Client ID'),
      '#description' => $this->t('The Client ID of the Brightcove API Authentication credentials, available <a href=":link" target="_blank">here</a>.', [':link' => 'https://studio.brightcove.com/products/videocloud/admin/oauthsettings']),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->getClientID(),
      '#required' => TRUE,
    );

    $form['secret_key'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Brightcove API Secret Key'),
      '#description' => $this->t('The Secret Key associated with the Client ID above, only visible once when Registering New Application.
'),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->getSecretKey(),
      '#required' => TRUE,
    );

    $form['secret_key_provider'] = [
      '#type' => 'select',
      '#title' => $this->t('Secret key provider'),
      '#options' => [
        'config' => $this->t('Configuration'),
        'file' => $this->t('File'),
      ],
    ];

    $form['secret_key_folder'] = [
      '#type' => 'textfield',
      '#title' => $this->t('Secret key folder'),
      '#description' => $this->t("Provide only the folder's location, the file will be created automatically."),
      '#default_value' => 'private://keys',
      '#states' => [
        'visible' => [
          ':input[name="secret_key_provider"]' => ['value' => 'file'],
        ],
      ],
    ];

    // Set default values for the key fields.
    $key = $this->key_repository->getKey($brightcove_api_client->getSecretKeyId());
    if (!empty($key)) {
      $provider = $key->getKeyProvider();

      $form['secret_key_provider']['#default_value'] = $provider->getPluginId();

      if ($provider->getPluginId() == 'file') {
        $config = $provider->getConfiguration();
        $form['secret_key_folder']['#default_value'] = substr($config['file_location'], 0, strrpos(str_replace('\\', '/', $config['file_location']), '/'));
      }
    }

    $form['account_id'] = array(
      '#type' => 'textfield',
      '#title' => $this->t('Brightcove Account ID'),
      '#description' => $this->t('The number of one of the Brightcove accounts "selected for authorization" with the API Client above.'),
      '#maxlength' => 255,
      '#default_value' => $brightcove_api_client->getAccountID(),
      '#required' => TRUE,
    );

    $form['default_player'] = array(
      '#type' => 'select',
      '#title' => $this->t('Default player'),
      '#options' => BrightcovePlayer::getList($brightcove_api_client->id()),
      '#default_value' => $brightcove_api_client->getDefaultPlayer() ? $brightcove_api_client->getDefaultPlayer() : BrightcoveAPIClient::DEFAULT_PLAYER,
      '#required' => TRUE,
    );
    if ($brightcove_api_client->isNew()) {
      $form['default_player']['#description'] = t('The rest of the players will be available after saving.');
    }

    // Count BrightcoveAPIClients.
    $api_clients_number = $this->query_factory->get('brightcove_api_client')
      ->count()->execute();
    $form['default_client'] = [
      '#type' => 'checkbox',
      '#title' => $this->t('Default API Client'),
      '#description' => $this->t('Enable to make this the default API Client.'),
      '#default_value' => $api_clients_number == 0 || ($this->config->get('defaultAPIClient') == $brightcove_api_client->id()),
    ];

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function validateForm(array &$form, FormStateInterface $form_state) {
    try {
      // Try to authorize client and save values on success.
      $client = Client::authorize($form_state->getValue('client_id'), $form_state->getValue('secret_key'));

      // Test account ID.
      $cms = new CMS($client, $form_state->getValue('account_id'));
      $cms->countVideos();

      $this->key_value_expirable_store->setWithExpire($form_state->getValue('id'),  $client->getAccessToken(), intval($client->getExpiresIn()) - 30);
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
  public function submitForm(array &$form, FormStateInterface $form_state) {
    /** @var \Drupal\brightcove\Entity\BrightcoveAPIClient $entity */
    $entity = $this->entity;

    $client = new Client($this->key_value_expirable_store->get($form_state->getValue('id')));
    $cms = new CMS($client, $form_state->getValue('account_id'));

    /** @var \Brightcove\Object\CustomFields $video_fields */
    $video_fields = $cms->getVideoFields();
    $entity->setMaxCustomFields($video_fields->getMaxCustomFields());

    foreach ($video_fields->getCustomFields() as $custom_field) {
      // Create queue item.
      $this->custom_field_queue->createItem([
        'api_client_id' => $this->entity->id(),
        'custom_field' => $custom_field,
      ]);
    }

    parent::submitForm($form, $form_state);

    if ($entity->isNew()) {
      // Get Players the first time when the API client is being saved.
      $pm = new PM($client, $form_state->getValue('account_id'));
      $player_list = $pm->listPlayers();
      $players = [];
      if (!empty($player_list) && !empty($player_list->getItems())) {
        $players = $player_list->getItems();
      }
      foreach ($players as $player) {
        // Create queue item.
        $this->player_queue->createItem([
          'api_client_id' => $this->entity->id(),
          'player' => $player,
        ]);
      }

      // Get Custom fields the first time when the API client is being saved.
      /** @var \Brightcove\Object\CustomFields $video_fields */
      $video_fields = $cms->getVideoFields();
      foreach ($video_fields->getCustomFields() as $custom_field) {
        // Create queue item.
        $this->custom_field_queue->createItem([
          'api_client_id' => $this->entity->id(),
          'custom_field' => $custom_field,
        ]);
      }
    }
  }

  /**
   * {@inheritdoc}
   */
  public function save(array $form, FormStateInterface $form_state) {
    /** @var \Drupal\brightcove\Entity\BrightcoveAPIClient $brightcove_api_client */
    $brightcove_api_client = $this->entity;

    try {
      $brightcove_api_client->setSecretKey($form_state->getValue('secret_key'), [
        'key_provider' => $form_state->getValue('secret_key_provider'),
        'key_folder' => $form_state->getValue('secret_key_folder'),
      ]);
      $status = $brightcove_api_client->save();
    }
    catch (\Exception $e) {
      watchdog_exception('brightcove', $e, 'Failed to create the Brightcove API Client entity.');
      drupal_set_message(t('Failed to create the Brightcove Video entity: %message', array(
        '%message' => $e->getMessage(),
      )), 'error');

      $form_state->setRebuild(true);
      return;
    }

    switch ($status) {
      case SAVED_NEW:
        drupal_set_message($this->t('Created the %label Brightcove API Client.', [
          '%label' => $brightcove_api_client->label(),
        ]));

        // Initialize batch.
        batch_set([
          'operations' => [
            [[BrightcoveUtil::class, 'runQueue'], ['brightcove_player_queue_worker']],
            [[BrightcoveUtil::class, 'runQueue'], ['brightcove_custom_field_queue_worker']],
          ],
        ]);
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
