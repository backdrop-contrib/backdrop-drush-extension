<?php
/**
 * @file
 * Drush generate boilerplate code for modules.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_boilerplate_drush_command() {
  $items = array();
  $items['backdrop-boilerplate'] = array(
    'description' => 'Generate boilerplate code for developers; *.info and *.module files.',
    'aliases' => array('bp', 'boilerplate'),
    'callback' => 'backdrop_command_boilerplate',
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
    'examples' => array(
      'drush bp module' => 'Prompts users for the necessary info to create the stub module structure.',
    ),
  );
  return $items;
}

/**
 * Command callback for boilerplate.
 */
function backdrop_command_boilerplate() {
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
  elseif ($type[0] == 'layout') {
    $project_path = BACKDROP_ROOT . '/layouts';
  }
  else {
    drush_print_r(
      dt('drush boilerplate only knows about module, theme, or layout.' . "\n")
    );
    return;
  }

  // Get the info to generate the project from the user.
  $project_name = drush_prompt(
    "\n\t\033[94mEnter the $type[0] name: \033[0m",
    'mymodule'
  );
  $project_path = $project_path . "/$project_name";

  $project_description = drush_prompt(
    "\n\t\033[94mEnter the $type[0] description: \033[0m",
    'This module extends Backdrop ...'
  );

  $project_package = drush_prompt(
    "\n\t\033[94mEnter the $type[0] package: (optional)\033[0m",
    "custom"
  );
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

    if ($info !== FALSE && $module !== FALSE) {
      drush_print_r(
        "\n\n\t" .
        dt(
          "Contgratulations! !project was created in !project_path.",
          array(
            '!project' => $project_name,
            '!project_path' => $project_path,
          )
        ) .
        "\n\n"
      );
    }
  }
  else {
    drush_print_r(
      dt(
        "The project !project already exists.",
        array('!project' => $project_name)
      )
    );
  }
  return TRUE;
}
