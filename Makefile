CXX=g++
CXX_FLAGS=-O2

SRC_DIR=./src
BUILD_DIR=./build

all:
	mkdir -p ${BUILD_DIR}
	${CXX} ${CXX_FLAGS} ${SRC_DIR}/dep_solver.cpp -o ${BUILD_DIR}/birb_dep_solver

install:
	cp ./birb ${BUILD_DIR}/birb_dep_solver /usr/bin/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf

clean:
	rm -rf ${BUILD_DIR}
