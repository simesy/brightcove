/**
 * @file upload.js
 * Handles submitting upload form - making a throbber.
 */
(function ($) {

  Drupal.brightcove_field = {};

  Drupal.behaviors.brightcove_field_upload = {
    attach: function(context) {
      $('#brightcove-field-upload-form').submit(Drupal.brightcove_field.upload);
      //$('iframe.overlay-active').contents().find('#brightcove-field-upload-form').submit(Drupal.brightcove_field.upload);
      //  <div class="ahah-progress ahah-progress-throbber"><div class="throbber">&nbsp;</div></div>
    }
  };

  Drupal.brightcove_field.upload = function() {
    $('#brightcove-field-upload-form #edit-submit').replaceWith('<div class="ahah-progress ahah-progress-throbber"><div class="throbber">&nbsp;</div><div>' + Drupal.t('The video is being created, please wait.') + '</div></div>');
  };
})(jQuery);