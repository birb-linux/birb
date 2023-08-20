CXX=g++
override CXX_FLAGS+=-std=c++20 -I./include -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wundef -Werror -Wno-unused
DEBUG_CXX_FLAGS=-g -Weffc++
RELEASE_CXX_FLAGS=-O1 -flto=auto -march=native -DNDEBUG

SRC_DIR=./src
BUILD_DIR=./build

all: debug_dep_solver debug_pkg_search debug_birb_db

build_dir:
	mkdir -p ${BUILD_DIR}

debug_birb: build_dir
	${CXX} ${CXX_FLAGS} ${DEBUG_CXX_FLAGS} -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o

debug_dep_solver: debug_birb
	${CXX} ${CXX_FLAGS} ${DEBUG_CXX_FLAGS} ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o

debug_pkg_search: debug_birb
	${CXX} ${CXX_FLAGS} ${DEBUG_CXX_FLAGS} ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o

debug_birb_db: debug_birb
	${CXX} ${CXX_FLAGS} ${DEBUG_CXX_FLAGS} ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o


optimized: optimized_dep_solver optimized_pkg_search optimized_birb_db

optimized_birb: build_dir
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o

optimized_dep_solver: optimized_birb
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o

optimized_pkg_search: optimized_birb
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o

optimized_birb_db: optimized_birb
	${CXX} ${CXX_FLAGS} ${RELEASE_CXX_FLAGS} ${SRC_DIR}/birb_db.cpp -o ${BUILD_DIR}/birb_db ${BUILD_DIR}/birb.o


install:
	cp ./birb ${BUILD_DIR}/{birb_dep_solver,birb_pkg_search,birb_db} /usr/bin/
	mkdir -p /usr/lib/birb
	cp ./birb_funcs /usr/lib/birb/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf
	[ -f /etc/birb-sources.conf ] || cp ./birb-sources.conf /etc/birb-sources.conf

clean:
	rm -rf ${BUILD_DIR}
