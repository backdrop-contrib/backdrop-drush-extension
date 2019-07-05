<?php
/**
 * @file
 * Core commands for Backdrop's Drush integration.
 */

use \Drush\Boot\BackdropBoot;

define('BACKDROP_DRUSH_VERSION', '1.x-0.x');

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
      'field-labels' => array(
        'module' => 'Module',
        'update_id' => 'Update ID',
        'description' => 'Description',
      ),
      'fields-default' => array('module', 'update_id', 'description'),
      'output-data-type' => 'format-table',
    ),
    'hidden' => TRUE,
  );

  $items['backdrop-core-status'] = array(
    'description' => 'Provides a birds-eye view of the current Backdrop installation, if any.',
    'callback' => 'backdrop_command_core_status',
    'aliases' => array('status', 'st'),
    'examples' => array(
      'drush core-status version' => 'Show all status lines that contain version information.',
      'drush core-status --pipe' => 'A list key=value items separated by line breaks.',
      'drush core-status drush-version --pipe' => 'Emit just the drush version with no label.',
      'drush core-status config-sync --pipe' => 'Emit just the sync Config directory with no label.',
    ),
    'arguments' => array(
      'item' => 'Optional.  The status item line(s) to display.',
    ),
    'options' => array(
      'show-passwords' => 'Show database password.  Defaults to --no-show-passwords.',
      'full' => 'Show all file paths and drush aliases in the report, even if there are a lot.',
      'project' => array(
        'description' => 'One or more projects that should be added to the path list',
        'example-value' => 'foo,bar',
      ),
    ),
    'outputformat' => array(
      'default' => 'key-value',
      'pipe-format' => 'json',
      'field-labels' => array(
        'backdrop-version' => 'Backdrop version',
        'uri' => 'Site URI',
        'db-driver' => 'Database driver',
        'db-hostname' => 'Database hostname',
        'db-port' => 'Database port',
        'db-username' => 'Database username',
        'db-password' => 'Database password',
        'db-name' => 'Database name',
        'db-status' => 'Database',
        'bootstrap' => 'Backdrop bootstrap',
        'user' => 'Backdrop user',
        'theme' => 'Default theme',
        'admin-theme' => 'Administration theme',
        'php-bin' => 'PHP executable',
        'php-conf' => 'PHP configuration',
        'php-os' => 'PHP OS',
        'drush-script' => 'Drush script',
        'drush-version' => 'Drush version',
        'backdrop-drush-version' => 'Backdrop Drush',
        'drush-temp' => 'Drush temp directory',
        'drush-conf' => 'Drush configuration',
        'drush-alias-files' => 'Drush alias files',
        'install-profile' => 'Install profile',
        'root' => 'Drupal root',
        'drupal-settings-file' => 'Backdrop Settings File',
        'site-path' => 'Site path',
        'site' => 'Site path',
        'themes' => 'Themes path',
        'modules' => 'Modules path',
        'files' => 'File directory path',
        'private' => 'Private file directory path',
        'temp' => 'Temporary file directory path',
        'config-sync' => 'Sync config path',
        'files-path' => 'File directory path',
        'temp-path' => 'Temporary file directory path',
        '%paths' => 'Other paths',
      ),
      'formatted-filter' => '_drush_backdrop_core_status_format_table_data',
      'private-fields' => 'db-password',
      'simplify-single' => TRUE,
      'table-metadata' => array(
        'list-separator' => ' ',
      ),
      'output-data-type' => 'format-list',
    ),
    'topics' => array('docs-readme'),
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
 * Helper function for _backdrop_core_site_status_table().
 */
function _drush_backdrop_core_is_named_in_array($key, $the_array) {
  $is_named = FALSE;
  $simplified_key = str_replace(array(' ', '_', '-'), array('', '', ''), $key);
  foreach ($the_array as $name) {
    if (
      stristr(
        $simplified_key,
        str_replace(array(' ', '_', '-'), array('', '', ''), $name))
    ) {
      $is_named = TRUE;
    }
  }

  return $is_named;
}
/**
 * Adjust the status output for any non-pipe output format.
 */
function _drush_backdrop_core_status_format_table_data($output, $metadata) {
  if (drush_get_option('full', FALSE) == FALSE) {
    // Hide any key that begins with a %.
    foreach ($output as $key => $value) {
      if ($key[0] == '%') {
        unset($output[$key]);
      }
    }
    // Hide 'Modules path' and 'Themes path' as well.
    unset($output['modules']);
    unset($output['themes']);
    // Shorten the list of alias files if there are too many.
    if (isset($output['drush-alias-files']) && count($output['drush-alias-files']) > 24) {
      $msg = dt("\nThere are !count more alias files. Run with --full to see the full list.", array('!count' => count($output['drush-alias-files']) - 1));
      $output['drush-alias-files'] = array($output['drush-alias-files'][0] , $msg);
    }
    if (isset($output['drupal-settings-file']) && empty($output['drupal-settings-file'])) {
      $output['drupal-settings-file'] = dt('MISSING');
    }
  }
  return $output;
}

/**
 * Helper function for backdrop_core_status().
 */
function _backdrop_core_site_status_table($project = '') {
  $phase = drush_get_context('DRUSH_BOOTSTRAP_PHASE');
  if ($backdrop_root = drush_get_context('DRUSH_BACKDROP_ROOT')) {

    $boot_object = drush_get_bootstrap_object();
    $status_table['backdrop-version'] = $boot_object->get_version($backdrop_root);
    $conf_dir = $boot_object->conf_path();
    $settings_file = "$conf_dir/settings.php";
    $status_table['drupal-settings-file'] = file_exists($settings_file) ? $settings_file : '';
    if ($site_root = drush_get_context('DRUSH_BACKDROP_SITE_ROOT')) {
      $status_table['uri'] = drush_get_context('DRUSH_URI');
      try {
        $sql = drush_sql_get_class();
        $db_spec = $sql->db_spec();
        $status_table['db-driver'] = $db_spec['driver'];
        if (!empty($db_spec['unix_socket'])) {
          $status_table['db-socket'] = $db_spec['unix_socket'];
        }
        elseif (isset($db_spec['host'])) {
          $status_table['db-hostname'] = $db_spec['host'];
        }
        $status_table['db-username'] = isset($db_spec['username']) ? $db_spec['username'] : NULL;
        $status_table['db-password'] = isset($db_spec['password']) ? $db_spec['password'] : NULL;
        $status_table['db-name'] = isset($db_spec['database']) ? $db_spec['database'] : NULL;
        $status_table['db-port'] = isset($db_spec['port']) ? $db_spec['port'] : NULL;

        if ($phase > BackdropBoot::BOOTSTRAP_CONFIGURATION) {//DRUSH_BOOTSTRAP_BACKDROP_CONFIGURATION) {
          $status_table['install-profile'] = $boot_object->get_profile();
          if ($phase > BackdropBoot::BOOTSTRAP_FULL) {
            $status_table['bootstrap'] = dt('Successful');
            if ($phase == BackdropBoot::BOOTSTRAP_LOGIN) {
              // @TODO: figure out how to really load a user;
              //   add a BackdropUser class?
              // This doesn't really load a user, but it is better than
              // blowing up the commands, i.e. it is usable.
              global $user;
              $status_table['user'] = $user->name;
            }
          }
        }

        // Set Backdrop Drush version for status output.
        $status_table['backdrop-drush-version'] = BACKDROP_DRUSH_VERSION;
      }
      catch (Exception $e) {
        // Don't worry be happy.
      }
    }
    // TODO: what is going on here gff.
    // if (drush_has_boostrapped(DRUSH_BOOTSTRAP_BACKDROP_FULL)) {
    // $status_table['theme'] = drush_theme_get_default();
    // $status_table['admin-theme'] = drush_theme_get_admin();
    // }
  }
  if ($php_bin = drush_get_option('php')) {
    $status_table['php-bin'] = $php_bin;
  }
  $status_table['php-os'] = PHP_OS;
  if ($php_ini_files = _drush_core_config_php_ini_files()) {
    $status_table['php-conf'] = $php_ini_files;
  }
  $status_table['drush-script'] = DRUSH_COMMAND;
  $status_table['drush-version'] = DRUSH_VERSION;
  $status_table['drush-temp'] = drush_find_tmp();
  $status_table['drush-conf'] = drush_flatten_array(drush_get_context_options('context-path', ''));
  $alias_files = _drush_sitealias_find_alias_files();
  $status_table['drush-alias-files'] = $alias_files;

  $paths = _core_path_aliases($project);
  if (!empty($paths)) {
    foreach ($paths as $target => $one_path) {
      $name = $target;
      if (substr($name, 0, 1) == '%') {
        $name = substr($name, 1);
      }
      $status_table[$name] = $one_path;
    }
  }

  // Store the paths into the '%paths' index; this will be
  // used by other code, but will not be included in the output
  // of the drush status command.
  $status_table['%paths'] = $paths;

  return $status_table;
}

/**
 * Command callback. Provides a birds-eye view of the current Backdrop install.
 */
function backdrop_command_core_status() {
  $status_table = _backdrop_core_site_status_table(drush_get_option('project', ''));
  // If args are specified, filter out any entry that is not named
  // (in other words, only show lines named by one of the arg values).
  $args = func_get_args();
  if (!empty($args)) {
    $field_list = $args;
    $metadata = drush_get_command_format_metadata('backdrop-core-status');

    foreach ($metadata['field-labels'] as $field_key => $field_label) {
      if (_drush_backdrop_core_is_named_in_array($field_label, $args)) {
        $field_list[] = $field_key;
      }
    }
    foreach ($status_table as $key => $value) {
      if (!_drush_backdrop_core_is_named_in_array($key, $field_list)) {
        unset($status_table[$key]);
      }
    }
  }
  return $status_table;
}

/**
 * Command callback. Informs the user that the given command is not supported.
 */
function backdrop_command_unsupported() {
  drush_log(dt('This command is not supported yet by Backdrop.'), 'failed');
  return FALSE;
}
