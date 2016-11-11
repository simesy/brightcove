<?php

/**
 * @file
 * Contains \Drupal\brightcove\Entity\BrightcoveAPIClient.
 */

namespace Drupal\brightcove\Entity;

use Brightcove\API\CMS;
use Brightcove\API\Exception\APIException;
use Drupal\Core\Config\Entity\ConfigEntityBase;
use Drupal\brightcove\BrightcoveAPIClientInterface;
use Brightcove\API\Client;
use Drupal\key\Entity\Key;

/**
 * Defines the Brightcove API Client entity.
 *
 * @ConfigEntityType(
 *   id = "brightcove_api_client",
 *   label = @Translation("Brightcove API Client"),
 *   handlers = {
 *     "list_builder" = "Drupal\brightcove\BrightcoveAPIClientListBuilder",
 *     "form" = {
 *       "add" = "Drupal\brightcove\Form\BrightcoveAPIClientForm",
 *       "edit" = "Drupal\brightcove\Form\BrightcoveAPIClientForm",
 *       "delete" = "Drupal\brightcove\Form\BrightcoveAPIClientDeleteForm"
 *     },
 *   },
 *   config_prefix = "brightcove_api_client",
 *   admin_permission = "administer brightcove configuration",
 *   entity_keys = {
 *     "id" = "id",
 *     "label" = "label",
 *     "uuid" = "uuid"
 *   },
 *   links = {
 *     "canonical" = "/admin/config/media/brightcove_api_client/{brightcove_api_client}",
 *     "add-form" = "/admin/config/media/brightcove_api_client/add",
 *     "edit-form" = "/admin/config/media/brightcove_api_client/{brightcove_api_client}/edit",
 *     "delete-form" = "/admin/config/media/brightcove_api_client/{brightcove_api_client}/delete",
 *     "collection" = "/admin/config/media/brightcove_api_client"
 *   }
 * )
 */
class BrightcoveAPIClient extends ConfigEntityBase implements BrightcoveAPIClientInterface {
  /**
   * The Brightcove API Client ID (Drupal-internal).
   *
   * @var string
   */
  protected $id;

  /**
   * The Brightcove API Client label.
   *
   * @var string
   */
  protected $label;

  /**
   * The Brightcove API Client account ID.
   *
   * @var string
   */
  protected $account_id;

  /**
   * The Brightcove API Client ID.
   *
   * @var string
   */
  protected $client_id;

  /**
   * The Brightcove API Client default player.
   *
   * @var string
   */
  protected $default_player;

  /**
   * The Brightcove API Client secret key.
   *
   * @var string
   */
  protected $secret_key;

  /**
   * The ID for the Secret key in the key repository.
   *
   * @var string
   */
  protected $secret_key_id;

  /**
   * Key provider configuration.
   *
   * @var string
   */
  protected $key_config;

  /**
   * The loaded API client.
   *
   * @var \Brightcove\API\Client
   */
  protected $client;

  /**
   * Client connection status.
   *
   * @var int
   */
  protected $client_status;

  /**
   * Client connection status message.
   *
   * If the connection status is OK, then it's an empty string.
   *
   * @var string
   */
  protected $client_status_message;

  /**
   * Indicate if there was an re-authorization attempt or not.
   *
   * @var bool
   */
  private $re_authorization_tried = FALSE;

  /**
   * Maximum number of Custom fields.
   *
   * @var array
   */
  protected $max_custom_fields;

  /**
   * Expirable key/value store for the client.
   *
   * @var \Drupal\Core\KeyValueStore\KeyValueStoreExpirableInterface
   */
  protected $key_value_expirable_store;

  /**
   * @inheritdoc
   */
  public function getLabel() {
    return $this->label;
  }

  /**
   * @inheritdoc
   */
  public function getAccountID() {
    return $this->account_id;
  }

  /**
   * @inheritdoc
   */
  public function getClientID() {
    return $this->client_id;
  }

  /**
   * @inheritdoc
   */
  public function getDefaultPlayer() {
    return $this->default_player;
  }

