Backdrop `drush bmake` Command
-----------------------------

The `drush bmake` command contstructs a functioning Backdrop CMS codebase based on specifications placed in 
a `makefile.yml`.

Dependencies
------------

To parse the `yaml` file PHP needs the [PHP yaml extension](http://php.net/manual/en/yaml.setup.php). On MacOSX you can use `homebrew` to install the extension if you don't have it:

```bash
brew install php71-yaml
```

Replace the `php71` part with the version of PHP you are using.

Usage
-----

First you have to construct a `myfile.make.yml` makefile. The file should specify the Backdrop core you want,
a list of modules, themes, layouts that you want for the project. The file is written in `yaml` and must adhere to the `yaml` syntax. There is working and commented `example.make.yml` file in the `commands/make` directory or the Backdrop Drush Extension. You can use this file to learn about the structure and even get a working Backdrop Project from it or you can copy and manipulate this file for your purposes.

Once you have a `myfile.make.yml` file you can issue a `drush bmake` command to build the project:

```bash
drush bmake myfile.make.yml mybuildpath
```

This will read the `myfile.make.yml` and build the Backdrop codebase in the build direcotry `mybuildpath`. In order for `drush` to download modules, themes, and layouts it needs to bootstrap Backdrop. In order to do that Backdrop has to be installed. Therefore, `drush bmake` will install Backdrop. In the command above you will be prompted for the installation parameters. If you wish to install non-interactively you can provide all the installation information in a `--db-url` option:

```bash
drush bmake myfile.make.yml mybuildpath --db-url=mysql://root:pass@localhost/mydb
```

Issues
------

If you experience issues using `bmake` or would like to request a feature visit the [issue queue](https://github.com/backdrop-contrib/drush/issues).
