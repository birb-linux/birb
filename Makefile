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
	${BUILD_DIR}/birb_db --is-installed ncurses &>/dev/null
	${BUILD_DIR}/birb_db --is-installed alweiuhlawiuefhu &>/dev/null
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-use -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-use ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -flto -fprofile-use ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -fprofile-use ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o


install:
	cp ./birb ${BUILD_DIR}/{birb_dep_solver,birb_pkg_search,birb_db} /usr/bin/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf

clean:
	rm -rf ${BUILD_DIR}
