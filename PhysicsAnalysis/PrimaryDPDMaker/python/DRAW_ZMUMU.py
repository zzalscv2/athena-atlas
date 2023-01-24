## Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def DRAW_ZmumuKernelCfg(flags, name = "DRAW_ZMUMUKernel", **kwargs):
    result = ComponentAccumulator()
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import InvariantMassToolCfg

    massEntryName = "DRZmumuMass"

    # Object selection strings
    sel_muon1  = 'Muons.pt > 25*GeV && Muons.ptcone40/Muons.pt < 0.3'
    sel_muon2  = 'Muons.pt > 20*GeV && Muons.ptcone40/Muons.pt < 0.3'
    ### Ensure that it's scheduled after the Isolation builder
    kwargs.setdefault("ExtraInputs", [('xAOD::MuonContainer', 'StoreGateSvc+Muons.ptcone40')])

    mass_tool = result.getPrimaryAndMerge(InvariantMassToolCfg(flags,
                                                               name = "DRZmumuMassTool",
                                                                ContainerName              = "Muons",
                                                                ObjectRequirements         = sel_muon1,
                                                                SecondObjectRequirements   = sel_muon2,
                                                                MassHypothesis             = 105.66,
                                                                SecondMassHypothesis       = 105.66,
                                                                StoreGateEntryName         = massEntryName))
    
    
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import AsgSelectionToolWrapperCfg
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    muon_sel_tool = result.popToolsAndMerge(MuonSelectionToolCfg(flags, name = "DRAW_ZMUMU_MuonsSelector",
                                                            MaxEta = 3, MuQuality = 2 ### Use Medium muons
                                                            ))
    result.addPublicTool(muon_sel_tool)    
    muonSkimmingTool = result.getPrimaryAndMerge(AsgSelectionToolWrapperCfg(flags,
                                                           name = "DRAW_ZMUMU_GoodMuon_SkimmingTool",
                                                           ContainerName = "Muons",
                                                           StoreGateEntryName = "isMedium_DRAWZmumu",
                                                           AsgSelectionTool = muon_sel_tool))
    
    dimuonMassString = "( count (  {mass} > 70*GeV   &&  {mass} < 110*GeV ) >= 1 )".format(mass = massEntryName)

    muonSkimmingString = "( count (Muons.pt > 20*GeV && Muons.isMedium_DRAWZmumu == 1) >= 1)"
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import xAODStringSkimmingToolCfg
    dimuonMassSkimmingTool = result.getPrimaryAndMerge(xAODStringSkimmingToolCfg(flags,
                                                       name = "DRAW_ZMUMU_DiMuonMass_SkimmingTool",
                                                       expression = "{mass} && {muon} ".format(mass = dimuonMassString,
                                                                                              muon = muonSkimmingString)))

    
    # needed for dynamic determination of lowest-unprescaled single-muon and dimuon triggers
    from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
    from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType
    periods = TriggerPeriod.y2015 | TriggerPeriod.y2016 | TriggerPeriod.y2017 | TriggerPeriod.y2018 | TriggerPeriod.future
    allUnprescaledTriggers = TriggerAPI.getLowestUnprescaledAnyPeriod(periods, TriggerType.mu) + ["HLT_noalg_L1MU14FCH", "L1_MU14FCH" ]
    print("DRAW_ZMUMU: will skim on an OR of the following muon triggers (list provided at run-time by the TriggerAPI):")
    for trig in allUnprescaledTriggers:
        print (" **** {trig}".format(trig = trig))

    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import TriggerSkimmingToolCfg
    triggerSkimmingTool = result.getPrimaryAndMerge(TriggerSkimmingToolCfg(flags,  
                                                                name = "DRAWZMUMUTriggerSkimmingTool", 
                                                                TriggerListOR = allUnprescaledTriggers))

    # Event selection tool
    from DerivationFrameworkTools.DerivationFrameworkToolsConfig import FilterCombinationANDCfg
    DRAW_ZMUMU_SkimmingTool = result.getPrimaryAndMerge(FilterCombinationANDCfg(flags, 
                                                        name = "DRAW_ZMUMU_FinalFilter",
                                                        FilterList=[dimuonMassSkimmingTool,                                                                                   
                                                                    triggerSkimmingTool] ))

    kwargs.setdefault("AugmentationTools", [muonSkimmingTool, mass_tool])
    kwargs.setdefault("SkimmingTools", [DRAW_ZMUMU_SkimmingTool])
    the_alg = CompFactory.DerivationFramework.DerivationKernel(name, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result



def DRAW_ZmumuCfg(flags):
    result = ComponentAccumulator()
    stream_name = "StreamDRAW_ZMUMU"
    from AthenaCommon.CFElements import seqAND
    result.addSequence(seqAND("DRAWZMUMU_Sequence"))
    result.merge(DRAW_ZmumuKernelCfg(flags), sequenceName = "DRAWZMUMU_Sequence")
    
    bsesoSvc = CompFactory.ByteStreamEventStorageOutputSvc(
        name = "BSEventStorageOutputSvc",
        MaxFileMB      = 15000,
        MaxFileNE      = 15000000,
        OutputDirectory= './',
        StreamType     = '',
        StreamName     = stream_name,
        SimpleFileName = flags.Output.DRAW_ZmumuFileName)
    result.addService(bsesoSvc)
    
    bsIS = CompFactory.ByteStreamEventStorageInputSvc('ByteStreamInputSvc')
    result.addService(bsIS)
            
    bsCopyTool = CompFactory.ByteStreamOutputStreamCopyTool(
        ByteStreamOutputSvc = bsesoSvc,
        ByteStreamInputSvc  = bsIS)
    result.addPublicTool(bsCopyTool)

    bsCnvSvc = CompFactory.ByteStreamCnvSvc(
        ByteStreamOutputSvcList=[bsesoSvc.getName()])
    result.addService(bsCnvSvc)
        
    result.addEventAlgo(CompFactory.AthenaOutputStream(
        name             = 'BSOutputStreamAlgDRAW_ZMUMU',
        WritingTool      = bsCopyTool,
        EvtConversionSvc = bsCnvSvc.name,
        RequireAlgs      = ['DRAW_ZMUMUKernel'],
        ExtraInputs      = [('xAOD::EventInfo','StoreGateSvc+EventInfo')]),
                           domain='IO', primary = True)
        
    from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
    from IOVDbSvc.IOVDbSvcConfig import IOVDbSvcCfg
    result.merge(IOVDbSvcCfg(flags))
    result.merge(MetaDataSvcCfg(flags, ["IOVDbMetaDataTool", "ByteStreamMetadataTool"]))

    
    return result


