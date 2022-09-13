#!/usr/bin/env bash
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# Run-3 MC but with AODFULL so it can be used as input to the HLT DQ monitoring.
# Once ATLASDQ-879 is fixed, this script can be deleted and the test defintion
# (without the --preExec) moved to Athena.cmake.

RunWorkflowTests_Run3.py --CI -r -w MCReco -e '--runNumber 601229 --inputHITSFile "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/50events.HITS.pool.root" --conditionsTag="OFLCOND-MC21-SDR-RUN3-07" --geometryVersion="ATLAS-R3S-2021-03-00-00" --maxEvents 25 --preExec="all:from AthenaConfiguration.AllConfigFlags import ConfigFlags; ConfigFlags.Trigger.AODEDMSet=\"AODFULL\""' --no-output-checks