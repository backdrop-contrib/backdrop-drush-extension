<?php
/**
 * @file
 * Drush project management enable command.
 */

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
    'aliases' => array('en', 'pm-enable'),
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );

  return $items;
}

/**
 * Command callback enable modules.
 *
 * @see _enable_project().
 */
function backdrop_command_pm_enable() {
  $projects = func_get_args();

  if (!isset($projects)) {
    drush_print_r(dt("\tPlease provide module name(s)\n\n"));
    return;
  }

  $clear_cache = FALSE;
  foreach($projects as $project ) {
    if (_enable_project($project)) {
      $clear_cache = TRUE;
    }
  }

  if ($clear_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to enable module or theme.
 *
 * @param string $project
 *  The project machine name to be enabled.
 *
 * @return bool
 *
 * @see backdrop_command_pm_enable_validate()
 */
function _enable_project($project) {
  $project_exists = backdrop_command_pm_enable_validate($project);
  // If the $project directory does not exist then gracefully fail.
  if (!$project_exists) {
    drush_print_r("\n\t\e[031mError\e[0m $project does not exist in your Backdrop installation.");
    drush_print_r("\tTry downloading $project first with the command: drush dl $project\n");
    return FALSE;
  }
  $query = db_select('system', 's')
    ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();

  if ($module['status']) {
    drush_print_r(
      "\n\t\e[31m Failed\e[0m to enable module " . $module['name']. ": it is already enabled.\n"
    );
    return FALSE;
  }

  if (!drush_confirm(dt("Do you want to enable $project?"))) {
    drush_print_r(
      dt("\n\t\e[033mCancelled\e[0m $project not enabled.\n")
    );
    return FALSE;
  }

  if (module_enable(array($project), FALSE)) {
    drush_print_r("\n\t\e[32mSuccess\e[0m module $project enabled.\n");
    return TRUE;
  }
  drush_print_r("\n\t\e[31mFailed\e[0m to enable module " . $project);
  return FALSE;
}

/**
 * Command pm-update validate function.
 *
 * @param string $project
 *   The project that the user is attempting to enable.
 */
function backdrop_command_pm_enable_validate($project) {
  $file = file_exists(BACKDROP_ROOT . "/modules/contrib/$project") ||
    file_exists(BACKDROP_ROOT . "/modules/$project") ||
    file_exists(BACKDROP_ROOT . "/core/modules/$project") ||
    file_exists(BACKDROP_ROOT . "/modules/custom/$project");
  if ($file) {
    return TRUE;
  }
  return FALSE;
}
