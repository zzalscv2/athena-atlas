#Matt A's custom python accumulator for R22 
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

#Trigger xAODType.ObjectType dict entry loading
import cppyy
try:
    cppyy.load_library('libxAODBaseObjectTypeDict')
except Exception:
    pass
from ROOT import xAODType
xAODType.ObjectType

def config_CHS_CSSK(flags,**kwargs):

    #Trigger xAODType.ObjectType dict entry loading
    import cppyy
    try:
        cppyy.load_library('libxAODBaseObjectTypeDict')
    except Exception:
        pass
    from ROOT import xAODType
    xAODType.ObjectType
    CHS_CSSK_CA=ComponentAccumulator()
    # add storegate
    StoreGateSvc=CompFactory.StoreGateSvc
    CHS_CSSK_CA.addService(StoreGateSvc("DetectorStore"))
    from JetRec.JetRecStandardToolManager import jtm
    from JetRecTools.ConstitToolManager import ctm
    from JetRecTools.JetRecToolsConf import CorrectPFOTool
    ctm.add( CorrectPFOTool("CorrectPFOTool",
                            WeightPFOTool = jtm.pflowweighter,
                            InputIsEM = True,
                            CalibratePFO = False,
                            UseChargedWeights = True,
                            InputType = xAODType.FlowElement
                        ),
             alias = 'correctPFO' )
    
    jtm += ctm.buildConstitModifSequence( "JetConstitSeq_PFlowCHS",
                                          InputContainer = "JetETMiss",
                                          OutputContainer = "CHS",  #"ParticleFlowObjects" will be appended later
                                          modList = ['correctPFO', 'chsPFO'] )
    
    CHS_CSSK_CA.merge(jtm)
    return CHS_CSSK_CA



def UFOConfig(flags, **kwargs):
    # Configure UFO reconstruction algorithm instead of TCC
    UFO_CA=ComponentAccumulator()
    #add Storegate
    StoreGateSvc=CompFactory.StoreGateSvc
    
    UFO_CA.addService(StoreGateSvc("DetectorStore"))
    from TrackCaloClusterRecTools.TrackCaloClusterConfig import runUFOReconstruction_redux
    from JetRecConfig.StandardJetConstits import stdConstitDic as cst
    constituents=cst.EMPFlow
    UFO_reco=runUFOReconstruction_redux(flags,constits=constituents,charged=True)
    UFO_CA.merge(UFO_reco)

    return UFO_CA
    


if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Input.isMC=True
    flags.Input.Format="AOD"
    #    flags.Input.Files=["/scratch/anthony/TEST_AOD/TCC/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3126_r10724/mc16_13TeV/AOD.14795494._007454.pool.root.1"]
    #    flags.Input.Files=["/scratch/anthony/TEST_AOD/TCC/valid1_test/valid1/AOD.24855256._000001.pool.root.1"]
    flags.Input.Files=["/scratch/anthony/GPF_CODE//TCC_STUDIES/CustomAlg/myAOD.root"]
    flags.Output.AODFileName="/scratch/anthony/GPF_CODE/TCC_STUDIES/CustomAlg/output_AOD.root"
    flags.Exec.MaxEvents=20
    flags.Output.doWriteAOD=True
    flags.Common.ProductionStep=""
    flags.dump()
    flags.lock()
    

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg=MainServicesCfg(flags)

    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    inputList=["xAOD::TrackParticleContainer#InDetTrackParticles", "xAOD::TrackParticleAuxContainer#InDetTrackParticlesAux."]
    inputList.append("xAOD::FlowElementContainer#JetETMissChargedParticleFlowObjects")
    inputList.append("xAOD::FlowElementContainer#JetETMissNeutralParticleFlowObjects")
    inputList.append("xAOD::FlowElementAuxContainer#JetETMissNeutralParticleFlowObjectsAux.")
    inputList.append("xAOD::FlowElementAuxContainer#JetETMissChargedParticleFlowObjectsAux.")
    inputList.append("xAOD::FlowElementContainer#TrackCaloClustersCharged")
    inputList.append("xAOD::FlowElementAuxContainer#TrackCaloClustersChargedAux.")
    inputList.append("xAOD::FlowElementContainer#TrackCaloClustersNeutral")
    inputList.append("xAOD::FlowElementAuxContainer#TrackCaloClustersNeutralAux.")

    inputList.append("xAOD::MuonContainer#Muons")
    inputList.append("xAOD::MuonAuxContainer#*")
    inputList.append("xAOD::PhotonContainer#Photons")
    inputList.append("xAOD::PhotonAuxContainer#*")
    inputList.append("xAOD::ElectronContainer#Electrons")
    inputList.append("xAOD::ElectronAuxContainer#*")
    inputList.append("xAOD::TauJetContainer#*")
    inputList.append("xAOD::TauJetAuxContainer#*")
    
    
    cfg.merge(OutputStreamCfg(flags,"AOD",ItemList=inputList))
    
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    
    cfg.merge(PoolReadCfg(flags))
    #cfg.merge(config_CHS_CSSK(flags)) # this config doesn't work
    cfg.merge(UFOConfig(flags))
    print("MASTER CA CONFIG")
    print(cfg)
    cfg.printConfig()

    cfg.run()

