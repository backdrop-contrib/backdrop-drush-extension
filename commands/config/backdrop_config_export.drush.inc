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
    $config_dir_staging = $config_directories['staging'];
    passthru(
      "rm -rf " . BACKDROP_ROOT . "/$config_dir_staging/" &&
      "mkdir " . BACKDROP_ROOT . "/$config_dir_stagig"
    );
    // passthru(
    //   "cp " . BACKDROP_ROOT . "/$config_dir_active/\* " . BACKDROP_ROOT . "/$config_dir_staging"
    // );
    drush_print_r("\n\n\tmaybe this is the big time?\n\n");
    foreach (glob(BACKDROP_ROOT . "/$config_dir_active/*.json") as $config_file) {
      copy($config_file, BACKDROP_ROOT . "/$cofig_dir_staging");
    }
    drush_log(dt('Configuration is exported to !staging', array('!staging' => $config_dir_staging)), LogLevel::SUCCESS);
  }
  catch(Exception $e) {
    drush_print_r('couldn\'t do it ' . $e);
  }
}
