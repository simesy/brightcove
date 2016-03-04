<?php

/**
 * @file brightcove-field-dialog.tpl.php
 * Image -> Dialog player theme implementation.
 *
 * This template is used when viewing a Brightcove video with a thumbnail or poster image linking to Dialog.
 *
 * Available variables:
 *   - $output: Formatted image with a link to Dialog, ready for printing.
 *   - $video_width: Real video width.
 *   - $video_height: Real video height.
 *   - $dialog_width: Dialog window width.
 *   - $dialog_height: Dialog window height.
 *   - $image_field: The name of the image field used from Media API. Can be thumbnail or poster.
 *   - $destination: URL for the Brightcove video player.
 *   - $video_id: Video ID from Media API.
 *   - $field_name: Field name.
 *   - $entity_type: Entity type.
 *   - $style: If printed with image style, image style name.
 *
 * @see theme_brightcove_field_dialog_player()
 */
?>

<?php if ($output): ?>
<?php echo $output; ?>
<?php endif; ?>
