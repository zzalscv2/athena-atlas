#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging
log = logging.getLogger('decodeBS_TLA_AOD.py')



def decodingCfg(flags, ds_id='PhysicsTLA'):
    """ Configure deserialization of RAW byte stream """
    acc = ComponentAccumulator()

    from AthenaCommon.CFElements import seqAND
    from TrigEDMConfig import DataScoutingInfo

    acc.addSequence(seqAND("Decoding"))

    acc.addEventAlgo(CompFactory.HLTResultMTByteStreamDecoderAlg(), "Decoding")

    from TrigDecisionMaker.TrigDecisionMakerConfig import Run3DecisionMakerCfg
    
    acc.merge(Run3DecisionMakerCfg(flags))


    deserialiser = CompFactory.TriggerEDMDeserialiserAlg("TLATrigDeserialiser")
    deserialiser.ModuleID = DataScoutingInfo.DataScoutingIdentifiers[ds_id]
    acc.addEventAlgo(deserialiser, "Decoding")

    return acc

# Configure AOD output
def outputCfg(flags):
    """ Configure AOD output """
    acc = ComponentAccumulator()

    from TrigEDMConfig.TriggerEDM import getTriggerEDMList
    edmList = getTriggerEDMList(flags.Trigger.ESDEDMSet, flags.Trigger.EDMVersion)

    ItemList = []
    for edmType, edmKeys in edmList.items():
        for key in edmKeys:
            ItemList.append(edmType+'#'+key)
    ItemList += [ "xAOD::EventInfo#EventInfo", "xAOD::EventAuxInfo#EventInfoAux." ]
    ItemList += [ 'xAOD::TrigCompositeContainer#*' ]
    ItemList += [ 'xAOD::TrigCompositeAuxContainer#*' ]
    ItemList += [ 'xAOD::TrigDecision#*' ]
    ItemList += [ 'xAOD::TrigDecisionAuxInfo#*']

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    acc.merge(OutputStreamCfg(flags, "AOD", ItemList=ItemList))
    

    return acc

def GetCustomAthArgs():
    from argparse import ArgumentParser
    parser = ArgumentParser(description='Parser for IDPVM configuration')
    parser.add_argument("--filesInput", action='append')
    return parser.parse_args()


def setupDecodeCfgCA(flags):
  
    
    # Setup main services
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Setup input

    # ByteStream input
    from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
    cfg.merge(ByteStreamReadCfg(flags))

    # Decoding sequence
    cfg.merge(decodingCfg(flags))

    # Trigger metadata
    from TriggerJobOpts.TriggerRecoConfig import TriggerMetadataWriterCfg
    metadataAcc, _ = TriggerMetadataWriterCfg(flags)

    cfg.merge( metadataAcc )

    # Output
    cfg.merge(outputCfg(flags))
    
    # Dump the pickle file
    with open("DecodeBS_TLA_AOD.pkl", "wb") as f:
        cfg.store(f)

    return cfg


if __name__ == "__main__":
   
    from AthenaConfiguration.AllConfigFlags import initConfigFlags

    args = GetCustomAthArgs()
    flags = initConfigFlags()

    # Setup custom input file
    flags.Input.Files = args.filesInput

    flags.Trigger.triggerConfig = "DB"
    flags.Input.isMC = False

    # decoding flags for TrigDecisionMakerMT    
    flags.Trigger.DecodeHLT = False
    flags.Trigger.L1.doCTP = False
    flags.Trigger.DecisionMakerValidation.Execute = False
    
    flags.lock()

    # setup CA 
    cfg = setupDecodeCfgCA(flags)

    # Execute
    import sys
    sys.exit(not cfg.run().isSuccess())
