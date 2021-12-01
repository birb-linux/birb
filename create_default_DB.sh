#!/bin/bash

for i in /sources/*
do
	case $i in
		*patch*) continue ;;
		*)
			fullName=$(basename $i)
			packageName=$(basename $(echo "$i" | sed 's/-[0-9].*//g; s/[0-9]\..*//g; s/\.tar\.gz//g; s/^.*_[0-9].*//g; s/-v[0-9].*//g'))

			version=$(sed "s/${packageName}-//g; s/\.tar.*//g; s/\.orig//g; s/\.tgz//g" <<< $fullName)
			birb -Da "$packageName|$version|"
			;;
	esac
done
