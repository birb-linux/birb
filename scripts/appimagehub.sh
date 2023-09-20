#!/bin/sh
cacheFile="$HOME/.cache/appimagehub_tmp"
[ -e $cacheFile ] || wget -O $cacheFile "https://appimage.github.io/apps/"
program=$(grep "<tr id=" $cacheFile | sed 's/^[[:space:]]*//g; s/<tr id="\///g; s/\/">//g' | fzf)

firefox "https://appimage.github.io/$program/"
