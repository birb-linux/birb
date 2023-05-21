CACHE_DIR="/var/cache"
LIB_DIR="/var/lib/birb"
DB_DIR="/var/db"
REPO_DIR="$DB_DIR/pkg"
BIRB_SRC_DIR="$CACHE_DIR/distfiles/birb"

CXX=g++
CXX_FLAGS=-I./include
RELEASE_CXX_FLAGS=-O2 -march=native

SRC_DIR=./src
BUILD_DIR=./build

all: debug_dep_solver debug_pkg_search debug_birb_db

build_dir:
	mkdir -p ${BUILD_DIR}

debug_birb: build_dir
	${CXX} ${CXX_FLAGS} -g -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o

debug_dep_solver: debug_birb
	${CXX} ${CXX_FLAGS} -g ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o

debug_pkg_search: debug_birb
	${CXX} ${CXX_FLAGS} -g ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o

debug_birb_db: debug_birb
	${CXX} ${CXX_FLAGS} -g ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o


release:
	mkdir -p ${BUILD_DIR}
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-generate -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-generate ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-generate ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -fprofile-generate ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o
	${BUILD_DIR}/birb_dep_solver xterm &>/dev/null
	${BUILD_DIR}/birb_pkg_search iceauth luit mkfontscale sessreg setxkbmap smproxy x11perf xauth xbacklight xcmsdb xcursorgen xdpyinfo xdriinfo xev xgamma xhost xinput xkbcomp xkbevd xkbutils xkill xlsatoms xlsclients xmessage xmodmap xpr xprop xrandr xrdb xrefresh xset xsetroot xvinfo xwd xwininfo xwud &>/dev/null
	${BUILD_DIR}/birb_db --update this_package_will_not_exist 1.2.3
	${BUILD_DIR}/birb_db --list &>/dev/null
	${BUILD_DIR}/birb_db --remove this_package_will_not_exist
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-use -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-use ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-use ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -fprofile-use ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o


install:
	cp ./birb ${BUILD_DIR}/{birb_dep_solver,birb_pkg_search,birb_db} /usr/bin/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf


bootstrap:
	# Make sure that all documentation directories exist and have a file in them
	mkdir -p ${LFS}/usr/share/{info,man/man{1..8},doc}
	touch ${LFS}/usr/share/info/.birb_symlink_lock
	touch ${LFS}/usr/share/man/.birb_symlink_lock
	touch ${LFS}/usr/share/man/man{1..8}/.birb_symlink_lock
	# Protect some misc. directories from stow takeover
	mkdir -p ${LFS}/usr/share/pkgconfig
	touch ${LFS}/usr/share/pkgconfig/.birb_symlink_lock
	mkdir -p ${LFS}/lib/udev/rules.d
	touch ${LFS}/lib/udev/rules.d/.birb_symlink_lock
	touch ${LFS}/sbin/.birb_symlink_lock
	mkdir -pv ${LFS}/usr/share/fonts/TTF
	touch ${LFS}/usr/share/fonts/TTF/.birb_symlink_lock
	mkdir -pv ${LFS}/usr/share/applications
	touch ${LFS}/usr/share/applications/.birb_symlink_lock
	# Create directories for birb
	mkdir -p ${LFS}/${DB_DIR}/fakeroot
	mkdir -p ${LFS}/${CACHE_DIR}/distfiles
	mkdir -p ${LFS}/${LIB_DIR}
	mkdir -p ${LFS}/usr/python_dist
	# Create the nest file
	touch ${LFS}/${LIB_DIR}/nest
	# Update the repository
	cd ${LFS}/${REPO_DIR} ; git reset --hard ; git config pull.rebase true ; git fetch ; git pull


clean:
	rm -rf ${BUILD_DIR}
