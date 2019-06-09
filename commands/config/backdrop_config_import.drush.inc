<?php
/**
 * @file
 * Contains command callback for drush bakdrop-config-import.
 */

use \Drush\Boot\BackdropBoot;
use Drush\Log\LogLevel;

/**
 * Implements hook_drush_command().
 */
function backdrop_config_import_drush_command() {
  $items = array();
  $items['backdrop-config-import'] = array(
    'description' => 'Import Backdrop site configuration.',
    'callback' => 'backdrop_config_import',
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
    'aliases' => array('bcim'),
    'examples' => array(
      'drush bcim' => 'Imports the configuration for the current site from the staging config directory to the active config directory.',
    ),
  );

  return $items;
}

/**
 * Command callback for backdrop-config-import.
 */
function backdrop_config_import() {
  require_once BACKDROP_ROOT . '/settings.php';
  require_once BACKDROP_ROOT . '/core/modules/config/config.admin.inc';
  require_once BACKDROP_ROOT . '/core/includes/config.inc';
  $config_dir_active = config_get_config_directory('active');
  $config_dir_staging = config_get_config_directory('staging');
  $config_statuses = config_get_statuses();

  // Prepare the table of config changes to show the user.
  foreach ($config_statuses as $key => $config_status) {
    if ($config_status == 'update') {
      $rows[] = array("\t", $key, "\033[33mupdate\033[0m");
    }
    elseif ($config_status == 'delete') {
      $rows[] = array("\t", $key, "\033[31mdelete\033[0m");
    }
    elseif ($config_status == 'create') {
      $rows[] = array("\t", $key, "\033[32mcreate\033[0m");
    }
  }

  if (!empty($rows)) {
    drush_print(dt("The following configuration changes will be imported: \n"));
    $table = drush_format_table($rows);
    drush_print_r($table);
  }
  else {
    drush_log(dt("There are no configuration changes to import."), LogLevel::SUCCESS);
    return;
  }

  if (!drush_confirm(dt('Do you really want to continue?'))) {
    return drush_user_abort();
  }
  try {
    if (!empty($config_dir_active)) {
      passthru(
        "rm -rf " . BACKDROP_ROOT . "/$config_dir_active/*"
      );
      passthru(
        "cp -a " . BACKDROP_ROOT . "/$config_dir_staging/*  " . BACKDROP_ROOT . "/$config_dir_active/"
      );
    }
    drush_log(dt('Configuration imported to !active', array('!active' => $config_dir_active)), LogLevel::SUCCESS);
  }
  catch (Exception $e) {
    drush_log(dt("Configuration import failed. $e"), LogLevel::DEBUG);
  }
}
