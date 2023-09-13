CXX=g++

override CXXFLAGS+=-std=c++20 -static -I./include -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wundef -Werror -Wno-unused
FRONTEND_CXXFLAGS=-flto=auto -DDOCTEST_CONFIG_DISABLE

SRC_DIR=./src
LDFLAGS=-L$(BUILD_DIR) -l:libbirb.a

.PHONY: all
all: birb_dep_solver birb_pkg_search birb_db

#### Backend ####
database.o: $(SRC_DIR)/backend/database.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

dependencies.o: $(SRC_DIR)/backend/dependencies.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

utils.o: $(SRC_DIR)/backend/utils.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@


libbirb.a: database.o dependencies.o utils.o
	ar -rcs $@ $^

#### Testing ####
birb_test.o: $(SRC_DIR)/birb_test.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

birb_test: birb_test.o libbirb.a
	$(CXX) $(CXXFLAGS) birb_test.o -o $@ libbirb.a

#### Frontend ####

# Dependency solver
birb_dep_solver.o: $(SRC_DIR)/frontend/dep_solver.cpp
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) -c $^ -o $@

birb_dep_solver: libbirb.a birb_dep_solver.o
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) birb_dep_solver.o -o $@ libbirb.a

# Package search tool
birb_pkg_search.o: $(SRC_DIR)/frontend/pkg_search.cpp
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) -c $^ -o $@

birb_pkg_search: libbirb.a birb_pkg_search.o
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) birb_pkg_search.o -o $@ libbirb.a

# Database tool
birb_db.o: $(SRC_DIR)/frontend/birb_db.cpp
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) -c $^ -o $@

birb_db: libbirb.a birb_db.o
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) birb_db.o -o $@ libbirb.a

check: check_sh check_cpp

check_cpp:
	cppcheck --suppress="syntaxError" ${SRC_DIR}

check_sh:
	shellcheck -x -s bash ./birb
	shellcheck -x -s bash ./birb ./birb_funcs ./pgo_run.sh

valgrind:
	valgrind --error-exitcode=10 ./birb_dep_solver birb-utils
	valgrind --error-exitcode=11 ./birb_dep_solver -r ncurses
	valgrind --error-exitcode=12 ./birb_dep_solver -o
	valgrind --error-exitcode=20 ./birb_db --is-installed ncurses
	valgrind --error-exitcode=21 ./birb_db --list
	valgrind --error-exitcode=22 ./birb_db --version ncurses
	valgrind --error-exitcode=23 ./birb_db --help
	valgrind --error-exitcode=30 ./birb_pkg_search ncurses
	valgrind --error-exitcode=31 ./birb_pkg_search vim firefox

install:
	cp ./birb {birb_dep_solver,birb_pkg_search,birb_db} /usr/bin/
	mkdir -p /usr/lib/birb
	cp ./birb_funcs ./libbirb.a /usr/lib/birb/
	mkdir -p /usr/include/birb
	cp ./include/*.hpp /usr/include/birb/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf
	[ -f /etc/birb-sources.conf ] || cp ./birb-sources.conf /etc/birb-sources.conf

clean:
	rm -rf *.o *.a *.gcda
	rm -f birb_db birb_dep_solver birb_pkg_search birb_test

.PHONY: check_cpp check_sh valgrind clean install
