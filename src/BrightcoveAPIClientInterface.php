<?php

/**
 * @file
 * Contains \Drupal\brightcove\BrightcoveAPIClientInterface.
 */

namespace Drupal\brightcove;

use Drupal\Core\Config\Entity\ConfigEntityInterface;

/**
 * Provides an interface for defining Brightcove API Client entities.
 */
interface BrightcoveAPIClientInterface extends ConfigEntityInterface {

  /**
   * Indicates the default player for any API Client.
   */
  const DEFAULT_PLAYER = 'default';

  /**
   * Indicates that the connection to the API was not successful.
   */
  const CLIENT_ERROR = 0;

  /**
   * Indicates that the connection to the API was successful.
   */
  const CLIENT_OK = 1;

  /**
   * Returns the API Client label.
   *
   * @return string
   *   The label for this API Client.
   */
  public function getLabel();

  /**
   * Returns the API Client account ID.
   *
   * @return string
   *   The account ID for this API Client.
   */
  public function getAccountID();

  /**
   * Returns the API Client ID.
   *
   * @return string
   *   The client ID for this API Client.
   */
  public function getClientID();

  /**
   * Returns the API Client default player.
   *
   * @return string
   *   The default player for this API Client.
   */
  public function getDefaultPlayer();

  /**
   * Returns the API Client secret key.
   *
   * @return string
   *   The secret key for this API Client.
   */
  public function getSecretKey();

  /**
   * Returns the loaded API client.
   *
   * @return \Brightcove\API\Client
   *   Loaded API client.
   */
  public function getClient();

  /**
   * Returns the connection status.
   *
   * @return int
   *   Possible values:
   *     - CLIENT_OK
   *     - CLIENT_ERROR
   */
  public function getClientStatus();

  /**
   * Returns the connection status message.
   *
   * @return string
   *   The connection status message.
   */
  public function getClientStatusMessage();

  /**
   * Returns access token.
   *
   * @return string
   *   The access token.
   */
  public function getAccessToken();

  /**
   * Returns access token expire date.
   *
   * @return int
   *   The access token expire date.
   */
  public function getAccessTokenExpireDate();

  /**
   * Sets the API Client label.
   *
   * @param string $label
   *   The desired label.
   *
   * @return $this
   */
  public function setLabel($label);

  /**
   * Sets the API Client account ID.
   *
   * @param string $account_id
   *   The desired account ID.
   *
   * @return $this
   */
  public function setAccountID($account_id);

  /**
   * Sets the API Client ID.
   *
   * @param string $client_id
   *   The desired client ID.
   *
   * @return $this
   */
  public function setClientID($client_id);

  /**
   * Sets the API Client default player.
   *
   * @param string $default_player
   *   The desired default player.
   *
   * @return $this
   */
  public function setDefaultPlayer($default_player);

  /**
   * Sets the API Client secret key.
   *
   * @param string $secret_key
   *   The desired secret key.
   *
   * @return $this
   */
  public function setSecretKey($secret_key);

  /**
   * Sets access token.
   *
   * @param string $access_token
   *   The access token.
   *
   * @return $this
   */
  public function setAccessToken($access_token);

  /**
   * Sets access token expire date.
   *
   * @param int $expire_date
   *   The date which on the access token will expire.
   *
   * @return $this
   */
  public function setAccessTokenExpireDate($expire_date);
}
