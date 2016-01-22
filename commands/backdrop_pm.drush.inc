<?php
/**
 * Package management Drush commands for Backdrop.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_pm_drush_command() {
  $items = array();

  $items['backdrop-pm-download'] = array(
    'description' => 'Download Backdrop CMS contrib modules.',
    'callback' => 'drush_backdrop_pm_download',
    'hidden' => TRUE,
    'arguments' => array(
      'module-name' => array('The name of the module you would like to download.'),
    ),
    'required-arguments' => TRUE,
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_SITE,
  );

  return $items;
}

/**
 * Command callback. Download a Backdrop CMS contrib project.
 *
 * @param $projects
 *  Array of Backdrop CMS contrib projects to download.
 */
function drush_backdrop_pm_download() {
  $projects = func_get_args();
  foreach ($projects as $project) {
    if ($project != 'backdrop') {
      $html = drush_backdrop_get_from_github(
        "https://github.com/backdrop-contrib/$project/releases/latest"
      );

      $html = explode("\"", $html);
      $url = $html[1];
      $latest = explode('/', $url);
      $latest = array_reverse($latest);
      if (file_exists(BACKDROP_ROOT . '/modules/contrib')) {
        $module_path = BACKDROP_ROOT . '/modules/contrib';
      }
      else {
        $module_path = BACKDROP_ROOT . '/modules';
      }
      $module_install_location = $module_path . '/' . $project;
      if (is_dir($module_install_location)) {
        drush_log(dt('Module is already installed ... exiting without re-writing module.'), 'error');
        continue;
      }
      exec(
        "wget --quiet --show-progress --directory-prefix $module_path https://github.com/backdrop-contrib/$project/releases/download/$latest[0]/$project.zip"
      );
      // Extract the zip file.
      exec(
        "unzip $module_path/$project.zip -d $module_path"
      );
      // Remove the zip file.
      exec(
        "rm $module_path/$project.zip"
      );
    }
    // Downloading backdrop itself is a special case.
    elseif ($project == 'backdrop') {
      $html = drush_backdrop_get_from_github(
        "https://github.com/backdrop/backdrop/releases/latest"
      );

      $html = explode("\"", $html);
      $url = $html[1];
      $latest = explode('/', $url);
      $latest = array_reverse($latest);

      // Get the core zip file.
      exec(
        "wget --quiet --show-progress https://github.com/$project/$project/releases/download/$latest[0]/backdrop.zip"
      );
      // Extract the zip file.
      exec(
        "unzip backdrop.zip"
      );
      // Remove the zip file.
      exec(
        "rm backdrop.zip"
      );
    }
    drush_log(dt('Successfully downloaded.'));
  }
}

/**
 * Helper function for drush_backdrop_pm_download().
 *
 * Gets the url for the github repo of the contrib module.
 */
function drush_backdrop_get_from_github($url) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 1);
  $content = curl_exec($ch);
  curl_close($ch);
  return $content;
}
