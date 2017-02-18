<?php
/**
 * Implements drush_hook_command().
 */
function backdrop_site_install_drush_command() {
  $items['backdrop-site-install'] = array(
    'description' => dt('Install Backdrop along with modules/themes/configuration using the specified install profile.'),
    'arguments' => array(
      'arguments' => dt('The install profile you wish to run. Default is "standard"'),
      'key=value' => array(
        'description' => dt('Any additional settings you wish to pass to the profile. The key is in the form [form name].[parameter name]'),
        'multiple' => TRUE,
      )
    ),
    'options' => array(
      'root' => 'Set the working directory for the script to the specified path. Required if running this script from a directory other than the Backdrop root.',
      'db-url' => array(
        'description' => 'A Drupal 6 style database URL. Only required for initial install - not re-install.',
        'example-value' => 'mysql://root:pass@localhost/db-name',
        'required' => TRUE,
      ),
      'db-prefix' => 'An optional table prefix to use for initial install.  Can be a key-value array of tables/prefixes in a drushrc file (not the command line).',
      'account-name' => 'uid1 name. Defaults to admin',
      'account-pass' => 'uid1 pass. Defaults to a randomly generated password. If desired, set a fixed password in drushrc.php.',
      'account-mail' => 'uid1 email. Defaults to admin@example.com',
      'locale' => array(
        'description' => 'A short language code. Sets the default site language. Language files must already be present. You may use download command to get them.',
        'example-value' => 'en-GB',
      ),
      'clean-url'=> 'Defaults to 1',
      'site-name' => 'Defaults to Site-Install',
      'site-mail' => 'From: for system mailings. Defaults to admin@example.com',
    ),
    'callback' => 'backdrop_site_install_drush_callback',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'aliases' => array('si'),
  );
  return $items;
}

/**
 * @param $arguments
 * @param $options
 * @return bool
 */
function backdrop_site_install_drush_callback($arguments = 'standard', $options = array()) {

  $options = array(
    'account-mail' => drush_get_option('account-mail', 'admin@example.com'),
    'account-name' => drush_get_option('account-name', 'admin'),
    'account-pass' => drush_get_option('account-pass', md5(microtime() . mt_rand())),
    'site-name' => drush_get_option('site-name', 'Backdrop'),
    'clean-url' => drush_get_option('clean-url', '1'),
    'db-prefix' => drush_get_option('db-prefix', ''),
    'db-url' => drush_get_option('db-url', ''),
    'locale' => drush_get_option('Locale', 'en'),
    'site-mail' => drush_get_option('site-mail', 'admin@example.com'),
    'root' => drush_get_option('root', ''),
    'user' => drush_get_option('user', ''),
    'database' => drush_get_option('database', ''),
  );

  $additional_form_options = array();
  if (isset($arguments['key=value']) && !empty($arguments['key=value'])) {
    foreach($arguments['key=value'] as $argument) {
      list($key, $value) = explode('=', $argument);
      $additional_form_options[$key] = $value;
    }
  }

  $profile = 'standard';
  if (isset($arguments['profile'])) {
    $profile = $arguments['profile'];
  }

  $url = parse_url($options['db-url']);
  $url += array(
    'driver' => NULL,
    'user' => NULL,
    'pass' => NULL,
    'host' => NULL,
    'port' => NULL,
    //'prefix' => NULL,
    'database' => NULL,
  );
  // If db-url is not specified use interactive prompts.
  if ($options['db-url'] === '') {
    $options['account-mail'] = drush_prompt("\n\t\033[94maccount-mail\033[0m", 'admin@example.com');
    $options['account-name'] = drush_prompt("\t\033[94maccount-name\033[0m", 'admin');
    $options['account-pass'] = drush_prompt("\t\033[94maccount-pass\033[0m", md5(microtime() . mt_rand()));
    $options['site-name'] = drush_prompt("\t\033[94msite-name\033[0m", 'Backdrop');
    $options['clean-url'] = drush_prompt("\t\033[94mclean-url\033[0m", '1');
    //$options['db-prefix'] = drush_prompt("\t\033[94mdb-prefix\033[0m", '');
    $options['locale'] = drush_prompt("\t\033[94mLocale\033[0m", 'en');
    $options['site-mail'] = drush_prompt("\t\033[94msite-mail\033[0m", 'admin@example.com');
    $options['root'] = drush_prompt("\t\033[94mroot\033[0m", '.');
    $url['driver'] = drush_prompt("\t\033[94mDatabase driver\033[0m", 'mysql');
    $url['user'] = drush_prompt("\t\033[94mDatabase user\033[0m");
    $url['pass'] = drush_prompt("\t\033[94mDatabase password\033[0m");
    $url['host'] = drush_prompt("\t\033[94mDatabase host\033[0m", 'localhost');
    $url['port'] = drush_prompt("\t\033[94mDatabase port\033[0m", '3306');
    //$options['prefix'] = drush_prompt("\tdriver: ");
    $options['database'] = drush_prompt("\t\033[94mDatabase name\033[0m");
    drush_print_r("\033[0m\n");
  }
  else {
    $url = (object)array_map('urldecode', $url);
    $db_spec = array(
      'driver' => 'mysql',
      'username' => $url->user,
      'password' => $url->pass,
      'port' => $url->port,
      'host' => $url->host,
      //'prefix' => $url->prefix,
      // Remove leading / character from database names.
      'database' => substr($url->path, 1),
    );
  }

  $settings = array(
    'parameters' => array(
      'profile' => $profile,
      'locale' => $options['locale'],
    ),
    'forms' => array(
      'install_settings_form' => array(
        'driver' => $db_spec['driver'],
        $db_spec['driver'] => $db_spec,
        'op' => 'Save and continue',
      ),
      'install_configure_form' => array(
        'site_name' => $options['site-name'],
        'site_mail' => $options['site-mail'],
        'account' => array(
          'name' => $options['account-name'],
          'mail' => $options['account-mail'],
          'pass' => array(
            'pass1' => $options['account-pass'],
            'pass2' => $options['account-pass'],
          ),
        ),
        'update_status_module' => array(
          1 => TRUE,
          2 => TRUE,
        ),
        'clean_url' => $options['clean-url'],
        'op' => 'Save and continue',
      ),
    ),
  );

  // Merge in the additional options.
  foreach($additional_form_options as $key => $value) {
    $current = &$settings['forms'];
    foreach(explode('.', $key) as $param) {
      $current = &$current[$param];
    }
    $current = $value;
  }

  define('MAINTENANCE_MODE', 'install');
  require_once './core/includes/install.core.inc';

  try {
    //render_text(array('value' => "Installing Backdrop. This may take a moment...", '#color' => 'bold_blue'));
    drush_print_r("\t" . dt("Installing Backdrop. This may take a moment...") . "\n\n");
    install_backdrop($settings);
    drush_print_r(dt("\t\033[92mBackdrop installed successfully.\033[0m\n"));
    drush_print_r(dt("\tUsername: " . $options['account-name']));
    drush_print_r(dt("\tpassword: " . $options['account-pass']));
    return TRUE;
  }
  catch (Exception $e) {
    echo $e->getMessage();
    drush_print_r(dt("An error occurred. Output of installation attempt is as follows: !error", array( '!error' => $e->getMessage())), 'error');
    return FALSE;
  }
}
