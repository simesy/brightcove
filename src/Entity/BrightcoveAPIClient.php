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
use Brightcove\API\Exception\AuthenticationException;

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
   * Access token.
   *
   * @var string
   */
  protected $access_token;

  /**
   * Access token expire date
   *
   * @var int
   */
  protected $access_token_expire_date;

  /**
   * Indicate if there was an re-authorization attempt or not.
   *
   * @var bool
   */
  private $re_authorization_tried = FALSE;

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
    return $this->access_token;
  }

  /**
   * @inheritdoc
   */
  public function getAccessTokenExpireDate() {
    return $this->access_token_expire_date;
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
    // This is intentional: currently only the 'default' player is supported.
    $this->default_player = 'default';
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setSecretKey($secret_key) {
    $this->secret_key = $secret_key;
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
  public function setAccessToken($access_token) {
    $this->access_token = $access_token;
    return $this;
  }

  /**
   * @inheritdoc
   */
  public function setAccessTokenExpireDate($expire_date) {
    $this->access_token_expire_date = $expire_date;
    return $this;
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
      if (REQUEST_TIME < $this->access_token_expire_date) {
        // Create new client.
        $this->setClient(new Client($this->access_token));
      }
      // Otherwise get a new access token.
      else {
        $json = self::authorize($this->client_id, $this->secret_key);

        // Update access information. This will ensure that in the current
        // session we will get the correct access data.
        $this->setAccessToken($json['access_token']);
        // Set token expire date and subtract the default php
        // max_execution_time from it.
        // We have to use the default php max_execution_time because if we
        // would get the value from ini_get('max_execution_time'), then it
        // could be larger than the Brightcove's expire date causing to always
        // get a new access token.
        $this->setAccessTokenExpireDate(REQUEST_TIME + intval($json['expires_in'] - 30));
        $this->save();

        // Create new client.
        $this->setClient(new Client($this->access_token));
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
   * Authorize client with Brightcove API.
   *
   * @param $client_id
   *   Brightcove client ID.
   * @param $secret_key
   *   Brightcove secret key.
   *
   * @return \Brightcove\API\Client
   *   Authorized Brightcove client.
   *
   * @throws \Brightcove\API\Exception\AuthenticationException
   */
  public static function authorize($client_id, $secret_key) {
    // Copied from Brightcove API to be able to save access_token until the
    // expire date is not passed.
    list($code, $response) = Client::HTTPRequest('POST', 'https://oauth.brightcove.com/v3/access_token',
      array('Content-Type: application/x-www-form-urlencoded'),
      'grant_type=client_credentials',
      function ($ch) use ($client_id, $secret_key) {
        curl_setopt($ch, CURLOPT_USERPWD, "{$client_id}:{$secret_key}");
      });

    if ($code !== 200) {
      throw new AuthenticationException(t("Can't authenticate with the given credentials."));
    }

    $json = json_decode($response, TRUE);
    if ($json['token_type'] !== 'Bearer') {
      throw new AuthenticationException(t('Unsupported token type: @token_type', array('@token_type' => $json['token_type'])));
    }

    return $json;
  }

}
