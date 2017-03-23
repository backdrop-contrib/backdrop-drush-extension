<?php
/**
 * @file backdrop_config_export.drush.inc
 *
 * Export Backdrop site configuration with drush.
 */

use \Drush\Boot\BackdropBoot;
use Drush\Log\LogLevel;

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
    'options' => array(
      'destination' => 'Provide a path/to/directory where you wish to export the config.',
    ),
    'examples' => array(
      'drush bcex' => 'Exports the configuration for the current site to the staging config directory.',
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
    $config_dir_active = $config_directories['active'];

    $destination = drush_get_option('destination', '');
    // Determine where to export the config.
    if (empty($destination)) {
      $config_export = BACKDROP_ROOT . "/" . $config_directories['staging'];
    }
    else {
      $config_export = BACKDROP_ROOT . "/$destination";
      drush_print_r("\n\n\t\tmade it to destination: $config_export\n\n");
    }

    passthru("rm -rf " . BACKDROP_ROOT . "/$config_export/");
    passthru("mkdir -p $config_export");

    if (file_exists($config_export)) {
      passthru(
        "cp -r " . BACKDROP_ROOT . "/$config_dir_active/* " . " $config_export"
      );
    }
    else {
      drush_print_r("\n\n\tThe $config_export directory does not exist.\n");
      return;
    }

    drush_log(dt('Configuration is exported to !staging', array('!staging' => $config_export)), LogLevel::SUCCESS);
  }
  catch(Exception $e) {
    drush_print_r('Could not export: ' . $e);
  }
}
