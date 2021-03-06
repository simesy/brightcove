<?php

/**
 * @file
 * An extended subclass for field handling that adds multiple field grouping.
 *
 * Fields that want multiple value grouping options in addition to basic
 * field and formatter handling can extend this class.
 */
class brightcove_cck_handler_field_video_date_multiple extends brightcove_cck_handler_field_video_date {
  var $defer_query;

  function init(&$view, $options) {
    $field = $this->content_field;
    parent::init($view, $options);


    $this->defer_query = !empty($options['multiple']['group']) && $field['multiple'];

    if ($this->defer_query) {
      // Grouped field: ditch the existing additional_fields (field columns + delta).
      // In the main query we'll only need:
      // - vid, which will be used to retrieve the actual values in pre_render,
      // - node type and nid, which wil be used in the pseudo-node used when
      // rendering.
      $this->additional_fields = array(
        'type' => array('table' => 'node', 'field' => 'type'),
        'nid' => array('table' => 'node', 'field' => 'nid'),
      );
      if ($view->base_table == 'node_revisions') {
        $this->additional_fields['vid'] = array('table' => 'node_revisions', 'field' => 'vid');
      }
      else {
        $this->additional_fields['vid'] = array('table' => 'node', 'field' => 'vid');
      }
    }
  }

  function options(&$options) {
    parent::options($options);
  }

  /**
   * Provide 'group multiple values' option.
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $field = $this->content_field;
    $options = $this->options;

  }

  function options_validate($form, &$form_state) { }

  /**
   * Determine if this field is click sortable.
   */
  function click_sortable() {
    $field = $this->content_field;
    $options = $this->options;

    // Grouped fields are not click-sortable.
    return !empty($this->definition['click sortable']) && !$this->defer_query;
  }

  function query() {
    // If this is not a grouped field, use the generic query().
    if (!$this->defer_query) {
      return parent::query();
    }

    // Grouped field: do NOT call ensure_my_table, only add additional fields.
    $this->add_additional_fields();
    $this->field_alias = $this->aliases[$this->real_field];
  }

  function pre_render($values) {
    // If there are no values to render (displaying a summary, or query returned no results),
    // or if this is not a grouped field, do nothing specific.
    if (isset($this->view->build_info['summary']) || empty($values) || !$this->defer_query) {
      return parent::pre_render($values);
    }

    $field = $this->content_field;
    $db_info = content_database_info($field);
    $options = $this->options;
    $this->field_alias = $this->real_field;

    // Build the list of vids to retrieve.
    // TODO: try fetching from cache_content first ??
    $vids = array();
    $this->field_values = array();
    foreach ($values as $result) {
        $vids[] = $result->node_vid;
    }

    // It may happend that the multiple values field is related to a non
    // required relation for which no node data related to the field being
    // processed here is available.
    if (empty($vids)) {
      return parent::pre_render($values);
    }

    // List columns to retrieve.
    $alias = content_views_tablename($field);
    // Prefix aliases with '_' to avoid clashing with field columns names.
    $query_columns = array(
      'vid AS _vid',
      "delta as _delta",
      // nid is needed to generate the links for 'link to node' option.
      'nid AS _nid',
    );
    // The actual field columns.
    foreach ($db_info['columns'] as $column => $attributes) {
      $query_columns[] = "$attributes[column] AS $column";
    }
    $query = 'SELECT '. implode(', ', $query_columns) .
             ' FROM {'. $db_info['table'] ."}".
             " WHERE vid IN (". implode(',', $vids) .')'.
             " ORDER BY _nid ASC, _delta ". ($options['multiple']['multiple_reversed'] ? 'DESC' : 'ASC');
    $result = db_query($query);

    while ($item = db_fetch_array($result)) {
      // Clean up the $item from vid and delta. We keep nid for now.
      $vid = $item['_vid'];
      unset($item['_vid']);
      $delta = !empty($item['_delta']) ? $item['_delta'] : 0;
      $item['#delta'] = $item['_delta'];
      unset($item['_delta']);
      $this->field_values[$vid][$delta] = $item;
    }
  }

  /**
   * Return DIV or SPAN based upon the field's element type.
   *
   * Fields rendered with the 'group multiple' option use <div> markers,
   * and thus shouldn't be wrapped in a <span>.
   */
  function element_type() {
    if (!$this->defer_query) {
      return parent::element_type();
    }

    if (isset($this->definition['element type'])) {
      return $this->definition['element type'];
    }
    return 'div';
  }

  function render($values) {
    // If this is not a grouped field, use content_handler_field::render().
    if (!$this->defer_query) {
      return parent::render($values);
    }
    $options = $this->options;

    $vid = $values->node_vid;
    if (isset($this->field_values[$vid])) {
      // Gather items, respecting the 'Display n values starting from m' settings.
      $count_skipped = 0;
      $items = array();
      foreach ($this->field_values[$vid] as $item) {
        if (empty($options['multiple']['multiple_from']) || ($count_skipped >= $options['multiple']['multiple_from'])) {
          if (empty($options['multiple']['multiple_number']) || (count($items) < $options['multiple']['multiple_number'])) {
            // Grab the nid - needed for render_link().
            $video_id = $item['video_id'];
            $items[] = $item;
          }
          else {
            break;
          }
        }
        $count_skipped++;
      }
      foreach ($items as $item) {
        $video = brightcove_video_load($item['video_id']);
        if (!empty($video)) {
          if (isset($video->{$this->bc_field})) {
            $value = floor($video->{$this->bc_field} / 1000);
            $time_diff = $_SERVER['REQUEST_TIME'] - $value; // will be positive for a datetime in the past (ago), and negative for a datetime in the future (hence)
            switch ($format) {
              case 'raw time ago':
                $output = format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2);
              case 'time ago':
                $output = t('%time ago', array('%time' => format_interval($time_diff, is_numeric($custom_format) ? $custom_format : 2)));
              case 'raw time span':
                $output = ($time_diff < 0 ? '-' : '') . format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2);
              case 'time span':
                $output = t(($time_diff < 0 ? '%time hence' : '%time ago'), array('%time' => format_interval(abs($time_diff), is_numeric($custom_format) ? $custom_format : 2)));
              case 'custom':
                $output = format_date($value, $format, $custom_format);
              default:
                $output = format_date($value, $format);

            }
            $rendered[] = $this->render_link($output, (object) array('nid' => $item['_nid']));
          }
        }
      }
      if (count($rendered) > 1) {
        // TODO: could we use generic field display ?
        return theme('content_view_multiple_field', $rendered, $field, $values);
      }
      elseif ($rendered) {
        return $rendered[0];
      }
    }
    return '';
  }


}
