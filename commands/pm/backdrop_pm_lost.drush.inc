<?php
/**
 * @file
 * Drush command to list disabled or not-installed projects in Backdrop sites.
 *
 * This can be useful for detecting projects which were downloaded and then
 * forgotten, or projects which have been uninstalled/disabled but not removed
 * from the file system.
 *
 * Inspired by the Drupal version: https://gist.github.com/kalabro/3187485
 */

/**
 * Implementation of hook_drush_command().
 */
function backdrop_pm_lost_drush_command() {
  return array(
    'backdrop-pm-lost' => array(
      'description' => 'Show a list of lost (downloaded but disabled) projects.',
      'aliases' => array('lost'),
    ),
  );
}

/**
 * Callback function for the 'backdrop-pm-lost' command.
 */
function drush_backdrop_pm_lost() {
  // Output a leading newline.
  drush_print();

  // Get 'lost' projects.
  $projects = backdrop_pm_lost_get_list();

  if (!empty($projects)) {
    // Sort projects.
    uasort($projects, '_backdrop_pm_lost_sort_projects');

    // Output table with header row.
    $rows = array(
      array(dt('NAME'), dt('TYPE'), dt('PACKAGE'), dt('VERSION'), dt('STATUS')),
    );

    // Output each project's details.
    foreach ($projects as $name => $project) {
      $rows[] = array(
        $project->name . ' (' . $project->info['name'] . ')',
        ucfirst($project->type),
        !empty($project->info['package']) ? $project->info['package'] : '',
        $project->info['version'],
        _backdrop_pm_lost_status($project),
      );
    }

    // Display output.
    drush_print_table($rows, TRUE);
  }
  else {
    drush_print(dt("No 'lost' projects."), 1);
  }

  // Output a trailing newline.
  drush_print();
}

/**
 * Return a list of 'lost' projects.
 */
function backdrop_pm_lost_get_list() {
  $lost_projects = array();

  // Get all modules & themes.
  // Copied from modules_list_callback() in backdrop_pm_list.drush.inc.
  $modules = system_rebuild_module_data();
  $themes = system_rebuild_theme_data();
  $query = db_select('system', 's')
    ->fields('s')
    ->execute();
  $projects = $query->fetchAll();

  // Find 'lost' projects.
  foreach ($projects as $project) {
    if ($project->status == 0) {
      $lost = TRUE;

      $project->info = unserialize($project->info);
      if (!empty($project->info['project'])) {
        // Ignore core projects.
        if ($project->info['project'] == 'backdrop') {
          $lost = FALSE;
        }

        // Ignore sub-projects.
        if ($project->info['project'] != $project->name) {
          $lost = FALSE;
        }
      }

      // Ignore base themes that have an active sub-theme.
      if ($project->type == 'theme') {
        foreach ($themes as $theme) {
          if (!empty($theme->info['base theme']) && $theme->info['base theme'] == $project->name && $theme->status == 1) {
            $lost = FALSE;
          }
        }
      }

      // Make a list of 'lost' projects.
      if ($lost) {
        $lost_projects[$project->name] = $project;
      }
    }
  }

  return $lost_projects;
}

/**
 * Sort callback function for sorting projects.
 *
 * It will sort by type, then machine name.
 *
 * Partly copied from _drush_pm_sort_extensions() in pm.drush.inc.
 */
function _backdrop_pm_lost_sort_projects($a, $b) {
  if ($a->type == 'module' && $b->type == 'theme') {
    return -1;
  }
  if ($a->type == 'theme' && $b->type == 'module') {
    return 1;
  }

  $cmp = strcasecmp($a->name, $b->name);

  return $cmp;
}

/**
 * Determine a project's status based on its current status and schema version.
 *
 * Partly copied from drush_get_extension_status() in pm.drush.inc.
 */
function _backdrop_pm_lost_status($project) {
  $status = 'Disabled';

  if ($project->type == 'module') {
    // Reminder to uninstall first.
    $status .= ' (needs uninstalling)';

    if ($project->schema_version == -1) {
      $status = 'Not installed';
    }
  }

  return $status;
}

