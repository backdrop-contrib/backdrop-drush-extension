<?php
/**
 * @file
 * Command(s) for displaying watchdog (log) information.
 */

/**
 * Implements hook_drush_command().
 */
function watchdog_drush_command() {
  $items['watchdog-show'] = array(
    'bootstrap' => \Drush\Boot\BackdropBoot::BOOTSTRAP_FULL,
    'description' => 'Show dblog messages like a dog blog.',
    'arguments' => array(
      'wid' => 'Optional id of a watchdog message to show in detail. If not provided, a listing of most recent 10 messages will be displayed.',
    ),
    'options' => array(
      'count' => 'The number of messages to show. Defaults to 10.',
      'severity' => 'Restrict to messages of a given severity level.',
      'type' => 'Restrict to messages of a given type.',
      'tail' => 'Continuously show new watchdog messages until interrupted.',
      'sleep-delay' => 'To be used in conjunction with --tail. This is the number of seconds to wait between each poll to the database. Delay is 1 second by default.',
    ),
    'callback' => 'watchdog_command_show',
    'aliases' => array('wd-show', 'ws'),
  );

  return $items;
}

/**
 * Callback for drush ws.
 */
function watchdog_command_show() {
  $arguments = drush_get_arguments();
  $options['count'] = drush_get_option('count', NULL);
  $options['severity'] = drush_get_option('severity', NULL);
  $options['type'] = drush_get_option('type', NULL);
  $options['tail'] = drush_get_option('tail', NULL);
  $options['sleep-delay'] = drush_get_option('sleep-delay', NULL);

  if (isset($arguments[1]) && is_numeric($arguments[1])) {
    return watchdog_show_one($arguments[1]);
  }
  $output = watchdog_show_many($options);
  $output = drush_format_table($output);
  drush_print($output);return;
}

/**
 * Print a table of watchdog messages.
 *
 * @param array $options
 *   An array of user-provided options to the command.
 */
function watchdog_show_many($options) {
  if (!isset($options['count'])) {
    $options['count'] = 10;
  }
  $query = db_select('watchdog', 'wd')->extend('PagerDefault')
    ->fields('wd')
    ->orderBy('wid', 'DESC');

  if (isset($options['type'])) {
    $query->condition('type', $options['type']);
  }

  if (isset($options['severity'])) {
    $severities = watchdog_get_severities();
    if ($key = array_search($options['severity'], $severities)) {
      $query->condition('severity', $key);
    }
    else {
      return dt('There is no Severity !name', array('!name' => $options['severity']));
    }
  }

  $messages = $query->limit($options['count'])->execute()->fetchAll();

  $header = array(
    'ID',
    'Date',
    'Type',
    'Severity',
    'Message',
  );

  $rows = array();
  $rows[] = $header;
  foreach ($messages as $message) {
    $rows[] = array(
      $message->wid,
      format_date($message->timestamp, 'custom', 'd/M H:i'),
      $message->type,
      watchdog_get_severities($message->severity),
      watchdog_return_message($message),
    );
  }

  return $rows;
}

/**
 * Return message with already replaced variables and made plain.
 */
function watchdog_return_message($message) {
  $variables = $message->variables;
  if (is_string($variables)) {
    $variables = unserialize($variables);
  }
  if (is_array($variables)) {
    $message->message = dt($message->message, $variables);
  }
  $message->message = str_replace("\n", " ", $message->message);
  return strip_tags($message->message);
}

/**
 * Return array of human readable severities.
 */
function watchdog_get_severities($severity = FALSE) {
  $severities = array(
    WATCHDOG_DEBUG => 'debug',
    WATCHDOG_INFO => 'info',
    WATCHDOG_NOTICE => 'notice',
    WATCHDOG_WARNING => 'warning',
    WATCHDOG_ERROR => 'error',
    WATCHDOG_CRITICAL => 'critical',
    WATCHDOG_ALERT => 'alert',
    WATCHDOG_EMERGENCY => 'emergency',
  );

  if ($severity) {
    return $severities[$severity];
  }
  return $severities;
}

/**
 * Print a watchdog message.
 *
 * @param int $wid
 *   The id of the message to show.
 */
function watchdog_show_one($wid) {
  $message = db_select('watchdog', 'wd')
    ->fields('wd')
    ->condition('wid', $wid)
    ->execute()
    ->fetchObject();

  if (!$message) {
    drush_set_message(dt('Recent log message #!wid not found.', array('!wid' => $wid)), 'error');
    return FALSE;
  }

  $rows = array();

  $rows[] = array(
    'ID',
    $message->wid,
  );

  $rows[] = array(
    'Date',
    format_date($message->timestamp, 'custom', 'd/M H:i'),
  );

  $rows[] = array(
    'Type',
    $message->type,
  );

  $rows[] = array(
    'Severity',
    watchdog_get_severities($message->severity),
  );

  $rows[] = array(
    'Message',
    watchdog_return_message($message),
  );
  $output = drush_format_table($rows);
  drush_print($output);
}
