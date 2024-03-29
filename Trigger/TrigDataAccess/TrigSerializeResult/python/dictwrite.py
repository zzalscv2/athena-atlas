#!/usr/bin/env pyroot.py 

# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#----------------------------------------------
#
# PLEASE DO NOT UPDATE bs-streamerinfos.root YOURSELF!!!!
# Contact the person mentioned on: 
#    https://twiki.cern.ch/twiki/bin/view/Atlas/TrigSerializeResult
# and/or the Trigger EDM Coordinator 
#
#----------------------------------------------

import sys

"""
MN: NOTE about xAOD containers in ROOT5 
For an xAOD container like xAOD::HIEventShapeContainer_v1
you need to either:
a) use the real class name:  DataVector<xAOD::HIEventShape_v2>
b) have a real class in the 'objects' list BEFORE the typedef
   (so the dict library gets loaded and the typedefs resolved)
"""

def update_streamerinfos(objects, updated_objects):
  #doEDM=False
  doEDM=True
  doxAODonly=False
  bs_filename = 'bs-streamerinfos.root'
  import ROOT
  from ROOT import TFile
  from TrigSerializeResult import StreamerInfoGenerator
  SIG = StreamerInfoGenerator.StreamerInfoGenerator()
  from collections import defaultdict
  streamerChecksums = defaultdict(set)
  print("Reading streamerinfos from", bs_filename)
  file = TFile(bs_filename, 'UPDATE')
  streamer_n = 0
  if  file.GetStreamerInfoList():
    for i in file.GetStreamerInfoList():
      if i.GetName() != 'listOfRules':
        streamerChecksums[i.GetName()].add( i.GetCheckSum() )
        streamer_n += 1
  print("Read", streamer_n, 'streamers for', len(streamerChecksums), 'types')
  print("")

  if doEDM:
    from TrigEDMConfig.TriggerEDMRun3 import TriggerHLTListRun3
    from TrigEDMConfig.DataScoutingInfo import getAllDataScoutingIdentifiers
    BS_destinations = ["BS"] + getAllDataScoutingIdentifiers()
    print("BS_destinations = {}".format(BS_destinations))
    for item in TriggerHLTListRun3:
      if any(bs in item[1].split() for bs in BS_destinations):
        objects.append(item[0].split("#")[0])

  for pers in objects:
    SIG.inspect(pers)
    print("")

  fulllist = SIG.classlist
  print(fulllist)
  print('*******************************')

  from CLIDComps.clidGenerator import clidGenerator
  cgen = clidGenerator("")

  types_new = 0
  types_exist = 0
  types_bad = 0
  fulllist = list(set(fulllist))
  for item in fulllist:
    if doxAODonly and 'xAOD' not in item: continue # current issues seen because of missing xAOD libs not being loaded
    print("Trying to fill item", item, "to root file")
    c_clid = cgen.genClidFromName(item)
    c_typeinfo = cgen.getTidFromClid(c_clid)
    print("CLID", c_clid)
    print("TypeInfo", c_typeinfo)
    try:
      cls = ROOT.gROOT.GetClass(item)
    except Exception:
      cls = ROOT.gROOT.GetClass(c_typeinfo)
    print(cls)

    if cls is not None:
      streamerinfo = cls.GetStreamerInfo()
      if streamerinfo.GetCheckSum() == 0:
        # try to patch missing checksum in DataVectors
        print('Warning: no checksum in streamerinfo for type: ', cls.GetName())
        print('Attempting to fix with 0x%x' %  cls.GetCheckSum())
        streamerinfo.SetCheckSum( cls.GetCheckSum() )

      chksum = streamerinfo.GetCheckSum()
      if chksum not in streamerChecksums[cls.GetName()]:
        print('Writing: %s  streamer size=%d, checksum=0x%x' %(cls.GetName(), streamerinfo.Sizeof(), chksum))
        obj = cls.New()
        file.WriteObjectAny(obj, cls, cls.GetName())
        types_new += 1
        updated_objects.append( (cls.GetName(), chksum) )
      else:
        print('Skipping {} streamer checksum 0x{:x} - already in the file'.format(cls.GetName(), chksum))
        types_exist += 1
    else:
      print('skipping ', item)
      types_bad += 1
      #sys.exit()
    print('----')

  print('Wrote', types_new, 'types')
  print('Skipped', types_exist, ' existing types')
  print('Problems with', types_bad + len(SIG.problemclasses), ' types')
  for t in SIG.problemclasses:
    print('    ', t)

  return 0

