# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

# -------------------------------------------------------------
# L1 Getter of the result
# -------------------------------------------------------------
from AthenaCommon.GlobalFlags import jobproperties

from AthenaConfiguration.AllConfigFlags import ConfigFlags
from AthenaConfiguration.ComponentAccumulator import CAtoGlobalWrapper
from AthenaConfiguration.Enums import Format
from TrigEDMConfig.Utils import edmListToDict

from RecExConfig.RecFlags import rec

from RecExConfig.Configured import Configured
from RecExConfig.ObjKeyStore import objKeyStore

class Lvl1ResultBuilderGetter(Configured):

    def configure(self):

        if ConfigFlags.Input.Format is Format.BS:
            from TrigT1CaloByteStream.LVL1CaloRun2ByteStreamConfig import LVL1CaloRun2ReadBSCfg
            CAtoGlobalWrapper(LVL1CaloRun2ReadBSCfg, ConfigFlags)

        if rec.doTrigger():
            if (rec.doESD() or rec.doAOD()) and (not(rec.readAOD() or \
                                                         rec.readESD())):
                if jobproperties.Global.InputFormat() == 'bytestream':
                    # Decode L1 data from ByteStream
                    from TrigT1ResultByteStream.TrigT1ResultByteStreamConfig import L1TriggerByteStreamDecoderCfg
                    l1decodingAcc, l1EDMDict = CAtoGlobalWrapper(L1TriggerByteStreamDecoderCfg, ConfigFlags, returnEDM=True)

                    # This is *only* for the old job-options system compatibility (RecExCommon)
                    # because as of 15/08/2022 adding collections to output ESD/AOD file from ComponentAccumulator-based
                    # JO fragments imported in RecExCommon through CAtoGlobalWrapper is intentionally disabled.
                    # Normally L1TriggerByteStreamDecoderCfg above takes care of this, but in RecExCommon this doesn't work.
                    # See discussions in https://gitlab.cern.ch/atlas/athena/-/merge_requests/55891#note_5912844
                    objKeyStore.addManyTypesStreamESD(edmListToDict(l1EDMDict))
                    objKeyStore.addManyTypesStreamAOD(edmListToDict(l1EDMDict))

                from AnalysisTriggerAlgs.AnalysisTriggerAlgsCAConfig import RoIBResultToxAODCfg
                CAtoGlobalWrapper(RoIBResultToxAODCfg, ConfigFlags)

        from TrigEDMConfig.TriggerEDM import getLvl1ESDList, getLvl1AODList
        objKeyStore.addManyTypesStreamESD(getLvl1ESDList())
        objKeyStore.addManyTypesStreamAOD(getLvl1AODList())

        return True
