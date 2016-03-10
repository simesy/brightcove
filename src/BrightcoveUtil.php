<?php
/**
 * @file
 * Contains \Drupal\brightcove\BrightcoveUtil.
 */

namespace Drupal\brightcove;

use Brightcove\API\CMS;
use Brightcove\API\DI;
use Drupal\brightcove\Entity\BrightcoveAPIClient;
use Drupal\brightcove\Entity\BrightcoveCMSEntity;
use Drupal\brightcove\Entity\BrightcovePlaylist;
use Drupal\brightcove\Entity\BrightcoveVideo;
use Drupal\Core\Url;

/**
 * Utility class for Brightcove.
 */
class BrightcoveUtil {
  /**
   * Array of BrightcoveAPIClient objects.
   *
   * @var \Drupal\brightcove\Entity\BrightcoveAPIClient[]
   */
  protected static $api_clients = [];

  /**
   * Array of CMS objects.
   *
   * @var \Brightcove\API\CMS[]
   */
  protected static $cms_apis = [];

  /**
   * Array of DI objects.
   *
   * @var \Brightcove\API\DI[]
   */
  protected static $di_apis = [];

  /**
   * Convert Brightcove date make it digestible by Drupal.
   *
   * @param string $brightcove_date
   *   Brightcove date format.
   *
   * @return string|NULL
   *   Drupal date format.
   */
  public static function convertDate($brightcove_date) {
    if (empty($brightcove_date)) {
      return NULL;
    }

    return preg_replace('/\.\d{3}Z$/i', '', $brightcove_date);
  }

  /**
   * Gets BrightcoveAPIClient entity.
   *
   * @param $entity_id
   *   The entity ID of the BrightcoveAPIClient.
   *
   * @return \Drupal\brightcove\Entity\BrightcoveAPIClient
   *   Loaded BrightcoveAPIClient object.
   */
  public static function getAPIClient($entity_id) {
    // Load BrightcoveAPIClient if it wasn't already.
    if (!isset(self::$api_clients[$entity_id])) {
      self::$api_clients[$entity_id] = BrightcoveAPIClient::load($entity_id);
    }

    return self::$api_clients[$entity_id];
  }

  /**
   * Gets Brightcove client.
   *
   * @param int $entity_id
   *   BrightcoveAPIClient entity ID.
   *
   * @return \Brightcove\API\Client
   *   Loaded Brightcove client.
   */
  public static function getClient($entity_id) {
    $api_client = self::getAPIClient($entity_id);
    return $api_client->getClient();
  }

  /**
   * Gets Brightcove CMS API.
   *
   * @param int $entity_id
   *   BrightcoveAPIClient entity ID.
   *
   * @return \Brightcove\API\CMS
   *   Initialized Brightcove CMS API.
   */
  public static function getCMSAPI($entity_id) {
    // Create new \Brightcove\API\CMS object if it is not exists yet.
    if (!isset(self::$cms_apis[$entity_id])) {
      $client = self::getClient($entity_id);
      self::$cms_apis[$entity_id] = new CMS($client, self::$api_clients[$entity_id]->getAccountID());
    }

    return self::$cms_apis[$entity_id];
  }

  /**
   * Gets Brightcove DI API.
   *
   * @param int $entity_id
   *   BrightcoveAPIClient entity ID.
   *
   * @return \Brightcove\API\DI
   *   Initialized Brightcove CMS API.
   */
  public static function getDIAPI($entity_id) {
    // Create new \Brightcove\API\CMS object if it is not exists yet.
    if (!isset(self::$di_apis[$entity_id])) {
      $client = self::getClient($entity_id);
      self::$di_apis[$entity_id] = new DI($client, self::$api_clients[$entity_id]->getAccountID());
    }

    return self::$di_apis[$entity_id];
  }

  /**
   * Check updated version of the CMS entity.
   *
   * If the checked CMS entity has a newer version of it on Brightcove then
   * show a message about it with a link to be able to update the local
   * version.
   *
   * @param \Drupal\brightcove\Entity\BrightcoveCMSEntity $entity
   */
  public static function checkUpdatedVersion(BrightcoveCMSEntity $entity) {
    /** @var \Brightcove\API\CMS $cms */
    $cms = self::getCMSAPI($entity->getAPIClient());

    $entity_type = '';
    if ($entity instanceof BrightcoveVideo) {
      $cms_entity = $cms->getVideo($entity->getVideoId());
      $entity_type = 'video';
    }
    else if ($entity instanceof BrightcovePlaylist) {
      $cms_entity = $cms->getPlaylist($entity->getPlaylistId());
      $entity_type = 'playlist';
    }

    if (isset($cms_entity)) {
      if ($entity->getChangedTime() < strtotime($cms_entity->getUpdatedAt())) {
        $url = Url::fromRoute("brightcove_manual_update_{$entity_type}", ['entity_id' => $entity->id()], ['query' => ['token' => \Drupal::getContainer()->get('csrf_token')->get("brightcove_{$entity_type}/{$entity->id()}/update")]]);

        drupal_set_message(t("There is a newer version of this :type on Brightcove, you may want to <a href=':url'>update the local version</a> before editing it.", [
          ':type' => $entity_type,
          ':url' => $url->toString(),
        ]), 'warning');
      }
    }
  }
}
