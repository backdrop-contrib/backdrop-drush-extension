<?php

/**
 * Fix permissions Drush commands for Backdrop.
 * Based on fix-permissions.sh script.
 *
 * Example:
 * drush fix-permissions --backdrop_path=/path/to/the/bacdrop/installation --backdrop_user=your-user --httpd_group=_www
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_fix_permissions_drush_command() {
  $items = array();
  $items['fix-permissions'] = array(
    'description' => 'Set permissions, ownership and groups for Backdrop files and directories.',
    'callback' => 'backdrop_fix_permissions',
    'options' => array(
      'backdrop_path' => dt('The path to your Backdrop installation.'),
      'backdrop_user' => dt('This is the user that will own the Backdrop files.'),
      'httpd_group' => dt('This is the user that runs the webserver: mac: _www, ubuntu: www-data, centos: apache'),
    ),
    'required-arguments' => FALSE,
    'aliases' => array('fp'),
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_SITE,
    'examples' => array(
      'Example' => 'drush fix-permissions --backdrop_path=/path/to/the/backdrop/install --backdrop_user=geoff --httpd_group=_www',
    )
  );
  return $items;
}

/**
 * Command callback for fix-permissions.
 */
function backdrop_fix_permissions() {
  $backdrop_path = drush_get_option('backdrop_path', '');
  $backdrop_user = drush_get_option('backdrop_user', '');
  $httpd_group = drush_get_option('httpd_group', '');
  if ($backdrop_path == '') {
    $backdrop_path = drush_prompt("Enter the path to your Backdrop installation", BACKDROP_ROOT);
  }
  if ($backdrop_user == '') {
    $backdrop_user = drush_prompt("Enter the user that will own the Backdrop files");
  }
  if ($httpd_group == '') {
    $httpd_group = drush_prompt("Enter the group that the Backdrop files should belong to");
  }
  if (file_exists(__DIR__ . '/fix-permissions.sh')) {
    $exit = passthru(
      "sudo " . __DIR__ . "/fix-permissions.sh --backdrop_path=$backdrop_path --backdrop_user=$backdrop_user --httpd_group=$httpd_group"
    );
    if ($exit == 0) {
      drush_print_r("\n\t\e[32mSucessfully\e[0m: set permissions with fix-permissions script. :) happy coding!\n");
    }
  }
  else {
    drush_print_r("\n\t\e[31mFailed\e[0m:not so much.\n");
  }
}
