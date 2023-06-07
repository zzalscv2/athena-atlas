# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#from AthenaCommon import CfgMgr
#from AthenaCommon.CfgGetter import getPrivateTool
from MuonRecExample.MuonRecFlags import muonRecFlags

def getStreamRDO_ItemList(log):

    def standardDigiConfig():
        from Digitization.DigitizationFlags import digitizationFlags
        return not ('Split' in digitizationFlags.digiSteeringConf.get_Value() or 'Fast' in digitizationFlags.digiSteeringConf.get_Value())

    def standardDigiOutputExpected(keys=['doFastPixelDigi', 'doFastSCT_Digi', 'doFastTRT_Digi']):
        noStandardDigi=True
        from Digitization.DigitizationFlags import digitizationFlags
        if standardDigiConfig():
            for key in keys:
                noStandardDigi = noStandardDigi and key in digitizationFlags.experimentalDigi()
        else:
            noStandardDigi =  not set(['SplitNoMergeFFPileUpToolsAlg', 'FastPileUpToolsAlg']).isdisjoint(set([digitizationFlags.digiSteeringConf.get_Value()]))
        return not noStandardDigi

    def standardPixelDigiOutputExpected():
        return standardDigiOutputExpected(['doFastPixelDigi'])

    def standardSCT_DigiOutputExpected():
        return standardDigiOutputExpected(['doFastSCT_Digi'])

    def standardTRT_DigiOutputExpected():
        return standardDigiOutputExpected(['doFastTRT_Digi'])

    def fastDigiOutputExpected(keys=['doFastPixelDigi', 'doFastSCT_Digi', 'doFastTRT_Digi']):
        noFastDigi = False
        from Digitization.DigitizationFlags import digitizationFlags
        if standardDigiConfig():
            noFastDigi = set(keys).isdisjoint(set(digitizationFlags.experimentalDigi()))
        else:
            noFastDigi = set(['SplitNoMergeFFPileUpToolsAlg', 'SplitNoMergeSFPileUpToolsAlg', 'SplitNoMergeFSPileUpToolsAlg', 'FastPileUpToolsAlg', 'SplitSFPileUpToolsAlg']).isdisjoint(set([digitizationFlags.digiSteeringConf.get_Value()]))
        return not noFastDigi

    def fastPixelDigiOutputExpected():
        return fastDigiOutputExpected(['doFastPixelDigi'])

    def fastSCT_DigiOutputExpected():
        return fastDigiOutputExpected(['doFastSCT_Digi'])

    def fastTRT_DigiOutputExpected():
        return fastDigiOutputExpected(['doFastTRT_Digi'])

    ## print "XXXXXXXXXXXXXXX Expect standard digi output: ", standardDigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect Pixel standard digi output: ", standardPixelDigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect SCT standard digi output: ", standardSCT_DigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect TRT standard digi output: ", standardTRT_DigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect fast digi output: ", fastDigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect Pixel fast digi output: ", fastPixelDigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect SCT fast digi output: ", fastSCT_DigiOutputExpected()
    ## print "XXXXXXXXXXXXXXX Expect TRT fast digi output: ", fastTRT_DigiOutputExpected()

    from Digitization.DigitizationFlags import digitizationFlags
    from AthenaCommon.DetFlags import DetFlags
    if not(DetFlags.writeRDOPool.any_on()):
        log.warning( "this digitization job will not write any RDO object" )
    else:
        StreamRDO_ItemList = []

    # Event info
    if DetFlags.writeRDOPool.any_on():
        outputKey = 'EventInfo'
        if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
            from OverlayCommonAlgs.OverlayFlags import overlayFlags
            outputKey = overlayFlags.bkgPrefix() + 'EventInfo'
        #add to output stream
        StreamRDO_ItemList += [ "xAOD::EventInfo#" + outputKey, "xAOD::EventAuxInfo#" + outputKey + "Aux."]
        if DetFlags.pileup.any_on() or digitizationFlags.doXingByXingPileUp():
            StreamRDO_ItemList += [ "xAOD::EventInfoContainer#*", "xAOD::EventInfoAuxContainer#*"]

    # Set up for cosmics digitization
    from AthenaCommon.BeamFlags import jobproperties
    if jobproperties.Beam.beamType == "cosmics" :
        StreamRDO_ItemList += ["ComTime#*"]
    # Truth Output
    if DetFlags.writeRDOPool.Truth_on():
        StreamRDO_ItemList+=["McEventCollection#*"]
        StreamRDO_ItemList+=["TrackRecordCollection#*"]

        prefix = ''
        if digitizationFlags.PileUpPresampling and 'LegacyOverlay' not in digitizationFlags.experimentalDigi():
            from OverlayCommonAlgs.OverlayFlags import overlayFlags
            prefix = overlayFlags.bkgPrefix()

        dropped_jet_vars = ['constituentLinks',
                            'constituentWeights',
                            'ConeExclBHadronsFinal',
                            'ConeExclCHadronsFinal',
                            'ConeExclTausFinal',
                            'GhostPartons',
                            'GhostBHadronsFinal',
                            'GhostCHadronsFinal',
                            'GhostTausFinal']
        jet_var_str = '.-'.join ([''] + dropped_jet_vars)

        if 'PileUpAntiKt4TruthJets' in digitizationFlags.experimentalDigi():
            StreamRDO_ItemList+=[f"xAOD::JetContainer#{prefix}InTimeAntiKt4TruthJets"]
            StreamRDO_ItemList+=[f"xAOD::AuxContainerBase!#{prefix}InTimeAntiKt4TruthJetsAux" + jet_var_str]
            StreamRDO_ItemList+=[f"xAOD::JetContainer#{prefix}OutOfTimeAntiKt4TruthJets"]
            StreamRDO_ItemList+=[f"xAOD::AuxContainerBase!#{prefix}OutOfTimeAntiKt4TruthJetsAux" + jet_var_str]
        if 'PileUpAntiKt6TruthJets' in digitizationFlags.experimentalDigi():
            StreamRDO_ItemList+=[f"xAOD::JetContainer#{prefix}InTimeAntiKt6TruthJets"]
            StreamRDO_ItemList+=[f"xAOD::AuxContainerBase!#{prefix}InTimeAntiKt6TruthJetsAux" + jet_var_str]
            StreamRDO_ItemList+=[f"xAOD::JetContainer#{prefix}OutOfTimeAntiKt6TruthJets"]
            StreamRDO_ItemList+=[f"xAOD::AuxContainerBase!#{prefix}OutOfTimeAntiKt6TruthJetsAux" + jet_var_str]
        if 'PileUpTruthParticles' in digitizationFlags.experimentalDigi():
            StreamRDO_ItemList+=["xAOD::TruthParticleContainer#*"]
            StreamRDO_ItemList+=["xAOD::TruthParticleAuxContainer#*"]
        if DetFlags.writeRDOPool.Calo_on():
            StreamRDO_ItemList += ["CaloCalibrationHitContainer#*"]
            # Temporary for debugging MBTSHits
            StreamRDO_ItemList+=["TileHitVector#MBTSHits"]
        if DetFlags.writeRDOPool.ID_on():
            if standardDigiOutputExpected() or DetFlags.writeRDOPool.BCM_on():
                StreamRDO_ItemList+=["InDetSimDataCollection#*"]
            if fastDigiOutputExpected():
                StreamRDO_ItemList+=["PRD_MultiTruthCollection#*"]
        if DetFlags.writeRDOPool.Muon_on():
            StreamRDO_ItemList+=["CscSimDataCollection#*CSC_SDO"]
            StreamRDO_ItemList+=["MuonSimDataCollection#*"]
    # Forward Detector Output
    if DetFlags.digitize.Lucid_on():
        if DetFlags.writeRDOPool.Lucid_on():
            StreamRDO_ItemList += ["LUCID_DigitContainer#Lucid_Digits"]
    if DetFlags.digitize.ZDC_on():
        if DetFlags.writeRDOPool.ZDC_on():
            StreamRDO_ItemList += ["ZdcDigitsCollection#*"]
    if DetFlags.digitize.ALFA_on():
        if DetFlags.writeRDOPool.ALFA_on():
            StreamRDO_ItemList += ["ALFA_DigitCollection#*"]
            StreamRDO_ItemList += ["ALFA_ODDigitCollection#*"]
    if hasattr(DetFlags.digitize, 'AFP_on') and DetFlags.digitize.AFP_on():
        if DetFlags.writeRDOPool.AFP_on():
            StreamRDO_ItemList+=["AFP_SiDigiCollection#*"]
            StreamRDO_ItemList+=["AFP_TDDigiCollection#*"]
            StreamRDO_ItemList+=["xAOD::AFPSiHitContainer#*"]
            StreamRDO_ItemList+=["xAOD::AFPToFHitContainer#*"]
            StreamRDO_ItemList+=["xAOD::AFPSiHitAuxContainer#*"]
            StreamRDO_ItemList+=["xAOD::AFPToFHitAuxContainer#*"]
    # Inner Detector Output
    if DetFlags.digitize.BCM_on():
        if DetFlags.writeRDOPool.BCM_on():
            StreamRDO_ItemList+=["BCM_RDO_Container#*"]
    if DetFlags.digitize.pixel_on():
        if DetFlags.writeRDOPool.pixel_on():
            if fastPixelDigiOutputExpected():
                StreamRDO_ItemList+=["InDet::SiClusterContainer#*"]
                StreamRDO_ItemList+=["InDet::PixelGangedClusterAmbiguities#*"]
                StreamRDO_ItemList+=["InDet::PixelClusterContainer#*"]
            if standardPixelDigiOutputExpected():
                StreamRDO_ItemList+=["PixelRDO_Container#*"]
    if DetFlags.digitize.SCT_on():
        if DetFlags.writeRDOPool.SCT_on():
            if fastSCT_DigiOutputExpected():
                StreamRDO_ItemList+=["InDet::SiClusterContainer#*"]
            if standardSCT_DigiOutputExpected():
                StreamRDO_ItemList+=["SCT_RDO_Container#*"]
    if DetFlags.digitize.TRT_on():
        if DetFlags.writeRDOPool.TRT_on():
            if fastTRT_DigiOutputExpected():
                StreamRDO_ItemList += ["InDet::TRT_DriftCircleContainer#*"]
            if standardTRT_DigiOutputExpected():
                StreamRDO_ItemList+=["TRT_RDO_Container#*"]
    # Calorimeter Output
    if DetFlags.digitize.LAr_on():
        if DetFlags.writeRDOPool.LAr_on():
            if not digitizationFlags.PileUpPresampling:
                StreamRDO_ItemList+=["LArRawChannelContainer#*"]
            if 'AddCaloDigi' in digitizationFlags.experimentalDigi():
                StreamRDO_ItemList+=["LArDigitContainer#*"]
            elif digitizationFlags.PileUpPresampling:
                StreamRDO_ItemList+=["LArDigitContainer#*LArDigitContainer_MC"]
            else:
                StreamRDO_ItemList+=["LArDigitContainer#LArDigitContainer_MC_Thinned"]
    if hasattr(DetFlags.digitize, 'HGTD_on') and DetFlags.digitize.HGTD_on():
        if DetFlags.writeRDOPool.HGTD_on():
            StreamRDO_ItemList+=["LArHitContainer#HGTDDigitContainer_MC"]
    if DetFlags.digitize.Tile_on():
        if DetFlags.writeRDOPool.Tile_on():
            if not digitizationFlags.PileUpPresampling:
                StreamRDO_ItemList+=["TileRawChannelContainer#*"]
            if 'AddCaloDigi' in digitizationFlags.experimentalDigi():
                StreamRDO_ItemList+=["TileDigitsContainer#*"]
            elif digitizationFlags.PileUpPresampling:
                StreamRDO_ItemList+=["TileDigitsContainer#*TileDigitsCnt"]
            else:
                StreamRDO_ItemList+=["TileDigitsContainer#TileDigitsFlt"]
            StreamRDO_ItemList+=["TileL2Container#*TileL2Cnt"]
    # Muon Spectrometer Output
    if DetFlags.digitize.Muon_on():
        if DetFlags.writeRDOPool.CSC_on():
            StreamRDO_ItemList+=["CscRawDataContainer#*"]
        if DetFlags.writeRDOPool.MDT_on():
            StreamRDO_ItemList+=["MdtCsmContainer#*"]
        if DetFlags.writeRDOPool.RPC_on():
            StreamRDO_ItemList+=["RpcPadContainer#*"]
            if muonRecFlags.doNRPCs():
                StreamRDO_ItemList+=["xAOD::NRPCRDOContainer#*"]
                StreamRDO_ItemList+=["xAOD::NRPCRDOAuxContainer#*"]
       
        if DetFlags.writeRDOPool.TGC_on():
            StreamRDO_ItemList+=["TgcRdoContainer#*"]
        if DetFlags.writeRDOPool.sTGC_on():
            StreamRDO_ItemList+=["Muon::STGC_RawDataContainer#*"]
        if DetFlags.writeRDOPool.MM_on():
            StreamRDO_ItemList+=["Muon::MM_RawDataContainer#*"]
    # LVL1 Emulation Output
    if DetFlags.simulateLVL1.LAr_on():
        if DetFlags.writeRDOPool.LAr_on():
            StreamRDO_ItemList+=["LArTTL1Container#*"]
            from AtlasGeoModel.CommonGMJobProperties import CommonGeometryFlags as commonGeoFlags
            if commonGeoFlags.Run()=="RUN3":
                if digitizationFlags.PileUpPresampling:
                    StreamRDO_ItemList+=["LArDigitContainer#*LArDigitSCL2"]
                else:
                    StreamRDO_ItemList+=["CaloCellContainer#SCell"]
    if DetFlags.simulateLVL1.Tile_on():
        if DetFlags.writeRDOPool.Tile_on():
            StreamRDO_ItemList+=["TileTTL1Container#*"]
            StreamRDO_ItemList+=["TileDigitsContainer#*MuRcvDigitsCnt"]
            if not digitizationFlags.PileUpPresampling:
                StreamRDO_ItemList+=["TileRawChannelContainer#MuRcvRawChCnt"]
            StreamRDO_ItemList+=["TileMuonReceiverContainer#*"]
    if DetFlags.digitize.LVL1_on():
        if DetFlags.writeRDOPool.LVL1_on():
              StreamRDO_ItemList+=[
                               "ROIB::RoIBResult#*",
                               "MuCTPI_RDO#*",
                               "CTP_RDO#*",
                               "DataVector<LVL1::TriggerTower>#*",
                               "DataVector<LVL1::CPMTower>#*",
                               "DataVector<LVL1::JetElement>#*",
                               "DataVector<LVL1::CPMHits>#*",
                               "DataVector<LVL1::CPMRoI>#*",
                               "DataVector<LVL1::CMMCPHits>#*",
                               "DataVector<LVL1::CMMJetHits>#*",
                               "DataVector<LVL1::CMMEtSums>#*",
                               "DataVector<LVL1::JEMHits>#*",
                               "DataVector<LVL1::JEMRoI>#*",
                               "DataVector<LVL1::JEMEtSums>#*",
                               "LVL1::CMMRoI#*"]

    return StreamRDO_ItemList
