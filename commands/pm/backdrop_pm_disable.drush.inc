<?php

/**
 * @file
 * Drush project management disable command.
 */

Use \Drupal\Core\Logger\RfcLogLevel;

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
     'aliases' => array('dis',),
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
  if (!empty($projects)) {
    include_once BACKDROP_ROOT . '/core/includes/module.inc';
    module_disable($projects);
    $output_projects = '';
    foreach ($projects as $project) {
      $output_projects .= " $project";
    }
    if (count($projects) > 1) {
      $is_are = 'are';
    }
    else {
      $is_are = 'is';
    }
    drush_print_r("\t\033[32mSuccess\033[0m: $output_projects $is_are disabled.");
  }
}
