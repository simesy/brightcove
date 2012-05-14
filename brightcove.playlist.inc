<?php

/**
 * Renders the playlist overview page.
 *
 * @return string
 *   Rendered overivew page.
 */
function brightcove_playlist_overview_page() {
  $table = array('header' => array('Title', 'ops'));

  $playlists = brightcove_get_playlists();
  if ($playlists) {
    foreach ($playlists as $playlist) {
      $table['rows'][] = _brightcove_playlist_overview_row($playlist);
    }
  }
  else {
    $table['rows'][] = array(
      array(
        'data' => t('No playlist found.'),
        'colspan' => count($table['header']),
      ),
    );
  }

  return theme('table', $table);
}

/**
 * Renders a table row on the Brightcove playlist overview page.
 *
 * @param stdClass $playlist
 *   Playlist object.
 *
 * @return array
 *   Table row.
 *   @see theme_table()
 */
function _brightcove_playlist_overview_row(stdClass $playlist) {
  $operations = array(
    l(t('edit'), "admin/config/media/brightcove/playlist/{$playlist->id}/edit"),
  );

  return array(
    $playlist->name,
    implode(', ', $operations),
  );
}

/**
 * Get the playlists from Brightcove.
 *
 * This function is a cached wrapper around BCMAPI::find('allplaylists').
 *
 * @return bool|array
 *   Array of playlists or FALSE if none found.
 *   @see http://support.brightcove.com/en/docs/media-api-objects-reference#Playlist
 */
function brightcove_get_playlists() {
  $cache = cache_get('bc:playlist:list', 'cache');
  if ($cache && !empty($cache->data)) {
    return $cache->data;
  }

  $playlists = FALSE;
  $bc = brightcove_initialize();
  try {
    $playlists = $bc ? $bc->find('allplaylists') : NULL;
  }
  catch (Exception $e) {
    watchdog('brightcove', 'Loading brightcove playlists failed.', array(), WATCHDOG_ERROR);
  }

  if ($playlists) {
    cache_set('bc:playlist:list', $playlists, 'cache', REQUEST_TIME + 300);
  }

  return $playlists;
}
