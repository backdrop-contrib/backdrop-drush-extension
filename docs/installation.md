# Installation

Installation instructions.

{% video %}
  https://www.youtube.com/watch?v=1JJ182VhCMs
{% endvideo %}
* Download drush (if you haven't already (If you already have drush installed make sure it is an `8.x` version `drush --version` will tell you what version you have))
  * `git clone https://github.com/drush-ops/drush.git`
* Change directories into your newly downloaded drush
  * `cd drush`
* Checkout the `8.x` branch
  * `git checkout 8.x`
* Install dependencies with composer
  * `composer install`
* Change into the `commands` directory
  * `cd commands`
* Make a directory for the Backdrop Drush Extension
  * `mkdir backdrop`
* Change into the `backdrop` directory
  * `cd backdrop`
* Dowload the Backdrop Drush Extension
  * `git clone https://github.com/backdrop-contrib/drush.git`
