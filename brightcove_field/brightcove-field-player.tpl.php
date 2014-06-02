<?php

/**
 * @file brightcove-field-player.tpl.php
 * Default template for embeding brightcove players.
 *
 * Available variables:
 * - $id
 * - $width
 * - $height
 * - $classes_array
 * - $bgcolor
 * - $flashvars
 *
 * @see template_preprocess_brightcove_field_embed().
 */
global $is_https;
?>
<?php if ($responsive): ?>
<div class="BCLcontainingBlock">
  <div class="BCLvideoWrapper">
<?php endif; ?>

<object id="<?php print $id;?>" class="BrightcoveExperience <?php print join($classes_array, ',');?>">
  <param name="wmode" value="transparent" />
  <param name="bgcolor" value="<?php print $bgcolor;?>" />
  <param name="width" value="<?php print $width;?>" />
  <param name="height" value="<?php print $height;?>" />
  <param name="playerID" value="<?php print $player_id;?>" />
  <param name="playerKey" value="<?php print $player_key;?>" />
  <?php if ($is_https): ?>
    <param name="secureConnections" value="true" />
    <param name="secureHTMLConnections" value="true" />
  <?php endif; ?>
  <?php if ($is_vid): // for single video players ?>
    <param name="@videoPlayer" value="<?php print $brightcove_id; ?>" />
  <?php else: // for playlist players ?>
    <param name="@videoList" value="<?php print $brightcove_id; ?>" />
    <param name="@playlistTabs" value="<?php print $brightcove_id; ?>" />
    <param name="@playlistCombo" value="<?php print $brightcove_id; ?>" />
  <?php endif; ?>
  <param name="isVid" value="true" />
  <param name="isUI" value="true" />
  <param name="dynamicStreaming" value="true" />
  <?php if ($smart_api): // smart player api params ?>
  <param name="includeAPI" value="true" />
  <param name="templateLoadHandler" value="Drupal.brightcoveField.templateLoader" />
  <param name="templateReadyHandler" value="Drupal.brightcoveField.templateReady" />
  <?php endif; ?>
</object>

<?php if ($responsive): ?>
  </div>
</div>
<?php endif; ?>
