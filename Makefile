CXX=g++
CXX_FLAGS=-O2 -I./include

SRC_DIR=./src
BUILD_DIR=./build

all:
	mkdir -p ${BUILD_DIR}
	${CXX} ${CXX_FLAGS} -c ${SRC_DIR}/birb.cpp -o ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver ${BUILD_DIR}/birb.o
	${CXX} ${CXX_FLAGS} ${SRC_DIR}/pkg_search.cpp -o ${BUILD_DIR}/birb_pkg_search ${BUILD_DIR}/birb.o

install:
	cp ./birb ${BUILD_DIR}/{birb_dep_solver,birb_pkg_search} /usr/bin/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf

clean:
	rm -rf ${BUILD_DIR}
