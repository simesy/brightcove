<?php
/**
 * @file
 * Theme funciton for the module.
 */

/**
 * Theme callback for a Brightcove browse button.
 * Currently it's just a thin wrapper around the theme_button()
 * function which only returns a button of type submit. The themed
 * representation is just adapted to achieve an input html tag
 * of the type button.
 *
 * @see nodereference_explorer.module
 */
function theme_brightcove_field_browse_button($element) {
  drupal_add_js(drupal_get_path('module', 'brightcove') . '/js/brightcove.js');
  $element['#button_type'] = 'button';
  $button = theme('button', ['element' => $element]);
  return str_ireplace('type="submit"', 'type="button"', $button);
}

/**
 * Theme callback for Brightcove browse table item.
 *
 * @param item
 *   Video item.
 *
 * @return
 *   Themed item form.
 */
function theme_brightcove_field_browse_item($variables) {
  $form = drupal_get_form('brightcove_field_browser_form' . $variables['item']['brightcove_id'], $variables['item'], $variables['entity_type'], $variables['field_name'], $variables['entity_id_or_bundle'], $variables['bcid']);
  return drupal_render($form);
}

/**
 * Theme callback for Brightcove browse table.
 *
 * @param $variables
 *   Array of video items.
 *
 * @return
 *   Themed browse table.
 */
function theme_brightcove_field_browse_items($variables) {
  $rowcount = 0;
  $activerow = 0;
  $rows = [];
  foreach ($variables['items'] as $item) {
    $themed = theme('brightcove_field_browse_item', [
      'item' => $item,
      'entity_type' => $variables['entity_type'],
      'field_name' => $variables['field_name'],
      'entity_id_or_bundle' => $variables['entity_id_or_bundle'],
      'bcid' => $variables['bcid'],
    ]);
    $rowcount++;
    if ($rowcount == 4) {
      $activerow++;
      $rowcount = 1;
    }
    $rows[$activerow][] = $themed;
  }
  return '<div id="brightcove-browse-table-table">' . theme('table', ['header' => [], 'rows' => $rows]) . '</div>';
}

function theme_brightcove_integration($variables) {
  $name = check_plain($variables['name']);
  $description = filter_xss($variables['description'], [
    'a',
    'em',
    'strong',
    'cite',
    'blockquote',
    'code',
    'ul',
    'ol',
    'li',
    'dl',
    'dt',
    'dd',
    'br',
  ]);
  $project_link = '';
  if ($variables['project']) {
    $link = "https://drupal.org/project/{$variables['project']}";
    $project_link = l($link, $link);
  }
  $image = '';
  if ($variables['image']) {
    $image = theme('image', ['path' => $variables['image']]);
  }

  return <<<EOS
<div class="brightcove-integration">
  <div class="brightcove-integration-image">{$image}</div>
  <h3 class="brightcove-integration-name">{$name}</h3>
  <p class="brightcove-integration-description">{$description}</p>
  <p class="brightcove-integration-link">{$project_link}</p>
</div>
EOS;
}
