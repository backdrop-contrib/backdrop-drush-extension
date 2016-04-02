<?php
/**
 *  @file
 *  Drush generate boilerplate code for modules.
 */

/**
 *  Implements hook_drush_command().
 */
function backdrop_boilerplate_drush_command() {
  $items = array();
  $items['backdrop-boilerplate'] = array(
    'description' => 'Generate boilerplate code for developers; *.info and *.module files.',
    'aliases' => array('bp', 'boilerplate'),
    //'callback' => 'backdrop_command_boilerplate',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'options' => array(
      'module' => 'Tell drush that you are generating a module.',
      // TODO: add support for themes and layouts.
    ),
    'examples' => array(
      'drush bp module' => 'Prompts users for the necessary info to create the stub module structure.',
    ),
  );
  return $items;
}

/**
 * Command callback for boilerplate.
 * Generate boilerplate code for stub module structure.
 */
function drush_backdrop_boilerplate() {
  $type = func_get_args();
  $project_path = '';
  $project_name = '';
  $project_description = '';
  $project_package = '';

  if ($type[0] == 'module') {
    if (file_exists(BACKDROP_ROOT . '/modules/custom')) {
      $project_path = BACKDROP_ROOT . '/modules/custom';
    }
    else {
      $project_path = BACKDROP_ROOT . '/modules';
    }
  }
  elseif ($type[0] == 'theme') {
    $project_path = BACKDROP_ROOT . '/themes';
  }
  elseif($type[0] == 'layout') {
    $project_path = BACKDROP_ROOT . '/layouts';
  }
  else {
    drush_print_r(
      dt('drush boilerplate only knows about module, theme, or layout.' . "\n")
    );
    return;
  }
  
  // Get the info to generate the project from the user.
  $handle = fopen ("php://stdin","r");
  print("\n\tEnter the $type[0] name: ");
  print("\033[0m");
  $project_name = fgets($handle);
  $project_name = trim($project_name);
  $project_path = $project_path . "/$project_name";

  $handle = fopen ("php://stdin","r");
  print("\n\tEnter the $type[0] description: ");
  print("\033[0m");
  $project_description = fgets($handle);
  $project_description = trim($project_description);

  $handle = fopen ("php://stdin","r");
  print("\n\tEnter the $type[0] package (optional): ");
  print("\033[0m");
  $project_package = fgets($handle);
  $project_package = trim($project_package);

  if (!file_exists($project_path)) {
    exec(
      "mkdir -p $project_path"
    );

    // Write the info file.
    $info = fopen($project_path . "/$project_name.info", 'c');
      fwrite($info, "name = $project_name\n");
      fwrite($info, "description = $project_description\n");
      fwrite($info, "backdrop = 1.x\n");
      if (!empty($project_package)) {
        fwrite($info, "package = $project_package\n");
      }
      fwrite($info, "type = $type[0]\n");
    fclose($info);

    // Write the module file.
    $module = fopen($project_path . "/$project_name.module", 'c');
      fwrite($module, "<?php\n");
    fclose($module);
  }
  else {
    drush_print_r(dt("The project !project already exists.", array('!project' => $project_name)));
  }
  return TRUE;
}
