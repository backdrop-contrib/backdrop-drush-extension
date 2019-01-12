<?php
/**
 * @file
 * API for Git Release Notes Drush commands.
 */

/**
 * Modify release notes items before rendering.
 *
 * @param $items
 *  Array of individual, cleaned-up commits.
 *  Array values are HTML, keys are cleaned-up version of commits without HTML.
 */
function hook_release_notes_output_alter(&$items) {
  foreach ($items as $k => $item) {
    if (preg_match('/^Merge branch/S', $item)) {
      unset($items[$k]);
    }
  }
}

