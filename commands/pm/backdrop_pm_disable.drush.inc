<?php

/**
 * @file
 * Drush project management disable command.
 */

use Drush\Log\LogLevel;

/**
 * Implements hook_drush_command().
 */
function backdrop_pm_disable_drush_command() {
  $items = array();
  $items['backdrop-pm-disable'] = array(
    'description' => 'Disable backdrop modules.',
    'callback' => 'backdrop_command_pm_disable',
    'arguments' => array(
      'module-name' => array('The name of the module(s) you would like to disable.'),
    ),
    'aliases' => array('dis'),
    'required-arguments' => TRUE,
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );

  return $items;
}

/**
 * Command callback for pm-disable.
 */
function backdrop_command_pm_disable() {
  $projects = func_get_args();

  // Get modules present in files system that are possible to disable.
  $module_list = system_rebuild_module_data();

  foreach ($projects as $project) {
    // Check if requested module is required by other modules.
    $required_bys = $module_list[$project]->required_by;

    $kids = [];
    if (!empty($required_bys)) {
      foreach ($required_bys as $key => $required) {
        if (module_exists($key) && !in_array($key, $projects)) {
          array_unshift($projects, $key);
          $kids = array_merge($projects, $kids);
        }
        else {
          // Kids is already accounted for.
        }
      }
    }
  }

  $operating_list = ($kids) ? implode(',', $kids) : implode(', ', $projects);
  $proceed = drush_confirm(
    "The following projects will be disabled: $operating_list.
    Do you want to disable the projects?"
  );

  if (!$proceed) {
    drush_print_r(
      dt("\n\t\e[033mCancelled\e[0m $operating_list not disabled.\n")
    );
  }
  elseif (!empty($kids)) {
    foreach ($kids as $kid) {
      module_disable([$kid]);
    }
  }
  else {
    // Now disable the projects specified on the command line.
    foreach ($projects as $project) {
      module_disable([$project]);
    }
  }

  drush_print_r(
    dt("\n\t\033[32mSuccess\033[0m: $operating_list are disabled.\n")
  );
}
