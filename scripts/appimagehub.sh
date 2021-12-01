#!/bin/sh
cacheFile="$HOME/.cache/appimagehub_tmp"
[ -e $cacheFile ] || wget -O $cacheFile "https://appimage.github.io/apps/"
program=$(grep "<tr id=" $cacheFile | sed 's/^[[:space:]]*//g; s/<tr id="\///g; s/\/">//g' | fzf)

[ -e ${cacheFile}_${program} ] || wget -O ${cacheFile}_${program} "https://appimage.github.io/$program/"
surf ${cacheFile}_${program}
