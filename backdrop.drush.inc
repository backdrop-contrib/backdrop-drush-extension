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
    case 'pm-enable':
      $backdrop_command = 'backdrop-pm-enable';
      break;
    case 'pm-disable':
      $backdrop_command = 'backdrop-pm-disable';
      break;
    case 'user-password':
      $backdrop_command = 'backdrop-user-password';
      break;
    case 'config-export':
      $backdrop_command = 'backdrop-config-export';
      break;
    case 'config-import':
    case 'backdrop-config-import':
      $backdrop_command = 'backdrop-config-import';
      break;
    case 'fix-permissions':
      $backdrop_command = 'fix-permissions';
    case 'site-install':
      $backdrop_command = 'backdrop-site-install';
      break;
    case 'pm-list':
      $backdrop_command = 'backdrop-pm-list';
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
    'site-alias',
    'sql-cli',
    'sql-conf',
    'sql-connect',
    'sql-create',
    'sql-drop',
    'sql-dump',
    'sql-query',
    'sql-sanitize',
    'release-notes',
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
  // Commands provided by Backdrop modules relative to this site installation.
  if (strpos($command['path'], '/') !== 0 && strpos($command['path'], 'modules') !== FALSE) {
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
