<?php

/**
 * @file
 * Contains \Drupal\brightcove\Entity\BrightcovePlaylist.
 */

namespace Drupal\brightcove\Entity;

use Brightcove\Object\Playlist;
use Drupal\brightcove\BrightcoveUtil;
use Drupal\Core\Entity\EntityStorageInterface;
use Drupal\Core\Field\BaseFieldDefinition;
use Drupal\Core\Entity\EntityTypeInterface;
use Drupal\brightcove\BrightcovePlaylistInterface;
use Drupal\Core\Field\FieldStorageDefinitionInterface;

/**
 * Defines the Brightcove Playlist.
 *
 * @ingroup brightcove
 *
 * @ContentEntityType(
 *   id = "brightcove_playlist",
 *   label = @Translation("Brightcove Playlist"),
 *   handlers = {
 *     "view_builder" = "Drupal\Core\Entity\EntityViewBuilder",
 *     "list_builder" = "Drupal\brightcove\BrightcovePlaylistListBuilder",
 *     "views_data" = "Drupal\brightcove\Entity\BrightcovePlaylistViewsData",
 *
 *     "form" = {
 *       "default" = "Drupal\brightcove\Form\BrightcovePlaylistForm",
 *       "add" = "Drupal\brightcove\Form\BrightcovePlaylistForm",
 *       "edit" = "Drupal\brightcove\Form\BrightcovePlaylistForm",
 *       "delete" = "Drupal\Core\Entity\ContentEntityDeleteForm",
 *     },
 *     "access" = "Drupal\brightcove\Access\BrightcovePlaylistAccessControlHandler",
 *     "route_provider" = {
 *       "html" = "Drupal\brightcove\BrightcovePlaylistHtmlRouteProvider",
 *     },
 *   },
 *   base_table = "brightcove_playlist",
 *   admin_permission = "administer brightcove playlists",
 *   entity_keys = {
 *     "id" = "bcplid",
 *     "label" = "name",
 *     "uuid" = "uuid",
 *     "uid" = "uid",
 *     "langcode" = "langcode",
 *     "status" = "status",
 *   },
 *   links = {
 *     "canonical" = "/brightcove_playlist/{brightcove_playlist}",
 *     "add-form" = "/brightcove_playlist/add",
 *     "edit-form" = "/brightcove_playlist/{brightcove_playlist}/edit",
 *     "delete-form" = "/brightcove_playlist/{brightcove_playlist}/delete",
 *     "collection" = "/admin/content/brightcove_playlist",
 *   },
 *   field_ui_base_route = "brightcove_playlist.settings"
 * )
 */
class BrightcovePlaylist extends BrightcoveVideoPlaylistCMSEntity implements BrightcovePlaylistInterface {
  /**
   * Indicates that the playlist type is manual.
   */
  const TYPE_MANUAL = 'EXPLICIT';

  /**
   * Indicates that the playlist type is smart.
   *
   * TODO: Add support for other types of "smart" playlists.
   *
   * @see http://docs.brightcove.com/en/video-cloud/cms-api/references/playlist-fields-reference.html
   */
  const TYPE_SMART = 'ALPHABETICAL';

