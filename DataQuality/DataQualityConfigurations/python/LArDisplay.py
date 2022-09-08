# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

from DataQualityUtils.DQWebDisplayConfig import DQWebDisplayConfig
import os
from ._resolve_data_path import resolve_data_path


dqconfig                = DQWebDisplayConfig()
dqconfig.config         = "larcomm"

# Use this setup to test your new han configuration before committing it. Set "hcfg_dir" manually if the default doesn't work for you.
# You can change these settings in your local working copy of this file, but please do not commit the change to SVN.
hcfg_dir = resolve_data_path('DataQualityConfigurations')
if hcfg_dir:
    print ("Found DataQualityConfigurations data directory %s, using it" % hcfg_dir)
else:
    hcfg_dir = os.getcwd()
    print ("DataQualityConfigurations data directory not found, using $PWD instead.")
print ("Looking for collisions_*.hcfg files in %s" % (hcfg_dir))

dqconfig.hcfg           = "%s/collisions_run.hcfg"       % (hcfg_dir)
dqconfig.hcfg_min10     = "%s/collisions_minutes10.hcfg" % (hcfg_dir)
dqconfig.hcfg_min30     = "%s/collisions_minutes30.hcfg" % (hcfg_dir)

dqconfig.histogramCache = "/afs/cern.ch/user/a/atlasdqm/dqmdisk1/histogram_web_display_cache"
dqconfig.hanResultsDir  = "/eos/atlas/atlascerngroupdisk/data-dqm/han_results/larcomm"
dqconfig.htmlDir        = "/eos/atlas/atlascerngroupdisk/data-dqm/han_results/larcomm"
dqconfig.htmlWeb        = "http://cern.ch/atlasdqm/larcomm"
dqconfig.runlist        = "runlist_larcomm.xml"
dqconfig.indexFile      = "results_larcomm.html"
dqconfig.lockFile       = "DQWebDisplay_larcomm.lock"
dqconfig.doHandi        = False
