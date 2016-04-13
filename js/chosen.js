/**
 * @file
 * JS to apply chosen on select field.
 */

(function ($) {
  Drupal.behaviors.brightcoveChosen = {
    attach: function() {
      $(document).ready(function() {
       $('.field--name-tags select').chosen({
          inherit_select_classes: true,
          placeholder_text_multiple: Drupal.t('Select tags'),
          max_shown_results: 5,
          width: "100%"
        });
      })
    }
  }
})(jQuery);
