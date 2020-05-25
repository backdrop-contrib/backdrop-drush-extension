<?php
/**
 * @file
 * Create a subtheme of basis.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_subtheme_basis_drush_command() {
  $items = array();
  $items['backdrop-subtheme-basis'] = array(
    'description' => 'Create a subtheme of basis.',
    'aliases' => array('stb', 'sb'),
    'callback' => 'backdrop_subtheme_basis',
    'options' => array(
      'name' => 'The name of the subtheme.',
    ),
    'examples' => array(
      'drush sb' => 'Prompts the user for info for subtheme.',
      'drush sb --name=my_theme' => 'Provide the the name as an option.',
    ),
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );

  return $items;
}

/**
 * Command callback for subtheme_basis.
 */
function backdrop_subtheme_basis() {
  $name = drush_get_option('name', NULL);
  if (!$name) {
    $name = drush_prompt("\n\t\033[34mEnter a name for your theme \033[0m");
  }

  $bold_name = drush_chalk($name, '1m');
  drush_print_r("\n\t\tCreating $bold_name theme structure ...\n");
  exec(
    "mkdir -p themes/$name"
  );

  exec(
    "mkdir -p themes/$name/css"
  );

  exec(
    "mkdir -p themes/$name/templates"
  );

  exec(
    "cp core/modules/system/templates/page.tpl.php themes/$name/templates/"
  );

  exec(
    "cp core/themes/basis/css/skin.css themes/$name/css/"
  );

  exec(
    "cp -a core/themes/basis/images themes/$name/images"
  );

  drush_print_r("\t\tWriting $name.info file ...\n");
  $file = fopen("themes/$name/$name.info", 'x');
  fwrite($file, "type = theme\n");
  fwrite($file, "name = $name\n");
  fwrite($file, "description = $name subtheme of Basis\n");
  fwrite($file, "backdrop = 1.x\n\n");
  fwrite($file, "base theme = basis\n\n");
  fwrite($file, "stylesheets[all][] = css/skin.css\n");
  fclose($file);

  if (file_exists("themes/$name/$name.info")) {
    $success = drush_chalk('Success: ', '32m');
    drush_print_r("\t$success$bold_name has been created.\n");
  }
  else {
    $fail = drush_chalk("Failure: ", '31m');
    drush_print_r("$failSomething has gone wrong; check the contents of your themes/ directory.\n");
  }
}
