<?php

/**
 * @file brightcove-field-player.tpl.php
 * Default template for embeding brightcove players.
 *
 * Available variables:
 * - $id
 * - $account_id
 * - $player_id
 * - $video_id
 * - $width
 * - $height
 *
 * @see template_preprocess_brightcove_field_embed().
 */
?>

<video
  id="<?php print $id; ?>"
  data-account="<?php print $account_id; ?>"
  data-player="<?php print $player_id; ?>"
  data-embed="<?php print $embed; ?>"
  data-video-id="<?php print $video_id; ?>"
  width="<?php print $width; ?>"
  height="<?php print $height; ?>"
  class="video-js" controls></video>
<script src="//players.brightcove.net/<?php print $account_id; ?>/<?php print $player_id; ?>_<?php print $embed; ?>/index.min.js"></script>
