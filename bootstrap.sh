#!/bin/bash

# Setup the environment for birb
#
# This might be useful if you birb installation is broken or
# if you want to install birb to your distro for whatever reason

CACHE_DIR=/var/cache
LIB_DIR=/var/lib/birb
DB_DIR=/var/db
REPO_DIR=${$DB_DIR}/pkg
BIRB_SRC_DIR=${CACHE_DIR}/distfiles/birb

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
mkdir -p ${LFS}/${REPO_DIR}
mkdir -p ${LFS}/${BIRB_SRC_DIR}

# Create the nest file
touch ${LFS}/${LIB_DIR}/nest

# Update the repository
cd ${LFS}/${REPO_DIR}
git init
git remote add origin https://github.com/toasterbirb/birbos-packages
git fetch
git pull origin master
