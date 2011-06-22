Drupal.brightcove_field = {};

Drupal.behaviors.brightcove_field_attach = {
  attach: function(context) {
    $('.brightcove-field-attach-button', context).click(function() {});
  }
};


Drupal.brightcove_field.submit = function(settings) {
  return function(args) {
          alert(args);
  };
}
/**
  * Overriding Drupal.modalFrameChild.behaviors.parseLinks function from
  * modalframe in order to ensure that pager links do not point to
  * target="_new"
  * Just return for now.
  */
Drupal.modalFrameChild.behaviors.parseLinks = function(context) {
  return;
};


