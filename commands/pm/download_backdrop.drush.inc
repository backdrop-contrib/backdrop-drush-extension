<?php
/**
 * @file
 * Download Backdrop CMS with drush.
 */

/**
 * Implements hook_drush_command().
 */
function download_backdrop_drush_command() {
  $items = array();
  $items['download-backdrop'] = array(
    'description' => 'Download Backdrop CMS.',
    'callback' => 'backdrop_download_backdrop',
    'hidden' => TRUE,
    'arguments' => array(
      'module-name' => array('description' => 'The name of the module(s) you would like to download.'),
    ),
    'options' => array(
      'select' => 'Select the version of the module.',
      'path' => "Specify a path to download backdrop source code to.",
    ),
    'required-arguments' => TRUE,
    'help' => "Download Backdrop CMS source code.",
    'aliases' => array('dlb', 'dl-backdrop'),
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
  );

  return $items;
}

/**
 * Command callback. Download a Backdrop CMS.
 */
function backdrop_download_backdrop() {
  $package_handler = backdrop_download_backdrop_validate();
  if (!$package_handler) {
    return;
  }

  $projects = func_get_args();
  $options = drush_get_option('select', '');
  $download_location = drush_get_option('path', './');

  if (!empty($options)) {
    drush_print_r("\t\033[32mChoose one of the available releases for $projects[0] \033[0m\n");
    // Get available releases.
    $tags = backdrop_pm_get_tags(
      "https://updates.backdropcms.org/release-history/$projects[0]/1.x"
    );

    $project_path = $download_location;

    for ($i = 0; $i < count($tags->releases->release); $i++) {
      $version = (string) $tags->releases->release[$i]->version;
      drush_print_r("\t\t [$i] \t:\t " . $version);
    }
    // Get users's menu selection.
    $handle = fopen("php://stdin", "r");
    print ("\n\tSelect one [0]: ");
    print ("\033[0m");
    $selection = fgets($handle);
    $selection = trim($selection);
    // Default to latest release if user just hits enter.
    if (empty($selection)) {
      $selection = 0;
    }

    $sel_url = $tags->releases->release[(int) $selection]->download_link;

    // Download verssion of the project user selected.
    exec(
      "wget --quiet -O $project_path/$projects[0].zip $sel_url"
    );
    // Extract the zip file.
    exec(
      "unzip $project_path/$projects[0].zip -d $download_location"
    );
    // Remove the zip file.
    exec(
      "rm $project_path/$projects[0].zip"
    );
    backdrop_download_backdrop_dl_outcome($project_path, $projects[0]);
  }
  else {
    // Downloading backdrop itself is a special case.
    // TODO: Downloading backdrop itself does not currently work since
    // the backdrop drush command hooks are only picked up once a valid
    // backdrop installation is detected, so we have a chicken and the egg
    // problem here.
    $url = backdrop_download_backdrop_from_github(
      "https://github.com/backdrop/backdrop/releases/latest"
    );

    $latest = explode('/', $url);
    $latest = array_reverse($latest);

    // Get the core zip file.
    exec(
      "wget https://github.com/$projects[0]/$projects[0]/releases/download/$latest[0]/backdrop.zip"
    );
    // Make the Directory.
    exec(
      "mkdir -p my_tmp"
    );
    // Extract the zip file.
    exec(
      "unzip backdrop.zip -d my_tmp"
    );
    // Move to desired location.
    exec(
      "mv my_tmp/backdrop $download_location"
    );
    // Remove the zip file.
    exec(
      "rm backdrop.zip"
    );
    // Remove my_tmp.
    exec(
      "rm -rf my_tmp"
    );
    backdrop_download_backdrop_dl_outcome($download_location, 'Backdrop CMS');
  }
}

/**
 * Helper function for backdrop_command_pm_download().
 *
 * Gets the url that the input redirects to.
 * This should contain the number of the latest version.
 * For example:
 * https://github.com/backdrop/backdrop/releases/latest
 * picks up a version like:
 * https://github.com/backdrop/backdrop/releases/tag/1.25.1
 */
function backdrop_download_backdrop_from_github($url) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 1);
  curl_exec($ch);
  $curl_info = curl_getinfo($ch);
  $redirect_url = "$curl_info[redirect_url]";
  curl_close($ch);
  return $redirect_url;
}

/**
 * Helper function for --select option for download command.
 */
function backdrop_download_backdrop_get_tags($url) {
  $xml = simplexml_load_file($url);
  return $xml;
}

/**
 * Determine success or failure of drush dlb command.
 *
 * @param string $module_install_location
 *   String directory path where module should end up.
 *
 * @param string $project
 *   Project name.
 */
function backdrop_download_backdrop_dl_outcome($module_install_location, $project) {
  if (file_exists($module_install_location)) {
    print "\n\t\033[32mSuccess: \033[0m Project $project downloaded to $module_install_location.\n\n";
  }
  else {
    print "\n\t\033[31mError: \033[0m Project $project could not be found.\n\n";
  }
}

/**
 * Command backdrop_download_backdrop validate function.
 */
function backdrop_download_backdrop_validate() {
  // Check that wget command exists. Disable possible output.
  $debug = drush_get_context('DRUSH_DEBUG');
  drush_set_context('DRUSH_DEBUG', FALSE);
  $success = drush_shell_exec('wget --version');
  drush_set_context('DRUSH_DEBUG', $debug);
  if (!$success) {
    return drush_set_error(
      'DRUSH_SHELL_COMMAND_NOT_FOUND',
      dt("\n\t\033[1mwget\033[0m executable not found.
      Downloading projects with drush depends on wget consider installing wget on your system.\n")
    );
  }
  return TRUE;
}
