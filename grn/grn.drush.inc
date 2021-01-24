<?php
/**
 * @file
 * Generates HTML release notes based on commit logs in a Git repository.
 *
 * Parses all Git log messages between 2 release tags and automatically
 * generates initial HTML for the release notes. This script must be
 * run inside the root directory of a local Git repo of the project
 * you want to generate release notes for.  Assumes "git" is in your
 * PATH. Otherwise you should set the --git option. The author of the
 * CVS version was dww. Josh The Geek ported the script to Git
 * (for http://drupal.org/node/1002410).
 *
 * Usage:
 * drush release-notes [previous-release-tag] [current-release-tag]
 *
 * This is the Drush version of this script for Backdrop CMS.
 *
 * @author dww (http://drupal.org/user/46549)
 * @author Josh The Geek (http://drupal.org/user/926382)
 */

/**
 * Implements hook_drush_help().
 */
function grn_drush_help($section) {
  switch ($section) {
    case 'drush:release-notes':
      return dt("Generate release notes for a project between two Git tags.");
  }
}

/**
 * Implements hook_drush_command().
 */
function grn_drush_command() {
  $items = array();

  // The 'release-notes' command.
  $items['release-notes'] = array(
    'description' => 'Generate release notes using all commits between two tags',
    'arguments' => array(
      'tag 1' => 'The previous tag, the starting point for the log.',
      'tag 2' => 'The current tag, the ending point for the log. This can be also be a branch',
    ),
    'options' => array(
      'baseurl' => 'Set the base url for all issue links. Defaults to github issue queue for Backdrop CMS usage. Issue number will be appended to path or replace "%s".',
      'changelog' => 'Display the commits in the format for release notes as expected by github.com.',
      'md' => 'Display the commits in MD format',
      'commit-count' => 'If set, output will show the number of commits between the two tags',
      'commit-links' => 'Attach a link to the commit in drupalcode.org repository viewer to the end of the commit lines.',
      'git' => array(
        'description' => 'Path to the git binary, defaults to "git"',
        'example-value' => '/path/to/git',
        'value' => 'required',
        'required' => FALSE,
      ),
      'nouser' => 'Do not try to link to user page using the /u/alias, as used in drupal.org',
      'pretty' => array(
        'description' => 'pretty format of the message, see the git-log man page (section "PRETTY FORMATS")',
        'example-value' => '%s',
        'value' => 'required',
        'required' => FALSE,
      ),
      'reverse' => 'Display the commits from old to new instead of the default Git behavior that is new to old.',
    ),
    'examples' => array(
      'drush release-notes' => 'Generate release notes from all commits between the two last tags',
      'drush rn 6.x-1.1' => 'Generate release notes from all commits between 6.x-1.1 (as end) and the previous tag (as start)',
      'drush rn 6.x-1.0 6.x-1.1' => 'Generate release notes from all commits between 6.x-1.0 and 6.x-1.1',
      'drush rn 6.x-1.0 6.x-1.x' => 'Use a branch for tag2 (6.x-1.x)',
      'drush rn 6.x-1.0 origin/6.x-1.x' => 'If you don\'t have the branch locally, you might need to use "[remote-name]/[branch-name]"',
      'drush rn 6.x-1.0 6.x-1.x --baseurl="http://community.openatrium.com/node/"' => 'You can specify the changelog to direct issues to other issue trackers.',
      'drush rn 6.x-1.0 6.x-1.x --changelog' => 'Generate release notes from the commits between the two tags in the format for CHANGELOG.txt as expected by drupal.org.',
      'drush rn 6.x-1.0 6.x-1.1 --git=/usr/local/git/bin/git' => 'Use git in /usr/local/git/bin/git, and using alias',
      'drush rn 6.x-1.0 6.x-1.x --pretty="%s (%h)"' => 'Generate release notes using a custom pretty format string',
      'drush rn 6.x-1.0 6.x-1.x --reverse' => 'Generate release notes from the commits between the two tags in reverse order',
    ),
    'aliases' => array('rn', 'relnotes'),
    // I keep typing it, but not intuitive for others.
    'deprecated-aliases' => array('grn'),
    // No bootstrap needed.
    'bootstrap' => DRUSH_BOOTSTRAP_NONE,
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_grn_release_notes($tag1n = NULL, $tag2n = NULL) {
  $git = drush_get_option('git', 'git');
  // Make sure we are in the working directory started.
  drush_shell_exec('chdir ' . drush_cwd());
  if (!is_dir(".git")) {
    drush_log("This must be run from the root directory of your Git project.");
  }

  // Fill in calculated tags if both are not given.
  if (!isset($tag2n)) {
    // Get all the defined tags in this repository and sort them.
    drush_shell_exec('%s rev-parse --abbrev-ref HEAD', $git);
    $branch = substr(array_shift(drush_shell_exec_output()), 0, -1);
    drush_shell_exec('%s tag -l %s*', $git, $branch);
    $tags = drush_shell_exec_output();
    usort($tags, 'version_compare');

    if (!isset($tag1n) && count($tags)) {
      // If no tags are provided, use the two most recent ones.
      $tag2n = array_pop($tags);

      $tag1n = count($tags) ? array_pop($tags) : $tag2n;
    }
    else {
      // If only one tag is given, it is considered to be <end> and <start> is
      // taken to be one tag before it.

      $key = array_search($tag1n, $tags);
      if (is_numeric($key)) {
        if ($key > 0) {
          // Rearrange our tags: the given tag is in fact tag 2.
          $tag2n = $tag1n;
          // The <start> tag is one before the given <end> tag.
          $tag1n = $tags[$key - 1];
        }
        else {
          return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is the first in the branch.', array('!tag' => $tag1n)));
        }
      }
      else {
        return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is not a valid Git tag.', array('!tag' => $tag1n)));
      }
    }
  }

  // '^' is the escape character on Windows (like '\' on *nix) - has to be
  // contained in the escaped shell argument string ("%s").
  if (!drush_shell_exec('%s show -s --pretty=format:%%H %s', $git, $tag1n . '^{commit}')) {
    return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is not a valid Git tag.', array('!tag' => $tag1n)));
  }
  $tag1 = drush_shell_exec_output();
  // '^' is the escape character on Windows (like '\' on *nix) - has to be
  // contained in the escaped shell argument string ("%s").
  if (!drush_shell_exec('%s show -s --pretty=format:%%H %s', $git, $tag2n . '^{commit}')) {
    return drush_set_error('DRUSH_INVALID_TAG', dt('!tag is not a valid Git tag.', array('!tag' => $tag2n)), 'error');
  }
  $tag2 = drush_shell_exec_output();
  $changes = _drush_grn_get_changes($tag1[0], $tag2[0], $git);

  // Check if we need to produce commit links.
  $commit_path = drush_get_option('commit-links', FALSE) ? _drush_grn_get_commit_path($git) : '';

  $items = _drush_grn_get_items_array($changes, $commit_path);
  if (drush_get_option('changelog', FALSE)) {
    // $formatted_items = _drush_grn_format_changelog($items, $tag2n);
  }
  elseif (drush_get_option('md', FALSE)) {
    $formatted_items = _drush_grn_format_md($items, $tag1n, $tag1[0], $tag2[0], $git);
  }
  else {
    $formatted_items = _drush_grn_format_md($items, $tag1n, $tag1[0], $tag2[0], $git);
  }
  if (drush_get_context('DRUSH_PIPE')) {
    drush_print_pipe(array_keys($formatted_items['raw']));
  }
  else {
    drush_print($formatted_items['rendered']);
  }

  return TRUE;
}

// Other functions.

/**
 * Generates the output.
 */
function _drush_grn_format_changes($items, $prev_tag, $tag1, $tag2, $git) {
  $rendered = "<p>Changes since $prev_tag";
  if (drush_get_option('commit-count')) {
    $rendered .= ' (' . trim(drush_get_option('commit-count')) . ' commits)';
  }
  $rendered .= ":</p>\n";

  if (!empty($items)) {
    $rendered .= "<ul>\n  <li>" . implode("</li>\n  <li>", $items) . "</li>\n</ul>";
  }

  return array('rendered' => $rendered, 'raw' => $items);
}

/**
 * Generated output in MD format.
 */
function _drush_grn_format_md($items, $prev_tag, $tag1, $tag2, $git) {
  $rendered = "**Changes since $prev_tag";
  if (drush_get_option('commit-count')) {
    $rendered .= ' (' . trim(drush_get_option('commit-count')) . ' commits)';
  }
  $rendered .= "**\n\n";

  if (!empty($items)) {
    $rendered .= "* " . implode("\n* ", $items) . "\n";
  }

  return array('rendered' => $rendered, 'raw' => $items);
}

/**
 * Generated output in Changelog format.
 */
function _drush_grn_format_changelog($items, $tag) {
  if ($infos = glob('*.info')) {
    foreach ($infos as $info) {
      $ini_array = parse_ini_file($info);
      $name = trim($ini_array['name'], " \t\n\r\0\x0B\"");
    }
  }
  drush_shell_exec("git show -s --pretty=format:%ad --date=short $tag");
  $date = drush_shell_exec_output();
  $changelog = empty($name) ? '' : ($name . " ");
  $changelog .= $tag . ", " . $date[0];
  if (drush_get_option('commit-count')) {
    $changelog .= ' (' . trim(drush_get_option('commit-count')) . ' commits)';
  }
  $changelog .= "\n" . str_pad("", strlen($changelog), "-");
  $changelog .= "\n";
  foreach ($items as $raw => $html) {
    $changelog .= '- ';
    $line = ucfirst(trim((strpos($raw, "#") === 0) ? substr(strstr($raw, ':'), 1) : $raw));
    $changelog .= substr($line, -1) == "." ? $line : $line . ".";
    $changelog .= "\n";
  }
  return array('raw' => $items, 'rendered' => $changelog);
}

/**
 * Discover the URL to the 'origin' repository viewer on drupalcode.org.
 *
 * @param string $git
 *   the actual git command we are going to use.
 *
 * @return string
 *   The URL to the repository viewer.
 */
function _drush_grn_get_commit_path($git) {
  // Execute the git config --get remote.origin.url
  if (!drush_shell_exec("%s config --get remote.origin.url", $git)) {
    return drush_set_error('DRUSH_GIT_CONFIG_ERROR', 'git config returned an error.');
  }
  // Get the output of the above command.
  $output = drush_shell_exec_output();
  // Check to see if this is a drupal.org project.
  if (!strpos($output[0], 'drupal.org')) {
    return drush_set_error('DRUSH_COMMIT_URL_UNKNOWN_ERROR', 'The commits url is not in drupal.org.');
  }
  // Check to see if this is a sandbox project.
  $regexp = (strpos($output[0], '@git.drupal.org:')) ? '/(\w+?@git\.drupal\.org:project\/)/s' : '/http:\/\/git\.drupal\.org\//s';

  // Format the URL of the project's repository.
  $count = 0;
  $return = preg_replace($regexp, 'http://drupalcode.org/', $output[0], -1, $count);
  // Maybe an error occured?
  if ($count <= 0) {
    return drush_set_error('DRUSH_COMMIT_URL_UNKNOWN_ERROR', 'The commits url could not be discovered.');
  }
  // Done.
  return $return . '/commit/';
}

/**
 * Gets the changes and returns them in an array.
 */
function _drush_grn_get_changes($tag1, $tag2, $git) {
  $changes = array();

  $reverse = drush_get_option('reverse', FALSE) ? '--reverse' : '';
  $pretty = drush_get_option('pretty', FALSE);
  if (is_bool($pretty)) {
    // Default pretty format.
    $pretty = '%s';
  }
  $pretty = '%h ' . $pretty;

  if (!drush_shell_exec("%s log -s --pretty=format:%s %s %s..%s", $git, $pretty, $reverse, $tag1, $tag2)) {
    return drush_set_error('DRUSH_GIT_LOG_ERROR', 'git log returned an error.');
  }
  $output = drush_shell_exec_output();
  foreach ($output as $line) {
    if (empty($line)) {
      // Skip blank lines that are left behind in the messages.
      continue;
    }
    $changes[substr($line, 0, 7)] = substr($line, 8);
  }
  if (drush_get_option('commit-count')) {
    drush_set_option('commit-count', count($changes));
  }
  return $changes;
}

/**
 * Get the items array.
 *
 * @param array $issues
 *   Array of issues.
 * @param string $commit_path
 *   The path to the commit.
 */
function _drush_grn_get_items_array($issues, $commit_path) {
  $baseurl = drush_get_option('baseurl', 'https://github.com/backdrop/backdrop-issues/issues/');
  if (strpos($baseurl, '%s') == FALSE) {
    $baseurl .= '%s';
  }

  $items = array();
  foreach ($issues as $hash => $line) {
    // Pattern is:
    // "Issue #[issue number] by [comma-separated usernames]: \
    // [Short summary of the change]."
    // Clean up commit log.
    $raw = preg_replace('/^(Patch |- |Issue ){0,3}/', '', $line);
    // Add issue links.
    $item = (drush_get_option('md', FALSE)) ? preg_replace('/#(\d+)/S', '[#$1](' . str_replace('%s', '$1', $baseurl) . ')', $raw) :
      preg_replace('/#(\d+)/S', '<a href="' . str_replace('%s', '$1', $baseurl) . '">#$1</a>', $raw);
    // Replace usernames with link to user page.
    if (drush_get_option('nouser', FALSE) === FALSE) {
      // Anything between by and ':' is a comma-separated list of usernames.
      $item = preg_replace_callback(
        '/by ([^:]+):/S',
        // @codingStandardsIgnoreStart
        function($matches) {
          $out = array();
          // Separate the different usernames.
          foreach (explode(',', $matches[1]) as $user) {
            // Trim spaces, convert to lowercase, and spaces in middle are
            // replaced with dashes
            $ualias = str_replace(' ', '-', strtolower(trim($user)));
            $out[] = (drush_get_option('md', FALSE)) ? "[$ualias](/u/$ualias)" :
              ("<a href='/u/$ualias'>" . trim($user) . '</a>');
          }

          return 'by ' . implode(', ', $out) . ':';
        },
        $item
      );
      // @codingStandardsIgnoreEnd
    }
    // If the command was invoked with the --commit-links option.
    if (!empty($commit_path)) {
      $item .= ' (<a href="' . $commit_path . $hash . '" title="View commit">#</a>)';
    }
    $items[$raw] = $item;
  }
  drush_command_invoke_all_ref('release_notes_output_alter', $items);
  return $items;
}
