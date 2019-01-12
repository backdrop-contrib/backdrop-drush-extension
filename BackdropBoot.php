<?php

namespace Drush\Boot;

class BackdropBoot extends BaseBoot {

  /**
   * Set up and test for a valid Backdrop root, either through the -r/--root
   * options, or evaluated based on the current working directory.
   *
   * Any code that interacts with an entire Backdrop installation, and not a
   * specific site on the Backdrop installation should use this bootstrap phase.
   */
  const BOOTSTRAP_ROOT = 1;

  /**
   * Set up a Backdrop site directory and the correct environment variables to
   * allow Backdrop to find the configuration file.
   *
   * If no site is specified with the -l / --uri options, Drush will assume the
   * site uses the root settings.php file.
   *
   * If you want to avoid this behaviour, it is recommended that you use the
   * BOOTSTRAP_ROOT bootstrap phase instead.
   *
   * Any code that needs to modify or interact with a specific Backdrop site's
   * settings.php file should bootstrap to this phase.
   */
  const BOOTSTRAP_SITE = 2;

  /**
   * Load the settings from the Backdrop settings.php file.
   *
   * This phase is analogous to the BACKDROP_BOOTSTRAP_CONFIGURATIONURATION bootstrap
   * phase in Backdrop itself, and this is also the first step where Backdrop-
   * specific code is included.
   *
   * This phase is commonly used for code that interacts with the Backdrop
   * install API, as both install.php and update.php start at this phase.
   */
  const BOOTSTRAP_CONFIGURATION = 3;

  /**
   * Connect to the Backdrop database using the database credentials loaded
   * during the previous bootstrap phase.
   *
   * This phase is analogous to the BACKDROP_BOOTSTRAP_DATABASE bootstrap phase
   * in Backdrop.
   *
   * Any code that needs to interact with the Backdrop database API needs to
   * be bootstrapped to at least this phase.
   */
  const BOOTSTRAP_DATABASE = 4;

  /**
   * Fully initialize Backdrop.
   *
   * This is analogous to the BACKDROP_BOOTSTRAP_FULL bootstrap phase in
   * Backdrop.
   *
   * Any code that interacts with the general Backdrop API should be
   * bootstrapped to this phase.
   */
  const BOOTSTRAP_FULL = 5;

  /**
   * Log in to the initialized Backdrop site.
   *
   * This is the default bootstrap phase all commands will try to reach,
   * unless otherwise specified.
   *
   * This bootstrap phase is used after the site has been fully bootstrapped.
   *
   * This phase will log you in to the Backdrop site with the username
   * or user ID specified by the --user/ -u option.
   *
   * Use this bootstrap phase for your command if you need to have access
   * to information for a specific user, such as listing nodes that might
   * be different based on who is logged in.
   */
  const BOOTSTRAP_LOGIN = 6;

  function valid_root($path) {
    if (!empty($path) && is_dir($path) && file_exists($path . '/index.php')) {
      if (file_exists($path . '/core/includes/bootstrap.inc') && file_exists($path . '/core/misc/backdrop.js')) {
        return TRUE;
      }
    }
    return FALSE;
  }

  function get_version($root) {
    $path = $root . '/core/includes/bootstrap.inc';
    if (is_file($path)) {
      require_once $path;
      if (defined('BACKDROP_VERSION')) {
        return BACKDROP_VERSION;
      }
    }
  }

  function get_profile() {
    return backdrop_get_profile();
  }

  function conf_path($require_settings = TRUE, $reset = FALSE) {
    return conf_path($require_settings = TRUE, $reset = FALSE);
  }


