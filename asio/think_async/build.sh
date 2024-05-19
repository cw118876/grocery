#!/bin/bash

set -e

build_type=Release
source_dir=$(pwd)
export_pkg=0

function do_build() {
    local build_dir=${source_dir}/build/${build_type}
    local install_dir=${source_dir}/install/${build_type}
    if [ "${build_type}" = "Release" ]; then 
       conan build . -of ${build_dir} -s build_type=${build_type} \
         --build missing
    else 
       conan build . -of ${build_dir} -s build_type=${build_type} \
          --build "*"
    fi
    echo -e "conan build finished"
    if [ ${export_pkg} = 1 ]; then
       conan export-pkg . -of ${build_dir} --user sii --channel test -pr x64 
    fi  
    echo -e "build success\n"

}


while getopts ":b:ce" opt; do
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
        e)
          echo "export package"
          export_pkg=1
          ;;
    esac
done

do_build

exit 0