  /**
   * @inheritdoc
   */
  public function getSecretKey() {
    return $this->secret_key;
  }

  /**
   * @inheritdoc
   */
  public function getSecretKeyId() {
    // Return empty string instead of NULL if the secret_key_id is not set yet.
    return empty($this->secret_key_id) ? '' : $this->secret_key_id;
  }

  /**
   * @inheritdoc
   */
  public function getClient() {
    $this->authorizeClient();
    return $this->client;
  }

  /**
   * @inheritdoc
   */
  public function getClientStatus() {
    return $this->client_status;
  }

  /**
   * @inheritdoc
   */
  public function getClientStatusMessage() {
    return $this->client_status_message;
  }

  /**
   * @inheritdoc
   */
  public function getAccessToken() {
    return $this->key_value_expirable_store->get($this->client_id, NULL);
  }

  /**
   * {@inheritdoc}
   */
  public function getMaxCustomFields() {
    return $this->max_custom_fields;
  }

  /**
   * @inheritdoc
   */
  public function setLabel($label) {
    $this->label = $label;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setAccountID($account_id) {
    $this->account_id = $account_id;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setClientID($client_id) {
    $this->client_id = $client_id;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setDefaultPlayer($default_player) {
    $this->default_player = $default_player;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setSecretKey($secret_key, array $key_config) {
    $this->secret_key = $secret_key;
    $this->key_config = $key_config;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setSecretKeyId($secret_key_id) {
    $this->secret_key_id = $secret_key_id;
    return $this;
  }

  /**
   * Set Brightcove API client.
   *
   * @param \Brightcove\API\Client $client
   *   The initialized Brightcove API Client.
   *
   * @return $this
   */
  public function setClient(Client $client) {
    $this->client = $client;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setAccessToken($access_token, $expire) {
    $this->key_value_expirable_store->setWithExpire($this->client_id, $access_token, $expire);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function setMaxCustomFields($max_custom_fields) {
    $this->max_custom_fields = $max_custom_fields;
    return $this;
  }

  /**
   * File system helper service.
   *
   * @var \Drupal\Core\File\FileSystem
   */
  protected $fileSystem;

  /**
   * Key Repository.
   *
   * @var \Drupal\key\KeyRepositoryInterface;
   */
  protected $keyRepository;

  /**
   * {@inheritdoc}
   */
  public function __construct(array $values, $entity_type) {
    parent::__construct($values, $entity_type);

    // Not possible to use dependency injection in entities, so the services
    // are getting set here.
    $this->key_value_expirable_store = \Drupal::keyValueExpirable('brightcove_access_token');
    $this->fileSystem = \Drupal::service('file_system');
    $this->keyRepository = \Drupal::service('key.repository');

    $key = !empty($this->getSecretKeyId()) ? $this->keyRepository->getKey($this->getSecretKeyId()) : NULL;
    $this->secret_key = !empty($key) ? $key->getKeyValue() : NULL;
  }

  /**
   * Authorize client with Brightcove API and store client on the entity.
   *
   * @return $this
   * @throws \Brightcove\API\Exception\AuthenticationException
   */
  public function authorizeClient() {
    try {
      // Use the got access token while it is not expired.
      if ($this->key_value_expirable_store->has($this->client_id)) {
        // Create new client.
        $this->setClient(new Client($this->getAccessToken()));
      }
      // Otherwise get a new access token.
      else {
        $client = Client::authorize($this->getClientID(), $this->getSecretKey());

        // Update access information. This will ensure that in the current
        // session we will get the correct access data.
        // Set token expire time in seconds and subtract the default php
        // max_execution_time from it.
        // We have to use the default php max_execution_time because if we
        // would get the value from ini_get('max_execution_time'), then it
        // could be larger than the Brightcove's expire date causing to always
        // get a new access token.
        $this->setAccessToken($client->getAccessToken(), intval($client->getExpiresIn()) - 30);
        $this->save();

        // Create new client.
        $this->setClient(new Client($this->getAccessToken()));
      }

      // Test account ID.
      $cms = new CMS($this->client, $this->account_id);
      $cms->countVideos();

      // If client authentication was successful store it's state on the
      // entity.
      $this->client_status = self::CLIENT_OK;
    }
    catch (\Exception $e) {
      if ($e instanceof APIException) {
        // If we got an unauthorized error, try to re-authorize the client
        // only once.
        if ($e->getCode() == 401 && !$this->re_authorization_tried) {
          $this->re_authorization_tried = TRUE;
          $this->authorizeClient();
        }
      }

      // Store an error status and message on the entity if there was an
      // error.
      $this->client_status = self::CLIENT_ERROR;
      $this->client_status_message = $e->getMessage();
    }

    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function save() {
    // Save secret key if it's changed.
    if (!empty($this->key_config)) {
      $key_id = "brightcove_secret_{$this->id()}";
      $key_file_location = "{$this->key_config['key_folder']}/{$key_id}.key";

      // Store secret key in file and create a new one if not exists.
      if ($this->key_config['key_provider'] == 'file') {
        // Try to create the folder.
        $folder = $this->fileSystem->realpath($this->key_config['key_folder']);
        if (!empty($folder) && !is_dir($folder) && $this->fileSystem->mkdir($folder, NULL, TRUE));
        else if (!is_writable($this->key_config['key_folder'])) {
          throw new \Exception(strtr('Unable to create the secret key file. The @folder folder is not writable.', ['@folder' => $this->key_config['key_folder']]));
        }

        // Try to create the secret key file.
        $secret_key_file = $this->fileSystem->realpath($key_file_location);
        if (file_put_contents($secret_key_file, $this->secret_key) === FALSE) {
          throw new \Exception(strtr('Unable to create the secret key file: @file', ['@file' => $secret_key_file]));
        }
      }

      // Key configuration.
      $config = [
        'key_provider' => $this->key_config['key_provider'],
      ];
      if ($this->key_config['key_provider'] == 'file') {
        $config['key_provider_settings'] = [
          'file_location' => $key_file_location,
        ];
      }
      else {
        $config['key_provider_settings'] = [
          'key_value' => $this->secret_key,
        ];
      }

      // Create new key in the key repository if not exists.
      if (($key = $this->keyRepository->getKey($this->getSecretKeyId())) == NULL) {
        $config += [
          'id' => $key_id,
          'label' => "Brightcove ({$this->getLabel()})",
          'description' => 'Brightcove API Secret key.',
          'key_type' => 'authentication',
        ];

        $key = new Key($config, 'key');
        $key->save();

        $this->setSecretKeyId($key->id());
      }
      // Update the existing key settings.
      else {
        // Clean-up file if we switched from file provider to configuration or if
        // the file's location is changed.
        $provider = $key->getKeyProvider();
        $key_provider_config = $provider->getConfiguration();
        if ($provider->getPluginId() == 'file' && (($this->key_config['key_provider'] == 'config' && is_file($this->fileSystem->realpath($key_provider_config['file_location']))) || ($this->key_config['key_provider'] == $provider->getPluginId() && $key_provider_config['file_location'] != $key_file_location && is_file($this->fileSystem->realpath($key_provider_config['file_location']))))) {
          $this->fileSystem->unlink($key_provider_config['file_location']);
        }

        foreach ($config as $property_name => $value) {
          $key->set($property_name, $value);
        }
        $key->save();
      }
    }

    return parent::save();
  }

  /**
   * {@inheritdoc}
   */
  public function delete() {
    parent::delete();

    $key = $this->keyRepository->getKey($this->getSecretKeyId());
    if (!empty($key)) {
      $provider = $key->getKeyProvider();
      $config = $provider->getConfiguration();

      // Delete secret key file.
      if ($provider->getPluginId() == 'file' && is_file($this->fileSystem->realpath($config['file_location']))) {
        $this->fileSystem->unlink($config['file_location']);
      }

      // Delete key provider.
      $key->delete();
    }
  }
}
