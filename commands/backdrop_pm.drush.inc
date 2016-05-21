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
  $items['backdrop-pm-update'] = array(
    'description' => 'Update Backdrop core or contrib modules.',
    'callback' => 'backdrop_command_pm_update',
    'arguments' => array(
      'module-name' => array('The name of the module(s) you would like to update.'),
    ),
    // TODO: show available updates and allow for selection of which to apply.
    //'options' => array(
    //  'show' => "Select from available updates:"
    //),
    'aliases' => array('up',),
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

/**
 * Command callback for pm-update.
 * Currently supports druhs up backdrop only.
 * TODO: support updating modules.
 */
function backdrop_command_pm_update() {
  $projects = func_get_args();
  $projects[0] = strtolower($projects[0]);
  if ($projects[0] === 'backdrop') {
    // get current Backdrop version info.
    $current_version = BACKDROP_VERSION;
    $parse_version = explode('.', $current_version);
    $current_version_major = $parse_version[0];
    $current_version_minor = $parse_version[1];
    $current_version_patch = $parse_version[2];
    // Get proposed Backdrop version info.
    $xml = backdrop_pm_get_tags("https://updates.backdropcms.org/release-history/backdrop/1.x");
    $download_link = $xml->releases->release->download_link;
    $proposed_version = $xml->releases->release->version;
    $proposed_version_major = $xml->releases->release->version_major;
    $proposed_version_minor = $xml->releases->release->version_minor;
    $proposed_version_patch = $xml->releases->release->version_patch;
    if ($current_version == $proposed_version) {
      drush_print_r("\n\t\tBackdrop is up to date :).\n");
    }
    elseif ($current_version_major < $proposed_version_major) {
      drush_print_r("\n\t\tYou can not use drush up to move between Major Versions of Backdrop.");
    }
    elseif (
      (
        $current_version_major == $proposed_version_major &&
        $current_version_minor == $proposed_version_minor &&
        $current_version_patch < $proposed_version_patch
      ) ||
      (
        $current_version_major == $proposed_version_major &&
        $current_version_minor < $proposed_version_minor
      )
    ) {
      drush_print_r("\n\t\tCurrent Backdrop Version: $current_version
      \t\tProposed Backdrop Version: $proposed_version\n");
      drush_print_r("\tCode updates will be made to Backdrop core.\n");
      drush_print_r("\t\e[33mWARNING\033[0m:  Updating core will discard any
      modifications made to Backdrop core files.  If you have made any
      modifications to these files, please back them up before updating so that
      you can re-create your modifications in the updated version of the file.\n\n");
      drush_print_r("\tNote: Updating core can potentially break your site.
      It is NOT recommended to update production sites without prior testing.\n\n");
      $answer = drush_prompt("Do you wish to continue? (y or n)");
      $answer = strtolower($answer);
      if ($answer == 'y' || $answer == 'yes') {
        drush_print_r("\t Backing up current core folder to ~/.drush/core-$current_version");
        $backdrop_root = BACKDROP_ROOT;
        exec("mv $backdrop_root/core ~/.drush/core-$current_version");
        exec("mkdir -p $backdrop_root/drush-tmp");
        exec("wget --quiet -P $backdrop_root/drush-tmp $download_link");
        exec("unzip $backdrop_root/drush-tmp/backdrop.zip -d $backdrop_root/drush-tmp");
        exec("mv $backdrop_root/drush-tmp/backdrop/core $backdrop_root/core");
        exec("rm -r $backdrop_root/drush-tmp");
        exec("drush updb -y");
        drush_print_r("\n\tYou can retrieve your old core files, should you need
        to from: ~/.drush/core-$current_version");
        drush_print_r("\t\e[32mSuccess\033[0m: Backdrop updated to $proposed_version.");
      }
      else {
        drush_print_r("\tBackdrop core update cancelled.");
      }
    }
    else {
      drush_print_r("\n\tNo Backdrop core updates available. Your Backdrop is up to date!\n");
    }
  }
}
