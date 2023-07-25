# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def configureMuonInputProviderHistograms(alg, flags):
    alg.MonTool.defineHistogram('MuonTOBPt', path='EXPERT', type='TH1I',
                                title='Muon TOB Pt;p_{T} [GeV];', xbins=40, xmin=0, xmax=40)
    alg.MonTool.defineHistogram('MuonTOBPtTGC', path='EXPERT', type='TH1I',
                                title='TGC Muon TOB Pt;p_{T} [GeV];', xbins=40, xmin=0, xmax=40)
    alg.MonTool.defineHistogram('MuonTOBPtRPC', path='EXPERT', type='TH1I',
                                title='RPC Muon TOB Pt;p_{T} [GeV];', xbins=40, xmin=0, xmax=40)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBPt; MuonTOBPtEta', path='EXPERT', type='TH2I',
                                title='Muon TOB Pt vs Eta;#eta#times40;p_{T} [GeV]',
                                xbins=200, xmin=-200, xmax=200, ybins=40, ymin=0, ymax=40)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBPhi; MuonTOBPhiEta', path='EXPERT', type='TH2I',
                                title='Muon TOB Location;#eta#times40;#phi#times20',
                                xbins=50, xmin=-200, xmax=200, ybins=64, ymin=0, ymax=128)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBBW2or3; MuonTOBBW2or3Eta', path='EXPERT', type='TH2I',
                                title='Muon TOB BW2or3 vs Eta;#eta#times40;TGC full-station coincidence',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBInnerCoin; MuonTOBInnerCoinEta', path='EXPERT', type='TH2I',
                                title='Muon TOB InnerCoin vs Eta;#eta#times40;TGC inner coincidence',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBGoodMF; MuonTOBGoodMFEta', path='EXPERT', type='TH2I',
                                title='Muon TOB GoodMF vs Eta;#eta#times40;good magnetic field',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBCharge; MuonTOBChargeEta', path='EXPERT', type='TH2I',
                                title='Muon TOB Charge vs Eta;#eta#times40;charge',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBIs2cand; MuonTOBIs2candEta', path='EXPERT', type='TH2I',
                                title='Muon TOB Is2cand vs Eta;#eta#times40;>1 cand. in RPC pad',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('MuonTOBEta, MuonTOBIsTGC; MuonTOBIsTGCEta', path='EXPERT', type='TH2I',
                                title='Muon TOB IsTGC vs Eta;#eta#times40;Is a TGC muon',
                                xbins=200, xmin=-200, xmax=200, ybins=2, ymin=-0.5, ymax=1.5)
    alg.MonTool.defineHistogram('LateMuonTOBPt', path='EXPERT', type='TH1I',
                                title='LateMuon TOB Pt;p_{T} [GeV];', xbins=40, xmin=0, xmax=40)
    alg.MonTool.defineHistogram('LateMuonTOBPtTGC', path='EXPERT', type='TH1I',
                                title='TGC LateMuon TOB Pt;p_{T} [GeV];', xbins=40, xmin=0, xmax=40)
    alg.MonTool.defineHistogram('LateMuonTOBPtRPC', path='EXPERT', type='TH1I',
                                title='RPC LateMuon TOB Pt;p_{T} [GeV];', xbins=40, xmin=0, xmax=40)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBPt; LateMuonTOBPtEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB Pt vs Eta;#eta#times40;p_{T} [GeV]',
                                xbins=200, xmin=-200, xmax=200, ybins=40, ymin=0, ymax=40)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBPhi; LateMuonTOBPhiEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB Location;#eta#times40;#phi#times20',
                                xbins=50, xmin=-200, xmax=200, ybins=64, ymin=0, ymax=128)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBBW2or3; LateMuonTOBBW2or3Eta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB BW2or3 vs Eta;#eta#times40;TGC full-station coincidence',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBInnerCoin; LateMuonTOBInnerCoinEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB InnerCoin vs Eta;#eta#times40;TGC inner coincidence',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBGoodMF; LateMuonTOBGoodMFEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB GoodMF vs Eta;#eta#times40;good magnetic field',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBCharge; LateMuonTOBChargeEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB Charge vs Eta;#eta#times40;charge',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBIs2cand; LateMuonTOBIs2candEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB Is2cand vs Eta;#eta#times40;>1 cand. in RPC pad',
                                xbins=200, xmin=-200, xmax=200, ybins=3, ymin=-1, ymax=2)
    alg.MonTool.defineHistogram('LateMuonTOBEta, LateMuonTOBIsTGC; LateMuonTOBIsTGCEta', path='EXPERT', type='TH2I',
                                title='LateMuon TOB IsTGC vs Eta;#eta#times40;Is a TGC muon',
                                xbins=200, xmin=-200, xmax=200, ybins=2, ymin=-0.5, ymax=1.5)

