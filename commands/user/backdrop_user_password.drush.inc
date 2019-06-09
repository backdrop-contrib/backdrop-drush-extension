<?php
/**
 * @file
 * Drush user-password command
 * Reset the password for user(s).
 */

use Drush\Log\LogLevel;

/**
 * Implements hook_drush_command().
 */
function backdrop_user_password_drush_command() {
  $items['backdrop-user-password'] = array(
    'description' => '(Re)Set the password for the user account with the specified name.',
    'callback' => 'backdrop_user_password',
    'aliases' => array('upwd'),
    'arguments' => array(
      'name' => 'The name of the account to modify.',
    ),
    'required-arguments' => TRUE,
    'options' => array(
      'password' => array(
        'description' => 'The new password for the account.',
        'required' => TRUE,
        'example-value' => 'foo',
      ),
    ),
    'examples' => array(
      'drush user-password someuser --password="correct horse battery staple"' =>
      'Set the password for the username someuser. @see xkcd.com/936',
    ),
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
  );
  return $items;
}


/**
 * Sets the password for the account with the given username.
 */
function backdrop_user_password() {
  $inputs = func_get_args();
  if (!drush_get_context('DRUSH_SIMULATE')) {
    $pass = drush_get_option('password');
    // If no password has been provided, prompt for one.
    if (empty($pass)) {
      $pass = drush_prompt(dt('Password'), NULL, TRUE, TRUE);
    }

    $users = '';
    foreach ($inputs as $input) {
      $user = user_load_by_name($input);
      $user->pass = $pass;
      user_save($user);
      $users .= $input . ', ';
    }
    $users = trim($users, ', ');
    drush_log(dt('Changed password for !users', array('!users' => $users)), LogLevel::SUCCESS);
  }
  backdrop_flush_all_caches();
}
