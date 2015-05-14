<?php
/**
 * @file
 * Brightcove video related functions.
 */

/**
 * Entity controller class for Brightcove video.
 *
 * brightcove_video entities and videos in the Britghtcove cloud can (and will)
 * get out of sync in the sense that there can be entities that are not in the
 * Brightcove cloud and there can be videos in the cloud we don't have an
 * entity for.
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
    elseif ($video = brightcove_load_video($values['video_id'], $values['client'])) {
      $values['video'] = $video;
      $values['account_id'] = $values['client']->account_id;
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
            if ($video = brightcove_load_video($entity->video_id, $client)) {
              $entity->video = $video;
              $entity->client = $client;
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
          $entity->video = brightcove_load_video($entity->video_id, $entity->client);
        }
      }
    }
    return $entities;
  }

  /**
   * @inheritdoc
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    $entity->video = brightcove_save_video($entity->video, $entity->client);
    return parent::save($entity, $transaction);
  }
}

/**
 * Saves a BrightcoveVideo.
 *
 * @param BrightcoveVideo $video
 * @param Entity $client
 *
 * @return BrightcoveVideo
 */
function brightcove_save_video(BrightcoveVideo $video, Entity $client) {
  list($cms,) = _brightcove_create_classes($client);
  $video_id = $video->getId();
  if (!empty($video_id)) {
    try {
      return $cms->updateVideo($video);
    }
    catch (BrightcoveAPIException $e) {
      watchdog('brightcove', 'Updating BrightcoveVideo failed with message "@e".', ['@e' => $e->getMessage()], WATCHDOG_ERROR);
    }
  }
  else {
    try {
      return $cms->createVideo($video);
    }
    catch (BrightcoveAPIException $e) {
      watchdog('brightcove', 'Creating BrightcoveVideo failed with message "@e".', ['@e' => $e->getMessage()], WATCHDOG_ERROR);
    }
  }
}

function brightcove_video_form($form, &$form_state, $video_entity) {

}

/**
 * Save a video entity.
 *
 * This call will take care of creating/updating the BrightcoveVideo at
 * $video->video as well.
 *
 * @param Entity $video
 */
function brightcove_video_save(Entity $video) {
  return entity_get_controller('brightcove_video')->save($video);
}

/**
 * Load a video entity by bvid.
 *
 * This will try to guess a client to use and puts it under the returned
 * entity object's client property. If you need to use another client, feel
 * free to change it in a subsequent call.
 *
 * @param string $bvid
 *  The bvid of the video entity.
 *
 *  @return Entity
 *   The video entity.
 */
function brightcove_video_load($bvid) {
  return entity_load_single('brightcove_video', $bvid);
}

/**
 * Load a video entity by BrightcoveVideo->getId().
 *
 * This very smart function loads a brightcove_video entity by video_id
 * or if it does not exist, it creates it.
 *
 * @param string $video_id
 * @param Entity $client
 *  The brightcove_client entity object.
 *
 * @return Entity|boolean
 *  The brightcove_video entity object or FALSE on failure.
 */
function brightcove_video_load_by_video_id($video_id, $client) {
  $query = new EntityFieldQuery();
  $query->entityCondition('entity_type', 'brightcove_video')
    ->propertyCondition('video_id', $video_id)
    ->addMetaData('account', user_load(1));
  $result = $query->execute();
  if (isset($result['brightcove_video'])) {
    $bvids = array_keys($result['brightcove_video']);
    $bvid = reset($bvids);
    $video = brightcove_video_load($bvid);
    if (!empty($client)) {
      $video->client = $client;
    }
  }
  else {
    $values = [
      'type' => 'brightcove_video',
      'video_id' => $video_id,
      'client' => $client,
    ];
    $video = entity_get_controller('brightcove_video')->create($values);
  }
  return $video;
}
