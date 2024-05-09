#!/bin/bash

set -e

build_type=Release
source_dir=$(pwd)
export_package=0

function do_build() {
    local build_dir=${source_dir}/build
    local install_dir=${source_dir}/install

    if [ "${build_type}" = "Release" ]; then 
       conan install . -if ${build_dir} -pr x64 -s build_type=${build_type} --build missing
    else 
       conan install . -if ${build_dir} -pr x64 -s build_type=${build_type} --build
    fi
    echo -e "conan install finished"
    conan build . -bf ${build_dir} -pf ${install_dir}
    conan package . -bf ${build_dir} -pf ${install_dir}

    echo -e "build success\n"

}


while getopts ":b:c" opt; do
    case ${opt} in
        b)
            case $OPTARG in
                Release|release)
                    echo -e "using Release build type"
                    build_type=Release
                    ;;
                Debug|debug)
                    echo -e "using Debug build type"
                    build_type=Debug
                    ;;
                *)
                    echo "Unsupported build type ${OPTARG}"
                    exit 1
                    ;;
            esac
            ;;
        c)
        rm -rf build install
        echo -e "cleanup success"
        exit 0
        ;;
    esac
done

do_build

exit 0
