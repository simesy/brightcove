(function ($) {
  Drupal.behaviors.brightcovePlayerAdmin = {
    attach: function(context, settings) {
      // Export form machine-readable JS
      $('.brightcove-player-display-name:not(.processed)', context).each(function() {
        $('.brightcove-player-display-name')
          .addClass('processed')
          .after(' <small class="brightcove-player-name-suffix">&nbsp;</small>');
        if ($('.brightcove-player-name').val() === $('.brightcove-player-display-name').val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_') || $('.brightcove-player-name').val() === '') {
          $('.brightcove-player-name').parents('.form-item').hide();
          $('.brightcove-player-display-name').bind('keyup change', function() {
            var machine = $(this).val().toLowerCase().replace(/[^a-z0-9]+/g, '_').replace(/_+/g, '_');
            if (machine !== '_' && machine !== '') {
              $('.brightcove-player-name').val(machine);
              $('.brightcove-player-name-suffix').empty().append(' Machine name: ' + machine + ' [').append($('<a href="#">'+ Drupal.t('Edit') +'</a>').click(function() {
                $('.brightcove-player-name').parents('.form-item').show();
                $('.brightcove-player-name-suffix').hide();
                $('.brightcove-player-display-name').unbind('keyup');
                return false;
              })).append(']');
            }
            else {
              $('.brightcove-player-name').val(machine);
              $('.brightcove-player-name-suffix').text('');
            }
          });
          $('.brightcove-player-display-name').keyup();
        }
      });
     }
  };
})(jQuery);
