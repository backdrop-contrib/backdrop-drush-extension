<?php
/**
 * @file
 * Some output helpers.
 */

/**
 * Drush chalk lets you format output text for color, bold, etc.
 *
 * @param string $output
 *   The string of text you wish to format.
 * @param string $color
 *   The color or format you wish to use ie. '32m' for green.
 *
 * @return string
 *   The formatted string.
 */
function drush_chalk(string $output, string $color) {
  $output = "\033[$color$output\033[0m";

  return $output;
}
