<?php

namespace Drupal\brightcove\Access;

use Drupal\Core\Access\AccessResult;
use Drupal\Core\Database\Database;
use Drupal\Core\Routing\Access\AccessInterface;
use Drupal\Core\Routing\RouteMatchInterface;

class CSRFCallbackAccessCheck implements AccessInterface {
  /**
   * Custom access callback.
   *
   * @param \Drupal\Core\Routing\RouteMatchInterface $route_match
   *   RouterMatch object.
   *
   * @return \Drupal\Core\Access\AccessResult
   *   Access allowed only if the token is exists and did not expired.
   */
  public function access(RouteMatchInterface $route_match) {
    $token = $route_match->getParameter('token');

    try {
      $result = Database::getConnection()->select('brightcove_callback', 'b')
        ->fields('b', ['token'])
        ->condition('token', $token)
        ->condition('expires', REQUEST_TIME, '>')
        ->execute()
        ->fetchAssoc();
    }
    catch (\Exception $e) {
      watchdog_exception('brightcove', $e);
    }

    return AccessResult::allowedIf(isset($result) && !empty($result['token']));
  }
}