#!/bin/bash

_packageQuery()
{
	urlBase="https://packages.debian.org/search?arch=amd64&keywords="
	pkgUrlBase="https://packages.debian.org/bullseye"

	echo "Searching..."
	content=$(curl -s ${urlBase}${query})
	results=$(echo "$content" | grep "bullseye" | sed '/\[<a/d; s|<li.*/bullseye/||g; s/">.*//g; s/^.*[[:space:]]//g')
	chosen=$(echo "$results" | fzf)

	# Get the package site
	content=$(curl -s ${pkgUrlBase}/${chosen})
	[ -n "$verbose" ] && echo "$content"

	downloads=$(grep -i "http://deb.debian.org/debian/pool/main" <<< $content | sed 's/^[[:space:]]*<li><a href="//g; s/">.*//g')

	packageName=$(grep -i "<h1>Package:" <<< $content | sed 's/<h1>Package: //g')
	deps=$(echo "$content" | grep -A 2 "dep:" | sed '/span class/d; /--/d; s/<a.*">//g; s|</a>||g; s/^. *//g' | uniq)

	# Combine version text with package name
	deps=$(echo "$deps" | sed 'N;s/\n/;/')

	#IFS=$(echo -en "\n\b")
	#for i in $deps
	#do
	#	echo $i
	#done

	echo "Link: ${urlBase}${query}"
	echo "Package: $packageName\n"
	echo -e "## Dependencies ##\n$deps\n"
	echo -e "## Downloads ##\n$downloads"
}

query=":null:"

for i in $@
	do
	case $1 in
		-q)
			query=$2
			shift 2
			;;

		-v)
			verbose=true
			shift 1
	esac
done

[ "$query" != ":null:" ] && _packageQuery
