<?php

/**
 * @file
 *   Core drush commands.
 */

include_once __DIR__ . '/BackdropBoot.php';

/**
 * Implements hook_bootstrap_candidates().
 *
 * This returns an array of classes that may be used to bootstrap Drush. Right
 * now there is only a single Backdrop version, but in the future if newer
 * versions required separate bootstrap classes per version, we would add them
 * here.
 */
function backdrop_bootstrap_candidates() {
  return array('Drush\Boot\BackdropBoot');
}

/**
 * Adjust the contents of any command structure prior to dispatch.
 *
 * @see core_drush_command_alter()
 */
function backdrop_drush_command_alter(&$command) {
  $bootstrap = drush_get_bootstrap_object();
  if (!is_a($bootstrap, 'Drush\Boot\BackdropBoot')) {
    return;
  }

  $backdrop_command = NULL;

  // Commands that need replacements with Backdrop versions.
  // @todo: Automate this based on the contents of the commands directory.
  switch ($command['command']) {
    case 'core-cron':
      $backdrop_command = 'backdrop-cron';
      break;
    case 'updatedb':
      $backdrop_command = 'backdrop-updatedb';
      break;
    case 'updatedb-batch-process':
      $backdrop_command = 'backdrop-updatedb-batch-process';
      break;
    case 'updatedb-status':
      $backdrop_command = 'backdrop-updatedb-status';
      break;
    case 'pm-download':
      $backdrop_command = 'backdrop-pm-download';
      break;
    case 'user-login':
      $backdrop_command = 'backdrop-user-login';
      break;
    case 'core-status':
      $backdrop_command = 'backdrop-core-status';
      break;
    case 'pm-update':
      $backdrop_command = 'backdrop-pm-update';
      break;
  }

  // Commands that work with Backdrop with no adjustments.
  $safe_commands = array(
    'cache-get',
    'cache-set',
    'core-exec',
    'help',
    'topic',
    'php-eval',
    'php-script',
    'sql-cli',
    'sql-conf',
    'sql-connect',
    'sql-create',
    'sql-drop',
    'sql-dump',
    'sql-query',
    'sql-sanitize',
  );

  $compatible_commands = array(
    'cache-clear',
  );

  // Commands native to Backdrop.
  if (strpos($command['command'], 'backdrop') !== FALSE) {
    return;
  }
  // Commands that work as-is.
  if (in_array($command['command'], $safe_commands)) {
    return;
  }
  // Commands that are fine to be run through the compatibility layer.
  if (in_array($command['command'], $compatible_commands)) {
    require_once BACKDROP_ROOT . '/core/includes/drupal.inc';
    $GLOBALS['settings']['backdrop_drupal_compatibility'] = TRUE;
    $backdrop_command = $command['command'];
  }

  // And finally commands that are not supported (yet) use the fallback command.
  $commands = drush_get_commands();

  if (!isset($backdrop_command) || !array_key_exists($backdrop_command, $commands)) {
    $backdrop_command = 'backdrop-unsupported';
  }

  // Replace the command with a Backdrop-supported one.
  $arguments = $command['arguments'];
  $command = $commands[$backdrop_command];

  drush_set_command($command);

  $command['arguments'] = $arguments;

  // Add command-specific options, if applicable.
  drush_command_default_options($command);
}

/**
 * Implements hook_module_implements_alter().
 *
 * This is a hack to remove core Drush's implementation of hook_watchdog() from
 * the hook registry. See system_watchdog() in Drush core.
 */
function system_module_implements_alter(&$implementations, $hook) {
  // Remove system_watchdog() from being called.
  if ($hook === 'watchdog') {
    $implementations = array_diff_key($implementations, array('system' => NULL));
  }
}

/**
 * Impements hook_drush_invoke_alter().
 */
function backdrop_drush_invoke_alter(&$modules, $hook) {
  // The sql_drush_sql_sync_sanitize() function has Drupal-specific code for
  // hashing passwords. We disable it and replace it with our own verion.
  if ($hook === 'drush_sql_sync_sanitize') {
    $key = array_search('sql', $modules);
    if (isset($key)) {
      unset($modules[$key]);
    }
  }
}

/**
 * Implements hook_drush_sql_sync_sanitize().
 *
 * Replaces sql_drush_sql_sync_sanitize(), which has Drupal-specific code.
 *
 * @see sql_drush_sql_sync_sanitize()
 * @see backdrop_drush_invoke_alter()
 */
function backdrop_drush_sql_sync_sanitize($site) {
  $site_settings = drush_sitealias_get_record($site);
  $databases = sitealias_get_databases_from_record($site_settings);
  if (drush_get_option('db-prefix') || !empty($databases['default']['default']['prefix'])) {
    $wrap_table_name = TRUE;
  }
  else {
    $wrap_table_name = FALSE;
  }
  $user_table_updates = array();
  $message_list = array();

  // Sanitize passwords.
  $newpassword = drush_get_option(array('sanitize-password', 'destination-sanitize-password'), 'password');
  if ($newpassword != 'no' && $newpassword !== 0) {
    $core = DRUSH_BACKDROP_CORE;
    include_once $core . '/includes/password.inc';
    include_once $core . '/includes/bootstrap.inc';
    $hash = user_hash_password($newpassword);
    $user_table_updates[] = "pass = '$hash'";
    $message_list[] =  "passwords";
  }

  // Sanitize email addresses.
  $newemail = drush_get_option(array('sanitize-email', 'destination-sanitize-email'), 'user+%uid@localhost.localdomain');
  if ($newemail != 'no' && $newemail !== 0) {
    if (strpos($newemail, '%') !== FALSE) {
      $email_map = array('%uid' => "', uid, '", '%mail' => "', replace(mail, '@', '_'), '", '%name' => "', replace(name, ' ', '_'), '");
      $newmail =  "concat('" . str_replace(array_keys($email_map), array_values($email_map), $newemail) . "')";
      $user_table_updates[] = "mail = $newmail, init = $newmail";
    }
    else {
      $user_table_updates[] = "mail = '$newemail', init = '$newemail'";
    }
    $message_list[] = 'email addresses';
  }

  if (!empty($user_table_updates)) {
    $table = 'users';
    if ($wrap_table_name) {
      $table = "{{$table}}";
    }
    $sanitize_query = "UPDATE {$table} SET " . implode(', ', $user_table_updates) . " WHERE uid > 0;";
    drush_sql_register_post_sync_op('user-email', dt('Reset !message in !table table', array('!message' => implode(' and ', $message_list), '!table' => $table)), $sanitize_query);
  }

  // Seems quite portable (SQLite?) - http://en.wikipedia.org/wiki/Truncate_(SQL)
  $table = 'sessions';
  if ($wrap_table_name) {
    $table = "{{$table}}";
  }
  $sql_sessions = "TRUNCATE TABLE {$table};";
  drush_sql_register_post_sync_op('sessions', dt('Truncate Backdrop\'s sessions table'), $sql_sessions);
}
