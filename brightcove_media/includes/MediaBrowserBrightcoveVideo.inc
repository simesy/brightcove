<?php

/**
 * @file
 * Definition of MediaBrowserUpload.
 */

/**
 * Media browser plugin for showing the upload form.
 *
 * @deprecated
 */
class MediaBrowserBrightcoveVideo extends MediaBrowserPlugin {

  /**
   * {@inheritdoc}
   */
  public function access($account = NULL) {
    return file_entity_access('create', NULL, $account);
  }

  /**
   * {@inheritdoc}
   */
  public function view() {
    $upload_form = drupal_get_form('brightcove_media_upload_form');
    $upload_form = drupal_render($upload_form);
    $filter_form = drupal_get_form('brightcove_media_browser_filter_form');
    $filter_form = drupal_render($filter_form);
    $bcpath = drupal_get_path('module', 'brightcove_media');

    return [
      '#title' => t('Brightcove videos'),
      '#settings' => [
        'getMediaUrl' => url('brightcove/media/list'),
        'multiselect' => isset($params['multiselect']) && $params['multiselect'] != 'false',
      ],
      '#attached' => [
        'library' => [

        ],
        'js' => [
          $bcpath . '/js/brightcove_media.js',
          $bcpath . '/js/brightcove_media.video.js',
        ],
        'css' => [
          $bcpath . '/css/brightcove_media.css',
        ],
      ],
      // If the #form and #markup parameters are not empty the media module will
      // not render the fake submit button.
      '#form' => [FALSE],
      '#markup' => '<div id="container"><div id="scrollbox">' . $upload_form . $filter_form . '</div></div>',
    ];
  }

}
