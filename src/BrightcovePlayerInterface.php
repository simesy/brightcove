<?php
/**
 * @file
 * Contains \Drupal\brightcove\BrightcovePlayerInterface.
 */
namespace Drupal\brightcove;

/**
 * Provides an interface for defining Brightcove Player.
 *
 * @ingroup brightcove
 */
interface BrightcovePlayerInterface {
  /**
   * Returns the Brightcove Player ID.
   *
   * @return string
   *   The Brightcove Player ID (not the entity's).
   */
  public function getPlayerId();

  /**
   * Sets The Brightcove Player ID.
   *
   * @param string $player_id
   *   The Brightcove Player ID (not the entity's).
   *
   * @return \Drupal\brightcove\BrightcovePlayerInterface
   *   The called Brightcove Player.
   */
  public function setPlayerId($player_id);
}