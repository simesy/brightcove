<?php
/**
 * Created by PhpStorm.
 * User: yce
 * Date: 11/05/15
 * Time: 14:05
 */

class BrightcoveVideoEntityController extends EntityAPIController {
  /**
   * Overrides EntityAPIController::save().
   */
  public function save($video, DatabaseTransaction $transaction = NULL) {
    return parent::save($video, $transaction);
  }
}
