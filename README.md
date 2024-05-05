# birb
Package manager used for [BirbOS](https://github.com/birb-linux/BirbOS). Package repository available at https://github.com/birb-linux/BirbOS-packages

**Automated checks**

| Branch | Checks |
| ------ | ------ |
| Main   | [![Linting checks](https://github.com/birb-linux/birb/actions/workflows/linting.yml/badge.svg?branch=main)](https://github.com/birb-linux/birb/actions/workflows/linting.yml) [![Build](https://github.com/birb-linux/birb/actions/workflows/build.yml/badge.svg?branch=main)](https://github.com/birb-linux/birb/actions/workflows/build.yml) |
| Dev    | [![Linting checks](https://github.com/birb-linux/birb/actions/workflows/linting.yml/badge.svg?branch=dev)](https://github.com/birb-linux/birb/actions/workflows/linting.yml) [![Build](https://github.com/birb-linux/birb/actions/workflows/build.yml/badge.svg?branch=dev)](https://github.com/birb-linux/birb/actions/workflows/build.yml) |

> **Warning**
> Please don't use this (outside of [BirbOS](https://github.com/birb-linux/BirbOS) of course). It will cause nuclear war and your computer will catch fire. **You have been warned!**

Birb will be installed as a part of the BirbOS installation process and doesn't have to be manually installed.

For usage instructions, please check the manual either by running `man birb` in BirbOS or manually opening the birb.1 file in the repository with the `man` command.

## Table of contents
- [About](#about)
- [Basic usage](#basic-usage)
    - [Installing packages](#installing-packages)
    - [Uninstalling packages](#uninstalling-packages)
    - [Remove orphan packages](#remove-orphan-packages)
    - [Update birb](#update-birb)
    - [Update packages](#update-packages)
    - [Search for packages](#search-for-packages)
- [Feature checklist](#feature-checklist)
- [Project structure](#project-structure)
- [Packaging guidelines](#packaging-guidelines)
    - [Variables](#variables)
    - [Functions](#functions)
    - [Package naming conventions](#package-naming-conventions)
- [Releases](#releases)
- [Notes on stability](#notes-on-stability)


## About
birb is a source-based package manager purpose made for [BirbOS](https://github.com/birb-linux/BirbOS). It compiles programs from source and installs them into "fakeroots". Those fakeroots are then symlinked to the actual system root with [stow](https://www.gnu.org/software/stow/).

The core of birb is written in bash, but some of the features are split into separate programs written in C++ to make things a bit faster. The package files that birb uses to install things are written in bash compatible syntax.


## Basic usage
Running the command `birb --help` will give you a simple summary of how to do stuff and `man birb` will give even more details

### Installing packages
You can install one or more packages with birb in the following ways as the root user
```sh
birb vim htop
birb --install mpv yt-dlp
```

The `--install` flag doesn't do anything extra in this case, but if you use it, you can specify some extra options like `--test` etc. Adding the `--test` flag would run the tests after compiling the program, if it had any.

> **Note**
> If anything fails when installing multiple packages at once, the entire process will cancel and birb will not attempt installing the rest of the packages

If a package you want to install isn't available, you can try running `birb --sync` to update the repositories in case there are new packages available.


### Uninstalling packages
You can uninstall one or more packages with birb in the following way as the root user
```sh
birb --uninstall emacs feh
```

> **Note**
> The order of the packages to uninstall is important, because birb doesn't do any preprocessing of the list, so it will raise alarms if you try to uninstall a package that is a dependency for another package that you are also trying to uninstall.

If you installed a package that pulled in other packages as dependencies, you don't have to remove those dependencies manually. You can simply install the package that pulled in those dependencies and then [remove orphans](#remove-orphan-packages).

If the package you tried to uninstall is system critical, you'll get the following warning:
```
!! WARNING !!
Removing this package might cause some serious harm to your system
and render it partially unusable.

Please make sure you have a way to restore this package afterwards
without the use of a package manager (and possibly other tools
that aren't available)

Continue? [y/N]:
```
There shouldn't be any situations where you'd be answering *yes* to this menu, but if you do so, you might end up with an unusable system that can only be recovered by restoring backups. These important packages are marked with the "important" flag in their seed.sh files.


### Remove orphan packages
In some cases you might be left with packages that were a dependency for something but that something is not installed anymore. These packages are called "orphans".

You can find and delete orphan packages by running the following command as the root user:
```sh
birb --depclean
```
This will scan your installed packages looking for things that were not installed by the user and aren't a dependency for anything. This scan might take a while if you have lots of packages and it might also run in multiple passes, though everything past the first pass should be near instant due to caching.


### Update birb
The updates for birb are fetched directly from this git repository. You can download and compile the latest version by running the following command as the root user:
```sh
birb --upgrade
```

If for some reason compiling/installing birb with `birb --upgrade` fails, you can alternatively try to update birb with `birb --upgrade --debug`. This will compile the binaries with debug symbols and skip all of the optimization stuff like PGO and LTO etc. After a successful upgrade you can then reinstall birb with `birb --upgrade` to make things way faster.


### Update packages
> **Warning**
> Package updates are still work in progress and things might break here and there. This feature is deeply in the alpha realm

To update packages to their latest versions available in the repositories, first sync the repositories and then run the update command
```sh
birb --sync
birb --update
```


### Search for packages
You can look for packages by name with birb in the following way
```sh
birb -s vim
```
The output will list all packages that match the search query. The output will also include version information, descriptions and a note if the package is installed.


## Feature checklist
- [x] Install packages
- [x] Remove packages
- [ ] Remove packages temporarily
- [x] Recursive dependency resolution
- [x] Orphan handling
- [x] Updates
    - [x] Package manager updates
    - [x] Package updates
        - [ ] Take the dependency tree into account when ordering updates
- [x] Customizable repository sources
- [x] Package search
- [ ] Compile time statistics
- [ ] Verify source code integrity with GPG where possible
- [ ] Recommend rebuilding already installed software in cases of circular dependencies


## Project structure
- birb
    - The main script
    - Frontend for pretty much everything, like installing things, uninstalling things etc.
    - Uses the programs listed below behind the scenes
- birb_db
    - Version management system
    - Keeps track of what is installed at any moment
    - Locates packages in case there are multiple repositories in use
- birb_dep_solver
    - Dependency solver
    - Orphan package finder
- birb_pkg_search
    - Package finder/browser


## Packaging guidelines
Birb packages are bash compatible shell scripts. When a package is installed, the seed.sh for that package gets sourced by birb and the setup, compiling and installation functions are called in a specific order.

Here is the basic minimal skeleton for a birb package
```sh
NAME=""
DESC=""
VERSION=""
SOURCE=""
CHECKSUM=""
DEPS=""
FLAGS=""

_setup()
{
	tar -xf $DISTFILES/$(basename $SOURCE)
	cd ${NAME}-${VERSION}
}

_build()
{
	# make -j${BUILD_JOBS}
}

_install()
{
	# make DESTDIR=$FAKEROOT/$NAME install
}
```
There are more functions available if needed (for running tests, compiling 32bit versions of the package etc.), but the minimal skeleton above is the minimum required at the moment. There's a chance that it will be slimmed down in the future however, since not all packages need the \_build() function.

To make creating packages easier, the [Birb core package repository](https://github.com/birb-linux/BirbOS-packages) contains a [script](https://github.com/birb-linux/BirbOS-packages/blob/master/create_package.sh) that can be used to create package templates to a repository. It also attempts to spot obvious errors in the package that may cause it to malfunction.

### Variables
Each package should contain **at least** a name, description, version, source and a checksum. The rest are optional and not always needed. Due to the way that some of the variables are parsed, **variables can't be more than one line each in length**. The length of that line doesn't matter.

#### NAME
Name of the package, should match the parent directory name of the seed.sh file and conform to the [package naming conventions](#package-naming-conventions)

#### DESC
A description that describes what the programs / libraries contained in the package do. The description length shouldn't go too much over 80 characters in length, but there's no limit to it. The description however will be truncated if it doesn't fit to one terminal line in the package browsing commands.

#### VERSION
A version string that repeats in the package source URL and possibly in file names. It is used to see if there are updates available for the package and it should also make it easier to bump the package version without modifying other parts of the package (with the exception of updating the checksum of course).

If there are different shorter forms of the version string in parts of the package, you can use the following functions to automatically truncate the version string (replace the [version] part with the *VERSION* variable):
- **short_version [version]** Transform a version string X.X.X to X.X taking out the patch number
- **major_version [version]** Transform a version string X.X.X to X leaving only the major version. This will also convert version X.X to X

#### SOURCE
URL that points to the source code tarball or other compressed form of release that is downloaded

#### CHECKSUM
MD5 hash of the file that is downloaded from the [SOURCE](#SOURCE) URL. **This checksum isn't meant to signify any sort of trust or integrity and it is only used for checking if the file downloaded is corrupted or not**. If integrity and trust are needed, that should be achieved with better hashing algorithms (like SHA256, SHA512 etc.) and/or GPG keys.

#### DEPS
Build time and runtime dependencies that are required for using and building the package. This doesn't have to include things like the compiler, linker and/or lower level things that are assumed to be always on the system. Multiple dependencies can be defined in a whitespace separated list on one line. You can leave out any dependencies that get pulled in by one of the other dependencies, since birb solves the dependencies of the dependencies you add recursively.

#### FLAGS
The flags variable controls how birb sees the package.

There are several flags available:
- **32bit** Enables the running of \_build32() and \_install32() functions. You can use them to build and install 32-bit libraries for the package
- **test32** Enables the running of \_test32(). It is called after \_build32() and is meant to be used for running tests
- **font** Marks the package as a font. Causes birb to run fc-cache after finishing the installation to update the font cache
- **important** Marks the package as system critical and important. Important packages won't be found as orphan packages and also when attempting to uninstall them, there's a separate warning
- **proprietary** Marks the package as proprietary, as in it contains binary blobs. This flag should be used for binary releases too even if there's source code available online. Attempting to install a package marked as proprietary will produce a warning message
- **python** Marks the package as a python package. Birb will internally use pip when uninstalling these packages
- **test** Enables the running of \_test(). It is called after \_build() and is meant to be used for running tests
- **wip** Marks the package was "work in progress", i.e. not ready to be used. Attempting to install a package marked as wip produces a warning. Generally packages that might not be fully functional yet and/or fail to compile repeatedly should be marked as wip

#### NOTES
Optional variable that can be used to print out a highlighted message at the end of the package installation. This could include instructions on what to do after installing the package, like adding your user to some specific group for example.

#### CONFLICTS
Optional variable that lists conflicting packages. If a package that is in the conflict list is installed, the installation gets aborted with a message telling about the conflict.

### Functions
When a package is installed, birb will call pre-defined functions in a specific order. Some of the functions require flags before they get run, but this may change in the future to simplify the flag usage.

The following functions should exist in all packages:
- \_setup
- \_build
- \_install

There are also some optional functions that might also require flags to be used:
- \_test
- \_build32
- \_test32
- \_install32
- \_post_install

The package functions will be called in the following order:
1. \_setup
2. \_build
3. \_test
4. \_install
5. \_build32
6. \_test32
7. \_install32
8. \_post_install

#### _setup
The setup function extracts the source tarball/archive and enters the extracted directory. Usually this function doesn't need to be changed at all and the default generated by the create_package script is fine

#### _build
Build the 64bit (or possibly multilib in some cases) libraries and binaries. If the package doesn't involve building anything this function can be left empty with a simple `printf ""` call for example that does nothing. This function may become optional in the future.

#### _test
Run included test suites. Requires the *test* [flag](#FLAGS).

#### _install
Install the package and/or copy compiled binary files to the fakeroot directory. You can also create any needed directories in the root file system, but avoid dropping files outside of the package fakeroot.

The installation step may also include creating any default configuration files or creating user groups etc. Basically anything needed to use the program/library/file after birb has finished installing it.

#### _build32
Build 32bit binaries/libraries. Requires the *32bit* [flag](#FLAGS).

#### _test32
Run included test suites against the 32bit binaries/libraries. Requires the *test32* [flag](#FLAGS).

#### _install32
Install the compiled 32bit binaries/libraries. This usually requires a bit more manual installation so that the libraries go to their correct places like */usr/lib32* instead of */usr/lib* for example. Requires the *32bit* [flag](#FLAGS).

#### _post_install
An optional function that doesn't require any flags and can be used to run commands after birb has finished installing the package. This may be useful in cases where you need to execute something that isn't available on the system before the installation has been fully completed.

### Package naming conventions
There aren't any hard coded "requirements" on the package names when it comes to the meaning, but there are some limitations on the formatting.

The package name may only include the following things:
- Lowercase letters a-z
- Numbers
- Underscores
- Lines
- Plus signs

There may be more characters allowed in the future and the current list is there mainly to make finding and installing packages easier. Due to the nature of the programming languages used, avoiding special characters also gets rid of a lot of bugs related to shell expansions and file globbing.

If you can echo your package name through this grep command: `grep -w '^[0-9a-z_+-]*$'`, birb should allow it.


## Releases
Updates to birb come in a rolling release manner. However if you are in a need of some stable point, you can use release tarballs from [here](https://github.com/birb-linux/birb/releases). However if you are using a release tarball, make sure that you are using a matching birb-core repository version from [here](https://github.com/birb-linux/BirbOS-packages/releases). As a general rule of thumb, if the major versions match, the repository should be fully compatible with the package manager.

## Notes on stability
As a small safeguard, birb checks for updates every time you sync the repositories. If there's an update available, birb will notify about that and recommend upgrading. Upgrading at this point isn't *mandatory*, but highly recommended.

Keeping birb up-to-date alongside the repositories is important, because there might be some sweeping change to how packages work at this stage of development. This shouldn't be that big of a deal anymore, since most of the basic features are implemented, but as an example, at one point a lot of important packages were not marked as important. Because of this, the orphan removal feature would have removed lots of system critical packages if your repositories were out-of-date since those important packages weren't marked as a dependency for anything.

If you want to be sure that your system stays fully functional through the quirks of running a package manager that is still at alpha stage (even though its version is marked as 1.0, I know, don't ask why), **take backups** and take them often. You might need them some day. If you don't want to take a full system backup, backup at least the `/var/db/fakeroot` directory. If you can restore that, you can most likely bring your installation back to life.
