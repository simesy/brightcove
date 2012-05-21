/**
 * @file
 * brightcove_media.playlist.js
 *
 * This file contain the necessary javascript code to integrate
 * brightcove playlist feature in the media browser.
 */

(function ($) {

namespace('Drupal.media.browser');

Drupal.brightcovePlaylistLibrary = Drupal.brightcovePlaylistLibrary || {};
Drupal.brightcovePlaylistLibrary.library = Drupal.brightcovePlaylistLibrary.library || {};
Drupal.brightcovePlaylistLibrary.loaded = Drupal.brightcovePlaylistLibrary.loaded || false;

Drupal.behaviors.brightcovePlaylistLibrary = {
  attach: function (context, settings) {
    Drupal.brightcovePlaylistLibrary.library = new Drupal.media.browser.library(Drupal.settings.media.browser.brightcove_playlist);
    $('#media-browser-tabset').bind('tabsshow', function (event, ui) {
      if (ui.tab.hash === '#media-tab-brightcove_playlist' && !Drupal.brightcovePlaylistLibrary.loaded) {
        // Grab the parameters from the Drupal.settings object
        var params = {};
        for (var parameter in Drupal.settings.media.browser.brightcove_playlist) {
          params[parameter] = Drupal.settings.media.browser.brightcove_playlist[parameter];
        }
        Drupal.brightcovePlaylistLibrary.library.start($(ui.panel), params);
        $('#scrollbox').bind('scroll', Drupal.brightcovePlaylistLibrary.library, Drupal.brightcovePlaylistLibrary.library.scrollUpdater);
        Drupal.brightcovePlaylistLibrary.loaded = true;
      }
    });

    $(document).delegate('#media-tab-brightcove_playlist #media-browser-library-list a', 'mousedown', function() {
      var uri = $(this).attr('data-uri');
      $("input[name='submitted-video']").val(uri);
      Drupal.media.browser.selectMedia([{uri: uri}]);
    });
  }
};

}(jQuery));