def main():
  objects = [
    'xAOD::TrigEMCluster_v1',
    'xAOD::TrigEMClusterContainer_v1',
    'xAOD::TrigEMClusterAuxContainer_v1',
    'xAOD::TrigRingerRings_v1',
    'xAOD::TrigRingerRingsContainer_v1',
    'xAOD::TrigRingerRingsAuxContainer_v1',
    'xAOD::TrigRNNOutput_v1',
    'xAOD::TrigRNNOutputContainer_v1',
    'xAOD::TrigRNNOutputAuxContainer_v1',
    'xAOD::CaloClusterContainer_v1',
    'xAOD::CaloClusterAuxContainer_v2',
    'xAOD::L2StandAloneMuonContainer_v1',
    'xAOD::L2StandAloneMuonAuxContainer_v1',
    'xAOD::L2StandAloneMuonAuxContainer_v2',
    'xAOD::L2CombinedMuonContainer_v1',
    'xAOD::L2CombinedMuonAuxContainer_v1',
    'xAOD::L2IsoMuonContainer_v1',
    'xAOD::L2IsoMuonAuxContainer_v1',
    'xAOD::MuonContainer_v1',
    'xAOD::MuonAuxContainer_v1',
    'xAOD::MuonAuxContainer_v2',
    'xAOD::MuonAuxContainer_v3',
    'xAOD::MuonAuxContainer_v4',
    'xAOD::TrackParticleContainer_v1',
    'xAOD::TrackParticleAuxContainer_v1',
    'xAOD::TrackParticleAuxContainer_v2',
    'xAOD::TrackParticleAuxContainer_v3',
    'xAOD::TauJetContainer_v2',
    'xAOD::TauJetAuxContainer_v2',
    'xAOD::TauJetAuxContainer_v3',
    'xAOD::VertexContainer_v1',
    'xAOD::VertexAuxContainer_v1',
    'xAOD::TrigCompositeContainer_v1',
    'xAOD::TrigCompositeAuxContainer_v1',
    'xAOD::TrigCompositeAuxContainer_v2',
    'xAOD::MuonRoIContainer_v1',
    'xAOD::MuonRoIAuxContainer_v1',
    'xAOD::EmTauRoIContainer_v2',
    'xAOD::EmTauRoIAuxContainer_v2',
    'xAOD::JetRoIContainer_v2',
    'xAOD::JetRoIAuxContainer_v2',
    'xAOD::JetEtRoI_v1',
    'xAOD::JetEtRoIAuxInfo_v1',
    'xAOD::EnergySumRoI_v1',
    'xAOD::EnergySumRoIAuxInfo_v1',
    'xAOD::TriggerTowerContainer_v2',
    'xAOD::TriggerTowerAuxContainer_v2',
    'xAOD::ElectronContainer_v1',
    'xAOD::ElectronAuxContainer_v1',
    'xAOD::PhotonContainer_v1',
    'xAOD::PhotonAuxContainer_v1',
    'xAOD::TrigBphysContainer_v1',
    'xAOD::TrigBphysAuxContainer_v1',
    'xAOD::TrigT2MbtsBitsAuxContainer_v1',
    'xAOD::TrigT2MbtsBitsContainer_v1',
    'xAOD::TrigSpacePointCountsContainer_v1',
    'xAOD::TrigSpacePointCountsAuxContainer_v1',
    'xAOD::TrigVertexCountsContainer_v1',
    'xAOD::TrigVertexCountsAuxContainer_v1',
    'xAOD::TrigTrackCounts_v1',
    'xAOD::TrigTrackCountsContainer_v1',
    'xAOD::TrigTrackCountsAuxContainer_v1',
    'xAOD::TrigMissingETContainer_v1',
    'xAOD::TrigMissingETAuxContainer_v1',
    'xAOD::TrigPhotonContainer_v1',
    'xAOD::TrigPhotonAuxContainer_v1',
    'xAOD::TrigElectronContainer_v1',
    'xAOD::TrigElectronAuxContainer_v1',
    'xAOD::JetContainer_v1',
    'xAOD::JetTrigAuxContainer_v1',
    'xAOD::JetTrigAuxContainer_v2',
    'xAOD::TrigDecision_v1',
    'xAOD::TrigDecisionAuxInfo_v1',
    'xAOD::TrigConfKeys_v1',
    'xAOD::TrigNavigation_v1',
    'xAOD::TrigNavigationAuxInfo_v1',
    'xAOD::BTaggingContainer_v1',
    'xAOD::BTaggingAuxContainer_v1',
    'xAOD::BTaggingTrigAuxContainer_v1',
    'xAOD::BTagVertexContainer_v1',
    'xAOD::BTagVertexAuxContainer_v1',
    'xAOD::HIEventShapeAuxContainer_v2',
    'xAOD::HIEventShapeContainer_v2',
    'xAOD::TrigT2ZdcSignalsAuxContainer_v1',
    'xAOD::TrigT2ZdcSignalsContainer_v1',
    'xAOD::TrigPassBitsContainer_v1',
    'xAOD::TrigPassBitsAuxContainer_v1',
    'xAOD::CaloClusterTrigAuxContainer_v1',
    'xAOD::ElectronTrigAuxContainer_v1',
    'xAOD::PhotonTrigAuxContainer_v1',
    'xAOD::TrigEMClusterAuxContainer_v2',
    'xAOD::TrigRingerRingsAuxContainer_v2',
    'xAOD::TrigRNNOutput_v2',
    'xAOD::TrigRNNOutputContainer_v2',
    'xAOD::TrigRNNOutputAuxContainer_v2',
    'xAOD::TauTrack_v1',
    'xAOD::TauTrackContainer_v1',
    'xAOD::TauTrackAuxContainer_v1',
    #'TrigMuonEFIsolation_p2',
    'xAOD::BTaggingAuxContainer_v2',
    'xAOD::jFexMETRoI_v1',
    'xAOD::jFexMETRoIAuxContainer_v1',
    'xAOD::jFexSumETRoI_v1',
    'xAOD::jFexSumETRoIAuxContainer_v1',
    'xAOD::eFexTauRoIAuxContainer_v1',
    'xAOD::gFexGlobalRoI_v1',
    'xAOD::gFexGlobalRoIAuxContainer_v1',
    'xAOD::gFexJetRoI_v1',
    'xAOD::gFexJetRoIAuxContainer_v1',
    'xAOD::AFPToFHit_v1',
    'xAOD::AFPVertex_v1',
    'xAOD::AFPVertexAuxContainer_v1',
    'xAOD::AFPSiHit_v2',
    'xAOD::AFPProton_v1',
    'xAOD::AFPProtonAuxContainer_v1',
    'xAOD::AFPToFTrack_v1',
    'xAOD::AFPToFTrackAuxContainer_v1',
    'xAOD::AFPSiHitsClusterAuxContainer_v1',
    'xAOD::jFexTauRoIAuxContainer_v1',
    'xAOD::jFexSRJetRoI_v1',
    'xAOD::jFexSRJetRoIAuxContainer_v1',
    'xAOD::jFexLRJetRoI_v1',
    'xAOD::jFexLRJetRoIAuxContainer_v1',
  ]
  updated_objects = []
  return update_streamerinfos(objects, updated_objects)

if __name__ == "__main__":
    sys.exit(main())
