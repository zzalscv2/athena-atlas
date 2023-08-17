
#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
set -ex
[ $# -eq 1 ] || { echo "$0: The only argument must be path to '${ATLAS_CTEST_PACKAGE}' package directory" >&2; exit 2; }

inputAODFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_DATA[0])")
outputAODFile=${ATLAS_CTEST_TESTNAME}.pool.root

AODEventPicking.py --eventList ${1}/share/${ATLAS_CTEST_TESTNAME}.ref --inputAODFile "$inputAODFile" --outputAODFile "$outputAODFile"

AodEventInfo.py --inputAODFiles "$outputAODFile"
