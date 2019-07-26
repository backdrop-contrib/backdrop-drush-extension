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
  if (!empty($make['core']['core'])) {
    passthru("drush dlb backdrop --path=$project_path");
  }

  // Change into the Backdrop project directory.
  if (!is_dir($project_path)) {
    drush_log(dt("The 'build path' does not exist and 'core' was not specified."), 'error');
    return;
  }
  chdir($project_path);

  // We need to install Backdrop in order to use drush to dl modules .. idk.
  if (!empty($make['core']['core'])) {
    $db_url = drush_get_option('db-url', '');
    if (!empty($db_url)) {
      passthru(
        "drush si --db-url=$db_url"
      );
    }
    else {
      passthru("drush si");
    }
  }

  // Download the modules specified by the make file.
  if (!empty($make['modules'])) {
    bmake_parse_projects($make, 'modules');
  }

  // Download the themes specified by the make file.
  if (!empty($make['themes'])) {
    bmake_parse_projects($make, 'themes');
  }

  // Download the layouts specified by the make file.
  if (!empty($make['layouts'])) {
    bmake_parse_projects($make, 'layouts');
  }
}

/**
 * Parse a section of the makefile and download projects accordingly.
 */
function bmake_parse_projects($make, $section) {
  foreach ($make[$section] as $project => $details) {
    if (!is_array($details)) {

      // $details is a version string.
      if ($details == 'latest') {
        passthru("drush dl $project");
      }
      else {
        passthru("drush dl $project --version=$details");
      }

    }
    else {

      // $details is an array.
      if (!empty($details['git'])) {
        $valid = bmake_validate('git');
        if (!$valid) {
          return;
        }

        // Clone a git repository.
        $tags = (object) array(
          'type' => 'project_' . rtrim($section, 's'),
        );
        $path = backdrop_pm_get_path($tags);
        passthru("git clone $details[git] $path/$project");
      }

    }
  }
}

/**
 * Validate bmake commands.
 */
function bmake_validate($test) {
  // Disable possible output during testing.
  $debug = drush_get_context('DRUSH_DEBUG');
  drush_set_context('DRUSH_DEBUG', FALSE);

  // Run validation.
  switch ($test) {
    case 'git':
      $success = drush_shell_exec('git --version');
      $error = drush_set_error('DRUSH_SHELL_COMMAND_NOT_FOUND',
        dt("\n\033[1mgit\033[0m executable not found. Cloning repositories depends on git. Consider installing git on your system.\n"));
      break;

    default:
      $success = TRUE;
  }

  // Reset debugging.
  drush_set_context('DRUSH_DEBUG', $debug);

  // Display any error messages.
  if (!$success) {
    return $error;
  }
  return TRUE;
}
