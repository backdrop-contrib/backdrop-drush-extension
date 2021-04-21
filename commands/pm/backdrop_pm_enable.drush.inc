<?php
/**
 * @file
 * Drush project management enable command.
 */

use Drupal\Core\Logger\RfcLogLevel;

/**
 * Implements hook_drush_command().
 */
function backdrop_pm_enable_drush_command() {
  $items = array();
  $items['backdrop-pm-enable'] = array(
    'description' => 'Enable backdrop modules.',
    'callback' => 'backdrop_command_pm_enable',
    'hidden' => TRUE,
    'arguments' => array(
      'module-name' => array('The name of the module(s) you would like to enable.'),
    ),
    'required-arguments' => TRUE,
    'aliases' => array('en'),
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );

  return $items;
}

/**
 * Command callback enable modules.
 *
 * @see _enable_project()
 */
function backdrop_command_pm_enable() {
  $projects = func_get_args();

  if (!isset($projects)) {
    drush_print_r(dt("\tPlease provide module name(s)\n\n"));
    return;
  }

  // Get modules present in files system that are possible to enable.
  $module_list = system_rebuild_module_data();

  // Loop through projects to handle dependencies.
  foreach ($projects as $project) {
    // Check if requested module depends on other modules.
    $dependencies = $module_list[$project]->info['dependencies'];
    if (!empty($dependencies)) {
      foreach ($dependencies as $dependency) {
        // Prepend dependency to projects list.
        if (!module_exists($dependency) && !in_array($dependency, $projects)) {
          array_unshift($projects, $dependency);
        }
      }
    }
  }

  $projects_list = implode(', ', $projects);
  $projects_list = drush_chalk($projects_list, '1m');
  $proceed = drush_confirm(
    "The following projects will be enabled: $projects_list.
    Do you want to enable the projects?"
  );

  if (!$proceed) {
    $cancelled = drush_chalk('Cancelled', '33m');
    drush_print_r(
      dt("\n\t$cancelled: $projects_list not enabled.\n")
    );
  }
  else {
    // Enable the projects specified on the cli with dependencies.
    foreach ($projects as $project) {
      $status = _enable_project($project, $module_list);
      if (!$status) {
        // _enable_project() already output an error message.
        return FALSE;
      }
    }

    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to enable module or theme.
 *
 * @param string $project
 *   The project machine name to be enabled.
 *
 * @param array $module_list
 *   List of modules that exist in the file system.
 *
 * @return bool
 *   TRUE if the module is enabled; FALSE otherwise.
 *
 * @see backdrop_command_pm_enable_validate()
 */
function _enable_project($project, $module_list) {
  // Check against the module_list.
  $project_exists = backdrop_command_pm_enable_validate($project, $module_list);
  // If the $project directory does not exist then gracefully fail.
  if (!$project_exists) {
    $error = drush_chalk('Error', '31m');
    $project = drush_chalk($project, '1m');
    drush_print_r("\n\t$error $project does not exist in your Backdrop installation.");
    drush_print_r("\tTry downloading $project first with the command: drush dl $project\n");
    return FALSE;
  }
  $query = db_select('system', 's')
    ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();

  if ($module['status']) {
    $failed = drush_chalk('Failed', '31m');
    $module_name = drush_chalk($module['name'], '1m');
    drush_print_r(
      "\n\t$failed to enable module $module_name: it is already enabled.\n"
    );
    return FALSE;
  }

  if (module_enable(array($project), FALSE)) {
    $success = drush_chalk('Success', '32m');
    $project = drush_chalk($project, '1m');
    drush_print_r("\n\t$success: module $project enabled.\n");
    return TRUE;
  }
  $failed = drush_chalk('Failed', '31m');
  $project = drush_chalk($project, '1m');
  drush_print_r("\n\t$failed: to enable module " . $project);
  return FALSE;
}

/**
 * Command pm-update validate function.
 *
 * @param string $project
 *   The project that the user is attempting to enable.
 */
function backdrop_command_pm_enable_validate($project, $module_list) {
  if (array_key_exists($project, $module_list)) {
    return TRUE;
  }

  return FALSE;
}
