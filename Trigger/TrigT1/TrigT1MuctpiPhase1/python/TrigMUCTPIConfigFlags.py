# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags

def createTrigMUCTPIConfigFlags():
    flags = AthConfigFlags()

    # Configure MUCTPI overlap handling strategy
    flags.addFlag('Trigger.MUCTPI.OverlapStrategy', 'LUT')

    # Configure xml file for the MUCTPI overlap handling (if "LUT" option is chosen as overlap strategy)
    flags.addFlag('Trigger.MUCTPI.LUTXMLFile', 'TrigConfMuctpi/overlapRun3_20201214.xml') 

    # Configure flags for the MUCTP output generation for L1Topo
    flags.addFlag('Trigger.MUCTPI.BarrelRoIFile', 'TrigConfMuctpi/Data_ROI_Mapping_Barrel_040422.txt')
    flags.addFlag('Trigger.MUCTPI.EndcapForwardRoIFile', 'TrigConfMuctpi/Data_RoI_Mapping_EF_040422.txt')
    flags.addFlag('Trigger.MUCTPI.Side0LUTFile', 'TrigConfMuctpi/lookup_0_040422.json')
    flags.addFlag('Trigger.MUCTPI.Side1LUTFile', 'TrigConfMuctpi/lookup_1_040422.json')


    return flags


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RAW_RUN2

    flags.lock()
    flags.dump("MUCTPI|Trigger")
