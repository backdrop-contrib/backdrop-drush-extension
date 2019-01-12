#!/bin/bash

# Help menu
print_help() {
cat <<-HELP
This script is used to fix permissions of a Backdrop installation
you need to provide the following arguments:

  1) Path to your Backdrop installation.
  2) Username of the user that you want to give files/directories ownership.
  3) HTTPD group name (defaults to www-data for Apache).

Usage: (sudo) bash ${0##*/} --backdrop_path=PATH --backdrop_user=USER --httpd_group=GROUP
Example: (sudo) bash ${0##*/} --backdrop_path=/usr/local/apache2/htdocs --backdrop_user=john --httpd_group=www-data
HELP
exit 0
}

if [ $(id -u) != 0 ]; then
  printf "**************************************\n"
  printf "* Error: You must run this with sudo or root*\n"
  printf "**************************************\n"
  print_help
  exit 1
fi

backdrop_path=${1%/}
backdrop_user=${2}
httpd_group="${3:-www-data}"

# Parse Command Line Arguments
while [ "$#" -gt 0 ]; do
  case "$1" in
    --backdrop_path=*)
        backdrop_path="${1#*=}"
        ;;
    --backdrop_user=*)
        backdrop_user="${1#*=}"
        ;;
    --httpd_group=*)
        httpd_group="${1#*=}"
        ;;
    --help) print_help;;
    *)
      printf "***********************************************************\n"
      printf "* Error: Invalid argument, run --help for valid arguments. *\n"
      printf "***********************************************************\n"
      exit 1
  esac
  shift
done

if [ -z "${backdrop_path}" ] || [ ! -d "${backdrop_path}/sites" ] || [ ! -f "${backdrop_path}/core/modules/system/system.module" ] && [ ! -f "${backdrop_path}/modules/system/system.module" ]; then
  printf "*********************************************\n"
  printf "* Error: Please provide a valid Backdrop path. *\n"
  printf "*********************************************\n"
  print_help
  exit 1
fi

if [ -z "${backdrop_user}" ] || [[ $(id -un "${backdrop_user}" 2> /dev/null) != "${backdrop_user}" ]]; then
  printf "*************************************\n"
  printf "* Error: Please provide a valid user. *\n"
  printf "*************************************\n"
  print_help
  exit 1
fi

cd $backdrop_path
printf "Changing ownership of all contents of "${backdrop_path}":\n user => "${backdrop_user}" \t group => "${httpd_group}"\n"
chown -R ${backdrop_user}:${httpd_group} .

printf "Changing permissions of all directories inside "${backdrop_path}" to "rwxr-x---"...\n"
find . -type d -exec chmod u=rwx,g=rx,o= '{}' \;

printf "Changing permissions of all files inside "${backdrop_path}" to "rw-r-----"...\n"
find . -type f -exec chmod u=rw,g=r,o= '{}' \;

printf "Changing permissions of "files" directories in "${backdrop_path}/sites" to "rwxrwx---"...\n"
cd sites
find . -type d -name files -exec chmod ug=rwx,o= '{}' \;

printf "Changing permissions of all files inside all "files" directories in "${backdrop_path}/sites" to "rw-rw----"...\n"
printf "Changing permissions of all directories inside all "files" directories in "${backdrop_path}/sites" to "rwxrwx---"...\n"
for x in ./*/files; do
  find ${x} -type d -exec chmod ug=rwx,o= '{}' \;
  find ${x} -type f -exec chmod ug=rw,o= '{}' \;
done
echo "Done setting proper permissions on files and directories"
exit 0
