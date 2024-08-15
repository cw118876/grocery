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

lint_check=0
tidy_check=0
format_check=0

function pretty_usage() {
    echo "Usage: $0 [OPTIONS]..."
    echo
    echo "Options:"
    echo "  -l         enable cpplint check."
    echo "  -t         enable clang-tidy check."
    echo "  -f         enable clang-format check."
    echo "  -a         enalble all code check."
    echo
    echo "Examples:"
    echo "  $0 -l -t"
    exit 1
}

function print_for_test_start() {
    echo -e "${BLUE}------------------------------------------${NC}"
    echo "start $1 test."
    echo -e "${BLUE}------------------------------------------${NC}"
}

function print_for_test_end() {
    echo -e "${GREEN}------------------------------------------${NC}"
    echo "$1 passed."
    echo -e "${GREEN}------------------------------------------${NC}"
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

# Default values
cpplint_executable="cpplint"
dirs_need_check=("include" "src" "test" "example")
function do_lint_check() {
    check_command_existence ${cpplint_executable}
    print_for_test_start ${cpplint_executable}
    for dir in "${dirs_need_check[@]}"; do
        ${cpplint_executable} --recursive --quiet ${dir}
        if [ "$?" -ne 0 ]; then
            exit_when_failure "${cpplint_executable} for ${dir} failed"
        fi
    done
    print_for_test_end ${cpplint_executable}
}

clang_tidy_executable="run-clang-tidy"
compiled_command_database="compile_commands.json"
function do_tidy_check() {
    check_command_existence ${clang_tidy_executable}
    print_for_test_start ${clang_tidy_executable}
    ## check compiled command database existence or not
    if [ ! -f "${compiled_command_database}" ]; then
        ./build.sh
        if [ "$?" -ne 0 ]; then
            exit_when_failure "build compile command database failed"
        fi
    fi
    ${clang_tidy_executable} -quiet -p . -j $(($(nproc) / 2 + 1)) ${dirs_need_check[@]}
    if [ "$?" -ne 0 ]; then
        exit_when_failure "${clang_tidy_executable} failed, return value $?"
    fi
    print_for_test_end ${clang_tidy_executable}
}

clang_format_executable="clang-format"
function do_format_check() {
    check_command_existence ${clang_format_executable}
    print_for_test_start ${clang_format_executable}
    find . -regex '.*\.\(cpp\|hpp\|cc\|cxx\|h\|c\)' -not -path './build/*' -not -path './install/*' \
        -exec ${clang_format_executable} --Werror --dry-run --verbose {} \;
    if [ "$?" -ne 0 ]; then
        exit_when_failure "${clang_format_executable} failed, return value $?"
    fi
    print_for_test_end ${clang_format_executable}
}

function do_check() {
    if [ ${lint_check} = 1 ]; then
        do_lint_check
    fi
    if [ ${tidy_check} = 1 ]; then
        do_tidy_check
    fi
    if [ ${format_check} = 1 ]; then
        do_format_check
    fi
    print_for_test_end "all test passed"

}

while getopts ":lhtfa" opt; do
    case ${opt} in
    l)
        lint_check=1
        ;;
    t)
        tidy_check=1
        ;;
    h)
        pretty_usage
        ;;
    f)
        format_check=1
        ;;
    a)
        format_check=1
        tidy_check=1
        lint_check=1
        ;;
    *)
        echo -e "${RED}------------------------------------------${NC}"
        echo -e "${RED}unsupported operation${NC}"
        echo -e "${RED}------------------------------------------${NC}"
        pretty_usage
        ;;
    esac
done

do_check
exit 0
