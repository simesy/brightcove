<?php
/**
 * @file
 * Brightcove video related functions.
 */

/**
 * Entity controller class for Brightcove video.
 */
class BrightcoveVideoEntityController extends EntityAPIController {

 /**
   * @inheritdoc
   */
  public function create(array $values = array()) {
    // Add is_new property if it is not set.
    $values += array('is_new' => TRUE);
    if (empty($values['client']) || !($values['client'] instanceof Entity)) {
      throw new Exception(t('Cannot create a brightcove_video entity without a client.'));
    }
    if (empty($values['video_id'])) {
      $values['video'] = new BrightcoveVideo;
      $values['video_id'] = NULL;
    }
    elseif ($video = brightcove_video_load($values['video_id'], $values['client'])) {
      $video['video'] = $video;
      $video['account_id'] = $values['client']->account_id;
    }
    else {
      throw new Exception(t('Failure to load the video @video with client @client', ['@video' => $values['video_id'], '@client' => $values['client']->bcid]));
    }
    // Add a BrightcoveVideo object to the entity object.
    brightcove_load_lib();
    $video_entity = new Entity($values, $this->entityType);

    return $video_entity;
  }

  /**
   * @inheritdoc
   */
  public function load($ids = array(), $conditions = array()) {
    $entities = parent::load($ids, $conditions);
    // Add the BrightcoveVideo object to each entity object.
    foreach ($entities as $key => $entity) {
      if (empty($entity->video) || !($entity->video instanceof BrightcoveVideo)) {
        if (empty($entity->client) || !($entity->client instanceof Entity)) {
          $clients = brightcove_get_clients_by_account_id($entity->account_id);
          $found_client = FALSE;
          foreach ($clients as $client) {
            if ($video = brightcove_video_load($entity->video_id, $client)) {
              $entity->video = $video;
              $found_client = TRUE;
              break;
            }
          }
          if (!$found_client) {
            $video_id = $entity->video_id;
            unset($entities[$key]);
            watchdog('brightcove', 'Could not load the video @video_id.', ['@video_id' => $video_id], WATCHDOG_WARNING);
          }
        }
        else {
          $entity->video = brightcove_video_load($entity->video_id, $entity->client);
        }
      }
    }
  }

  public function save($entity, DatabaseTransaction $transaction = NULL) {
    parent::save($entity, $transaction);
    $entity->video = brightcove_save_video($entity->video, $entity->client);
  }
}

/**
 *
 * @param BrightcoveVideo $video
 * @param BrightcoveClient $client
 *
 * @return BrightcoveVideo
 */
function brightcove_save_video(BrightcoveVideo $video, BrightcoveClient $client) {
  list($cms,) = _brightcove_create_classes($client);
  $video_id = $video->getId();
  if (!empty($video_id)) {
    try {
      return $cms->updateVideo($video);
    }
    catch (BrightcoveAPIException $e) {
      watchdog('brightcove', 'Updating BrightcoveVideo failed with message "@e".', ['@e' => $e->message], WATCHDOG_ERROR);
    }
  }
  else {
    try {
      return $cms->createVideo($video);
    }
    catch (BrightcoveAPIException $e) {
      watchdog('brightcove', 'Creating BrightcoveVideo failed with message "@e".', ['@e' => $e->message], WATCHDOG_ERROR);
    }
  }
}

function brightcove_video_form($form, &$form_state, $video_entity) {

}
