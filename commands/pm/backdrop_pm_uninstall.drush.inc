<?php

/**
 * @file
 * Drush project management uninstall command.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_pm_uninstall_drush_command() {
  $items = array();
  $items['backdrop-pm-uninstall'] = array(
    'description' => 'Uninstall backdrop modules.',
    'callback' => 'backdrop_command_pm_uninstall',
    'arguments' => array(
      'module-name' => array('description' => 'The name of the module(s) you would like to uninstall.'),
    ),
    'aliases' => array('pmu'),
    'required-arguments' => TRUE,
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );

  return $items;
}

/**
 * Command callback for pm-uninstall.
 */
function backdrop_command_pm_uninstall() {
  include_once BACKDROP_ROOT . '/core/includes/install.inc';
  $projects = func_get_args();

  $all_modules = system_rebuild_module_data();
  $disabled_modules = array();
  $operating_list = array();
  foreach ($projects as $key => $project) {
    if (
      $all_modules[$project]->status == '0' &&
      $all_modules[$project]->schema_version > -1
    ) {
      // Check if requested module is required by other modules.
      $required_bys = $all_modules[$project]->required_by;
      $kids = array();
      if (!empty($required_bys)) {
        foreach ($required_bys as $kid_key => $required) {
          if (!in_array($kid_key, $projects)) {
            array_unshift($projects, $kid_key);
            $kids = array_merge($projects, $kids);
          }
          else {
            // Kids is already accounted for.
          }
        }
      }
      $disabled_modules[$project] = $all_modules[$project];
    }
  }
  $operating_list = ($kids) ? implode(', ', $kids) : implode(', ', $projects);
  $is_are = (count($disabled_modules) == 1) ? 'is' : 'are';
  if (empty($operating_list)) {
    $project_list = implode(', ', $projects);
    drush_print_r(
      dt("\n\t\e[033mWarning\e[0m: No modules can be uninstalled; make sure \e[1m$project_list\e[0m $is_are disabled first.\n")
    );
    return;
  }
  $proceed = drush_confirm(
    "The following projects will be uninstalled: \e[1m$operating_list\e[0m.
    Do you want to uninstall (could lose data) the projects?"
  );

  if (!$proceed) {
    drush_print_r(
      dt("\n\t\e[033mCancelled\e[0m $operating_list not uninstalled.\n")
    );
  }
  elseif (!empty($kids)) {
    try {
      backdrop_uninstall_modules($kids);
      drush_print_r(
        dt("\n\t\033[32mSuccess\033[0m: \e[1m$operating_list\e[0m $is_are uninstalled.\n")
      );
    }
    catch(Exception $e) {
      drush_print_r(
        dt("\n\t\033[33mnope\033[0m: \e[1m$operating_list\e[0m $is_are uninstalled.\n")
      );
    }
  }
  else {
    try {
      backdrop_uninstall_modules($projects);
      drush_print_r(
        dt("\n\t\033[32mSuccess\033[0m: \e[1m$operating_list\e[0m $is_are uninstalled.\n")
      );
    }
    catch(Exception $e) {
      print_r('nope');
    }
  }
}
