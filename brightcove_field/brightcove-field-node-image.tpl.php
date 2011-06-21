<?php

/**
 * @file brightcove-field-node-image.tpl.php
 * Image theme implementation.
 *
 * This template is used when viewing a Brightcove video with a thumbnail or videoStill image without a link.
 *
 * Available variables:
 *   - $output: Formatted image, ready for printing.
 *   - $image_field: The name of the image field used from Media API. Can be thumbnailURL or videoStillURL.
 *   - $nid: Node NID.
 *   - $video_id: Video ID from Media API.
 *   - $field_name: Field name.
 *   - $type_name: Node type.
 *
 * @see theme_brightcove_field_node_image()
 */
?>

<?php if ($output): ?>
<?php echo $output; ?>
<?php endif; ?>