def configureeFexInputProviderHistograms(alg, flags):
    alg.MonTool.defineHistogram('eEmTOBEt', path='EXPERT', type='TH1I',
                                title='eEm TOB Et;E_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('eEmTOBREta', path='EXPERT', type='TH1I',
                                title='eEm TOB rEta isolation;rEta isolation;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('eEmTOBRHad', path='EXPERT', type='TH1I',
                                title='eEm TOB rHad isolation;rHad isolation;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('eEmTOBWsTot', path='EXPERT', type='TH1I',
                                title='eEm TOB WsTot isolation;WsTot isolation;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('eEmTOBEta, eEmTOBPhi; eEmTOBPhiEta', path='EXPERT', type='TH2I',
                                title='eEm TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    alg.MonTool.defineHistogram('eEmTOBEta, eEmTOBEt; eEmTOBEtEta', path='EXPERT', type='TH2I',
                                title='eEm TOB Et vs eta;#eta#times40;E_{t} [GeV]',
                                xbins=200, xmin=-200, xmax=200, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eEmTOBPhi, eEmTOBEt; eEmTOBEtPhi', path='EXPERT', type='TH2I',
                                title='eEm TOB Et vs phi;#phi#times20;E_{t} [GeV]',
                                xbins=128, xmin=0, xmax=128, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eEmTOBREta, eEmTOBEt; eEmTOBEtREta', path='EXPERT', type='TH2I',
                                title='eEm TOB Et vs rEta isolation;rEta isolation;E_{t} [GeV]',
                                xbins=4, xmin=0, xmax=4, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eEmTOBRHad, eEmTOBEt; eEmTOBEtRHad', path='EXPERT', type='TH2I',
                                title='eEm TOB Et vs rHad isolation;rHad isolation;E_{t} [GeV]',
                                xbins=4, xmin=0, xmax=4, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eEmTOBWsTot, eEmTOBEt; eEmTOBEtWsTot', path='EXPERT', type='TH2I',
                                title='eEm TOB Et vs WsTot isolation;WsTot isolation;E_{t} [GeV]',
                                xbins=4, xmin=0, xmax=4, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eTauTOBEt', path='EXPERT', type='TH1I',
                                title='eTau TOB Et;E_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('eTauTOBRCore', path='EXPERT', type='TH1I',
                                title='eTau TOB rCore isolation;rCore isolation;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('eTauTOBRHad', path='EXPERT', type='TH1I',
                                title='eTau TOB rHad isolation;rHad isolation;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('eTauTOBEta, eTauTOBPhi; eTauTOBPhiEta', path='EXPERT', type='TH2I',
                                title='eTau TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    alg.MonTool.defineHistogram('eTauTOBEta, eTauTOBEt; eTauTOBEtEta', path='EXPERT', type='TH2I',
                                title='eTau TOB Et vs eta;#eta#times40;E_{t} [GeV]',
                                xbins=200, xmin=-200, xmax=200, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eTauTOBPhi, eTauTOBEt; eTauTOBEtPhi', path='EXPERT', type='TH2I',
                                title='eTau TOB Et vs phi;#phi#times20;E_{t} [GeV]',
                                xbins=128, xmin=0, xmax=128, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eTauTOBRCore, eTauTOBEt; eTauTOBEtRCore', path='EXPERT', type='TH2I',
                                title='eTau TOB Et vs rCore isolation;rCore isolation;E_{t} [GeV]',
                                xbins=4, xmin=0, xmax=4, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('eTauTOBRHad, eTauTOBEt; eTauTOBEtRHad', path='EXPERT', type='TH2I',
                                title='eTau TOB Et vs rHad isolation;rHad isolation;E_{t} [GeV]',
                                xbins=4, xmin=0, xmax=4, ybins=200, ymin=0, ymax=400)

def configurejFexInputProviderHistograms(alg, flags):
    # jJet
    alg.MonTool.defineHistogram('jJetTOBPt', path='EXPERT', type='TH1I',
                                title='jJet TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('jJetTOBEta, jJetTOBPhi; jJetTOBPhiEta', path='EXPERT', type='TH2I',
                                title='jJet TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    # jLJet
    alg.MonTool.defineHistogram('jLJetTOBPt', path='EXPERT', type='TH1I',
                                title='jLJet TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=1000)
    alg.MonTool.defineHistogram('jLJetTOBEta, jLJetTOBPhi; jLJetTOBPhiEta', path='EXPERT', type='TH2I',
                                title='jLJet TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    # jTau
    alg.MonTool.defineHistogram('jTauTOBPt', path='EXPERT', type='TH1I',
                                title='jTau TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('jTauTOBIsolation', path='EXPERT', type='TH1I',
                                title='jTau TOB Isolation;Isolation [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('jTauTOBEta, jTauTOBPhi; jTauTOBPhiEta', path='EXPERT', type='TH2I',
                                title='jTau TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    alg.MonTool.defineHistogram('jTauTOBEta, jTauTOBIsolation; jTauTOBIsolationEta', path='EXPERT', type='TH2I',
                                title='jTau TOB Isolation vs eta;#eta#times40;Isolation [GeV]',
                                xbins=200, xmin=-200, xmax=200, ybins=200, ymin=0, ymax=400)
    # jEm
    alg.MonTool.defineHistogram('jEmTOBPt', path='EXPERT', type='TH1I',
                                title='jEm TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('jEmTOBIsolation', path='EXPERT', type='TH1I',
                                title='jEm TOB Isolation;Isolation;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('jEmTOBFrac1', path='EXPERT', type='TH1I',
                                title='jEm TOB Frac1;Frac1;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('jEmTOBFrac2', path='EXPERT', type='TH1I',
                                title='jEm TOB Frac2;Frac2;', xbins=4, xmin=0, xmax=4)
    alg.MonTool.defineHistogram('jEmTOBEta, jEmTOBPhi; jEmTOBPhiEta', path='EXPERT', type='TH2I',
                                title='jEm TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    alg.MonTool.defineHistogram('jEmTOBEta, jEmTOBIsolation; jEmTOBIsolationEta', path='EXPERT', type='TH2I',
                                title='jEm TOB Isolation vs eta;#eta#times40;Isolation',
                                xbins=200, xmin=-200, xmax=200, ybins=4, ymin=0, ymax=4)
    alg.MonTool.defineHistogram('jEmTOBEta, jEmTOBFrac1; jEmTOBFrac1Eta', path='EXPERT', type='TH2I',
                                title='jEm TOB Frac1 vs eta;#eta#times40;Frac1',
                                xbins=200, xmin=-200, xmax=200, ybins=4, ymin=0, ymax=4)
    alg.MonTool.defineHistogram('jEmTOBEta, jEmTOBFrac2; jEmTOBFrac2Eta', path='EXPERT', type='TH2I',
                                title='jEm TOB Frac2 vs eta;#eta#times40;Frac2',
                                xbins=200, xmin=-200, xmax=200, ybins=4, ymin=0, ymax=4)
    # jXE
    alg.MonTool.defineHistogram('jXETOBPt', path='EXPERT', type='TH1I',
                                title='jXE TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=2000)
    alg.MonTool.defineHistogram('jXETOBPhi', path='EXPERT', type='TH1I',
                                title='jXE TOB Phi;#phi;', xbins=64, xmin=-3.2, xmax=3.2)
    # jTE
    alg.MonTool.defineHistogram('jTETOBsaturation', path='EXPERT', type='TH1I',
                                title='jTE TOB saturation;jTE TOB saturation;', xbins=3, xmin=0, xmax=3)
    alg.MonTool.defineHistogram('jTETOBsumEt', path='EXPERT', type='TH1I',
                                title='jTE TOB sumEt;p_{T} [GeV];', xbins=400, xmin=0, xmax=4000)
    # jTEC
    alg.MonTool.defineHistogram('jTECTOBsumEt', path='EXPERT', type='TH1I',
                                title='jTEC TOB sumEt;p_{T} [GeV];', xbins=400, xmin=0, xmax=4000)
    # jTEFWD
    alg.MonTool.defineHistogram('jTEFWDTOBsumEt', path='EXPERT', type='TH1I',
                                title='jTEFWD TOB sumEt;p_{T} [GeV];', xbins=400, xmin=0, xmax=4000)
    # jTEFWDA
    alg.MonTool.defineHistogram('jTEFWDATOBsumEt', path='EXPERT', type='TH1I',
                                title='jTEFWDA TOB sumEt;p_{T} [GeV];', xbins=400, xmin=0, xmax=4000)
    # jTEFWDC
    alg.MonTool.defineHistogram('jTEFWDCTOBsumEt', path='EXPERT', type='TH1I',
                                title='jTEFWDC TOB sumEt;p_{T} [GeV];', xbins=400, xmin=0, xmax=4000)

def configuregFexInputProviderHistograms(alg, flags):
    # gJet
    alg.MonTool.defineHistogram('gJetTOBPt', path='EXPERT', type='TH1I',
                                title='gJet TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('gJetTOBEta, gJetTOBPhi; gJetTOBPhiEta', path='EXPERT', type='TH2I',
                                title='gJet TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    # gLJet
    alg.MonTool.defineHistogram('gLJetTOBPt', path='EXPERT', type='TH1I',
                                title='gLJet TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=1000)
    alg.MonTool.defineHistogram('gLJetTOBEta, gLJetTOBPhi; gLJetTOBPhiEta', path='EXPERT', type='TH2I',
                                title='gLJet TOB Location;#eta#times40;#phi#times20',
                                xbins=200, xmin=-200, xmax=200, ybins=128, ymin=0, ymax=128)
    # gXEJWOJ
    alg.MonTool.defineHistogram('gXEJWOJTOBPt', path='EXPERT', type='TH1I',
                                title='gXEJWOJ TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=2000)
    alg.MonTool.defineHistogram('gXEJWOJTOBPhi', path='EXPERT', type='TH1I',
                                title='gXEJWOJ TOB Phi;#phi;', xbins=64, xmin=-3.2, xmax=3.2)
    # gMHT
    alg.MonTool.defineHistogram('gMHTTOBPt', path='EXPERT', type='TH1I',
                                title='gMHT TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=2000)
    alg.MonTool.defineHistogram('gMHTTOBPhi', path='EXPERT', type='TH1I',
                                title='gMHT TOB Phi;#phi;', xbins=64, xmin=-3.2, xmax=3.2)
    # gXENC
    alg.MonTool.defineHistogram('gXENCTOBPt', path='EXPERT', type='TH1I',
                                title='gXENC TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=2000)
    # gXERHO
    alg.MonTool.defineHistogram('gXERHOTOBPt', path='EXPERT', type='TH1I',
                                title='gXERHO TOB Pt;p_{T} [GeV];', xbins=200, xmin=0, xmax=2000)
    # gTE
    alg.MonTool.defineHistogram('gTETOBsumEt', path='EXPERT', type='TH1I',
                                title='gTE TOB sumEt;p_{T} [GeV];', xbins=400, xmin=0, xmax=4000)

def configureEnergyInputProviderHistograms(alg, flags):
    alg.MonTool.defineHistogram('MET', path='EXPERT', type='TH1I',
                                title='Missing ET TOB;p_{T} [GeV];', xbins=200, xmin=0, xmax=2000)
    alg.MonTool.defineHistogram('METPhi', path='EXPERT', type='TH1I',
                                title='MET TOB Phi;#phi;', xbins=64, xmin=-3.2, xmax=3.2)

def configureEMTauInputProviderHistograms(alg, flags):
    alg.MonTool.defineHistogram('EMTOBEt', path='EXPERT', type='TH1I',
                                title='EM TOB Et;E_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('EMTOBEta, EMTOBPhi; EMTOBPhiEta', path='EXPERT', type='TH2I',
                                title='EM TOB Location;#eta#times10;#phi#times10',
                                xbins=200, xmin=-50, xmax=50, ybins=128, ymin=0, ymax=64)
    alg.MonTool.defineHistogram('EMTOBEta, EMTOBEt; EMTOBEtEta', path='EXPERT', type='TH2I',
                                title='Et vs eta;#eta#times10;E_{t} [GeV]',
                                xbins=200, xmin=-50, xmax=50, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('EMTOBPhi, EMTOBEt; EMTOBEtPhi', path='EXPERT', type='TH2I',
                                title='Et vs phi;#phi#times10;E_{t} [GeV]',
                                xbins=128, xmin=0, xmax=64, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('TauTOBEt', path='EXPERT', type='TH1I',
                                title='Tau TOB Et;E_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('TauTOBEta, TauTOBPhi; TauTOBPhiEta', path='EXPERT', type='TH2I',
                                title='Tau TOB Location;#eta#times10;#phi#times10',
                                xbins=200, xmin=-50, xmax=50, ybins=128, ymin=0, ymax=64)
    alg.MonTool.defineHistogram('TauTOBEta, TauTOBEt; TauTOBEtEta', path='EXPERT', type='TH2I',
                                title='Et vs eta;#eta#times10;E_{t} [GeV]',
                                xbins=200, xmin=-50, xmax=50, ybins=200, ymin=0, ymax=400)
    alg.MonTool.defineHistogram('TauTOBPhi, TauTOBEt; TauTOBEtPhi', path='EXPERT', type='TH2I',
                                title='Et vs phi;#phi#times10;E_{t} [GeV]',
                                xbins=128, xmin=0, xmax=64, ybins=200, ymin=0, ymax=400)

def configureJetInputProviderHistograms(alg, flags):
    alg.MonTool.defineHistogram('TOBPt1', path='EXPERT', type='TH1I',
                                title='Jet TOB Pt 1;p_{T} [GeV];', xbins=200, xmin=0, xmax=1000)
    alg.MonTool.defineHistogram('TOBPt2', path='EXPERT', type='TH1I',
                                title='Jet TOB Pt 2;p_{T} [GeV];', xbins=200, xmin=0, xmax=400)
    alg.MonTool.defineHistogram('TOBEta, TOBPhi; TOBPhiEta', path='EXPERT', type='TH2I',
                                title='Jet TOB Location;#eta#times10;#phi#times10',
                                xbins=200, xmin=-50, xmax=50, ybins=128, ymin=0, ymax=64)