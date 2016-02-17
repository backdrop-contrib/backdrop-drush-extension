<?php
/**
 *  @file
 *  Drush user management for Backdrop
 */

/**
 *  Implements hook_drush_command().
 */
function backdrop_user_drush_command() {
  $items = array();
  $items['backdrop-user-login'] = array(
    'description' => 'Display a one time login link for the given user account (defaults to uid 1).',
    'aliases' => array('uli'),
    'callback' => 'drush_backdrop_user_login',
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'handle-remote-commands' => TRUE,
    'arguments' => array(
      'user' => 'An optional uid, user name, or email address for the user to log in as. Default is to log in as uid 1. The uid/name/mail options take priority if specified.',
      'path' => 'Optional path to redirect to after logging in.',
    ),
    'options' => array(
      'browser' => 'Optional value denotes which browser to use (defaults to operating system default). Set to 0 to suppress opening a browser.',
      'uid' => 'A uid to log in as.',
      'name' => 'A user name to log in as.',
      'mail' => 'A user mail address to log in as.',
    ),
    'examples' => array(
      'drush user-login ryan node/add/blog' => 'Displays and opens default web browser (if configured or detected) for a one-time login link for the user with the username ryan and redirect to the path node/add/blog.',
      'drush user-login --browser=firefox --mail=drush@example.org admin/settings/performance' => 'Open firefox web browser, login as the user with the e-mail address drush@example.org and redirect to the path admin/settings/performance.',
    ),
  );
  return $items;
}

/**
 * Command callback for user-login.
 * Displays a one time login link for the given user.
 *
 * @param string $user
 *  Optional $user to login defaults to uid = 1.
 *
 * @param string $path
 *  Optional $path redirects user to $path once logged in.
 */
function drush_backdrop_user_login($user = NULL, $path = NULL) {
  // Fix up arguments based on our assumptions.
  $user_object = $uid = FALSE;
  $args = func_get_args();

  // Redispatch if called against a remote-host so a browser is started on the
  // the *local* machine.
  $alias = drush_get_context('DRUSH_TARGET_SITE_ALIAS');
  if (drush_sitealias_is_remote_site($alias)) {
    $return = drush_invoke_process($alias, 'user-login', $args, drush_redispatch_get_options(), array('integrate' => FALSE));
    if ($return['error_status']) {
      return drush_set_error('Unable to execute user login.');
    }
    else {
      $link = $return['object'];
    }
  }
  else {
    if (!drush_bootstrap(DRUSH_BOOTSTRAP_DRUPAL_FULL)) {
      // Fail gracefully if unable to bootstrap Drupal.
      // drush_bootstrap() has already logged an error.
      return FALSE;
    }

    if (drush_get_option('uid', FALSE) || drush_get_option('name', FALSE) || drush_get_option('mail', FALSE)) {
      // A user option was passed, prefer that to the user argument.
      $user = NULL;
      // If we only have a single argument and one of the user options is passed,
      // then we assume the argument is the path to open.
      if (count($args) == 1) {
        $path = $args[0];
      }
    }
    else if (empty($user)) {
      // No user option or argument was passed, so we default to uid 1.
      $uid = 1;
    }

    // Try to load a user from provided options and arguments.
    if ($uid || $uid = reset(_drush_user_get_users_from_options_and_arguments($user))) {
      if ($user_object = user_load($uid)) {
        if ($user_object->status) {
          $options = array();
          if ($path) {
            $options['query']['destination'] = $path;
          }
          $link = url(user_pass_reset_url($user_object) . '/login', $options);
        }
        else {
          return drush_set_error("The user account $uid is blocked.");
        }
      }
      else {
        return drush_set_error("The user account for uid $uid could not be loaded.");
      }
    }
    else {
      return FALSE; // An error has already been logged.
    }
  }

  drush_start_browser($link);
  return $link;
}
