Drush Integration for Backdrop CMS
==================================

This project allows you to use [Drush](https://github.com/drush-ops/drush) with
[Backdrop CMS](https://backdropcms.org). Drush is a command-line tool for
manipulating content management systems.

This integration is currently capable of executing the following commands:

- `drush cc`: Clear individual or all caches in Backdrop.
- `drush dl`: Download and unpack contrib modules, themes, and layouts.
- `drush updb`: Run database updates through the Backdrop update.php script.
- `drush sql-*`: MySQL connection commands, such as `sql-cli` or `sql-conf`.
- `drush cron`: Run the regular interval commands in hook_cron().
- `drush scr`: Execute scripts with the Backdrop API.
- `drush st` : Check the status of a Backdrop site; bootstrap, database connection etc.

There are many more commands that Drush may execute, but they need to be updated
for use with Backdrop. Although some commands may have worked through Backdrop's
compatibility layer, for now any untested (and possibly dangerous) commands are
not allowed to be run within a Backdrop installation.

Installation
------------

This project requires that you use the "8.x" branch of drush (https://github.com/drush-ops/drush/tree/8.x). Neither older versions of drush nor the new 9.x or master branches will work with this extension.

If you are using composer to install drush, you can run the following command to require the 8.x version:
`composer global require drush/drush:8.x`

To install the Backdrop integration for Drush, clone or download this project
into any location that supports Drush commands. The most common location for
custom Drush commands such as this is in your user's home directory.

- `mkdir ~/.drush/commands` (This may already exist, if so continue.)
- `cd ~/.drush/commands`
  -- Get either the latest head or the latest stable:
    --- Latest HEAD: `wget https://github.com/backdrop-contrib/drush/archive/1.x-0.x.zip`
    --- Latest Stable: `https://github.com/backdrop-contrib/drush/releases/latest`
- `unzip master.zip -d backdrop`
- Clear the drush cache
  -- `drush cc drush`

Now switch to a Backdrop site's directory and try a command! `drush cron` works well.

Usage
-----

Use Drush as you would normally with a Drupal website.

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

Thanks to all the Drush maintainers for their project, in particular:

- [Greg Anderson](https://github.com/greg-1-anderson)
- [Moshe Weitzman](https://github.com/weitzman)

for their help in making Drush for Backdrop possible.

![Drush Logo](backdrop-drush-extension-logopn.png)
