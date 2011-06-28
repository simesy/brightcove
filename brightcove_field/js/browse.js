/**
 * @file browse.js
 * Creates actions - browse and remove for Video form fields.
 */
(function ($) {

  Drupal.brightcove_field = {};
  Drupal.brightcove_field.actions = {};
  var brightcove_field_settings;

  Drupal.behaviors.brightcove_field_buttons = {
    attach: function(context, settings) {
      brightcove_field_settings = settings;
      $('.brightcove-field-browse-button', context).click(Drupal.brightcove_field.actions.browse);
      $('.brightcove-field-upload-button', context).click(Drupal.brightcove_field.actions.upload);
      $('.brightcove-field-remove-button', context).click(Drupal.brightcove_field.actions.remove);
      $('.form-text.brightcove-video-field').change(Drupal.brightcove_field.actions.change);
    }
  };

  Drupal.brightcove_field.actions.change = function() {
    var filt = $(this).attr('rel');
    var button = $('.brightcove-field-remove-button[rel*=' + filt + ']');
    button.attr('disabled', '');
    button.removeClass('form-button-disabled');
  }

  Drupal.brightcove_field.actions.remove = function() {
    event.preventDefault();
    $('.' + $(this).attr('rel')).val('');
    $(this).attr('disabled', '');
    $(this).addClass('form-button-disabled');
  }

  Drupal.brightcove_field.actions.browse = function() {
    event.preventDefault();
    console.log(brightcove_field_settings);
    var id = $(this).attr('rel');
    var field_name = $('.' + id).attr('data-field-name');

    var dialog = $('<div>').dialog({
      autoOpen: false,
      modal: true,
      open: function() {
        $(this).load(Drupal.settings.basePath + 'brightcove_field/browse/' + brightcove_field_settings.brightcove_field[field_name].entity_type + '/' + brightcove_field_settings.brightcove_field[field_name].field_name + '/' + brightcove_field_settings.brightcove_field[field_name].entity_id);
        Drupal.attachBehaviors($(this));
      },
      height: 600,
      width: 950,
      close: function() {
        Drupal.brightcove_field.submit(id);
        $(this).remove();
      }
    });

    dialog.dialog('open');

    //Drupal.modalFrame.open({onSubmit: Drupal.brightcove_field.submit(id), url: Drupal.settings.basePath + 'brightcove_field/browse/' + Drupal.settings.brightcove_field[field_name].node_type + '/' + Drupal.settings.brightcove_field[field_name].field_name + '/' + Drupal.settings.brightcove_field[field_name].nid, width: 950, height: 600, autoFit: false});
    return false;
  }

  Drupal.brightcove_field.actions.upload = function() {
    event.preventDefault();
    var id = $(this).attr('rel');
    var field_name = $('.' + id).attr('data-field-name');

    var dialog = $('<div>').dialog({
      autoOpen: false,
      modal: true,
      open: function() {
        $(this).load(Drupal.settings.basePath + 'brightcove_field/upload/' + brightcove_field_settings.brightcove_field[field_name].entity_type + '/' + brightcove_field_settings.brightcove_field[field_name].field_name + '/' + brightcove_field_settings.brightcove_field[field_name].entity_id + ' #brightcove-field-upload-form');
        Drupal.attachBehaviors($(this));
      },
      height: 600,
      width: 950,
      close: function() {
        Drupal.brightcove_field.submit(id);
        $(this).remove();
      }
    });

    dialog.dialog('open');


    //Drupal.modalFrame.open({onSubmit: Drupal.brightcove_field.submit(id), url: Drupal.settings.basePath + 'brightcove_field/upload/' + Drupal.settings.brightcove_field[field_name].node_type + '/' + Drupal.settings.brightcove_field[field_name].field_name + '/' + Drupal.settings.brightcove_field[field_name].nid, width: 950, height: 600, autoFit: false});
    return false;
  }

  Drupal.brightcove_field.submit = function(settings) {
    return function(args) {
      $("#" + settings).val(args.selected)
      $('.brightcove-field-remove-button[rel="' + settings + '"]').attr('disabled', '');
    };
  }
})(jQuery);