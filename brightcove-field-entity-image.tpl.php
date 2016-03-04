<?php

/**
 * @file brightcove-field-entity-image.tpl.php
 * Image theme implementation.
 *
 * This template is used when viewing a Brightcove video with a thumbnail or poster image without a link.
 *
 * Available variables:
 *   - $output: Formatted image, ready for printing.
 *   - $image_field: The name of the image field used from Media API. Can be thumbnail or poster.
 *   - $id: Entity ID.
 *   - $video_id: Video ID from Media API.
 *   - $field_name: Field name.
 *   - $type_name: Entity type.
 *
 * @see theme_brightcove_field_entity_image()
 */
?>

<?php if ($output): ?>
<?php echo $output; ?>
<?php endif; ?>
