#!/usr/bin/env bash

set -e

TESTS_PATH=$(dirname "$(realpath "$0")")

# list of apps required by tests that we want to build here
APPS=("ethereum" "ethereum_classic")

# list of SDKS
NANO_SDKS=("$NANOS_SDK" "$NANOX_SDK" "$NANOSP_SDK")
# list of target elf file name suffix
FILE_SUFFIXES=("nanos" "nanox" "nanosp");

# move to the tests directory
cd "$TESTS_PATH" || exit 1

# Do it only now since before the cd command, we might not have been inside the repository
GIT_REPO_ROOT=$(git rev-parse --show-toplevel)

# create elfs directory if it doesn't exist
mkdir -p elfs

# move to repo's root to build apps
cd "$GIT_REPO_ROOT" || exit 1

for ((sdk_idx=0; sdk_idx < "${#NANO_SDKS[@]}"; sdk_idx++))
do
    nano_sdk="${NANO_SDKS[$sdk_idx]}"
    elf_suffix="${FILE_SUFFIXES[$sdk_idx]}"
    echo "** Building app elf for $(basename "$nano_sdk")..."
    make clean BOLOS_SDK="$nano_sdk"
    BOLOS_SDK="$nano_sdk" make
    cp bin/app.elf "$TESTS_PATH/elfs/${elf_suffix}.elf"

done

echo "done"