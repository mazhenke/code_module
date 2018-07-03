#!/bin/bash

formator='clang-format-3.8'

USAGE="USAGE:
`basename $0` <file|dir> \n
\n
"
check_valid_source_code()
{
    file=$1
    if [ "${file##*.}" = "c" ] \
    || [ "${file##*.}" = "C" ] \
    || [ "${file##*.}" = "cc" ] \
    || [ "${file##*.}" = "cpp" ] \
    || [ "${file##*.}" = "CPP" ] \
    || [ "${file##*.}" = "c++" ] \
    || [ "${file##*.}" = "cp" ] \
    || [ "${file##*.}" = "cxx" ] \
    || [ "${file##*.}" = "c++" ] \
    || [ "${file##*.}" = "h" ] \
    || [ "${file##*.}" = "H" ] \
    || [ "${file##*.}" = "hh" ] \
    || [ "${file##*.}" = "hpp" ];
    then
      return 1
    else
      return 0
   fi
}

format_file()
{
    check_valid_source_code $1
    if [ $? -eq 0 ]; then
        ##echo "Skip Invalid file: $1"
        return -1
    fi

    echo "Formating file $1"
    $formator -style=file -i $1
    return 0
}

check_parameter()
{
    if [ $# -lt 1 ]; then
        echo -e $USAGE
        exit 1
    fi
}

parse_main()
{
    if [ -d $1 ]; then
        # for dir:
        all_files=`find $1`
        for file in $all_files
        do
            format_file $file
        done
    elif [ -f $1 ]; then
        format_file $1
    fi
}

check_parameter $*
parse_main $*
