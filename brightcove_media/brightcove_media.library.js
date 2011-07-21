(function ($) {
  Drupal.behaviors.brightcoveLibrary = {
    attach: function (context, settings) {
      var library = new Drupal.media.browser.library(
        Drupal.settings.media.browser.brightcove
      );
      $('#media-browser-tabset').bind('tabsselect', function (event, ui) {
        if (ui.tab.hash === '#media-tab-brightcove') {
          var params = {};
          for (var p in Drupal.settings.media.browser.brightcove) {
            params[p] = Drupal.settings.media.browser.library[p];
          }
          params.limit = 0;
          library.start($(ui.panel), params);
          $('#media-tab-brightcove #scrollbox')
            .bind('scroll', library, library.scrollUpdater);
        }
      });
    }
  };
})(jQuery);
