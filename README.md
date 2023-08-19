# birb
Package manager used for [BirbOS](https://github.com/birb-linux/BirbOS). Package repository available at https://github.com/birb-linux/BirbOS-packages

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
- [Releases](#releases)


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
> The order of the packages to uninstall is important, because birb doesn't do any pre-processing of the list, so it will raise alarms if you try to uninstall a package that is a dependency for another package that you are also trying to uninstall.

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

To update packages to their latest versions available in the repositories, first sync the repos and then run the update command
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

## Releases
Updates to birb come in a rolling release manner. However if you are in a need of some stable point, you can use release tarballs from [here](https://github.com/birb-linux/birb/releases). However if you are using a release tarball, make sure that you are using a matching birb-core repository version from [here](https://github.com/birb-linux/BirbOS-packages/releases). As a general rule of thumb, if the major versions match, the repository should be fully compatible with the package manager.

Even though it might be a bit unintuitive, the master branch of the [BirbOS-packages](https://github.com/birb-linux/BirbOS-packages) or the `birb --source stable` isn't actually as stable as the development branch. This is due to the way birb gets updated. If you want to use the master branch of the birb-core repository, you should avoid updating birb. However when the dev branch gets merged to master (the "stable" branch), you might end up with a situation where you have to update birb or you risk incompatibilities. This issue will most likely be fixed at some point by having some sort of way to let the package manager know when it needs to be updated to be fully compatible with the repository.
