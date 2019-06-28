<?php
/**
 * @file
 * Drush user-create command
 * Create new users with given parameters
 */

/**
 * Implements hook_drush_command().
 */
function backdrop_user_create_drush_command() {
  $items = array();
  $items['backdrop-user-create'] = array(
    'description' => 'Create a user account with the specified name.',
    'aliases' => array('ucrt', 'user:create'),
    'callback' => 'backdrop_command_user_create',
    'arguments' => array(
      'name' => 'The name of the account to add',
    ),
    'required-arguments' => TRUE,
    'examples' => array(
      'drush user-create newuser --mail="person@example.com" --password="letmein"' =>
      'Create a new user account with the name newuser, the email address person@example.com, and the password letmein',
    ),
    'options' => array(
      'password' => 'The password for the new account',
      'mail' => 'The email address for the new account',
    ),
    // 'outputformat' => $items['user-information']['outputformat'],
  );
  return $items;
}

/**
 * Creates a new user account.
 *
 * @param string $name
 *   The account name.
 *
 * @return bool
 *   TRUE if user created, FALSE if not.
 */
function backdrop_command_user_create($name) {
  $mail = drush_get_option('mail');
  $pass = drush_get_option('password');
  $new_user = array(
    'name' => $name,
    'pass' => $pass,
    'mail' => $mail,
    'access' => '0',
    'status' => 1,
  );
  if (!drush_get_context('DRUSH_SIMULATE')) {
    if ($account = entity_create('user', $new_user)) {
      $account->save();
      $output = [
        'User ID' => $account->uid,
        'User name' => $account->name,
        'User mail' => $account->mail,
        'User roles' => $account->roles,
        'User status' => $account->status,
      ];
      drush_print_r("\n\t\e[32mSuccess\e[0m: user $account->name created.\n");
      foreach ($output as $key => $property) {
        if (is_array($property)) {
          $property = implode($property);
        }
        drush_print("\t\t$key:\t$property");
      }
      drush_print("\n");

      return TRUE;
    }
    else {
      drush_set_error("Could not create a new user account with the name " . $name . ".");

      return FALSE;
    }
  }
}
