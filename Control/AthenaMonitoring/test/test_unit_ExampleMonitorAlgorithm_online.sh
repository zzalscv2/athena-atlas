#!/bin/bash

echo $PWD
# We need to set the source to 'INFILE' because usually we do not have an input file
# when running in Online mode (see ATR-27227).
python -m AthenaMonitoring.ExampleMonitorAlgorithm Common.isOnline=True Trigger.triggerConfig='INFILE' Output.HISTFileName=ExampleMonitorOutputOnline.root
# Grep to avoid RooFit lines
hist_file_dump.py -r name --no_onfile --hash ExampleMonitorOutputOnline.root | grep '^T' | tee hist-content-online
get_files -symlink test_unit_ExampleMonitorAlgorithm_online.ref
diff hist-content-online test_unit_ExampleMonitorAlgorithm_online.ref
exit $?
