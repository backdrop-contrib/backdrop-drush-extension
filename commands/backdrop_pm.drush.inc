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
function drush_backdrop_pm_download() {
  $projects = func_get_args();
  $options = drush_get_option('select', '');

  // get location for moudules.
  if (file_exists(BACKDROP_ROOT . '/modules/contrib')) {
    $module_path = BACKDROP_ROOT . '/modules/contrib';
  }
  else {
    $module_path = BACKDROP_ROOT . '/modules';
  }

  if (!empty($options)) {
    drush_print_r("\t\033[32mChoose one of the available releases for $projects[0] \n");
    // get available releases.
    $tags = drush_backdrop_get_tags("https://api.github.com/repos/backdrop-contrib/$projects[0]/tags");

    for($i = 0; $i < count($tags); $i++) {
      drush_print_r("\t\t [$i] \t:\t " . $tags[$i]->name);
    }
    // get users's menu selection
    $handle = fopen ("php://stdin","r");
    print("\n\tSelect one [0]: ");
    print("\033[0m");
    $selection = fgets($handle);
    $selection = trim($selection);
    if (empty($selection)) {
      $selection = 0;
    }
    $ver = $tags[$selection]->name;
    $sel_url = "https://github.com/backdrop-contrib/$projects[0]/releases/download/$ver/$projects[0].zip";
    // download verssion of the project user selected.
    exec(
      "wget --quiet --show-progress --directory-prefix $module_path $sel_url"
    );
    // Extract the zip file.
    exec(
      "unzip $module_path/$projects[0].zip -d $module_path"
    );
    // Remove the zip file.
    exec(
      "rm $module_path/$projects[0].zip"
    );
    return;
  }
  else {
    foreach ($projects as $project) {
      if ($project != 'backdrop') {
        $html = drush_backdrop_get_from_github(
          "https://github.com/backdrop-contrib/$project/releases/latest"
        );

        $html = explode("\"", $html);
        $url = $html[1];
        $latest = explode('/', $url);
        $latest = array_reverse($latest);
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

/**
 * Helper function for --select option for download command.
 * Gets the tags for a project.
 */
function drush_backdrop_get_tags($url) {
  $ch = curl_init();
  $useragent='Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)';
  $access_token = 'token bc52fb8c1cf19eb995c63f7a39568ec4a93f861e';

  // set URL and other appropriate options
  curl_setopt($ch, CURLOPT_URL, $url);
  curl_setopt($ch, CURLOPT_HTTPHEADER, array(
     "Authorization: $access_token",
     'Accept: application/json',
    )
  );
  //curl_setopt($ch, CURLOPT_HEADER, 0);
  curl_setopt($ch, CURLOPT_USERAGENT, $useragent); //set our user agent
  curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
  $output = curl_exec($ch);
  $output = json_decode($output);

  // close curl resource, and free up system resources
  curl_close($ch);
  return $output;
}
