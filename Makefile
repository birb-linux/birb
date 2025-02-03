CXX=g++

override CXXFLAGS+=-std=c++20 -g -static -I./include -I./vendor/clipp/include -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wundef -Werror -Wno-unused
FRONTEND_CXXFLAGS=-DDOCTEST_CONFIG_DISABLE

SRC_DIR=./src
LDFLAGS=-L$(BUILD_DIR) -l:libbirb.a

.PHONY: all
all: birb birb_db

#### Backend ####
%.o: $(SRC_DIR)/libbirb/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

libbirb.a: database.o dependencies.o utils.o install.o package_info.o cli.o symlink.o download.o uninstall.o package_search.o distclean.o depclean.o
	gcc-ar -rcs $@ $^

# Testing
birb_test: libbirb.a
	$(CXX) $(CXXFLAGS) $(SRC_DIR)/birb_test.cpp -o $@ $^

# Package manager
birb: $(SRC_DIR)/birb.cpp libbirb.a
	$(CXX) $(CXXFLAGS) $(FRONTEND_CXXFLAGS) -o $@ $^

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

install-lib:
	mkdir -p $(DESTDIR)/usr/lib/birb
	cp ./birb_funcs $(DESTDIR)/usr/lib/birb/

install:
	cp ./birb $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/lib/birb
	cp ./birb_funcs ./libbirb.a $(DESTDIR)/usr/lib/birb/
	mkdir -p $(DESTDIR)/usr/include/birb
	cp ./include/*.hpp $(DESTDIR)/usr/include/birb/
	cp ./birb.1 $(DESTDIR)/usr/share/man/man1/birb.1
	[ -f $(DESTDIR)/etc/birb.conf ] || cp ./birb.conf $(DESTDIR)/etc/birb.conf
	[ -f $(DESTDIR)/etc/birb-sources.conf ] || cp ./birb-sources.conf $(DESTDIR)/etc/birb-sources.conf

clean:
	rm -rf *.o *.a *.gcda
	rm -f birb_db birb_dep_solver birb_pkg_search birb_test

.PHONY: check_cpp check_sh valgrind clean install install-lib
