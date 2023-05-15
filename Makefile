CXX=g++
CXX_FLAGS=-I./include

SRC_DIR=./src
BUILD_DIR=./build

all:
	mkdir -p ${BUILD_DIR}
	${CXX} ${CXX_FLAGS} -g -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} -g ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} -g ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o

release:
	mkdir -p ${BUILD_DIR}
	${CXX} ${CXX_FLAGS} -O2 -flto -fprofile-generate -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} -O2 -flto -fprofile-generate ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} -O2 -flto -fprofile-generate ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o
	${BUILD_DIR}/birb_dep_solver mesa &>/dev/null
	${BUILD_DIR}/birb_pkg_search iceauth luit mkfontscale sessreg setxkbmap smproxy x11perf xauth xbacklight xcmsdb xcursorgen xdpyinfo xdriinfo xev xgamma xhost xinput xkbcomp xkbevd xkbutils xkill xlsatoms xlsclients xmessage xmodmap xpr xprop xrandr xrdb xrefresh xset xsetroot xvinfo xwd xwininfo xwud &>/dev/null
	${CXX} ${CXX_FLAGS} -O2 -flto -fprofile-use -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} -O2 -flto -fprofile-use ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} -O2 -flto -fprofile-use ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o


install:
	cp ./birb ${BUILD_DIR}/{birb_dep_solver,birb_pkg_search} /usr/bin/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf

clean:
	rm -rf ${BUILD_DIR}