  /**
   * Bootstrap phases used with Backdrop:
   *
   *     DRUSH_BOOTSTRAP_DRUSH   = Only Backdrop.
   *     BOOTSTRAP_ROOT          = Find a valid Drupal root.
   *     BOOTSTRAP_SITE          = Find a valid Drupal site.
   *     BOOTSTRAP_CONFIGURATION = Load the site's settings.
   *     BOOTSTRAP_DATABASE      = Initialize the database.
   *     BOOTSTRAP_FULL          = Initialize Drupal fully.
   *     BOOTSTRAP_LOGIN         = Log into Drupal with a valid user.
   *
   * The value is the name of the method of the Boot class to
   * execute when bootstrapping.  Prior to bootstrapping, a "validate"
   * method is called, if defined.  The validate method name is the
   * bootstrap method name with "_validate" appended.
   */
  function bootstrap_phases() {
    return array(
      BackdropBoot::BOOTSTRAP_ROOT          => 'bootstrap_backdrop_root',
      BackdropBoot::BOOTSTRAP_SITE          => 'bootstrap_backdrop_site',
      BackdropBoot::BOOTSTRAP_CONFIGURATION => 'bootstrap_backdrop_configuration',
      BackdropBoot::BOOTSTRAP_DATABASE      => 'bootstrap_backdrop_database',
      BackdropBoot::BOOTSTRAP_FULL          => 'bootstrap_backdrop_full',
      BackdropBoot::BOOTSTRAP_LOGIN         => 'bootstrap_backdrop_login');
  }

  /**
   * List of bootstrap phases where Drush should stop and look for commandfiles.
   *
   * For Backdrop, we try at these bootstrap phases:
   *
   *   - Drush preflight: to find commandfiles in any system location,
   *     out of a Backdrop installation.
   *   - Backdrop root: to find commandfiles based on Backdrop core version.
   *   - Backdrop full: to find commandfiles defined within a Backdrop directory.
   *
   * Once a command is found, Drush will ensure a bootstrap to the phase
   * declared by the command.
   *
   * @return array of PHASE indexes.
   */
  function bootstrap_init_phases() {
    return array(BackdropBoot::BOOTSTRAP_ROOT, BackdropBoot::BOOTSTRAP_FULL);
  }

