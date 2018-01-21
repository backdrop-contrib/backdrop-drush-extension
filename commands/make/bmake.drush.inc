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

  // Get the specifications from the users' makefile.
  $make = parse_ini_file($args[0], TRUE);

  // Get the path that user wants to put the Backdrop CMS build.
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
  
  // Download the modules specified by the make file.
  if (!empty($make['modules'])) {
    foreach ($make['modules'] as $key => $module) {
      if ($module == 'latest') {
        passthru("drush dl $key");
      }
      else {
        passthru("drush dl $key --version=$module");
      }
    }
  }

  // Download the themes specified by the make file.
  if (!empty($make['themes'])) {
    foreach ($make['themes'] as $key => $theme) {
      if ($theme == 'latest') {
        passthru("drush dl $key");
      }
      else {
        passthru("drush dl $key --version=$theme");
      }
    }
  }

    // Download the layouts specified by the make file.
  if (!empty($make['layouts'])) {
    foreach ($make['layouts'] as $key => $layout) {
      if ($layout == 'latest') {
        passthru("drush dl $key");
      }
      else {
        passthru("drush dl $key --version=$layout");
      }
    }
  }
}
