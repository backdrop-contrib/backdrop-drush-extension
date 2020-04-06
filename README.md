Drush Integration for Backdrop CMS
==================================

[![Build Status](https://travis-ci.org/backdrop-contrib/drush.svg?branch=1.x-1.x)](https://travis-ci.org/backdrop-contrib/drush)

This project allows you to use [Drush](https://github.com/drush-ops/drush) with
[Backdrop CMS](https://backdropcms.org). Drush is a command-line tool for
manipulating content management systems.

This integration is currently capable of executing the following commands:

* `drush dlb backdrop`: Download Backdrop CMS.
* `drush cc`: Clear individual or all caches in Backdrop.
* `drush uli`: Generate a one-time login link.
* `drush upwd`: Update the password for a selected user.
* `drush st` : Check the status of a Backdrop site; bootstrap, database connection etc.
* `drush cron`: Run the regular interval commands in hook_cron().
* `drush updb`: Run database updates through the Backdrop update.php script.

- `drush pml`: Show a list of available extensions (modules and themes).
- `drush dl`: Download and unpack contrib modules, themes, and layouts.
- `drush en`: Enable one or more extensions (modules or themes). Enable dependant extensions as well.
- `drush dis`: Disable one or more extensions (modules or themes). Disable dependant extensions as well.
- `drush pmu`: Uninstall one or more modules and their dependent modules.

* `drush bcex`: Export a Backdrop site configuration.
* `drush bcim`: Import Backdrop site configuration.
* `drush bp`: Generate boilerplate code for developers; *.info and *.module files.
* `drush sql-*`: MySQL connection commands, such as `sql-cli` or `sql-conf`.
* `drush scr`: Execute scripts with the Backdrop API.
* `drush ctl`: List the content types available on the site. (only for backdrush >= 1.1.0)

To see a full list of available drush commands (not all of which will 
necessarily work with Backdrop CMS):

* `drush help`

To get additional information about a specific command

* `drush help <command>`

There are many more commands that Drush may execute, but they need to be updated
for use with Backdrop. Although some commands may have worked through Backdrop's
compatibility layer, for now any untested (and possibly dangerous) commands are
not allowed to be run within a Backdrop installation.

As an alternative to Drush, you may like to consider using
[Backdrop Console](https://github.com/backdrop-contrib/b), which has been built
from the ground-up specifically for Backdrop.

Installation
------------

This project requires that you use the "8.x" branch of drush
(https://github.com/drush-ops/drush/tree/8.x). Neither older versions of drush
nor the new 9.x or master branches will work with this extension.

There are a number of ways to install the Backdrop drush extension. All of them
require the drush `8.x` upstream. The main two ways are to install globally or
site local installs.

The main advantage to the global install is you install the Backdrop Drush
Extension once it operates on all your Backdrop sites. The disadvantage is that
it can effect how drush operates on Drupal sites (sometimes breaking some
commands). We try our best to prevent that, but it has been known to happen.

The main advantage to site local installs is that you will not break your Drupal
sites or other Backdrop sites. The main disadvantage is you need to isntall
drush proper and backdrop drush once per site.

Install with Lando
------------------

I primarily use [Lando](https://docs.devwithlando.io) to do my development and
Lando comes with the Backdrop Drush Extension installed 'out of the box'. I
recommend Lando to do local development in general and as a great way to
isolate dev dependencies and tooling for Backdrop site development.

By default Lando will install the latest stable release of the Backdrop Drush
extension, but you can point it at any git tag or commit hash. If you wish to
live on the bleeding edge and run the Backdrop Drush extension from source you
can use this example
[.lando.yml](https://github.com/backdrop-contrib/drush/wiki/Contributing-code-to-the-Backdrop-drush-extension.)
file.

Install as Site Local
---------------------

Download drush proper to your `BACKDROP_ROOT` (the place where your Backdrop
code exists. The directory structure should loook something like this:

```bash
└─ $ ∴ ll
total 84
drwxrwxr-x 10 gff gff  4096 Jan 12 13:04 ./             <-- BACKDROP_ROOT
drwxrwxr-x  3 gff gff  4096 Jan 12 13:01 ../
drwxrwxr-x  9 gff gff  4096 Jan 12 13:03 core/
drwxr-xr-x 11 gff gff  4096 Jan 12 13:04 drush/         <-- Drush 8.x
-rw-rw-r--  1 gff gff   554 Jan 12 13:03 .editorconfig
drwxrwxr-x  4 gff gff  4096 Jan 12 14:16 files/
drwxrwxr-x  8 gff gff  4096 Jan 12 13:41 .git/
-rw-rw-r--  1 gff gff   257 Jan 12 13:03 .gitignore
-rw-rw-r--  1 gff gff  6017 Jan 12 13:03 .htaccess
-rwxrwxr-x  1 gff gff   578 Jan 12 13:03 index.php*
drwxrwxr-x  2 gff gff  4096 Jan 12 13:03 layouts/
drwxrwxr-x  3 gff gff  4096 Jan 12 14:21 modules/
-rw-rw-r--  1 gff gff  3978 Jan 12 13:03 README.md
```

Then add the Backdrop Drush Extension to the `BACKDROP_ROOT/drush/commands`
directory. Should look something like this:

```bash
└─ $ ∴ ll
total 40
drwxr-xr-x  9 gff gff 4096 Jan 12 13:04 ./      <-- BACKDROP_ROOT/drush/commands
drwxr-xr-x 11 gff gff 4096 Jan 12 13:04 ../
drwxr-xr-x  6 gff gff 4096 Jan 12 17:36 backdrop/  <-- Backdrop Drush Extension
drwxr-xr-x  4 gff gff 4096 Jan 12 13:42 core/
drwxr-xr-x  2 gff gff 4096 Jan 12 13:04 make/
drwxr-xr-x  4 gff gff 4096 Jan 12 13:04 pm/
drwxr-xr-x  2 gff gff 4096 Jan 12 13:04 runserver/
drwxr-xr-x  2 gff gff 4096 Jan 12 13:04 sql/
drwxr-xr-x  2 gff gff 4096 Jan 12 13:04 user/
-rw-r--r--  1 gff gff 3049 Jan 12 13:04 xh.drush.inc
```

Install Globally
----------------

If you are using composer to install drush, you can run the following command to
require the 8.x version:

```bash
composer global require drush/drush:8.x
```

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

Now switch to a Backdrop site's directory and try a command! `drush cron` works
well.

Usage
-----

Use Drush as you would normally with a Drupal website.

License
-------

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.

Maintainers
-----------

- Geoff St. Pierre [@serundeputy](https://github.com/serundeputy)
- Nate Haug [@quicksketch](https://github.com/quicksketch)

Credits
-------

Thanks to all the Drush maintainers for their project, in particular:

- [Greg Anderson](https://github.com/greg-1-anderson)
- [Moshe Weitzman](https://github.com/weitzman)

for their help in making Drush for Backdrop possible.

![Drush Logo](backdrop-drush-extension-logopn.png)

Releasing Versions
------------------

We are using [`release-it`](https://github.com/release-it/release-it) to manage releases.  When you are ready for a release:

```bash
node_modules/.bin/release-it
```

* NOTE that you need to set your `GITHUB_TOKEN` in your environment in order to push and cut releases on GitHub.
