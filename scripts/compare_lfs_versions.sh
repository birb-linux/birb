#!/bin/bash

# This script scrapes version information from the BLFS and LFS
# books. This is then compared against the local repositories
# to see if there are any packages that might be out-of-date.
#
# This obviously doesn't work 100%, because local packages might be
# more recent than what BLFS and LFS has, but those cases need to be
# check manually anyway


BLFS_PAGE="https://linuxfromscratch.org/blfs/view/stable/"
LFS_PAGE="https://linuxfromscratch.org/lfs/view/stable/"

CACHE_FILE="/tmp/lfs_version_cache"

if [ ! -f "${CACHE_FILE:?}" ]
then
	curl -o $CACHE_FILE -s $BLFS_PAGE
	curl -s $LFS_PAGE >> $CACHE_FILE
fi

BLFS_VERSION_LIST="$(cat $CACHE_FILE | grep -a "a href" | grep -o ">[A-Za-z\-]*-[0-9]*.*[<>]" | sed 's/^>//; s/<.*//' | awk '{print $1}' | tr '[:upper:]' '[:lower:]')"
BIRB_PACKAGE_LIST="$(cat /var/lib/birb/packages)"

# Scan for version differences
((progress=0))
((max_progress=$(echo "$BIRB_PACKAGE_LIST" | wc -w)))

for i in $BIRB_PACKAGE_LIST
do
	echo -e "\e[1A\e[KFinding out-of-date packages... [$progress/$max_progress]"
	((progress+=1))

	# Only get the first result in case there are many
	# shellcheck disable=SC1087
	PKG_LINE="$(echo "$BLFS_VERSION_LIST" | grep "^$i[-_v]" | head -n1)"

	# If a match was found, get the version in birb repos
	if [ -n "$PKG_LINE" ]
	then
		BIRB_PKG_VERSION="$(birb_pkg_search "$i" | cut -d';' -f2)"

		# Check if the version can be found line the PKG_LINE variable
		# If not, the version has changed
		if ! echo "$PKG_LINE" | grep -q "$BIRB_PKG_VERSION"
		then
			OUT_OF_DATE_PACKAGES+="$i "
			echo -e "\e[1A\e[K$i\n"
		fi
	fi
done

OUT_OF_DATE_PACKAGE_COUNT="$(echo "$OUT_OF_DATE_PACKAGES" | wc -w)"
echo "Found $OUT_OF_DATE_PACKAGE_COUNT packages that are possibly out-of-date"
