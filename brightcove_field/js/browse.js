/**
  * @file browse.js
  * Creates actions - browse and remove for Video form fields.
  */

Drupal.brightcove_field = {};
Drupal.brightcove_field.actions = {};

Drupal.behaviors.brightcove_field_buttons = {
  attach: function(context) {
    $('.brightcove-field-browse-button', context).click(Drupal.brightcove_field.actions.browse);
    $('.brightcove-field-upload-button', context).click(Drupal.brightcove_field.actions.upload);
    $('.brightcove-field-remove-button', context).click(Drupal.brightcove_field.actions.remove);
    $('.form-text.brightcove-video-field').change(Drupal.brightcove_field.actions.change);
  }
};

Drupal.brightcove_field.actions.change = function() {
  $('.brightcove-field-remove-button[rel="' + $(this).attr('id') + '"]').attr('disabled', '');
}
Drupal.brightcove_field.actions.remove = function() {
  $('#' + $(this).attr('rel')).val('');
}

Drupal.brightcove_field.actions.browse = function() {
  var id = $(this).attr('rel');
  var field_name = $('#' + id).attr('rel');
  Drupal.modalFrame.open({onSubmit: Drupal.brightcove_field.submit(id), url: Drupal.settings.basePath + 'brightcove_field/browse/' + Drupal.settings.brightcove_field[field_name].node_type + '/' + Drupal.settings.brightcove_field[field_name].field_name + '/' + Drupal.settings.brightcove_field[field_name].nid, width: 950, height: 600, autoFit: false});
  return false;
}

Drupal.brightcove_field.actions.upload = function() {
  var id = $(this).attr('rel');
  var field_name = $('#' + id).attr('rel');
  Drupal.modalFrame.open({onSubmit: Drupal.brightcove_field.submit(id), url: Drupal.settings.basePath + 'brightcove_field/upload/' + Drupal.settings.brightcove_field[field_name].node_type + '/' + Drupal.settings.brightcove_field[field_name].field_name + '/' + Drupal.settings.brightcove_field[field_name].nid, width: 950, height: 600, autoFit: false});
  return false;
}

Drupal.brightcove_field.submit = function(settings) {
  return function(args) {
    $("#" + settings).val(args.selected)
    $('.brightcove-field-remove-button[rel="' + settings + '"]').attr('disabled', '');
  };
}
