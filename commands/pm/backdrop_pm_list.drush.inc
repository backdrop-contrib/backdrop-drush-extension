<?php
/**
 * @file
 * Drush pm-list command.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_pm_list_drush_command() {
  $items['backdrop-pm-list'] = array(
    'description' => 'Show a list of available extensions (modules and themes).',
    'callback arguments' => array(array(), FALSE),
    'callback' => 'modules_list_callback',
    'options' => array(
      'type' => array(
        'description' => 'Filter by extension type. Choices: module, theme.',
        'example-value' => 'module',
      ),
      'status' => array(
        'description' => 'Filter by extension status. Choices: enabled, disabled and/or \'not installed\'. You can use multiple comma separated values. (i.e. --status="disabled,not installed").',
        'example-value' => 'disabled',
      ),
      'package' => 'Filter by project packages. You can use multiple comma separated values. (i.e. --package="Core - required,Other").',
      'core' => 'Filter out extensions that are not in backdrop core.',
      'no-core' => 'Filter out extensions that are provided by backdrop core.',
    ),
    'outputformat' => array(
      'default' => 'table',
      'pipe-format' => 'list',
      'field-labels' => array(
        'package' => 'Package',
        'name' => 'Name',
        'type' => 'Type',
        'status' => 'Status',
        'version' => 'Version',
      ),
      'output-data-type' => 'format-table',
    ),
    'aliases' => array('pml'),
  );

  return $items;
}

/**
 * Drush pm-list callback. Display list of modules, themes and layouts.
 *
 * @param array $arguments
 *   Arguments for pml
 * @param array $options
 *   Options for pml
 *
 * @return array
 *   The list of projects
 */
function modules_list_callback($arguments, $options) {
  if (!isset($options['type']) or $options['type'] != 'layout') {
    // Rescan for new modules.
    system_rebuild_module_data();
    // Rescan for new themes.
    system_rebuild_theme_data();
    $query = db_select('system', 's')
      ->fields('s');
    if (isset($options['type']) && in_array($options['type'], array('module', 'theme'))) {
      $query->condition('type', $options['type']);
    }
    $projects = $query->execute()->fetchAll();
    $rows = array();
    foreach ($projects as $project) {
      $project->info = unserialize($project->info);
      if (isset($project->info['hidden']) && $project->info['hidden']) {
        continue;
      }

      if ($project->status) {
        $status = 'enabled';
      }
      else {
        $status = 'disabled';
      }
      $name = array(
        'data' => $project->name,
        '#color' => 'bold_blue',
      );
      $rows[] = array(
        $name,
        $project->info['name'],
        $project->type,
        $status,
        isset($project->info['version']) ? $project->info['version'] : dt('Unknown'),
      );
    }
  }

  // TODO: add support for layouts.

  $header = array(
    "\t" . 'Project',
    'Type',
    'Status',
    'Version',
  );

  $pml[] = $header;
  foreach ($rows as $row) {
    $pml[] = array(
      "\t" . $row[0]['data'],
      "\t" . $row[2],
      "\t" . $row[3],
      "\t" . $row[4],
    );
  }
  $output = drush_format_table($pml);
  drush_print($output);
}
