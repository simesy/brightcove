/**
 * @file schedule.js
 * JS for schedule
 */
(function ($) {
  Drupal.behaviors.brightcoveScheduleFields = {
    attach: function(context, settings) {
      $('#brightcove-start-availability-date').appendTo('#brightcove-field-upload-form #edit-start-date label[for="edit-start-date-date-set"]').click(function(event) {
        $(this).closest('.form-type-radio').find('input[type="radio"]').attr('checked', true);
      });

      $('#brightcove-end-availability-date').appendTo('#brightcove-field-upload-form #edit-end-date label[for="edit-end-date-date-set"]').click(function(event) {
        $(this).closest('.form-type-radio').find('input[type="radio"]').attr('checked', true);
      });
    }
  };
})(jQuery);
