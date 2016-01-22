Drush Integration for Backdrop CMS
==================================

This project allows you to use [Drush](https://github.com/drush-ops/drush) with
[Backdrop CMS](https://backdropcms.org). Drush is a command-line tool for
manipulating content management systems.

This project *requires Drush 8* (currently the "master" version). Due to the
recent expansion of Drush capabilities, be sure that you use the very latest
development version with this project.

This integration is currently capable of executing the following commands:

- `drush updb`: Run database updates through the Backdrop update.php script.
- `drush cc`: Clear individual or all caches in Backdrop.
- `drush sql-*`: MySQL connection commands, such as `sql-cli` or `sql-conf`.
- `drush cron`: Run the regular interval commands in hook_cron().
- `drush scr`: Execute scripts with the Backdrop API.

There are many more commands that Drush may execute, but they need to be updated
for use with Backdrop. Although some commands may have worked through Backdrop's
compatibility layer, for now any untested (and possibly dangerous) commands are
not allowed to be run within a Backdrop installation.

Installation
------------

To install the Backdrop integration for Drush, clone or download this project
into any location that supports Drush commands. The most common location for
custom Drush commands such as this is in your user's home directory.

- `mkdir ~/.drush/commands` (This may already exist, if so continue.)
- `cd ~/.drush/commands`
- `wget https://github.com/quicksketch/backdrop-drush/archive/master.zip`
- `unzip master.zip -d backdrop`

Now switch to a Backdrop site's directory and try a command! `drush cron` works well.

Usage
-----

Use Drush as you would normally with a Drupal website. Commands such as
`drush cc all` work directly with Backdrop.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Maintainers
-----------

- Geoff St. Pierre (https://github.com/serundeputy)
- Nate Haug (https://github.com/quicksketch)

Credits
-------

Thanks to all the Drush maintainers for their project. In particular
@greg-1-anderson and @weitzman for their help in making Drush for Backdrop
possible.
