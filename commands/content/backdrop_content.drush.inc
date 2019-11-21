<?php
/**
 * @file
 * Drush content commands.
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_content_drush_command() {
  $itmes = array();
  $items['backdrop-content-type-list'] = array(
    'description' => 'List the content types available on the site.',
    'aliases' => array('ctl', 'content-type-list'),
    'callback' => 'backdrop_content_type_list',
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
    'arguments' => array(
      'type' => 'An optional machine name of a content type to get details on that content type.',
    ),
    'options' => array(
      'field' => 'Show info for the field (overrides --fields).',
      'fields' => 'Show the fields of the content type.',
    ),
    'examples' => array(
      'drush ctl' => 'Lists all the content types on the site.',
      'drush ctl page' => 'Lists the details of the page content type.',
      'drush ctl page --fields' => 'List the fields of the page content type.',
      'drush ctl page --field=body' => 'Get the details of the body field on page content type.',
    ),
  );

  return $items;
}

/**
 * Callback for backdrop_content_type_list.
 */
function backdrop_content_type_list($type = NULL) {
  $args = func_get_args();
  $type = isset($args[0]) ? $args[0] : NULL;
  $types = node_type_get_types();
  $fields_opt = drush_get_option('fields', FALSE);
  $field_opt = drush_get_option('field', FALSE);
  if ($type === NULL) {
    drush_print_r("\n\t\033[1mType\t\tName\t\tDescription\033[0m\n");
    foreach ($types as $my_type) {
      drush_print_r("\t$my_type->type\t\t$my_type->name\t\t$my_type->description");
    }
    drush_print_r("\n");
  }
  elseif ($field_opt) {
    $field = field_info_instance('node', $field_opt, $type);
    print_r($field);
  }
  elseif ($fields_opt) {
    if ($fields_opt) {
      $fields = field_info_instances('node', $type);
      print_r($fields);
    }
  }
  elseif ($type !== NULL) {
    $details = node_type_load($type);
    print_r($details);
  }
}
