CXX=g++

override CXXFLAGS+=-std=c++20 -I./include -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wundef -Werror -Wno-unused

SRC_DIR=./src
BUILD_DIR=./build

all: dep_solver pkg_search birb_db

build_dir:
	mkdir -p ${BUILD_DIR}

birb: build_dir
	${CXX} ${CXXFLAGS} -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o

dep_solver: birb
	${CXX} ${CXXFLAGS} ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o

pkg_search: birb
	${CXX} ${CXXFLAGS} ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o

birb_db: birb
	${CXX} ${CXXFLAGS} ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o



install:
	cp ./birb ${BUILD_DIR}/{birb_dep_solver,birb_pkg_search,birb_db} /usr/bin/
	mkdir -p /usr/lib/birb
	cp ./birb_funcs /usr/lib/birb/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf
	[ -f /etc/birb-sources.conf ] || cp ./birb-sources.conf /etc/birb-sources.conf

clean:
	rm -rf ${BUILD_DIR}
