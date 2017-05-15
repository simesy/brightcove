/**
 * @file
 * Provides functions for the brightcove module media browser integration.
 */

(function ($) {
  Drupal.brightcove = Drupal.brightcove || {};
  Drupal.brightcove.library = Drupal.brightcove.library || {};
  Drupal.brightcove.library.video = Drupal.brightcove.library.video || {};
  Drupal.brightcove.library.video.loaded = Drupal.brightcove.library.video.loaded || false;

  Drupal.behaviors.brightcoveVideoBrowser = {
    attach: function (context, settings) {
      Drupal.brightcove.library.video = new Drupal.brightcove.library(Drupal.settings.media.browser.brightcove);

      function resizeScrollbox() {
        var windowHeight = $(window).height();
        var brandingHeight = $("#branding").outerHeight(true);
        var tab = $("div.media-browser-tab:not(.ui-tabs-hide)");
        var tabPaddingHeight = parseInt(tab.css('padding-top')) + parseInt(tab.css('padding-bottom'));
        $("#scrollbox").height(windowHeight - brandingHeight - tabPaddingHeight);
      }

      $('#media-browser-tabset', context).bind('tabscreate tabsshow tabsactivate', function (event, ui) {
        var panel = $(ui.newPanel ? ui.newPanel : ui.panel);

        resizeScrollbox();

        if (typeof panel !== 'undefined' && panel.attr('id') === 'media-tab-brightcove') {
          // Prevent reloading of media list on tabselect if already loaded in media list
          if (!Drupal.brightcove.library.video.loaded) {
            var params = {};
            for (var p in Drupal.settings.media.browser.brightcove) {
              params[p] = Drupal.settings.media.browser.brightcove[p];
            }

            Drupal.brightcove.library.video.start($(panel), params, function() {
              resizeScrollbox();
              var scrollBox = $("#scrollbox")[0];
              if (scrollBox.scrollHeight <= scrollBox.offsetHeight) {
                $(scrollBox).scroll();
              }
            });
            $('#scrollbox').bind('scroll', Drupal.brightcove.library.video, Drupal.brightcove.library.video.scrollUpdater);
            Drupal.brightcove.library.video.loaded = true;
          }
        }
      });

      $('#edit-filter').not('.bc-processed').addClass('bc-processed').click(function(ev) {
        ev.preventDefault();

        // Acquire filter form values
        var searchVal = $(Drupal.brightcove.library.video.renderElement).find('.search-radio:checked').val();
        var keywordsVal = $(Drupal.brightcove.library.video.renderElement).find('#edit-keywords').val();
        // set library object parameters (used for ajax loading new media into list)
        Drupal.brightcove.library.video.params.filter = {search: searchVal, keywords: keywordsVal};

        // Remove the media list
        Drupal.brightcove.library.video.cursor = 0;
        Drupal.brightcove.library.video.mediaFiles = [];
        $(Drupal.brightcove.library.video.renderElement).find('#media-browser-library-list li').remove();
        $('#scrollbox').unbind('scroll').bind('scroll', Drupal.brightcove.library.video, Drupal.brightcove.library.video.scrollUpdater);

        // Set a flag so we don't make multiple concurrent AJAX calls
        Drupal.brightcove.library.video.loading = true;
        // Reload the media list
        Drupal.brightcove.library.video.loadMedia();
      });

      $('#edit-reset').not('.bc-processed').addClass('bc-processed').click(function(ev) {
        ev.preventDefault();

        // Reset filter form values
        delete Drupal.brightcove.library.video.params.filter;
        $(Drupal.brightcove.library.video.renderElement).find('.search-radio[value=name]').attr('checked', true);
        $(Drupal.brightcove.library.video.renderElement).find('#edit-keywords').val('');
        $('#scrollbox').unbind('scroll').bind('scroll', Drupal.brightcove.library.video, Drupal.brightcove.library.video.scrollUpdater);

        // Remove the media list
        Drupal.brightcove.library.video.cursor = 0;
        Drupal.brightcove.library.video.mediaFiles = [];
        $(Drupal.brightcove.library.video.renderElement).find('#media-browser-library-list li').remove();
        // Set a flag so we don't make multiple concurrent AJAX calls
        Drupal.brightcove.library.video.loading = true;
        // Reload the media list
        Drupal.brightcove.library.video.loadMedia();
      });

      $(document).delegate('#media-browser-library-list a', 'mousedown', function() {
        var uri = $(this).attr('data-uri');
        $("input[name='submitted-video']").val(uri);
        var file = {uri: uri};
        var files = new Array();
        files.push(file);
        Drupal.media.browser.selectMedia(files);
      });

      $("html:not(.bc-processed)").addClass("bc-processed").bind("resize", function() {
        resizeScrollbox();
      });
    }
  };

  /**
   * This function called after the user clicked on the "Upload and attach" button
   * in the media browser upload form.
   *
   * @param ajax
   * @param response
   * @param status
   */
  Drupal.ajax.prototype.commands.brightcove_media_upload = function (ajax, response, status) {
    $("input[name='submitted-video']").val(response.data.uri);
    Drupal.media.browser.selectMedia([{uri: response.data.uri}]);
    $('#bc-filter-form')
        .find('.form-actions input[type=submit][name=op]')
        .trigger('click');
  };
})(jQuery);
