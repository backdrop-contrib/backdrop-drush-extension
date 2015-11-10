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
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help <name-of-your-command>'
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function backdrop_drush_help($section) {
  switch ($section) {
    case 'meta:backdrop:title':
      return dt('Backdrop CMS commands');
    case 'meta:backdrop:summary':
      return dt('Commands that work specifically with Backdrop CMS.');
  }
}

/**
 * Implementation of hook_drush_command().
 *
 * In this hook, you specify which commands your
 * drush module makes available, what it does and
 * description.
 *
 * Notice how this structure closely resembles how
 * you define menu hooks.
 *
 * @return
 *   An associative array describing your command(s).
 */
function backdrop_drush_command() {
  $items = array();

  $items['backdrop-cron'] = array(
    'description' => 'Run all cron hooks in all active modules for specified site.',
    'hidden' => TRUE,
  );

  $items['backdrop-updatedb'] = array(
    'description' => 'Apply any database updates required (as with running update.php).',
    'hidden' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_SITE,
    'global-options' => array(
      'cache-clear',
    ),
    'options' => array(
      'entity-updates' => 'Run automatic entity schema updates at the end of any update hooks. Defaults to --no-entity-updates.',
    ),
  );

  $items['backdrop-updatedb-batch-process'] = array(
    'description' => 'Perform update functions',
    'hidden' => TRUE,
    'arguments' => array(
      'batch-id' => 'The batch ID that will be processed',
    ),
    'required-arguments' => TRUE,
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_CONFIGURATION,
  );

  $items['backdrop-updatedb-status'] = array(
    'description' => 'List any pending database updates.',
    'hidden' => TRUE,
    'outputformat' => array(
      'default' => 'table',
      'pipe-format' => 'csv',
      'field-labels' => array('module' => 'Module', 'update_id' => 'Update ID', 'description' => 'Description'),
      'fields-default' => array('module', 'update_id', 'description'),
      'output-data-type' => 'format-table',
    ),
  );

  $items['backdrop-unsupported'] = array(
    'description' => 'Fallback command if the provided command is not supported in Backdrop.',
    'hidden' => TRUE,
  );

  return $items;
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
  }

  // Commands that work with Backdrop with no adjustments.
  $safe_commands = array(
    'cache-get',
    'cache-set',
    'core-exec',
    'help',
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
 * Command callback. Runs all cron hooks.
 */
function drush_backdrop_cron() {
  $result = backdrop_cron_run();

  if ($result) {
    drush_log(dt('Cron run successful.'), 'success');
  }
  else {
    return drush_set_error('DRUSH_CRON_FAILED', dt('Cron run failed.'));
  }
}

/**
 * Command handler. Execute update.php code from drush.
 */
function drush_backdrop_updatedb() {
  if (drush_get_context('DRUSH_SIMULATE')) {
    drush_log(dt('updatedb command does not support --simulate option.'), 'ok');
    return TRUE;
  }

  include_once __DIR__ . '/includes/update.inc';
  if (update_main() === FALSE) {
    return FALSE;
  }

  drush_log(dt('Finished performing updates.'), 'ok');
}


/**
 * Command handler. Run a single update via the batch API.
 */
function drush_backdrop_updatedb_batch_process($id) {
  include_once __DIR__ . '/includes/update.inc';
  _update_batch_command($id);
}

/**
 * Command handler. List pending DB updates.
 */
function drush_backdrop_updatedb_status() {
  require_once DRUSH_BACKDROP_CORE . '/includes/install.inc';
  backdrop_load_updates();

  include_once __DIR__ . '/includes/update.inc';
  list($pending, $start) = updatedb_status();
  if (empty($pending)) {
    drush_log(dt("No database updates required"), 'ok');
  }
  return $pending;
}

/**
 * Command callback. Informs the user that the given command is not supported.
 */
function drush_backdrop_unsupported() {
  drush_log(dt('This command is not supported yet by Backdrop.'), 'failed');
  return FALSE;
}

