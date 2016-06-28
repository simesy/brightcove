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
  public function create(array $values = []) {
    brightcove_load_lib();
    // Add is_new property if it is not set.
    $values += ['is_new' => TRUE];
    if (empty($values['client']) || !($values['client'] instanceof Entity)) {
      throw new Exception(t('Cannot create a brightcove_video entity without a client.'));
    }
    if (empty($values['video_id'])) {
      $values['video'] = new \Brightcove\Object\Video\Video();
      $values['video_id'] = NULL;
    }
    elseif (($video = brightcove_load_video($values['video_id'], $values['client']))) {
      $values['video'] = $video;
      $values['account_id'] = $values['client']->account_id;
      $values['label'] = $video->getName();
    }
    else {
      throw new Exception(t('Failure to load the video @video with client @client', ['@video' => $values['video_id'], '@client' => $values['client']->bcid]));
    }
    // Add a Video object to the entity object.
    brightcove_load_lib();
    $video_entity = new Entity($values, $this->entityType);

    return $video_entity;
  }

  /**
   * @inheritdoc
   */
  public function load($ids = [], $conditions = []) {
    $entities = parent::load($ids, $conditions);
    // Add the Video object to each entity object.
    foreach ($entities as $key => $entity) {
      if (empty($entity->video) || !($entity->video instanceof \Brightcove\Object\Video\Video)) {
        if (empty($entity->client) || !($entity->client instanceof Entity)) {
          $clients = brightcove_get_clients_by_account_id($entity->account_id);
          $found_client = FALSE;
          foreach ($clients as $client) {
            if (($video = brightcove_load_video($entity->video_id, $client))) {
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
      $entity->label = $entity->video->getName();
    }
    return $entities;
  }

  /**
   * @inheritdoc
   */
  public function save($entity, DatabaseTransaction $transaction = NULL) {
    if (empty($entity->videoSaved)) {
      $entity->video = brightcove_save_video($entity->video, $entity->client);
    }
    $entity->account_id = $entity->client->account_id;
    if (empty($entity->video_id)) {
      $entity->video_id = $entity->video->getId();
    }
    return parent::save($entity, $transaction);
  }
}

/**
 * Saves a Video.
 *
 * @param \Brightcove\Object\Video\Video $video
 * @param Entity $client
 *
 * @return \Brightcove\Object\Video\Video
 */
function brightcove_save_video(\Brightcove\Object\Video\Video $video, Entity $client) {
  $video_id = $video->getId();
  $op = empty($video_id) ? 'create' : 'update';
  brightcove_try(function() use(&$video, $client, $client, $op) {
    list($cms,) = _brightcove_create_classes($client);
    $video = $cms->{"{$op}Video"}($video);
    brightcove_invalidate_cache("brightcove:video:{$video->getId()}:{$client->bcid}");
  });
  return $video;
}

/**
 * Brightcove video entity form.
 */
function brightcove_video_form($form, &$form_state, $video_entity, $entity_type = NULL, $bundle = NULL, $field_name = NULL) {
  brightcove_load_lib();
  $form['#entity'] = $video_entity;
  $form['#entity_type'] = 'brightcove_video';
  $form['#bundle'] = $video_entity->type;
  $form['#parent_entity_type'] = $entity_type;
  $form['#parent_bundle_name'] = $bundle;
  $form['#parent_field_name'] = $field_name;

  $client = $video_entity->client;
  $video = $video_entity->video;
  $video_id = $video->getId();
  $form['#op'] = empty($video_id) ? 'upload' : 'edit';
  // Edit form
  if (!empty($video_id) && empty($form_state['values'])) {
    $form_state['values'] = [
      'title' => $video->getName(),
      'short' => $video->getDescription(),
      'long' => $video->getLongDescription(),
      'reference_id' => $video->getReferenceId(),
      'tags' => implode(', ', $video->getTags()),
      'state' => $video->getState(),
      'start_availability_date' => _brightcove_format_date($video->getSchedule() ? $video->getSchedule()->getStartsAt() : NULL),
      'start_date' => $video->getSchedule() ? ($video->getSchedule()
        ->getStartsAt() === '' ? 'immidiately' : 'date_set') : NULL,
      'end_availability_date' => _brightcove_format_date($video->getSchedule() ? $video->getSchedule()->getEndsAt() : NULL),
      'end_date' => $video->getSchedule() ? ($video->getSchedule()
        ->getEndsAt() === '' ? 'no_end_date' : 'date_set') : NULL,
      'linktext' => $video->getLink() ? $video->getLink()->getText() : NULL,
      'linkurl' => $video->getLink() ? $video->getLink()->getUrl() : NULL,
      'economics' => $video->getEconomics(),
    ];

    foreach ($video->getCustomFields() as $field => $value) {
      $form_state['values']["custom_field_{$field}"] = $value;
    }
  }

  drupal_prepare_form('brightcove_upload_form', $form, $form_state);

  $form['client'] = [
    '#type' => 'hidden',
    '#value' => $client->bcid,
  ];

  if (empty($video_id)) {
    $form['file_upload'] = [
      '#type' => 'file',
      '#title' => t('Video') . ' <span class="form-required" title="' . t('This field is required.') . '">*</span>',
      '#size' => 40,
    ];

    $form['poster'] = [
      '#type' => 'file',
      '#title' => t('Poster image'),
      '#size' => 40,
    ];

    $form['thumbnail'] = [
      '#type' => 'file',
      '#title' => t('Thumbnail image'),
      '#size' => 40,
    ];

    $form['captions'] = [
      '#type' => 'container',
      '#prefix' => '<div id="caption-container">',
      '#suffix' => '</div>',
    ];

    $captions = isset($form_state['caption_num']) ? $form_state['caption_num'] : 0;
    $form_state['caption_num'] = $captions;

    for ($i = 0; $i < $captions; $i++) {
      $form['captions']["caption_{$i}"] = [
        '#type' => 'fieldset',
        '#title' => t('Caption'),
      ];

      $form['captions']["caption_{$i}"]["caption_{$i}_label"] = [
        '#type' => 'textfield',
        '#title' => t('Label'),
        '#required' => TRUE,
      ];

      $form['captions']["caption_{$i}"]["caption_{$i}_file"] = [
        '#type' => 'file',
        '#title' => t('WebVTT caption file'),
        '#description' => t('.vtt extension is required'),
        '#size' => 40,
      ];

      $form['captions']["caption_{$i}"]["caption_{$i}_srclang"] = [
        '#type' => 'textfield',
        '#title' => t('Source language code'),
        '#required' => TRUE,
        '#description' => t("Example: en"),
      ];

      $form['captions']["caption_{$i}"]["caption_{$i}_kind"] = [
        '#type' => 'select',
        '#title' => t('Kind'),
        '#options' => [
          'captions' => t('Captions'),
          'subtitles' => t('Subtitles'),
          'descriptions' => t('Descriptions'),
          'chapters' => t('Chapters'),
          'metadata' => t('Metadata'),
        ],
      ];

      $form['captions']["caption_{$i}"]["caption_{$i}_default"] = [
        '#type' => 'checkbox',
        '#title' => t("Default"),
      ];
    }

    $form['captions']['add_caption'] = [
      '#type' => 'submit',
      '#value' => t('Add caption'),
      '#submit' => ['_brightcove_video_form_add_caption_submit'],
      '#limit_validation_errors' => [],
      '#ajax' => [
        'callback' => '_brightcove_video_form_add_caption',
        'wrapper' => 'caption-container',
      ],
    ];

    if (variable_get('brightcove_3rdparty', FALSE)) {
      $form['captions']['3rdparty'] = [
        '#type' => 'container',
        'default' => [
          '#markup' => t('Note: automatic caption generation is available as a 3rd party integration on the !link.', [
            '!link' => l(t('Brightcove Video Connect page'), 'https://www.drupal.org/project/brightcove'),
          ]),
        ],
      ];
    }
  }

  $form['title'] = [
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#required' => TRUE,
    '#default_value' => !empty($form_state['values']['title']) ? $form_state['values']['title'] : '',
  ];

  $form['short'] = [
    '#type' => 'textarea',
    '#rows' => 2,
    '#title' => t('Short description'),
    '#default_value' => !empty($form_state['values']['short']) ? $form_state['values']['short'] : '',
  ];

  $form['long'] = [
    '#type' => 'textarea',
    '#rows' => 2,
    '#title' => t('Long description'),
    '#default_value' => !empty($form_state['values']['long']) ? $form_state['values']['long'] : '',
  ];

  $form['reference_id'] = [
    '#type' => 'textfield',
    '#title' => t('Reference ID'),
    '#default_value' => !empty($form_state['values']['reference_id']) ? $form_state['values']['reference_id'] : '',
    '#description' => t('Leave empty to generate ID automatically.'),
  ];

  $form['tags'] = [
    '#type' => 'textfield',
    '#title' => t('Tags'),
    '#description' => t('Separate tags with a comma; no tag > 128 characters'),
    '#default_value' => !empty($form_state['values']['tags']) ? $form_state['values']['tags'] : '',
  ];

  $form['state'] = [
    '#type' => 'select',
    '#title' => t('State'),
    '#options' => [
      'ACTIVE' => t('Active'),
      'INACTIVE' => t('Inactive'),
    ],
    '#default_value' => !empty($form_state['values']['state']) ? $form_state['values']['state'] : TRUE,
  ];

  $form['custom_fields'] = [
    '#type' => 'fieldset',
    '#title' => t('Custom fields'),
    '#collapsible' => TRUE,
  ];

  try {
    list($cms,) = _brightcove_create_classes($client);
    $fields = $cms->getVideoFields();
    $has_required = FALSE;
    $types = [
      'string' => 'textfield',
      'enum' => 'select',
    ];
    $has_custom_fields = FALSE;
    foreach ($fields->getCustomFields() as $field) {
      $has_custom_fields = TRUE;
      $has_required = $has_required || $field->isRequired();
      $form['custom_fields']["custom_field_{$field->getId()}"] = [
        '#title' => $field->getDisplayName(),
        '#description' => $field->getDescription(),
        '#required' => $field->isRequired(),
        '#type' => $types[$field->getType()],
        '#default_value' => isset($form_state['values']["custom_field_{$field->getId()}"]) ? $form_state['values']["custom_field_{$field->getId()}"] : NULL,
      ];
      if ($field->getType() === 'enum') {
        $enum_values = drupal_map_assoc($field->getEnumValues());
        // Show elements in custom fields in alphabetical order like in Video Cloud Studio.
        ksort($enum_values);
        $form['custom_fields']["custom_field_{$field->getId()}"]['#options'] = [NULL => t('- no value set -')] + $enum_values;
      }
    }
    $form['custom_fields']['#collapsed'] = !$has_required;
    $form['custom_fields']['#access'] = $has_custom_fields;
  }
  catch (\Brightcove\API\Exception\APIException $ex) {
    $form['custom_fields']['#access'] = FALSE;
  }
  catch (\Brightcove\API\Exception\AuthenticationException $ex) {
    return [
      'authfail' => [
        '#type' => 'markup',
        '#markup' => t('Failed to authenticate with Brightcove. Go to !settings-link to check the settings of the selected client.', [
          '!settings-link' => l(t('Edit @client', ['@client' => $client->label]), "admin/config/media/brightcove/client/manage/{$client->bcid}"),
        ]),
      ],
    ];
  }

  $form['schedule'] = [
    '#type' => 'fieldset',
    '#title' => t('Availability'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#attached' => [
      'js' => [
        drupal_get_path('module', 'brightcove') . '/js/schedule.js',
      ],
      'css' => [
        drupal_get_path('module', 'brightcove') . '/styles/schedule.css',
      ]
    ],
  ];

  $form['schedule']['start_date'] = [
    '#type' => 'radios',
    '#title' => t('Starting'),
    '#default_value' => isset($form_state['values']['start_date']) ? $form_state['values']['start_date'] : 'immediately',
    '#options' => [
      'immediately' => t('Immediately'),
      'date_set' => t('Starting from'),
    ],
  ];

  $form['schedule']['start_availability_date'] = [
    '#type' => 'date_popup',
    '#date_format' => 'Y-m-d H:i',
    '#date_timezone' => date_default_timezone(),
    '#date_label_position' => 'hidden',
    '#default_value' => !empty($form_state['values']['start_availability_date']) ? $form_state['values']['start_availability_date'] : format_date(REQUEST_TIME, 'custom', 'Y-m-d') . ' 00:00:00',
    '#prefix' => '<div id="brightcove-start-availability-date">',
    '#suffix' => '</div>',
    '#pre_render' => ['_brightcove_schedule_start_end_date_pre_render'],
    '#states' => [
      'visible' => [
        'input[name="start_date"]' => ['value' => 'date_set'],
      ],
    ],
  ];

  $form['schedule']['end_date'] = [
    '#type' => 'radios',
    '#title' => t('Ending'),
    '#default_value' => isset($form_state['values']['end_date']) ? $form_state['values']['end_date'] : 'no_end_date',
    '#options' => [
      'no_end_date' => t('No end date'),
      'date_set' => t('Until'),
    ],
  ];

  $form['schedule']['end_availability_date'] = [
    '#type' => 'date_popup',
    '#date_format' => 'Y-m-d H:i',
    '#date_timezone' => date_default_timezone(),
    '#date_label_position' => 'hidden',
    '#default_value' => !empty($form_state['values']['end_availability_date']) ? $form_state['values']['end_availability_date'] : format_date(REQUEST_TIME, 'custom', 'Y-m-d') . ' 23:59:59',
    '#prefix' => '<div id="brightcove-end-availability-date">',
    '#suffix' => '</div>',
    '#pre_render' => ['_brightcove_schedule_start_end_date_pre_render'],
    '#states' => [
      'visible' => [
        'input[name="end_date"]' => ['value' => 'date_set'],
      ],
    ],
  ];

  $form['advanced'] = [
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Advanced'),
  ];

  $form['advanced']['linktext'] = [
    '#type' => 'textfield',
    '#title' => t('Related link text'),
    '#default_value' => !empty($form_state['values']['linktext']) ? $form_state['values']['linktext'] : '',
  ];

  $form['advanced']['linkurl'] = [
    '#type' => 'textfield',
    '#title' => t('Related link url'),
    '#default_value' => !empty($form_state['values']['linkurl']) ? $form_state['values']['linkurl'] : '',
  ];

  $form['advanced']['economics'] = [
    '#type' => 'select',
    '#title' => t('Economic model'),
    '#options' => [
      BRIGHTCOVE_ECONOMICS_FREE => t('No advertising'),
      BRIGHTCOVE_ECONOMICS_AD_SUPPORTED => t('Advertising'),
    ],
    '#description' => t('If set to "Advertising", ads may be shown when viewers watch this video'),
    '#default_value' => !empty($form_state['values']['economics']) ? $form_state['values']['economics'] : BRIGHTCOVE_ECONOMICS_FREE
  ];

  if (empty($video_id)) {
    $form['advanced']['profile'] = [
      '#type' => 'select',
      '#title' => t('Video encoding profile'),
      '#options' => drupal_map_assoc(brightcove_standard_profile_list()),
    ];
  }

  $form['additional_fields'] = [
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#title' => t('Additional fields'),
  ];

  // Attach additional fields.
  // Edit form vs upload form.
  $entity = brightcove_video_load_by_video_id($video_id, $client);
  field_attach_form('brightcove_video', $entity, $form['additional_fields'], $form_state);

  $form['#validate'][] = '_brightcove_upload_form_validate';

  // Add video
  if (empty($video_id)) {
    $form['#prefix'] = '<div id="dialog-upload-form">';
    $form['submit'] = [
      '#type' => 'submit',
      '#name' => 'submit-',
      '#default_value' => t('Attach'),
      '#ajax' => [
        'callback' => 'ajax_upload_video_dialog_close_callback',
        'wrapper' => 'dialog-upload-form',
      ],
    ];
    $form['#suffix'] = '</div>';
  }
  // Edit form
  else {
    $form['#prefix'] = '<div id="dialog-edit-form">';

    $form['submit'] = [
      '#type' => 'submit',
      '#name' => 'submit-',
      '#default_value' => t('Save and Attach'),
      '#ajax' => [
        'callback' => 'ajax_edit_video_dialog_close_callback',
        'wrapper' => 'dialog-edit-form',
      ],
    ];

    $form['#suffix'] = '</div>';
  }
  $form['#attached'] = [
    'js' => [
        drupal_get_path('module', 'brightcove') . '/js/brightcove.js',
    ],
    'css' => [
      drupal_get_path('module', 'brightcove') . '/styles/upload.css',
      drupal_get_path('module', 'brightcove') . '/brightcove_field.upload.css',
    ],
  ];

  return $form;
}

/**
 * Save a video entity.
 *
 * This call will take care of creating/updating the Video at
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
 * Load a video entity by Video->getId().
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

function brightcove_video_create($values) {
  return entity_get_controller('brightcove_video')->create($values);
}
/**
 * Access callback for video entities.
 */
function brightcove_video_access($op, $entity, $account = NULL) {
  // Viewing the video should be restricted by the content access the
  // video is embedded into.
  // Video creation depends on the client anyway so let's grant access and
  // notify user on failure later.
  return $op == 'view' || $op == 'add' || $op == 'create' || brightcove_client_access('use', $entity->client);
}

/**
 * Menu callback to display the video upload dialog.
 */
function brightcove_video_upload($entity_type, $field_name, $bundle_or_id, $client) {
  $values = [
    'client' => $client,
    'type' => 'brightcove_video',
  ];
  $bundle = $bundle_or_id;
  if (is_numeric($bundle_or_id)) {
    $entity = entity_load_single($entity_type, $bundle_or_id);
    $info = entity_get_info($entity_type);
    $bundle_key = $info['entity keys']['bundle'];
    $bundle = $entity->{$bundle_key};
  }
  $video_entity = brightcove_video_create($values);
  $form = drupal_get_form('brightcove_video_form', $video_entity, $entity_type, $bundle, $field_name);
  return drupal_render($form);
}

/**
 * Menu access callback for editing a video.
 */
function brightcove_video_edit_access($perm, $entity_type, $field_name, $entity_id_or_bundle = NULL, Entity $video = NULL) {
  return brightcove_field_browse_access($perm, $entity_type, $field_name, $entity_id_or_bundle, $video->client);
}

/**
 * Menu callback to display a video edit form.
 *
 * @param Entity $client
 * @param string $video_id
 *
 * @return string
 */
function brightcove_video_edit($entity_type, $field_name, $entity_id, Entity $client, $video_id) {
  $video_entity = brightcove_video_load_by_video_id($video_id, $client);
  $entity_info = entity_get_info($entity_type);
  $bundle_key = $entity_info['entity keys']['bundle'];
  $entity = entity_load_single($entity_type, $entity_id);
  $bundle_name = $entity->{$bundle_key};
  $form = drupal_get_form('brightcove_video_form', $video_entity, $entity_type, $bundle_name, $field_name);
  return drupal_render($form);
}

/**
 * Format a date.
 */
function _brightcove_format_date($datestring, $expected_format = 'Y-m-d H:i') {
  if (!$datestring) {
    return NULL;
  }

  $datetime = new DateTime($datestring);
  return $datetime->format($expected_format);
}
