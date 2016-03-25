<?php
/**
 * @file
 * Core commands for Backdrop's Drush integration.
 */
use \Drush\Boot\BackdropBoot;

/**
 * Implements hook_drush_command().
 */
function backdrop_core_drush_command() {
  $items = array();

  $items['backdrop-cron'] = array(
    'description' => 'Run all cron hooks in all active modules for specified site.',
    'callback' => 'backdrop_command_cron',
    'hidden' => TRUE,
  );

  $items['backdrop-cache-clear'] = array(
    'description' => 'Clear a specific cache, or all drupal caches.',
    'callback' => 'backdrop_command_cache_clear',
    'arguments' => array(
      'type' => 'The particular cache to clear. Omit this argument to choose from available caches.',
    ),
    'bootstrap' => BackdropBoot::BOOTSTRAP_FULL,
    'hidden' => TRUE,
  );

  $items['backdrop-updatedb'] = array(
    'description' => 'Apply any database updates required (as with running update.php).',
    'callback' => 'backdrop_command_updatedb',
    'global-options' => array(
      'cache-clear',
    ),
    'options' => array(
      'entity-updates' => 'Run automatic entity schema updates at the end of any update hooks. Defaults to --no-entity-updates.',
    ),
    'bootstrap' => BackdropBoot::BOOTSTRAP_FULL,
    'hidden' => TRUE,
  );

  $items['backdrop-updatedb-batch-process'] = array(
    'description' => 'Perform update functions',
    'callback' => 'backdrop_command_updatedb_batch_process',
    'arguments' => array(
      'batch-id' => 'The batch ID that will be processed',
    ),
    'required-arguments' => TRUE,
    'bootstrap' => BackdropBoot::BOOTSTRAP_CONFIGURATION,
    'hidden' => TRUE,
  );

  $items['backdrop-updatedb-status'] = array(
    'description' => 'List any pending database updates.',
    'callback' => 'backdrop_command_updatedb_status',
    'outputformat' => array(
      'default' => 'table',
      'pipe-format' => 'csv',
      'field-labels' => array('module' => 'Module', 'update_id' => 'Update ID', 'description' => 'Description'),
      'fields-default' => array('module', 'update_id', 'description'),
      'output-data-type' => 'format-table',
    ),
    'hidden' => TRUE,
  );

  $items['backdrop-unsupported'] = array(
    'description' => 'Fallback command if the provided command is not supported in Backdrop.',
    'callback' => 'backdrop_command_unsupported',
    'hidden' => TRUE,
  );

  return $items;
}

/**
 * Command callback. Runs all cron hooks.
 */
function backdrop_command_cron() {
  $result = backdrop_cron_run();

  if ($result) {
    drush_log(dt('Cron run successful.'), 'success');
  }
  else {
    return drush_set_error('DRUSH_CRON_FAILED', dt('Cron run failed.'));
  }

  return TRUE;
}

/**
 * Command callback. Runs all cron hooks.
 */
function backdrop_command_cache_clear() {
  $result = backdrop_cron_run();

  if ($result) {
    drush_log(dt('Cron run successful.'), 'success');
  }
  else {
    return drush_set_error('DRUSH_CRON_FAILED', dt('Cron run failed.'));
  }

  return TRUE;
}

/**
 * Command handler. Execute update.php code from drush.
 */
function backdrop_command_updatedb() {
  if (drush_get_context('DRUSH_SIMULATE')) {
    drush_log(dt('updatedb command does not support --simulate option.'), 'ok');
    return TRUE;
  }

  include_once __DIR__ . '/../includes/update.inc';
  if (update_main() === FALSE) {
    return FALSE;
  }

  drush_log(dt('Finished performing updates.'), 'ok');
  return TRUE;
}


/**
 * Command handler. Run a single update via the batch API.
 */
function backdrop_command_updatedb_batch_process($id) {
  include_once __DIR__ . '/../includes/update.inc';
  _update_batch_command($id);
}

/**
 * Command handler. List pending DB updates.
 */
function backdrop_command_updatedb_status() {
  require_once DRUSH_BACKDROP_CORE . '/includes/install.inc';
  backdrop_load_updates();

  include_once __DIR__ . '/../includes/update.inc';
  list($pending) = updatedb_status();
  if (empty($pending)) {
    drush_log(dt('No database updates required'), 'ok');
  }
  return $pending;
}

/**
 * Command callback. Informs the user that the given command is not supported.
 */
function backdrop_command_unsupported() {
  drush_log(dt('This command is not supported yet by Backdrop.'), 'failed');
  return FALSE;
}
