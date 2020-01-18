<?php

/**
 * @file
 * Configuration getters and setters.
 */

use \Drush\Boot\BackdropBoot;

/**
 * Implements hook_drush_command().
 */
function config_helpers_drush_command() {
  $items = array();
  $items['backdrop-config-get'] = array(
    'description' => 'Get and print a configuration setting.',
    'callback' => 'backdrop_config_get',
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
    'aliases' => array('config-get', 'cget'),
    'arguments' => array(
      'file' => 'Config file to get setting from.',
    ),
    'options' => array(
      'setting' => 'The setting to get from the config file.',
    ),
    'examples' => array(
      'drush cget system.core' => 'Print all the settings in the system.core.json file.',
      'drush cget system.core --setting=site_name' => 'Gets the site_name value from system.core.json',
    ),
  );
  $items['backdrop-config-set'] = array(
    'description' => 'Set a configuration value.',
    'callback' => 'backdrop_config_set',
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
    'aliases' => array('config-set', 'cset'),
    'arguments' => array(
      'file' => 'Config file to write config setting to.',
      'setting' => 'The setting to set a value for.',
      'value' => 'The value to set.',
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'drush cset system.core site_name "MySite"' => 'Sets the value of site_name to "My Site" in system.core.json.',
    ),
  );

  return $items;
}

/**
 * Command callback for config-get.
 */
function backdrop_config_get() {
  require_once BACKDROP_ROOT . '/core/includes/config.inc';
  $args = drush_get_arguments();
  $file = $args[1];
  $setting = drush_get_option('setting', NULL);
  $config = config_get($file, $setting);
  drush_print_r($config);
}

/**
 * Command callback for config-set.
 */
function backdrop_config_set() {
  require_once BACKDROP_ROOT . '/core/includes/config.inc';
  $args = drush_get_arguments();
  $file = $args[1];
  $setting = $args[2];
  $value = $args[3];
  try {
    config_set($file, $setting, $value);
  }
  catch (Exception $e) {
    drush_print_r("There was a problem setting the $setting setting: $e");
  }
  drush_print_r(
    dt(
      "\n\t\033[32mSuccess\033[0m 
        The $setting has been set to $value in the $file.json file"
    )
  );
}