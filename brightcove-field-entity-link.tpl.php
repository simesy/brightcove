<?php

/**
 * @file brightcove-field-entity-link.tpl.php
 * Image linked to entity theme implementation.
 *
 * This template is used when viewing a Brightcove video with a thumbnail or poster image linking to entity.
 *
 * Available variables:
 *   - $output: Formatted image with a link to entity, ready for printing.
 *   - $image_field: The name of the image field used from Media API. Can be thumbnail or poster.
 *   - $destination: URL for the Brightcove video player.
 *   - $video_id: Video ID from Media API.
 *   - $field_name: Field name.
 *   - $type_name: Entity type.
 *   - $style: If printed with image style, image style name.
 *
 * @see theme_brightcove_field_entity_link()
 */
?>

<?php if ($output): ?>
<?php echo $output; ?>
<?php endif; ?>
