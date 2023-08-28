#!/bin/bash

BUILD_DIR="$(dirname $0)"
BINARIES_TO_RUN="birb_db birb_dep_solver birb_pkg_search"

# Make sure that the build directory exists
if [ ! -d "$BUILD_DIR" ]
then
	echo "Build directory doesn't exist!"
	exit 1
fi

# Make sure that all binaries were built
for i in $BINARIES_TO_RUN
do
	if [ ! -f "$BUILD_DIR/$i" ]
	then
		echo "Missing binary: $i"
		echo "Did you build everything?"
		exit 1
	fi
done

# PGO test run functions
_birb_db()
{
	echo "Running: birb_db"
	${BUILD_DIR}/birb_db --update this_package_will_not_exist 1.2.3
	${BUILD_DIR}/birb_db --list &>/dev/null
	${BUILD_DIR}/birb_db --remove this_package_will_not_exist
	${BUILD_DIR}/birb_db --is-installed ncurses &>/dev/null
	${BUILD_DIR}/birb_db --is-installed alweiuhlawiuefhu &>/dev/null
}

_birb_dep_solver()
{
	echo "Running: birb_dep_solver"
	${BUILD_DIR}/birb_dep_solver dde &>/dev/null
	${BUILD_DIR}/birb_dep_solver firefox &>/dev/null
	${BUILD_DIR}/birb_dep_solver -r ncurses &>/dev/null
}

_birb_pkg_search()
{
	echo "Running: birb_pkg_search"
	${BUILD_DIR}/birb_pkg_search iceauth luit mkfontscale sessreg setxkbmap smproxy x11perf xauth xbacklight xcmsdb xcursorgen xdpyinfo xdriinfo xev xgamma xhost xinput xkbcomp xkbevd xkbutils xkill xlsatoms xlsclients xmessage xmodmap xpr xprop xrandr xrdb xrefresh xset xsetroot xvinfo xwd xwininfo xwud &>/dev/null
}

# Run the functions
_birb_db &
_birb_dep_solver &
_birb_pkg_search &

wait
