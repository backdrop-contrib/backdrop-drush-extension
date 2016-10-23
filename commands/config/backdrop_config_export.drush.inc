<?php
/**
 * @file backdrop_config_export.drush.inc
 *
 * Export Backdrop site configuration with drush.
 */

use \Drush\Boot\BackdropBoot;

/**
 * Implements hook_drush_command().
 */
function backdrop_config_export_drush_command() {
  $items = array();
  $items['backdrop-config-export'] = array(
    'description' => 'Export a Backdrop site configuration.',
    'callback' => 'backdrop_config_export',
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_SITE,
    'aliases' => array('bcex'),
    'examples' => array(
      'drush cex' => 'Exports the configuration for the current site to the staging config directory.',
    ),
  );

  return $items;
}

/**
 * Command callback for config-export
 */
function backdrop_config_export() {
  try {
    require_once BACKDROP_ROOT . '/settings.php';
    //global $config_directories['active'];
    drush_print_r($config_directories);
    $config_dir_active = $config_directories['active'];
    $config_dir_staging = $config_directories['staging'];
    passthru(
      "rm -rf " . BACKDROP_ROOT . "/$config_dir_staging/*"
    );
    passthru(
      "cp -R " . BACKDROP_ROOT . "/$config_dir_active/  " . BACKDROP_ROOT . "/$config_dir_staging"
    );
  }
  catch(Exception $e) {
    drush_print_r('couldn\t do it' . $e);
  }
}