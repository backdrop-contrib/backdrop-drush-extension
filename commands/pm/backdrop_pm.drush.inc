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
      'select' => 'Select the version of the module.',
      'version' => 'Specify the tagged version of the module that you want. (This overrides the --select option.',
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
  $package_handler = backdrop_command_pm_update_validate();
  if (!$package_handler) {
    return;
  }
  $projects = func_get_args();
  $options['select'] = drush_get_option('select', '');
  $options['version'] = drush_get_option('version', '');

  if (!empty($options['select']) || !empty($options['version'])) {
    // get available releases.
    $tags = backdrop_pm_get_tags(
      "https://updates.backdropcms.org/release-history/$projects[0]/1.x"
    );

    $project_path = backdrop_pm_get_path($tags);

    if (empty($options['version'])) {
      drush_print_r("\t\033[32mChoose one of the available releases for $projects[0] \033[0m\n");

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
    }
    else {
      $releases = (array) $tags->releases;
      // Check how many releases we have. One release requires special handling.
      $num_releases = count($releases['release']);
      if ($num_releases > 1 && gettype($releases['release']) == 'array') {
        foreach ((array) $releases['release'] as $release) {
          if ((string) $release->version == $options['version']) {
            $sel_url = (string) $release->download_link;
          }
        }
      }
      elseif (gettype($releases['release']) == 'object') {
        $sel_url = (string) $releases['release']->download_link;
      }

      // If the $sel_url is still empty we will not be able to get the project.
      if (empty($sel_url)) {
        drush_print_r('You must specify a valid version string. Project ' . $projects[0] . ' does not have a version: ' . $options['version']. '.');
        return;
      }
    }

    // Download the version of the project user specified.
    exec(
      "wget --quiet -O $project_path/$projects[0].zip $sel_url"
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

        // Build the path to location to download the module.
        $module_install_location = $project_path . '/' . $project;

        // Build the download URL.
        $download_url = "https://github.com/backdrop-contrib/$project/releases/download/$latest[0]/$project.zip";

        // Get the HTTP/1.1 response to check against.
        $response = get_headers($download_url);

        // If the http response is 404 gracefully bow out; offer user the dev version?
        if (strpos($response[0], '404') !== FALSE) {
          drush_print_r(
            dt("\n\t\e[31mError\e[0m: There is no release located at $download_url." .
              "\n\tTry visiting the \e[1m$project\e[0m page directly: https://github.com/backdrop-contrib/$project\n"
            )
          );

          continue;
        }

        if (is_dir($module_install_location)) {
          drush_log(dt('Module is already installed ... exiting without re-writing module.'), 'error');
          continue;
        }
        exec(
          "wget --quiet -O $project_path/$project.zip $download_url"
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
  if (!empty(drush_get_context('DRUSH_BACKDROP_SITE_ROOT'))) {
    $site_root = realpath(BACKDROP_ROOT . '/' . drush_get_context('DRUSH_BACKDROP_SITE_ROOT'));
  }
  else {
    $site_root = BACKDROP_ROOT;
  }

  if ($type == 'project_module') {
    if (file_exists($site_root . '/modules/contrib')) {
      $project_path = $site_root . '/modules/contrib';
    }
    else {
      $project_path = $site_root . '/modules';
    }
  }
  elseif ($type == 'project_theme') {
    $project_path = $site_root . '/themes';
  }
  elseif ($type == 'project_layout') {
    $project_path = $site_root . '/layouts';
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
    print "\n\t\033[32mSuccess: \033[0m Project $project downloaded to $module_install_location.\n\n";
  }
  else {
    print "\n\t\033[31mError: \033[0m Project $project could not be found.\n\n";
  }
}

/**
 * Command pm-update validate function.
 */
function backdrop_command_pm_update_validate() {
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

/**
 * Command callback for pm-update.
 */
function backdrop_command_pm_update() {
  // Call validate function to check for required curl or wget.
  $package = backdrop_command_pm_update_validate();
  // If neither curl or wget are available bail.
  if (!$package) {
    return;
  }

  // Grab the transliteration.inc file so we can sanitize $site_name string.
  require_once 'core/includes/transliteration.inc';

  $projects = func_get_args();
  foreach($projects as $project) {
    $project = strtolower($project);
    $project = rtrim($project, '/');
    $backdrop_root = BACKDROP_ROOT;
    if ($project === 'backdrop' || $project == 'core') {
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
        drush_print_r("\n\t\tCurrent \033[1mBackdrop\033[0m Version:\t$current_version");
        drush_print_r("\t\tProposed \033[1mBackdrop\033[0m Version:\t$proposed_version\n");
        drush_print_r("\tCode updates will be made to Backdrop core.\n");
        drush_print_r("\t\e[33mWARNING\e[0m: Updating core will discard any");
        drush_print_r("\tmodifications made to Backdrop core files. If you have made any");
        drush_print_r("\tmodifications to these files, please back them up before updating so that");
        drush_print_r("\tyou can re-create your modifications in the updated version of the file.\n\n");
        drush_print_r("\tNote: Updating core can potentially break your site.");
        drush_print_r("\tIt is NOT recommended to update production sites without prior testing.\n\n");
        $answer = drush_prompt("\t\e[34mDo you wish to continue?\e[0m (y or n)");
        $answer = strtolower($answer);
        if ($answer == 'y' || $answer == 'yes') {
          $users_home_directory = $_SERVER['HOME'];
          $site_name = strtolower(transliteration_clean_filename(config_get('system.core', 'site_name')));
          $timestamp = date_timestamp_get(date_create());
          drush_print_r("\tBacking up current core folder to $users_home_directory/drush-backups/$site_name/$timestamp/core-$current_version");
          exec("mkdir -p $users_home_directory/.drush/drush-tmp");
          if (!file_exists("$users_home_directory/.drush/drush-tmp/")) {
            drush_print_r("\t\e[31mFailed\e[0m: The drush-tmp directory could not be created.\n\n");
            return FALSE;
          }
          exec("wget --quiet -O $users_home_directory/.drush/drush-tmp/backdrop.zip $download_link");
          exec("unzip $users_home_directory/.drush/drush-tmp/backdrop.zip -d $users_home_directory/.drush/drush-tmp");
          foreach (glob("$users_home_directory/.drush/drush-tmp/backdrop-backdrop*") as $glob) {
            rename($glob, "$users_home_directory/.drush/drush-tmp/backdrop");
          }
          if (file_exists("$users_home_directory/.drush/drush-tmp/backdrop")) {
            // Backup current core directory.
            exec("mkdir -p $users_home_directory/drush-backups/$site_name/$timestamp/");
            exec("mv $backdrop_root/core $users_home_directory/drush-backups/$site_name/$timestamp/core-$current_version");
            exec("mv $users_home_directory/.drush/drush-tmp/backdrop/core $backdrop_root/core");
            exec("rm -r $users_home_directory/.drush/drush-tmp");
          }
          else {
            drush_print_r("\t\e[031mError\e[0m: New version of backdrop could not be found.");
            return FALSE;
          }

          // Run db updates and clear cache.
          backdrop_command_updatedb();
          backdrop_flush_all_caches();

          drush_print_r("\n\tYou can retrieve your old core files, should you need
          to from: $users_home_directory/drush-backups/$site_name/$timestamp/core-$current_version");
          if (file_exists("$backdrop_root/core")) {
            drush_print_r("\t\e[32mSuccess\033[0m: Backdrop updated to $proposed_version.");
          }
          else {
            drush_print_r("\t\033[31mFailed\033[0m: Upgrade failed.");
          }
        }
        else {
          drush_print_r("\tBackdrop core update cancelled.");
        }
      }
      else {
        drush_print_r("\n\tNo Backdrop core updates available. Your Backdrop is up to date!\n");
      }
    } // end of drush up backdrop.
    elseif (!empty($project)) {
      if (!empty(drush_get_context('DRUSH_BACKDROP_SITE_ROOT'))) {
        $site_root = realpath($backdrop_root . '/' . drush_get_context('DRUSH_BACKDROP_SITE_ROOT'));
      }
      else {
        $site_root = $backdrop_root;
      }

      if (file_exists("$site_root/modules/contrib")) {
        $module_path = "$site_root/modules/contrib";
      }
      else {
        $module_path = "$site_root/modules";
      }
      $ls = scandir($module_path);
      if (in_array($project, $ls)) {
        $info = file("$module_path/$project/$project.info", FILE_IGNORE_NEW_LINES);
        foreach($info as $i) {
          if (strpos($i, 'version =') !== FALSE) {
            $module_version_raw = $i;
          }
        }
        if (!$module_version_raw) {
          drush_print_r("\tCurrent module version of \033[1m$project\033[0m could not be determined.");
          drush_print_r("\tThis is usually because the module doesn't have an official release.");
        }
        $module_version_arr = explode(' = ', $module_version_raw);
        $module_full_version = trim($module_version_arr[1]);
        if (strpos($module_full_version, '-') !== FALSE) {
          $module_sem_version_arr = explode('-', $module_version_arr[1]);
          $module_sem_version = trim($module_sem_version_arr[1]);
          $module_core_compat = trim($module_sem_version_arr[0]);
        }
        else {
           $module_sem_version = $module_full_version;
        }
        $module_sem_version_parts = explode('.', $module_sem_version);
        $module_version_major = $module_sem_version_parts[0];
        $module_version_minor = $module_sem_version_parts[1];
        $module_version_patch = $module_sem_version_parts[2];
        // Get proposed Backdrop version info.
        $xml = backdrop_pm_get_tags("https://updates.backdropcms.org/release-history/$project/1.x");
        $download_link = $xml->releases->release->download_link;
        $proposed_version = $xml->releases->release->version;
        $proposed_version_major = $xml->releases->release->version_major;
        $proposed_version_minor = $xml->releases->release->version_minor;
        $proposed_version_patch = $xml->releases->release->version_patch;

        // Check if proposed version has prepended Backrop compatibility string.
        if (strpos($proposed_version, "-") !== FALSE) {
          $proposed_version_array = explode('-', $proposed_version);
          $proposed_sem_version = $proposed_version_array[1];
        }

        if (empty($proposed_version)) {
          drush_print_r("\tThere are no official releases for \033[1m$project\033[0m.
          Upgrading is not possible.
          ");
        }
        elseif ($module_sem_version == $proposed_sem_version) {
          drush_print_r("\n\t\tProject \033[1m$project\033[0m is up to date :).\n");
        }
        elseif (
          (
            $module_version_major == $proposed_version_major &&
            $module_version_minor == $proposed_version_minor &&
            $module_version_patch < $proposed_version_patch
          ) ||
          (
            $module_version_major == $proposed_version_major &&
            $module_version_minor < $proposed_version_minor
          )
        ) {
          drush_print_r("\n\t\tCurrent \033[1m$project\033[0m Version:\t$module_full_version");
          drush_print_r("\t\tProposed \033[1m$project\033[0m Version:\t$proposed_version\n");
          drush_print_r("\t\e[33mWARNING\033[0m:\n
          Updating \033[1m$project\033[0m will discard anymodifications made to
          \033[1m$project\033[0m files.  If you have made any modifications to
          these files, please back them up before updating so that you can
          re-create your modifications in the updated version of the file.\n\n");
          drush_print_r("\tNote: Updating modules can potentially break your site.");
          drush_print_r("\tIt is NOT recommended to update production sites without prior testing.\n\n");
          $answer = drush_prompt("\t\e[34mDo you wish to continue?\e[0m (y or n)");
          $answer = strtolower($answer);
          if ($answer == 'y' || $answer == 'yes') {
            $users_home_directory = $_SERVER['HOME'];
            $site_name = strtolower(transliteration_clean_filename(config_get('system.core', 'site_name')));
            $timestamp = date_timestamp_get(date_create());
            exec("mkdir -p $users_home_directory/drush-backups/$site_name/$timestamp/modules/");
            drush_print_r("\t Backing up current \033[1m$project\033[0m folder to
            $users_home_directory/drush-backups/$site_name/$timestamp/modules/$project-$module_full_version");
            exec("mv $module_path/$project $users_home_directory/drush-backups/$site_name/$timestamp/modules/$project-$module_full_version");
            exec("mkdir -p $users_home_directory/.drush/drush-tmp");
            exec("wget --quiet -O $users_home_directory/.drush/drush-tmp/$project.zip $download_link");
            exec("unzip $users_home_directory/.drush/drush-tmp/$project.zip -d $users_home_directory/.drush/drush-tmp");
            foreach (glob("$users_home_directory/.drush/drush-tmp/backdrop-contrib-$project-*") as $glob) {
              rename($glob, "$users_home_directory/.drush/drush-tmp/$project");
            }
            exec("mv $users_home_directory/.drush/drush-tmp/$project $module_path/$project");
            exec("rm -r $users_home_directory/.drush/drush-tmp");

            // Run db updates and clear cache.
            backdrop_command_updatedb();
            backdrop_flush_all_caches();

            drush_print_r("\n\tYou can retrieve your old \033[1m$project\033[0m files, should you need
            to, from: $users_home_directory/drush-backups/$site_name/$timestamp/modules/$project-$module_full_version");
            if (file_exists("$module_path/$project")) {
              drush_print_r("\t\e[32mSuccess\033[0m: Project \033[1m$project\033[0m updated to $proposed_version.");
            }
            else {
              drush_print_r("\t\033[31mFailed\033[0m: Project $project upgrade failed.");
            }
          }
          else {
            drush_print_r("\tBackdrop core update cancelled.");
          }
        }
      }
      else {
        drush_print_r("\n\t\e[33mWARNING\033[0m: There is no project \e[1m$project\033[0m installed on your backdrop site.");
        drush_print_r("\tNo action taken.\n");
      }
    } // end of drush up $project
    else {
      drush_print_r("\n\tYou must specify a project to update. For example:\n");
      drush_print_r("\t\tdrush up project_name");
      drush_print_r("\n\tWhere project_name is the machine name of the project like webform or devel etc.\n");
    }
  }
}
