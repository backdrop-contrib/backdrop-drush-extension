<?php
/**
 * @file
 * Drush project management disable command.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_pm_disable_drush_command() {
  $items = array();
  $items['backdrop-pm-disable'] = array(
    'description' => 'Disable backdrop module(s).',
    'callback' => 'backdrop_command_pm_disable',
    'hidden' => TRUE,
    'arguments' => array(
      'module-name' => array('The name of the module(s) you would like to disable.'),
    ),
    'required-arguments' => TRUE,
    'aliases' => array('dis', 'pm-disable'),
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );

  return $items;
}

/**
 * pm-disable callback. Disable module or theme.
 *
 * @return bool
 *
 * @see _disable_project($project).
 */
function backdrop_command_pm_disable() {
  $projects = func_get_args();
  if (!isset($projects)) {
    drush_print_r(dt("\n\n\t\e[31mError[0m Please provide at least one project name.\n"));
    return FALSE;
  }

  $clear_cache = FALSE;
  foreach($projects as $project ) {
    if (_disable_project($project)) {
      $clear_cache = TRUE;
    }
  }

  if ($clear_cache) {
    backdrop_flush_all_caches();
  }
}

/**
 * Internal function to disable module(s).
 *
 * @param string $project
 *  Project machine name to be disabled.
 *
 * @return bool
 */
function _disable_project($project) {
  $query = db_select('system', 's')
    ->fields('s');
  $query->condition('name', $project);
  $query->condition('type', 'module');
  $module = $query->execute()->fetchAssoc();
  if (!$module) {
    return FALSE;
  }

  if (!$module['status']) {
    drush_print_r(
      "\n\t\e[031mFailed\e[0m to disable $project module: it is already disabled.\n"
    );
    return FALSE;
  }

  if (!drush_confirm(dt("\n\tDo you want to disable $project?"))) {
    drush_print_r(
      dt("\n\t\e[33mCancelled\e[0m $project was not disabled.\n")
    );
    return FALSE;
  }

  module_disable(array($project), FALSE);
  drush_print_r("\n\t\e[32mSuccess\e[0m module $project disabled.\n");
  return TRUE;
}