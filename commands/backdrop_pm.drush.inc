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
    'callback' => 'backdrop_command_pm_download',
    'hidden' => TRUE,
    'arguments' => array(
      'module-name' => array('The name of the module(s) you would like to download.'),
    ),
    'options' => array(
      'select' => 'Select the verison of the module.',
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
function backdrop_command_pm_download() {
  $projects = func_get_args();
  $options = drush_get_option('select', '');

  if (!empty($options)) {
    drush_print_r("\t\033[32mChoose one of the available releases for $projects[0] \n");
    // get available releases.
    $tags = backdrop_command_get_tags(
      "https://updates.backdropcms.org/release-history/$projects[0]/1.x"
    );

    $project_path = backdrop_pm_get_path($tags);

    for($i = 0; $i < count($tags->releases->release); $i++) {
      $version = (string) $tags->releases->release[$i]->version;
      drush_print_r("\t\t [$i] \t:\t " . $version);
    }
    // get users's menu selection
    $handle = fopen ("php://stdin","r");
    print("\n\tSelect one [0]: ");
    print("\033[0m");
    $selection = fgets($handle);
    $selection = trim($selection);
    // default to latest release if user just hits enter.
    if (empty($selection)) {
      $selection = 0;
    }

    $sel_url = $tags->releases->release[(int) $selection]->download_link;

    // download verssion of the project user selected.
    exec(
      "wget --quiet --directory-prefix $project_path $sel_url"
    );
    // Extract the zip file.
    exec(
      "unzip $project_path/$projects[0].zip -d $project_path"
    );
    // Remove the zip file.
    exec(
      "rm $project_path/$projects[0].zip"
    );
    backdrop_pm_dl_outcome($project_path, $projects[0]);
  }
  else {
    foreach ($projects as $project) {
      if ($project != 'backdrop') {
        $html = backdrop_pm_get_from_github(
          "https://github.com/backdrop-contrib/$project/releases/latest"
        );

        // Get the release info from backdropcms.org.
        $tags = backdrop_pm_get_tags(
          "https://updates.backdropcms.org/release-history/$project/1.x"
        );

        $project_path = backdrop_pm_get_path($tags);
        $html = explode("\"", $html);
        $url = $html[1];
        $latest = explode('/', $url);
        $latest = array_reverse($latest);
        $module_install_location = $project_path . '/' . $project;

        if (is_dir($module_install_location)) {
          drush_log(dt('Module is already installed ... exiting without re-writing module.'), 'error');
          continue;
        }
        exec(
          "wget --quiet --directory-prefix $project_path https://github.com/backdrop-contrib/$project/releases/download/$latest[0]/$project.zip"
        );
        // Extract the zip file.
        exec(
          "unzip $project_path/$project.zip -d $project_path"
        );
        // Remove the zip file.
        exec(
          "rm $project_path/$project.zip"
        );
        backdrop_pm_dl_outcome($module_install_location, $project);
      }
      // Downloading backdrop itself is a special case.
      // TODO: Downloading backdrop itself does not currently work since
      //   the backdrop drush command hooks are only picked up once a valid
      //   backdrop installation is detected, so we have a chicken and the egg
      //   problem here.
      elseif ($project == 'backdrop') {
        $html = backdrop_pm_get_from_github(
          "https://github.com/backdrop/backdrop/releases/latest"
        );

        $html = explode("\"", $html);
        $url = $html[1];
        $latest = explode('/', $url);
        $latest = array_reverse($latest);

        // Get the core zip file.
        exec(
          "wget --quiet https://github.com/$project/$project/releases/download/$latest[0]/backdrop.zip"
        );
        // Extract the zip file.
        exec(
          "unzip backdrop.zip"
        );
        // Remove the zip file.
        exec(
          "rm backdrop.zip"
        );
        backdrop_pm_dl_outcome('.', 'Backdrop CMS');
      }
    }
  }
}

/**
 * Helper function for backdrop_command_pm_download().
 *
 * Gets the url for the github repo of the contrib module.
 */
function backdrop_pm_get_from_github($url) {
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, TRUE);
  curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 1);
  $content = curl_exec($ch);
  curl_close($ch);
  return $content;
}

/**
 * Helper function for --select option for download command.
 * Gets the tags for a project.
 */
function backdrop_pm_get_tags($url) {
  $xml = simplexml_load_file($url);
  return $xml;
}

/**
 * Get the path to where to store the module, theme, or layout
 * determine project_type from the $tags xml object.
 * @param $tags
 *   xml object.
 */
function backdrop_pm_get_path($tags) {
  $type = (string)$tags->type;
  if ($type == 'project_module') {
    if (file_exists(BACKDROP_ROOT . '/modules/contrib')) {
      $project_path = BACKDROP_ROOT . '/modules/contrib';
    }
    else {
      $project_path = BACKDROP_ROOT . '/modules';
    }
  }
  elseif ($type == 'project_theme') {
    $project_path = BACKDROP_ROOT . '/themes';
  }
  elseif ($type == 'project_layout') {
    $project_path = BACKDROP_ROOT . '/layouts';
  }
  return $project_path;
}

/**
 * Determine success or failure of drush dl command and give the user some
 * relevent feedback.
 * @param string $module_install_location
 *  String directory path where module should end up.
 *
 * @param string $project
 *  Project name.
 */
function backdrop_pm_dl_outcome($module_install_location, $project) {
  if (file_exists($module_install_location)) {
    print "\n\t\033[32m Success: \033[0m Project $project downloaded to $module_install_location.\n\n";
  }
  else {
    print "\n\t\033[31m Error: \033[0m Project $project could not be found.\n\n";
  }
}
