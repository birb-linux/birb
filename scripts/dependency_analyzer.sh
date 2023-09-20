#!/bin/bash

# Check if the repository given as an argument has any packages
# that use a certain command but don't have something as a dependency
#
# This can be used to retroactively add in missing dependencies that
# were unexpected or simply not noticed at the time of packaging

if [ -z "$1" ] || [ -z "$2" ] || [ -z "$3" ]
then
	echo "Missing args!"
	echo "Usage: $0 [path to repo] [command] [dependency]"
	exit 1
fi

# Read in the args
REPOSITORY="$1"
COMMAND="$2"
DEPENDENCY="$3"

# Find seeds that contain the command
SEEDS_WITH_CMD="$(grep -rl "$COMMAND" "$REPOSITORY" | grep "seed.sh")"

# Find seeds that didn't have the required dependency
# Also remove the dependency package itself from the output
grep -L "^DEPS=.*$DEPENDENCY" $SEEDS_WITH_CMD | sed "/$DEPENDENCY\/seed\.sh/d"
