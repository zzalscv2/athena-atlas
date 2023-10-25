#!/bin/sh
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
# Thin wrapper to execute configurations with the CA. 
# Might be called from athena.py
#

#Separate the top-level python script (ending with .py) from
#arguments passed to that script 
scriptargs=()
for a in "$@"
do
    case "$a" in
	--config-only*)  export PICKLECAFILE=${a#*=};;
	*.py) topscriptfile=$a;;
	*.pkl) picklefile=$a;;
	*) scriptargs+=("$a");;
    esac
done

#Check if we got a pickle-file or top-level script
if [ -z "${picklefile}" ] && [ -z "${topscriptfile}" ] ;then
	echo "ERROR: No top-level python script or pickle file given"
	exit 1
fi

#If script, try to find it locally or in PYTHONPATH
if [ -n "${topscriptfile}" ]; then
    if [ -f "${topscriptfile}" ]; then
        topscript="${topscriptfile}"
    else
        for pp in . ${PYTHONPATH//:/ }; do
	            if [ -f "${pp}/${topscriptfile}" ]; then
	                topscript="${pp}/${topscriptfile}"
	                break
	            fi
        done
    fi
    if [ -z "$topscript" ]; then
        echo "Could not find python script $topscriptfile"
        exit 1
    fi
fi

#Finally: Execute it!
if [ -z "${picklefile}" ]; then
    python $topscript "${scriptargs[@]}"
else
    echo "Starting from pickle file $picklefile"
    CARunner.py $picklefile "${scriptargs[@]}"
fi
status=$?

#Handle exit code
if [ ! $status -eq 0 ]; then
    echo "leaving with code $status: \"failure\""
    exit $status
else
    echo "leaving with code 0: \"successful run\""
fi
