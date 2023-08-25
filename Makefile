CXX=g++

override CXXFLAGS+=-std=c++20 -static -I./include -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wundef -Werror -Wno-unused

SRC_DIR=./src
BUILD_DIR=./build
LDFLAGS=-L$(BUILD_DIR) -l:libbirb.a

all: dep_solver pkg_search birb_db

build_dir:
	mkdir -p $(BUILD_DIR)

#### Backend ####
database: build_dir
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/backend/$@.cpp -o $(BUILD_DIR)/$@.o

dependencies: build_dir
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/backend/$@.cpp -o $(BUILD_DIR)/$@.o

utils: build_dir
	$(CXX) $(CXXFLAGS) -c $(SRC_DIR)/backend/$@.cpp -o $(BUILD_DIR)/$@.o


birb_lib: database dependencies utils
	ar -rcs $(BUILD_DIR)/libbirb.a $(BUILD_DIR)/{database,dependencies,utils}.o


#### Frontend ####
dep_solver: birb_lib
	$(CXX) $(CXXFLAGS) -flto=auto $(SRC_DIR)/frontend/dep_solver.cpp -o $(BUILD_DIR)/birb_dep_solver $(BUILD_DIR)/libbirb.a

pkg_search: birb_lib
	$(CXX) $(CXXFLAGS) -flto=auto $(SRC_DIR)/frontend/pkg_search.cpp -o $(BUILD_DIR)/birb_pkg_search $(BUILD_DIR)/libbirb.a

birb_db: birb_lib
	$(CXX) $(CXXFLAGS) -flto=auto $(SRC_DIR)/frontend/birb_db.cpp -o $(BUILD_DIR)/birb_db $(BUILD_DIR)/libbirb.a



install:
	cp ./birb $(BUILD_DIR)/(birb_dep_solver,birb_pkg_search,birb_db) /usr/bin/
	mkdir -p /usr/lib/birb
	cp ./birb_funcs /usr/lib/birb/
	cp ./birb.1 /usr/share/man/man1/birb.1
	[ -f /etc/birb.conf ] || cp ./birb.conf /etc/birb.conf
	[ -f /etc/birb-sources.conf ] || cp ./birb-sources.conf /etc/birb-sources.conf

clean:
	rm -rf $(BUILD_DIR)
