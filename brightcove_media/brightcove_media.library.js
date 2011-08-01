(function ($) {
  Drupal.brightcoveLibrary = Drupal.brightcoveLibrary || {};
  Drupal.brightcoveLibrary.library = {};
  Drupal.brightcoveLibrary.origParams = {};
  var loaded = false;

  Drupal.behaviors.brightcoveLibrary = {
    attach: function (context, settings) {
      Drupal.media.browser.validateButtons = function() {
        if (this.id === 'media-tab-brightcove') {
          $('<a class="button fake-ok">Submit</a>').appendTo(this).bind('click', Drupal.media.browser.submit);
          if (!($('.fake-cancel', this).length > 0)) {
            $('<a class="button fake-cancel">Cancel</a>').appendTo(this).bind('click', Drupal.media.browser.submit);
          }
        } else {
          if (!($('.form-submit', this).length > 0)) {
            $('<a class="button fake-ok">Submit</a>').appendTo(this).bind('click', Drupal.media.browser.submit);
            if (!($('.fake-cancel', this).length > 0)) {
              $('<a class="button fake-cancel">Cancel</a>').appendTo(this).bind('click', Drupal.media.browser.submit);
            }
          } else if (!($('.fake-cancel', this).length > 0)) {
            var parent = $('.form-actions', this);
            if (!parent.length) {
              parent = $('form > div', this);
            }
            $('<a class="button fake-cancel">Cancel</a>').appendTo(parent).bind('click', Drupal.media.browser.submit);
          }
        }
      };

      Drupal.brightcoveLibrary.library = new Drupal.media.browser.library(Drupal.settings.media.browser.brightcove);

      $('#media-browser-tabset').bind('tabsselect', function (event, ui) {
        if (ui.tab.hash === '#media-tab-brightcove') {
          if (!loaded) {
            console.log('megint');
            var params = {};
            for (var p in Drupal.settings.media.browser.brightcove) {
              params[p] = Drupal.settings.media.browser.library[p];
            }
            params.limit = 0;

            Drupal.brightcoveLibrary.origParams = params;
            Drupal.brightcoveLibrary.library.start($(ui.panel), params);
            loaded = true;
          }
        }
      });

      $('#edit-filter').not('.processed').addClass('processed').click(function(ev) {
        ev.preventDefault();

        var searchVal = $(Drupal.brightcoveLibrary.library.renderElement).find('.search-radio:checked').val();
        var keywordsVal = $(Drupal.brightcoveLibrary.library.renderElement).find('#edit-keywords').val();
        Drupal.brightcoveLibrary.library.params.filter = {search: searchVal, keywords: keywordsVal};

        $(Drupal.brightcoveLibrary.library.renderElement).find('#media-browser-library-list li').remove();
        Drupal.brightcoveLibrary.library.loading = true;
        Drupal.brightcoveLibrary.library.loadMedia();
      });

      $('#edit-reset').not('.processed').addClass('processed').click(function(ev) {
        ev.preventDefault();

        delete Drupal.brightcoveLibrary.library.params.filter;

        $(Drupal.brightcoveLibrary.library.renderElement).find('#media-browser-library-list li').remove();
        Drupal.brightcoveLibrary.library.loading = true;
        Drupal.brightcoveLibrary.library.loadMedia();
      });
    }
  };


})(jQuery);
