#!/bin/bash

set -e

# Define colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Define text effects
BOLD='\033[1m'
UNDERLINE='\033[4m'

build_type=Release
source_dir=$(pwd)
export_pkg=0
build_custom=0
build_custom_flag=""
test_package=0

function pretty_usage() {
    echo "Usage: $0 [OPTIONS]..."
    echo
    echo "Options:"
    echo "  -b         build type, eg. release, debug."
    echo "  -B         customize bulding package patter, eg. never, *"
    echo "  -t         enable test."
    echo "  -c         clean up all stuff"
    echo
    echo "Examples:"
    echo "  $0 -l -t"
    exit 1
}

function exit_when_failure() {
    echo -e "${RED}------------------------------------------${NC}"
    echo "$1, please check."
    echo -e "${RED}------------------------------------------${NC}"
    exit 1
}

function check_command_existence() {
    if ! [ -x "$(command -v $1)" ]; then
        exit_when_failure "$1 isn't installed"
    fi
}

function execute_executables_in_dir() {
    local dir="$1"
    # Check if the directory exists
    if [ ! -d "$dir" ]; then
        exit_when_failure "Directory $dir does not exist."
    fi

    # Loop through all files in the directory
    for file in "$dir"/*; do
        # Check if the file is executable
        if [ ! -d "$file" ] && [ -x "$file" ]; then
            echo "Executing ${file}..."
            bash -c ${file}
            if [ "$?" -ne 0 ]; then
                exit_when_failure "run ${file} failed, return value: $?"
            fi
        fi
    done
}

function run_test_suit() {
    check_command_existence "gcovr"
    export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}/build/Debug/lib
    execute_executables_in_dir ${PWD}/build/Debug/test
    mkdir -p build/coverage/report
    gcovr --object-directory build/Debug

    # coverage html
    gcovr --html --html-details -o build/coverage/report/index.html \
        --object-directory build/Debug

}

function do_build() {
    local build_dir=${source_dir}/build/${build_type}
    local install_dir=${source_dir}/install/${build_type}
    local build_pattern=missing
    if [ ${build_custom} = 1 ]; then
        build_pattern=${build_custom_flag}
    fi
    conan build . -of ${build_dir} -s build_type=${build_type} -pr:h=x64 -pr:b=x64 \
        --build=${build_pattern}
    if [ ${test_package} = 1 ] && [ "${build_type}" = "Debug" ]; then
        run_test_suit
    fi
    echo -e "${GREEN}------------------------------------------${NC}"
    echo -e "${GREEN}build success${NC}"
    echo -e "${GREEN}------------------------------------------${NC}"
    if [ ${export_pkg} = 1 ]; then
        conan export-pkg . -of ${build_dir} --user sii --channel dev -pr:h=x64 -pr:b=x64
        echo -e "${GREEN}------------------------------------------${NC}"
        echo -e "${GREEN}conan export finished${NC}"
        echo -e "${GREEN}------------------------------------------${NC}"
        return 0
    fi

}

while getopts ":b:B:cte" opt; do
    case ${opt} in
    b)
        case $OPTARG in
        Release | release)
            echo -e "using Release build type"
            build_type=Release
            ;;
        Debug | debug)
            echo -e "using Debug build type"
            build_type=Debug
            ;;
        *)
            exit_when_failure "Unsupported build type ${OPTARG}"
            ;;
        esac
        ;;
    B)
        build_custom=1
        build_custom_flag=${OPTARG}
        ;;
    c)
        rm -rf build install
        rm -rf compile_commands.json
        echo -e "${YELLOW}------------------------------------------${NC}"
        echo -e "${YELLOW}cleanup success${NC}"
        echo -e "${YELLOW}------------------------------------------${NC}"
        exit 0
        ;;
    t)
        test_package=1
        ;;
    e)
        echo "export package"
        export_pkg=1
        ;;
    *)
        pretty_usage
        ;;
    esac
    
done

do_build

exit 0