  /**
   * Validate the BackdropBoot::BOOTSTRAP_ROOT phase.
   */
  function bootstrap_backdrop_root_validate() {
    $backdrop_root = drush_get_option('root');
    if (!isset($backdrop_root)) {
      $backdrop_root = drush_locate_root();
    }
    drush_set_context('DRUSH_SELECTED_BACKDROP_ROOT', $backdrop_root);

    if (empty($backdrop_root)) {
      return drush_bootstrap_error('DRUSH_NO_BACKDROP_ROOT', dt("A Backdrop installation directory could not be found"));
    }
    if (!$signature = drush_valid_root($backdrop_root)) {
      return drush_bootstrap_error('DRUSH_INVALID_BACKDROP_ROOT', dt("The directory !backdrop_root does not contain a valid Backdrop installation", array('!backdrop_root' => $backdrop_root)));
    }
    drush_bootstrap_value('backdrop_root', realpath($backdrop_root));
    define('DRUSH_BACKDROP_SIGNATURE', $signature);

    return TRUE;
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_ROOT phase.
   *
   * Bootstrap Drush with a valid Backdrop directory.
   *
   * In this function, the pwd() will be moved to the root of the Backdrop
   * installation.
   *
   * The DRUSH_BACKDROP_ROOT context, DRUSH_BACKDROP_CORE context,
   * BACKDROP_ROOT constant, and the DRUSH_BACKDROP_CORE constant are populated
   * from the value that we determined during the validation phase.
   *
   * We also now load the drushrc.php for this specific Backdrop site.
   * We can now include files from the Backdrop directory, and figure
   * out more context about the platform, such as the version of Backdrop.
   */
  function bootstrap_backdrop_root() {
    // Load the config options the installation's /drush directory.
    drush_load_config('backdrop');

    $backdrop_root = drush_set_context('DRUSH_BACKDROP_ROOT', drush_bootstrap_value('backdrop_root'));
    chdir($backdrop_root);
    $version = $this->get_version($backdrop_root);

    $core = $this->bootstrap_backdrop_core($backdrop_root);

    // DRUSH_BACKDROP_CORE should point to the /core folder.
    drush_set_context('DRUSH_BACKDROP_CORE', $core);
    define('DRUSH_BACKDROP_CORE', $core);

    _drush_preflight_global_options();

    drush_log(dt("Initialized Backdrop !version root directory at !backdrop_root", array("!version" => $version, '!backdrop_root' => $backdrop_root)));
  }


  /**
   * @param $backdrop_root
   * @return string
   */
  function bootstrap_backdrop_core($backdrop_root) {
    define('BACKDROP_ROOT', $backdrop_root);
    $core = $backdrop_root . '/core';

    require_once $core . '/includes/bootstrap.inc';

    return $core;
  }

  /**
   * Validate the BackdropBoot::BOOTSTRAP_SITE phase.
   *
   * In this function we determine the URL used for the command, and check for a
   * valid settings.php file.
   *
   * To do this, we need to set up the $_SERVER environment variable,
   * to allow us to use conf_path() to determine what Backdrop will load
   * as a configuration file.
   */
  function bootstrap_backdrop_site_validate() {
    // Define the selected conf path as soon as we have identified that
    // we have selected a Backdrop site.
    $drush_uri = $this->get_selected_uri();

    $this->setup_server_globals($drush_uri);
    $conf_path = $this->conf_path($drush_uri);

    drush_set_context('DRUSH_SELECTED_BACKDROP_SITE_CONF_PATH', $conf_path);

    $conf_file = "$conf_path/settings.php";
    if (!file_exists($conf_file)) {
      return drush_bootstrap_error('DRUPAL_SITE_SETTINGS_NOT_FOUND', dt("Could not find a Backdrop settings.php file at !file.", array('!file' => $conf_file)));
    }

    drush_bootstrap_value('site', $_SERVER['HTTP_HOST']);
    drush_bootstrap_value('conf_path', $conf_path);

    return TRUE;
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_SITE phase.
   *
   * Initialize a site on the Backdrop root.
   *
   * We now set various contexts that we determined and confirmed to be valid.
   * Additionally we load an optional drushrc.php file in the site directory.
   */
  function bootstrap_backdrop_site() {
    drush_load_config('site');

    $drush_uri = drush_get_context('DRUSH_SELECTED_URI');
    drush_set_context('DRUSH_URI', $drush_uri);
    $site = drush_set_context('DRUSH_BACKDROP_SITE', drush_bootstrap_value('site'));
    $conf_path = drush_set_context('DRUSH_BACKDROP_SITE_ROOT', drush_bootstrap_value('conf_path'));

    drush_log(dt("Initialized Backdrop site !site at !site_root", array('!site' => $site, '!site_root' => $conf_path)));

    _drush_preflight_global_options();

  }

  /**
   * Validate the BackdropBoot::BOOTSTRAP_CONFIGURATION phase.
   */
  function bootstrap_backdrop_configuration_validate() {
    // No validation yet.
    return TRUE;
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_CONFIGURATION phase.
   *
   * Load in the settings.php file an initialize the database information,
   * config directories, and $settings variable.
   */
  function bootstrap_backdrop_configuration() {
    backdrop_bootstrap(BACKDROP_BOOTSTRAP_CONFIGURATION);

    // Set the Drush option for "databases" to work with existing SQL commands
    // as though a --db-url or --databases option were passed in.
    // In the future it should be possible for bootstrap classes like this one
    // to provide the database string directly.
    // See https://github.com/drush-ops/drush/issues/1750.
    $database = drush_get_option('database', 'default');
    $target = drush_get_option('target', 'default');

    $db_url = $url = drush_get_option('db-url');
    $databases = drush_get_option('databases');
    if (!$databases) {
      if ($db_url) {
        $db_spec = drush_convert_db_from_db_url($url);
        $db_spec['db_prefix'] = drush_get_option('db-prefix');
        drush_set_option('databases', array($database => array($target => $db_spec)));
      }
      else {
        drush_set_option('databases', $GLOBALS['databases']);
      }
    }

    // Unset Backdrop error handler and restore drush's one.
    restore_error_handler();
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_DATABASE phase.
   */
  function bootstrap_backdrop_database_validate() {
    require_once BACKDROP_ROOT . '/core/includes/database/database.inc';
    return db_table_exists('system');
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_DATABASE phase.
   */
  function bootstrap_backdrop_database() {
    backdrop_bootstrap(BACKDROP_BOOTSTRAP_DATABASE);
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_FULL phase.
   */
  function bootstrap_backdrop_full() {
    if (!drush_get_context('DRUSH_QUIET', FALSE)) {
      ob_start();
    }
    backdrop_bootstrap(BACKDROP_BOOTSTRAP_FULL);
    if (!drush_get_context('DRUSH_QUIET', FALSE)) {
      ob_end_clean();
    }
  }

  /**
   * Execute the BackdropBoot::BOOTSTRAP_LOGIN phase.
   *
   * Log into the bootstrapped Backdrop site with a specific user name or ID.
   */
  function bootstrap_backdrop_login() {
    $uid_or_name = drush_set_context('DRUSH_USER', drush_get_option('user', 0));

    if (is_numeric($uid_or_name)) {
      $account = user_load($uid_or_name);
    }
    if (!$account) {
      $account = user_load_by_name($uid_or_name);
    }

    if ($account) {
      $GLOBALS['user'] = $account;
      // @todo: Convert Backdrop messages to drush output.
      //_drush_log_drupal_messages();
    }
    else {
      if (is_numeric($uid_or_name)) {
        $message = dt('Could not login with user ID !user.', array('!user' => $uid_or_name));
        if ($uid_or_name === 0) {
          $message .= ' ' . dt('This is typically caused by importing a MySQL database dump from a faulty tool which re-numbered the anonymous user ID in the users table. See !link for help recovering from this situation.', array('!link' => 'http://drupal.org/node/1029506'));
        }
      }
      else {
        $message = dt('Could not login with user account `!user\'.', array('!user' => $uid_or_name));
      }
      return drush_set_error('DRUPAL_USER_LOGIN_FAILED', $message);
    }
  }

  /**
   * Set up the $_SERVER globals so that Backdrop will see the same values
   * that it does when serving pages via the web server.
   *
   * @see \Drush\Boot\DrupalBoot::bootstrap_drupal_site_setup_server_global()
   */
  protected function setup_server_globals($drush_uri) {
    // Fake the necessary HTTP headers that Drupal needs:
    if ($drush_uri) {
      $backdrop_base_url = parse_url($drush_uri);
      // If there's no url scheme set, add http:// and re-parse the url
      // so the host and path values are set accurately.
      if (!array_key_exists('scheme', $backdrop_base_url)) {
        $drush_uri = 'http://' . $drush_uri;
        $backdrop_base_url = parse_url($drush_uri);
      }
      // Fill in defaults.
      $backdrop_base_url += array(
          'path' => '',
          'host' => NULL,
          'port' => NULL,
      );
      $_SERVER['HTTP_HOST'] = $backdrop_base_url['host'];

      if ($backdrop_base_url['scheme'] == 'https') {
        $_SERVER['HTTPS'] = 'on';
      }

      if ($backdrop_base_url['port']) {
        $_SERVER['HTTP_HOST'] .= ':' . $backdrop_base_url['port'];
      }
      $_SERVER['SERVER_PORT'] = $backdrop_base_url['port'];

      $_SERVER['REQUEST_URI'] = $backdrop_base_url['path'] . '/';
    }
    else {
      $_SERVER['HTTP_HOST'] = 'default';
      $_SERVER['REQUEST_URI'] = '/';
    }

    $_SERVER['PHP_SELF'] = $_SERVER['REQUEST_URI'] . 'index.php';
    $_SERVER['SCRIPT_NAME'] = $_SERVER['PHP_SELF'];
    $_SERVER['REMOTE_ADDR'] = '127.0.0.1';
    $_SERVER['REQUEST_METHOD']  = NULL;

    $_SERVER['SERVER_SOFTWARE'] = NULL;
    $_SERVER['HTTP_USER_AGENT'] = NULL;
    $_SERVER['SCRIPT_FILENAME'] = BACKDROP_ROOT . '/index.php';
    
    // Allows the user to drop in db connection info by setting BACKDROP_SETTINGS in the environment
    // This is helpful when backdrops database connection is not specified in settings.php such as on Pantheon or Kalabox
    if (getenv('BACKDROP_SETTINGS') !== false) {
      $_SERVER['BACKDROP_SETTINGS'] = getenv('BACKDROP_SETTINGS');
    }
  }

  /**
   * Called by Drush when a command is selected, but before it runs.  This gives
   * the Boot class an opportunity to determine if any minimum requirements
   * (e.g. minimum Backdrop version) declared in the command have been met.
   *
   * @return bool TRUE if command is valid. $command['bootstrap_errors']
   *   should be populated with an array of error messages if the command is not
   *   valid.
   */
  function enforce_requirement(&$command) {
    drush_enforce_requirement_bootstrap_phase($command);
    drush_enforce_requirement_core($command);
    drush_enforce_requirement_drush_dependencies($command);

    // @todo: Support Backdrop and(?) Drupal dependencies.
    // $this->drush_enforce_requirement_backdrop_dependencies($command);
    // $this->drush_enforce_requirement_drupal_dependencies($command);

    return empty($command['bootstrap_errors']);
  }

  function report_command_error($command) {
    // If we reach this point, command doesn't fit requirements or we have not
    // found either a valid or matching command.

    // If no command was found check if it belongs to a disabled module.
    // @todo: Implement checking if desired for Backdrop.
    //if (!$command) {
    //  $command = $this->drush_command_belongs_to_disabled_module();
    //}
    parent::report_command_error($command);
  }

  function command_defaults() {
    return array(
        'backdrop dependencies' => array(),
        'bootstrap' => BackdropBoot::BOOTSTRAP_FULL,
    );
  }

  function contrib_modules_paths() {
    $paths = array();
    if (conf_path() !== '.') {
      $paths[] = conf_path() . '/modules';
    }
    $paths[] = 'modules';
    return $paths;
  }

  function contrib_themes_paths() {
    $paths = array();
    if (conf_path() !== '.') {
      $paths[] = conf_path() . '/themes';
    }
    $paths[] = 'themes';
    return $paths;
  }

  function commandfile_searchpaths($phase, $phase_max = FALSE) {
    if (!$phase_max) {
      $phase_max = $phase;
    }

    $searchpath = array();
    switch ($phase) {
      case BackdropBoot::BOOTSTRAP_ROOT:
        $backdrop_root = drush_get_context('DRUSH_SELECTED_BACKDROP_ROOT');
        $searchpath[] = $backdrop_root . '/../drush';
        $searchpath[] = $backdrop_root . '/drush';
        $searchpath[] = $backdrop_root . '/sites/all/drush';

        // Add the drupalboot.drush.inc commandfile.
        // $searchpath[] = __DIR__;
        break;
      case BackdropBoot::BOOTSTRAP_SITE:
        // If we are going to stop bootstrapping at the site, then
        // we will quickly add all commandfiles that we can find for
        // any extension associated with the site, whether it is enabled
        // or not.  If we are, however, going to continue on to bootstrap
        // all the way to DRUSH_BOOTSTRAP_DRUPAL_FULL, then we will
        // instead wait for that phase, which will more carefully add
        // only those Drush commandfiles that are associated with
        // enabled modules.
        if ($phase_max < BackdropBoot::BOOTSTRAP_FULL) {
          $searchpath = array_merge($searchpath, $this->contrib_modules_paths());

          // Adding commandfiles located within /profiles. Try to limit to one
          // profile for speed. Note that Backdrop allows enabling modules from
          // a non-active profile so this logic is kinda dodgy.
          $cid = $this->install_profile_cid();
          if ($cached = drush_cache_get($cid)) {
            $profile = $cached->data;
            $searchpath[] = "profiles/$profile/modules";
            $searchpath[] = "profiles/$profile/themes";
          }
          else {
            // If install_profile is not available, scan all profiles.
            $searchpath[] = "profiles";
            $searchpath[] = "sites/all/profiles";
          }

          $searchpath = array_merge($searchpath, $this->contrib_themes_paths());
        }
        break;
      case BackdropBoot::BOOTSTRAP_CONFIGURATION:
        // Nothing to do here anymore. Left for documentation.
        break;
      case BackdropBoot::BOOTSTRAP_FULL:
        // Add enabled module paths, excluding the install profile. Since we are
        // bootstrapped we can use the Backdrop API.
        $ignored_modules = drush_get_option_list('ignored-modules', array());
        $cid = $this->install_profile_cid();
        if ($cached = drush_cache_get($cid)) {
          $ignored_modules[] = $cached->data;
        }
        foreach (array_diff($this->module_list(), $ignored_modules) as $module) {
          $filepath = backdrop_get_path('module', $module);
          if ($filepath && $filepath != '/') {
            $searchpath[] = $filepath;
          }
        }

        $searchpath[] = backdrop_get_path('theme', config_get('system.core', 'theme_default'));
        $searchpath[] = backdrop_get_path('theme', config_get('system.core', 'theme_admin'));
        break;
    }

    return $searchpath;
  }

  /**
   * Returns a list of enabled modules.
   *
   * This is a simplified version of core's module_list().
   */
  protected function module_list() {
    $enabled = array();
    $rsc = drush_db_select('system', 'name', 'type=:type AND status=:status', array(':type' => 'module', ':status' => 1));
    while ($row = drush_db_result($rsc)) {
      $enabled[$row] = $row;
    }

    return $enabled;
  }

  /**
   * Build a cache id to store the install_profile for a given site.
   *
   * @see drush_cid_install_profile().
   */
  protected function install_profile_cid() {
    drush_get_cid('install_profile', array(), array(drush_get_context('DRUSH_SELECTED_BACKDROP_SITE_CONF_PATH')));
  }

  /**
   * Find the URI that has been selected by the cwd if it was not previously set
   * via the --uri / -l option.
   *
   * @return string The site URI.
   *
   * @see _drush_bootstrap_selected_uri()
   */
  protected function get_selected_uri() {
   $uri = drush_get_context('DRUSH_SELECTED_URI');
    if (empty($uri)) {
      $site_path = $this->site_path();
      $elements = explode('/', $site_path);
      $current = array_pop($elements);
      if (!$current) {
        $current = 'default';
      }
      $uri = 'http://'. $current;
      $uri = drush_set_context('DRUSH_SELECTED_URI', $uri);
      $this->create_self_alias();
    }

    return $uri;
  }

  /**
   * Like Backdrop conf_path(), but searching from beneath.
   * Allows proper site uri detection in site sub-directories.
   *
   * Essentially looks for a settings.php file. Drush uses this
   * function to find a usable site based on the user's current
   * working directory.
   *
   * @param string
   *   Search starting path. Defaults to current working directory.
   *
   * @return
   *   Current site path (folder containing settings.php) or FALSE if not found.
   */
  protected function site_path($path = NULL) {
    $site_path = FALSE;

    $path = empty($path) ? drush_cwd() : $path;
    // Check the current path.
    if (file_exists($path . '/settings.php')) {
      $site_path = $path;
    }
    else {
      // Move up dir by dir and check each. Stop if we get to a Backdrop root.
      // We don't care if it is DRUSH_SELECTED_BACKDROP_ROOT or some other root.
      while (($path = _drush_shift_path_up($path)) && !drush_valid_root($path)) {
        if (file_exists($path . '/settings.php')) {
          $site_path = $path;
          break;
        }
      }
    }

    $site_root = drush_get_context('DRUSH_SELECTED_BACKDROP_ROOT');
    if (file_exists($site_root . '/sites/sites.php')) {
      $sites = array();
      // This will overwrite $sites with the desired mappings.
      include($site_root . '/sites/sites.php');
      // We do a reverse lookup here to determine the URL given the site key.
      if ($match = array_search($site_path, $sites)) {
        $site_path = $match;
      }
    }

    // Last resort: try from site root
    if (!$site_path) {
      if ($site_root) {
        if (file_exists($site_root . '/settings.php')) {
          $site_path = $site_root;
        }
      }
    }

    return $site_path;
  }

  /**
   * Check to see if a '@self' record was created during bootstrap.
   * If not, make one now.
   *
   * @see drush_sitealias_create_self_alias()
   */
  protected function create_self_alias() {
    $self_record = drush_sitealias_get_record('@self');
    if (!array_key_exists('root', $self_record) && !array_key_exists('remote-host', $self_record)) {
      $backdrop_root = drush_get_context('DRUSH_SELECTED_BACKDROP_ROOT');
      $uri = drush_get_context('DRUSH_SELECTED_URI');
      if (!empty($backdrop_root) && !empty($uri)) {
        // Create an alias '@self'
        _drush_sitealias_cache_alias('@self', array('root' => $backdrop_root, 'uri' => $uri));
      }
    }
  }
}
