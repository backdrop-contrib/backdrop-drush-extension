<?php

/**
 * @file
 * Make a working Backdrop CMS profile with drush bmake.
 */

/**
 * Implements hook_drush_command().
 */
function bmake_drush_command() {
  $itmes = array();
  $items['bmake'] = array(
    'description' => 'Turns a makefile into a working Backdrop codebase.',
    'callback' => 'bmake',
    'arguments' => array(
      'makefile' => 'Filename of the makefile to use for this build',
      'build path' => 'The path at which to build the makefile.',
    ),
    'examples' => array(
      'drush bmake example.yml mybuild' => 'Build the project described in example.yml in the mybuild directory.',
    ),
    'options' => array(
      'contrib-destination' => 'Specify a path to put the contrib modules. Defaults to BACKDROP_ROOT/modules.',
      'db-url' => 'Give the database paramaters for the site-install.',
    ),
    'required-arguments' => TRUE,
    'aliases' => array('bm'),
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
  );

  return $items;
}

/**
 * Command callback. Make Backdrop CMS project.
 */
function bmake() {
  $args = func_get_args();

  $make = yaml_parse_file($args[0]);
  $project_path = $args[1];

  // Get Backdrop and put it in the project_path.
  passthru("drush dlb backdrop --path=$project_path");

  // Change into the Backdrop project directory.
  chdir($project_path);

  // We need to install Backdrop in order to use drush to dl modules .. idk 
  $db_url = drush_get_option('db-url', '');
  if (!empty($db_url)) {
    passthru(
      "drush si --db-url=$db_url"
    );
  }
  else {
    passthru("drush si");
  }
  
  // Dowload the modules specified by the make file.
  if (!empty($make['modules'])) {
    foreach ($make['modules'] as $key => $module) {
      exec("drush dl $key");  
    }
  }

  // Dowload the themes specified by the make file.
  if (!empty($make['themes'])) {
    foreach ($make['themes'] as $key => $theme) {
      exec("drush dl $key");
    }
  }

    // Dowload the modules specified by the make file.
  if (!empty($make['layouts'])) {
    foreach ($make['layouts'] as $key => $layouts) {
      exec("drush dl $key");
    }
  }
}