  /**
   * {@inheritdoc}
   */
  public function getType() {
    return $this->get('type')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function setType($type) {
    switch ($type) {
      case self::TYPE_MANUAL:
        // Intentionally no break here.
      case self::TYPE_SMART:
        $this->set('type', $type);
        break;

      default:
        throw new \InvalidArgumentException('Invalid Brightcove Playlist type');
        break;
    }
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function isFavorite() {
    return (bool) $this->get('favorite')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function getPlaylistId() {
    return $this->get('playlist_id')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function setPlaylistId($playlist_id) {
    $this->set('playlist_id', $playlist_id);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getSearch() {
    return $this->get('search')->value;
  }

  /**
   * {@inheritdoc}
   */
  public function setSearch($search) {
    $this->set('search', $search);
    return $this;
  }

  /**
   * {@inheritdoc}
   */
  public function getVideos() {
    return $this->get('videos')->getValue();
  }

  /**
   * {@inheritdoc}
   */
  public function setVideos($videos) {
    $this->set('videos', $videos);
    return $this;
  }

  /**
   * {@inheritdoc}
   *
   * @param bool $upload
   *   Whether to upload the video to Brightcove or not.
   */
  public function save($upload = FALSE) {
    // Check if it will be a new entity or an existing one being updated.
    $status = $this->id() ? SAVED_UPDATED : SAVED_NEW;

    // Make sure that preSave runs before any modification is made for the
    // entity.
    $saved = parent::save();

    if ($upload) {
      $cms = BrightcoveUtil::getCMSAPI($this->getAPIClient());

      // Setup playlist object and set minimum required values.
      $playlist = new Playlist();
      $playlist->setName($this->getName());

      // Save or update type if needed.
      if ($this->isFieldChanged('type')) {
        $playlist->setType($this->getType());
      }

      // Save or update description if needed.
      if ($this->isFieldChanged('description')) {
        $playlist->setDescription($this->getDescription());
      }

      // Save or update reference ID if needed.
      if ($this->isFieldChanged('reference_id')) {
        $playlist->setReferenceId($this->getReferenceID());
      }

      // Save or update search if needed.
      if ($this->isFieldChanged('search')) {
        $playlist->setSearch($this->getSearch());
      }

      // Save or update videos list if needed.
      if ($this->isFieldChanged('videos')) {
        $video_entities = $this->getVideos();
        $videos = [];
        foreach ($video_entities as $video) {
          $videos[] = BrightcoveVideo::load($video['target_id'])->getVideoId();
        }

        $playlist->setVideoIds($videos);
      }

      // Create or update a playlist.
      switch ($status) {
        case SAVED_NEW:
          // Create new playlist on Brightcove.
          $saved_playlist = $cms->createPlaylist($playlist);

          // Set the rest of the fields on BrightcoveVideo entity.
          $this->setPlaylistId($saved_playlist->getId());
          $this->setCreatedTime(strtotime($saved_playlist->getCreatedAt()));
          break;

        case SAVED_UPDATED:
          // Set playlist ID.
          $playlist->setId($this->getPlaylistId());

          // Update playlist.
          $saved_playlist = $cms->updatePlaylist($playlist);
          break;
      }

      // Update changed time and playlist entity with the video ID.
      if (isset($saved_playlist)) {
        $this->setChangedTime(strtotime($saved_playlist->getUpdatedAt()));

        // Save the entity again to save some new values which are only
        // available after creating/updating the playlist on Brightcove.
        // Also don't change the save state to show the correct message when
        // the entity is created or updated.
        parent::save();
      }
    }

    return $saved;
  }

  /**
   * {@inheritdoc}
   *
   * @param bool $local_only
   *   Whether to delete the local version only or both local and Brightcove
   *   versions.
   */
  public function delete($local_only = FALSE) {
    // Delete playlist from Brightcove.
    if (!$this->isNew() && !$local_only) {
      $cms = BrightcoveUtil::getCMSAPI($this->getAPIClient());
      $cms->deletePlaylist($this->getPlaylistId());
    }

    parent::delete();
  }

  /**
   * {@inheritdoc}
   */
  public static function baseFieldDefinitions(EntityTypeInterface $entity_type) {
    // Set weights based on the real order of the fields.
    $weight = -30;

    /**
     * Drupal-specific fields first.
     *
     * bcplid - Brightcove Playlist ID (Drupal-internal).
     * uuid - UUID.
     * - "Playlist type" comes here, but that's a Brightcove-specific field.
     * - Title comes here, but that's the "Name" field from Brightcove.
     * langcode - Language.
     * api_client - Entityreference to BrightcoveAPIClient.
     * - Brightcove fields come here.
     * uid - Author.
     * created - Posted.
     * changed - Last modified.
     */
    $fields['bcplid'] = BaseFieldDefinition::create('integer')
      ->setLabel(t('ID'))
      ->setDescription(t('The Drupal entity ID of the Brightcove Playlist.'))
      ->setReadOnly(TRUE);

    $fields['uuid'] = BaseFieldDefinition::create('uuid')
      ->setLabel(t('UUID'))
      ->setDescription(t('The Brightcove Playlist UUID.'))
      ->setReadOnly(TRUE);

    $fields['type'] = BaseFieldDefinition::create('list_string')
      ->setLabel(t('Playlist Type'))
//      ->setRevisionable(TRUE)
      ->setRequired(TRUE)
      ->setDefaultValue(BrightcovePlaylist::TYPE_MANUAL)
      ->setSetting('allowed_values', array(
        BrightcovePlaylist::TYPE_MANUAL => 'Manual: Add videos manually',
        BrightcovePlaylist::TYPE_SMART => 'Smart: Add videos automatically based on tags',
      ))
      ->setDisplayOptions('form', array(
        'type' => 'options_buttons',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'list_default',
        'label' => 'inline',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE);

    $fields['name'] = BaseFieldDefinition::create('string')
      ->setLabel(t('Playlist name'))
      ->setDescription(t('The name of the Brightcove Playlist.'))
//      ->setRevisionable(TRUE)
      ->setRequired(TRUE)
      ->setSettings(array(
        'max_length' => 250,
        'text_processing' => 0,
      ))
      ->setDisplayOptions('form', array(
        'type' => 'string_textfield',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'string',
        'label' => 'hidden',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE);

    $fields['langcode'] = BaseFieldDefinition::create('language')
      ->setLabel(t('Language code'))
      ->setDescription(t('The language code for the Brightcove Video.'))
//      ->setRevisionable(TRUE)
      ->setDisplayOptions('form', array(
        'type' => 'language_select',
        'weight' => ++$weight,
      ))
      ->setDisplayConfigurable('form', TRUE);

    $fields['api_client'] = BaseFieldDefinition::create('entity_reference')
      ->setLabel(t('API Client'))
      ->setDescription(t('API Client to use for playing the video.'))
      ->setRequired(TRUE)
      ->setSetting('target_type', 'brightcove_api_client')
      ->setDisplayOptions('form', array(
        'type' => 'options_select',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'hidden',
        'label' => 'inline',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE);

    $fields['player'] = BaseFieldDefinition::create('entity_reference')
      ->setLabel(t('Player'))
      ->setDescription(t('Player to use for the playlist.'))
      ->setSetting('target_type', 'brightcove_player')
      ->setDisplayOptions('form', [
        'type' => 'options_select',
        'weight' => ++$weight,
      ])
      ->setDisplayOptions('view', [
        'type' => 'hidden',
        'label' => 'inline',
        'weight' => $weight,
      ])
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE);

    /**
     * Additional Brightcove fields, based on
     * @see http://docs.brightcove.com/en/video-cloud/cms-api/references/cms-api/versions/v1/index.html#api-playlistGroup-Get_Playlists
     *
     * description - string - Playlist description
     * favorite - boolean - Whether playlist is in favorites list
     * playlist_id - string - The playlist id
     * name - string - The playlist name
     * reference_id - string - The playlist reference id
     * type - string - The playlist type: EXPLICIT or smart playlist type
     * videos - entityref/string array of video ids (EXPLICIT playlists only)
     * search - string - Search string to retrieve the videos (smart playlists
     *   only)
     */
    $fields['favorite'] = BaseFieldDefinition::create('boolean')
      ->setLabel(t('Show Playlist in Sidebar'))
      ->setDescription(t('Whether playlist is in favorites list'))
//      ->setRevisionable(TRUE)
      ->setDefaultValue(FALSE)
      ->setDisplayOptions('view', array(
        'type' => 'string',
        'label' => 'inline',
        'weight' => ++$weight,
      ))
      ->setDisplayConfigurable('view', TRUE);

    $fields['playlist_id'] = BaseFieldDefinition::create('string')
      ->setLabel(t('Playlist ID'))
      ->setDescription(t('Unique Playlist ID assigned by Brightcove.'))
      ->setReadOnly(TRUE)
      ->setDisplayOptions('view', array(
        'type' => 'string',
        'label' => 'inline',
        'weight' => ++$weight,
      ))
      ->setDisplayConfigurable('view', TRUE);

    $fields['reference_id'] = BaseFieldDefinition::create('string')
      ->setLabel(t('Reference ID'))
      ->addConstraint('UniqueField')
      ->setDescription(t('Value specified must be unique'))
//      ->setRevisionable(TRUE)
      ->setSettings(array(
        'max_length' => 150,
        'text_processing' => 0,
      ))
      ->setDisplayOptions('form', array(
        'type' => 'string_textfield',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'string',
        'label' => 'inline',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE);

    $fields['description'] = BaseFieldDefinition::create('string_long')
      ->setLabel(t('Short description'))
//      ->setRevisionable(TRUE)
      ->setDisplayOptions('form', array(
        'type' => 'string_textarea',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'basic_string',
        'label' => 'above',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE)
      ->addPropertyConstraints('value', [
        'Length' => [
          'max' => 250,
        ],
      ]);

    $fields['search'] = BaseFieldDefinition::create('string_long')
      ->setLabel(t('Search'))
//      ->setRevisionable(TRUE)
      ->setDisplayOptions('form', array(
        'type' => 'string_textarea',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'basic_string',
        'label' => 'inline',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE)
      ->addPropertyConstraints('value', [
        'Length' => [
          'max' => 5000,
        ],
      ]);

    $fields['videos'] = BaseFieldDefinition::create('entity_reference')
      ->setLabel(t('Videos'))
      ->setDescription(t('Videos in the playlist.'))
      //->setDefaultValue('')
//      ->setRevisionable(TRUE)
      ->setCardinality(FieldStorageDefinitionInterface::CARDINALITY_UNLIMITED)
      ->setSetting('target_type', 'brightcove_video')
      ->setSetting('handler', 'default')
      ->setDisplayOptions('form', array(
        'type' => 'string_textfield',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('view', array(
        'type' => 'string',
        'label' => 'above',
        'weight' => $weight,
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE);

    $fields['uid'] = BaseFieldDefinition::create('entity_reference')
      ->setLabel(t('Authored by'))
      ->setDescription(t('The username of the Brightcove Playlist author.'))
//      ->setRevisionable(TRUE)
      ->setSetting('target_type', 'user')
      ->setDefaultValueCallback('Drupal\brightcove\Entity\BrightcovePlaylist::getCurrentUserId')
      ->setTranslatable(TRUE)
      ->setDisplayOptions('view', array(
        'label' => 'hidden',
        'type' => 'author',
        'weight' => ++$weight,
      ))
      ->setDisplayOptions('form', array(
        'type' => 'entity_reference_autocomplete',
        'weight' => $weight,
        'settings' => array(
          'match_operator' => 'CONTAINS',
          'size' => '60',
          'autocomplete_type' => 'tags',
          'placeholder' => '',
        ),
      ))
      ->setDisplayConfigurable('form', TRUE)
      ->setDisplayConfigurable('view', TRUE);

    $fields['status'] = BaseFieldDefinition::create('boolean')
      ->setLabel(t('Publishing status'))
      ->setDescription(t('A boolean indicating whether the Brightcove Playlist is published.'))
//      ->setRevisionable(TRUE)
      ->setDefaultValue(TRUE);

    $fields['created'] = BaseFieldDefinition::create('created')
      ->setLabel(t('Created'))
      ->setDescription(t('The time that the Brightcove Playlist was created.'))
//      ->setRevisionable(TRUE)
      ->setTranslatable(TRUE)
      ->setDisplayOptions('view', array(
        'label' => 'inline',
        'type' => 'timestamp',
        'weight' => ++$weight,
      ))
      ->setDisplayConfigurable('view', TRUE);

    $fields['changed'] = BaseFieldDefinition::create('changed')
      ->setLabel(t('Changed'))
      ->setDescription(t('The time that the Brightcove Playlist was last edited.'))
//      ->setRevisionable(TRUE)
      ->setTranslatable(TRUE);

    return $fields;
  }

  /**
   * Converts videos from \Brightcove\Object\Playlist to Drupal Field API array.
   *
   * @param \Brightcove\Object\Playlist $playlist
   *   The playlist whose videos should be extracted.
   * @param \Drupal\Core\Entity\EntityStorageInterface $video_storage
   *   Video entity storage.
   *
   * @return array|null
   *   The Drupal Field API array that can be saved into a multivalue
   *   entity_reference field (like brightcove_playlist.videos), or NULL if the
   *   playlist does not have any videos.
   *
   * @throws \Exception
   *   Thrown when any of the videos is unavailable on the Drupal side.
   */
  protected static function extractVideosArray(Playlist $playlist, EntityStorageInterface $video_storage) {
    $videos = $playlist->getVideoIds();
    if (!$videos) {
      return NULL;
    }
    $return = [];
    foreach ($playlist->getVideoIds() as $video_id) {
      // Try to retrieve the video from Drupal's brightcove_video storage.
      $bcvid = $video_storage->getQuery()
        ->condition('video_id', $video_id)
        ->execute();
      if ($bcvid) {
        $bcvid = reset($bcvid);
        $return[] = ['target_id' => $bcvid];
      }
      else {
        // If the video is not found, then throw this exception, which will
        // effectively stop the queue worker, so the playlist with any "unknown"
        // video will remain in the queue, so it could be picked up the next
        // time hoping the video will be available by then.
        throw new \Exception(t('Playlist contains a video that is not (yet) available on the Drupal site'));
      }
    }
    return $return;
  }

  /**
   * Create or update an existing playlist from a Brightcove Playlist object.
   *
   * @param \Brightcove\Object\Playlist $playlist
   *   Brightcove Playlist object.
   * @param \Drupal\Core\Entity\EntityStorageInterface $playlist_storage
   *   Playlist EntityStorage.
   * @param \Drupal\Core\Entity\EntityStorageInterface $video_storage
   *   Video EntityStorage.
   * @param int|NULL $api_client_id
   *   The ID of the BrightcoveAPIClient entity.
   *
   * @throws \Exception
   *   If BrightcoveAPIClient ID is missing when a new entity is being created.
   */
  public static function createOrUpdate(Playlist $playlist, EntityStorageInterface $playlist_storage, EntityStorageInterface $video_storage, $api_client_id = NULL) {
    // May throw an \Exception if any of the videos is not found.
    $videos = self::extractVideosArray($playlist, $video_storage);

    // Only save the brightcove_playlist entity if it's really updated or
    // created now.
    $needs_save = FALSE;

    // Try to get an existing playlist.
    $existing_playlist = $playlist_storage->getQuery()
      ->condition('playlist_id', $playlist->getId())
      ->execute();

    // Update existing playlist if needed.
    if (!empty($existing_playlist)) {
      // Load Brightcove Playlist.
      $playlist_entity_id = reset($existing_playlist);
      /** @var BrightcovePlaylist $playlist_entity */
      $playlist_entity = BrightcovePlaylist::load($playlist_entity_id);

      // Update playlist if it is changed on Brightcove.
      if ($playlist_entity->getChangedTime() < ($updated_at = strtotime($playlist->getUpdatedAt()))) {
        $needs_save = TRUE;
        // Update changed time.
        $playlist_entity->setChangedTime($updated_at);
      }
    }
    // Create playlist if it does not exist.
    else {
      $needs_save = TRUE;
      // Create new Brightcove Playlist entity.
      /** @var BrightcovePlaylist $playlist_entity */
      $playlist_entity = BrightcovePlaylist::create([
        'api_client' => [
          'target_id' => $api_client_id,
        ],
        'playlist_id' => $playlist->getId(),
        'created' => strtotime($playlist->getCreatedAt()),
        'changed' => strtotime($playlist->getUpdatedAt()),
      ]);
    }

    if ($needs_save) {
      // Update type field if needed.
      if ($playlist->getType() != ($type = $playlist->getType())) {
        $playlist_entity->setType($type);
      }

      // Update name field if needed.
      if ($playlist_entity->getName() != ($name = $playlist->getName())) {
        $playlist_entity->setName($name);
      }

      // Update favorite field if needed.
      if ($playlist_entity->isFavorite() != ($favorite = $playlist->isFavorite())) {
        // This is a non-modifiable field so it does not have a specific
        // setter.
        $playlist_entity->set('favorite', $favorite);
      }

      // Update reference ID field if needed.
      if ($playlist_entity->getReferenceID() != ($reference_id = $playlist->getReferenceId())) {
        $playlist_entity->setReferenceID($reference_id);
      }

      // Update description field if needed.
      if ($playlist_entity->getDescription() != ($description = $playlist->getDescription())) {
        $playlist_entity->setDescription($description);
      }

      // Update search field if needed.
      if ($playlist_entity->getSearch() != ($search = $playlist->getSearch())) {
        $playlist_entity->setSearch($search);
      }

      // Update videos field if needed.
      if ($playlist_entity->getVideos() != $videos) {
        $playlist_entity->setVideos($videos);
      }
      // @TODO: State/published.

      $playlist_entity->save();
    }
  }
}
