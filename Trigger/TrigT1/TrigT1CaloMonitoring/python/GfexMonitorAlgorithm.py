#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
def GfexMonitoringConfig(inputFlags):
    '''Function to configure LVL1 Gfex algorithm in the monitoring system.'''

    import math

    # get the component factory - used for merging the algorithm results
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()
    
    # uncomment if you want to see all the flags
    #inputFlags.dump() # print all the configs

    # make the athena monitoring helper
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'GfexMonitoringCfg') 
 
    # get any algorithms
    GfexMonAlg = helper.addAlgorithm(CompFactory.GfexMonitorAlgorithm,'GfexMonAlg')

    # add any steering
    groupName = 'GfexMonitor' # the monitoring group name is also used for the package name
    GfexMonAlg.PackageName = groupName

    mainDir = 'L1Calo'
    trigPath = 'Gfex/'

    etabins=32
    etamin=-3.3
    etamax=3.3

    # add monitoring algorithm to group, with group name and main directory 
    myGroup = helper.addGroup(GfexMonAlg, groupName , mainDir)

    # define gfex histograms
    
    ######  gJ  ######
    # Inclusive pT
    myGroup.defineHistogram('gFexSRJetTransverseEnergy;h_gFexSRJetTransverseEnergy', title='gFex SRJet Transverse Energy; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    # pt histograms with pt cuts
    myGroup.defineHistogram("gFexSRJetPtCutPt0;h_gFexSRJetPtCutPt0"    , title="gFex SRJet Transverse Energy - tobEt [200 MeV Scale]>0; #Pt; counts"   , type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram("gFexSRJetPtCutPt10;h_gFexSRJetPtCutPt10"  , title="gFex SRJet Transverse Energy - tobEt [200 MeV Scale]>10; #Pt; counts"  , type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram("gFexSRJetPtCutPt50;h_gFexSRJetPtCutPt50"  , title="gFex SRJet Transverse Energy - tobEt [200 MeV Scale]>50; #Pt; counts"  , type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram("gFexSRJetPtCutPt100;h_gFexSRJetPtCutPt100", title="gFex SRJet Transverse Energy - tobEt [200 MeV Scale]>100; #Pt; counts" , type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)

    # pT with FPGA region cuts
    myGroup.defineHistogram('gFexSRJetEtCutFPGAa;h_gFexSRJetEtCutFPGAa', title='gFex SRJet Transverse Energy - FPGA A; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexSRJetEtCutFPGAb;h_gFexSRJetEtCutFPGAb', title='gFex SRJet Transverse Energy - FPGA B; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexSRJetEtCutFPGAc;h_gFexSRJetEtCutFPGAc', title='gFex SRJet Transverse Energy - FPGA C; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    # Same as above, but requiring non-zero objects by enforcing pt>0
    myGroup.defineHistogram('gFexSRJetEtCutFPGAaPt0;h_gFexSRJetEtCutFPGAaPt0', title='gFex SRJet Transverse Energy - FPGA A, tobEt>0; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexSRJetEtCutFPGAbPt0;h_gFexSRJetEtCutFPGAbPt0', title='gFex SRJet Transverse Energy - FPGA B, tobEt>0; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexSRJetEtCutFPGAcPt0;h_gFexSRJetEtCutFPGAcPt0', title='gFex SRJet Transverse Energy - FPGA C, tobEt>0; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=100,xmin=-1,xmax=4096)

    # Inclusive eta
    myGroup.defineHistogram('gFexSRJetEta;h_gFexSRJetEta', title='gFex SRJet #eta; #eta; counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    # Eta histograms with pt cuts
    myGroup.defineHistogram("gFexSRJetEtaCutPt0;h_gFexSRJetEtaCutPt0"    , title="gFex SRJet #eta - tobEt [200 MeV Scale]>0; #eta; counts"  , type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutPt10;h_gFexSRJetEtaCutPt10"  , title="gFex SRJet #eta - tobEt [200 MeV Scale]>10; #eta; counts" , type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutPt50;h_gFexSRJetEtaCutPt50"  , title="gFex SRJet #eta - tobEt [200 MeV Scale]>50; #eta; counts" , type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutPt100;h_gFexSRJetEtaCutPt100", title="gFex SRJet #eta - tobEt [200 MeV Scale]>100; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    # Eta histograms with FPGA region cuts
    myGroup.defineHistogram("gFexSRJetEtaCutFPGAa;h_gFexSRJetEtaCutFPGAa", title="gFex SRJet #eta - FPGA A; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutFPGAb;h_gFexSRJetEtaCutFPGAb", title="gFex SRJet #eta - FPGA B; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutFPGAc;h_gFexSRJetEtaCutFPGAc", title="gFex SRJet #eta - FPGA C; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    
    myGroup.defineHistogram("gFexSRJetEtaCutFPGAaCutPt0;h_gFexSRJetEtaCutFPGAaCutPt0", title="gFex SRJet #eta - FPGA A, tobEt>0; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutFPGAbCutPt0;h_gFexSRJetEtaCutFPGAbCutPt0", title="gFex SRJet #eta - FPGA B, tobEt>0; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexSRJetEtaCutFPGAcCutPt0;h_gFexSRJetEtaCutFPGAcCutPt0", title="gFex SRJet #eta - FPGA C, tobEt>0; #eta; counts", type='TH1F', path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax)



    # Inclusive phi
    myGroup.defineHistogram('gFexSRJetPhi;h_gFexSRJetPhi', title='gFex SRJet #phi; #phi; counts',
                            type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    # Phi histograms with pt cuts
    myGroup.defineHistogram("gFexSRJetPhiCutPt0;h_gFexSRJetPhiCutPt0"    , title="gFex SRJet #phi - tobEt [200 MeV Scale]>0; #phi; counts"  , type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutPt10;h_gFexSRJetPhiCutPt10"  , title="gFex SRJet #phi - tobEt [200 MeV Scale]>10; #phi; counts" , type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutPt50;h_gFexSRJetPhiCutPt50"  , title="gFex SRJet #phi - tobEt [200 MeV Scale]>50; #phi; counts" , type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutPt100;h_gFexSRJetPhiCutPt100", title="gFex SRJet #phi - tobEt [200 MeV Scale]>100; #phi; counts", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    # Phi histograms with FPGA region cuts
    myGroup.defineHistogram("gFexSRJetPhiCutFPGAa;h_gFexSRJetPhiCutFPGAa", title="gFex SRJet #phi - FPGA A", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutFPGAb;h_gFexSRJetPhiCutFPGAb", title="gFex SRJet #phi - FPGA B", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutFPGAc;h_gFexSRJetPhiCutFPGAc", title="gFex SRJet #phi - FPGA C", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    
    myGroup.defineHistogram("gFexSRJetPhiCutFPGAaCutPt0;h_gFexSRJetPhiCutFPGAaCutPt0", title="gFex SRJet #phi - FPGA A, tobEt>0", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutFPGAbCutPt0;h_gFexSRJetPhiCutFPGAbCutPt0", title="gFex SRJet #phi - FPGA B, tobEt>0", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexSRJetPhiCutFPGAcCutPt0;h_gFexSRJetPhiCutFPGAcCutPt0", title="gFex SRJet #phi - FPGA C, tobEt>0", type='TH1F', path=trigPath+"gJ/", xbins=32,xmin=-math.pi,xmax=math.pi)

    # Eta/phi maps inclusive
    myGroup.defineHistogram('gFexSRJetEta,gFexSRJetPhi;h_gFexSRJetEtaPhiMap_weighted', title="gFexSRJet #eta vs #phi;gFexSRJet #eta;gFexSRJet #phi;gFexSRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gJ/",weight="gFexSRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEta,gFexSRJetPhi;h_gFexSRJetEtaPhiMap', title="gFexSRJet #eta vs #phi;gFexSRJet #eta;gFexSRJet #phi",
                            type='TH2F',path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    # Eta/phi maps with pt cuts
    myGroup.defineHistogram('gFexSRJetEtaCutPt0,gFexSRJetPhiCutPt0;h_gFexSRJetEtaPhiMapCutPt0_weighted', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>0;gFexSRJet #eta;gFexSRJet #phi;gFexSRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gJ/",weight="gFexSRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt0,gFexSRJetPhiCutPt0;h_gFexSRJetEtaPhiMapCutPt0', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>0;gFexSRJet #eta;gFexSRJet #phi",
                            type='TH2F',path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt10,gFexSRJetPhiCutPt10;h_gFexSRJetEtaPhiMapCutPt10_weighted', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>10;gFexSRJet #eta;gFexSRJet #phi;gFexSRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gJ/",weight="gFexSRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt10,gFexSRJetPhiCutPt10;h_gFexSRJetEtaPhiMapCutPt10', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>10;gFexSRJet #eta;gFexSRJet #phi",
                            type='TH2F',path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt50,gFexSRJetPhiCutPt50;h_gFexSRJetEtaPhiMapCutPt50_weighted', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>50;gFexSRJet #eta;gFexSRJet #phi;gFexSRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gJ/",weight="gFexSRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt50,gFexSRJetPhiCutPt50;h_gFexSRJetEtaPhiMapCutPt50', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>50;gFexSRJet #eta;gFexSRJet #phi",
                            type='TH2F',path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt100,gFexSRJetPhiCutPt100;h_gFexSRJetEtaPhiMapCutPt100_weighted', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>100;gFexSRJet #eta;gFexSRJet #phi;gFexSRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gJ/",weight="gFexSRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexSRJetEtaCutPt100,gFexSRJetPhiCutPt100;h_gFexSRJetEtaPhiMapCutPt100', title="gFexSRJet #eta vs #phi - tobEt [200 MeV Scale]>100;gFexSRJet #eta;gFexSRJet #phi",
                            type='TH2F',path=trigPath+"gJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)


    ######  gLJ  ######
    # Inclusive pT
    myGroup.defineHistogram('gFexLRJetTransverseEnergy;h_gFexLRJetTransverseEnergy', title='gFex LRJet Transverse Energy; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)

    # pt histograms with pt cuts
    myGroup.defineHistogram("gFexLRJetPtCutPt0;h_gFexLRJetPtCutPt0"    , title="gFex LRJet Transverse Energy - tobEt [200 MeV Scale]>0; #Pt; counts"  , type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram("gFexLRJetPtCutPt10;h_gFexLRJetPtCutPt10"  , title="gFex LRJet Transverse Energy - tobEt [200 MeV Scale]>10; #Pt; counts" , type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram("gFexLRJetPtCutPt50;h_gFexLRJetPtCutPt50"  , title="gFex LRJet Transverse Energy - tobEt [200 MeV Scale]>50; #Pt; counts" , type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram("gFexLRJetPtCutPt100;h_gFexLRJetPtCutPt100", title="gFex LRJet Transverse Energy - tobEt [200 MeV Scale]>100; #Pt; counts", type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)

    # pT with FPGA region cuts
    myGroup.defineHistogram('gFexLRJetEtCutFPGAa;h_gFexLRJetEtCutFPGAa', title='gFex LRJet Transverse Energy - FPGA A; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexLRJetEtCutFPGAb;h_gFexLRJetEtCutFPGAb', title='gFex LRJet Transverse Energy - FPGA B; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexLRJetEtCutFPGAc;h_gFexLRJetEtCutFPGAc', title='gFex LRJet Transverse Energy - FPGA C; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    # Same as above, but requiring non-zero objects by enforcing pt>0
    myGroup.defineHistogram('gFexLRJetEtCutFPGAaPt0;h_gFexLRJetEtCutFPGAaPt0', title='gFex LRJet Transverse Energy - FPGA A, tobEt>0; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexLRJetEtCutFPGAbPt0;h_gFexLRJetEtCutFPGAbPt0', title='gFex LRJet Transverse Energy - FPGA B, tobEt>0; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)
    myGroup.defineHistogram('gFexLRJetEtCutFPGAcPt0;h_gFexLRJetEtCutFPGAcPt0', title='gFex LRJet Transverse Energy - FPGA C, tobEt>0; tobEt [200 MeV Scale]; Counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=100,xmin=-1,xmax=4096)

    # Inclusive eta
    myGroup.defineHistogram('gFexLRJetEta;h_gFexLRJetEta', title='gFex LRJet #eta; #eta; counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    # Eta histograms with pt cuts
    myGroup.defineHistogram("gFexLRJetEtaCutPt0;h_gFexLRJetEtaCutPt0"    , title="gFex LRJet #eta - tobEt [200 MeV Scale]>0; #eta; counts"  , type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutPt10;h_gFexLRJetEtaCutPt10"  , title="gFex LRJet #eta - tobEt [200 MeV Scale]>10; #eta; counts" , type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutPt50;h_gFexLRJetEtaCutPt50"  , title="gFex LRJet #eta - tobEt [200 MeV Scale]>50; #eta; counts" , type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutPt100;h_gFexLRJetEtaCutPt100", title="gFex LRJet #eta - tobEt [200 MeV Scale]>100; #eta; counts", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    # Eta histograms with FPGA region cuts
    myGroup.defineHistogram("gFexLRJetEtaCutFPGAa;h_gFexLRJetEtaCutFPGAa", title="gFex LRJet #eta - FPGA A", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutFPGAb;h_gFexLRJetEtaCutFPGAb", title="gFex LRJet #eta - FPGA B", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutFPGAc;h_gFexLRJetEtaCutFPGAc", title="gFex LRJet #eta - FPGA C", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    
    myGroup.defineHistogram("gFexLRJetEtaCutFPGAaCutPt0;h_gFexLRJetEtaCutFPGAaCutPt0", title="gFex LRJet #eta - FPGA A, tobEt>0; #eta; counts", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutFPGAbCutPt0;h_gFexLRJetEtaCutFPGAbCutPt0", title="gFex LRJet #eta - FPGA B, tobEt>0; #eta; counts", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)
    myGroup.defineHistogram("gFexLRJetEtaCutFPGAcCutPt0;h_gFexLRJetEtaCutFPGAcCutPt0", title="gFex LRJet #eta - FPGA C, tobEt>0; #eta; counts", type='TH1F', path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax)

    # Inclusive phi
    myGroup.defineHistogram('gFexLRJetPhi;h_gFexLRJetPhi', title='gFex LRJet #phi; #phi; counts',
                            type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    # Phi histograms with pt cuts
    myGroup.defineHistogram("gFexLRJetPhiCutPt0;h_gFexLRJetPhiCutPt0"    , title="gFex LRJet #phi - tobEt [200 MeV Scale]>0; #phi; counts"  , type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutPt10;h_gFexLRJetPhiCutPt10"  , title="gFex LRJet #phi - tobEt [200 MeV Scale]>10; #phi; counts" , type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutPt50;h_gFexLRJetPhiCutPt50"  , title="gFex LRJet #phi - tobEt [200 MeV Scale]>50; #phi; counts" , type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutPt100;h_gFexLRJetPhiCutPt100", title="gFex LRJet #phi - tobEt [200 MeV Scale]>100; #phi; counts", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    # Phi histograms with FPGA region cuts
    myGroup.defineHistogram("gFexLRJetPhiCutFPGAa;h_gFexLRJetPhiCutFPGAa", title="gFex LRJet #phi - FPGA A", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutFPGAb;h_gFexLRJetPhiCutFPGAb", title="gFex LRJet #phi - FPGA B", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutFPGAc;h_gFexLRJetPhiCutFPGAc", title="gFex LRJet #phi - FPGA C", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    
    myGroup.defineHistogram("gFexLRJetPhiCutFPGAaCutPt0;h_gFexLRJetPhiCutFPGAaCutPt0", title="gFex LRJet #phi - FPGA A, tobEt>0", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutFPGAbCutPt0;h_gFexLRJetPhiCutFPGAbCutPt0", title="gFex LRJet #phi - FPGA B, tobEt>0", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)
    myGroup.defineHistogram("gFexLRJetPhiCutFPGAcCutPt0;h_gFexLRJetPhiCutFPGAcCutPt0", title="gFex LRJet #phi - FPGA C, tobEt>0", type='TH1F', path=trigPath+"gLJ/", xbins=32,xmin=-math.pi,xmax=math.pi)

    # Eta/phi maps inclusive
    myGroup.defineHistogram('gFexLRJetEta,gFexLRJetPhi;h_gFexLRJetEtaPhiMap_weighted', title="gFexLRJet #eta vs #phi;gFexLRJet #eta;gFexLRJet #phi;gFexLRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gLJ/",weight="gFexLRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)  

    myGroup.defineHistogram('gFexLRJetEta,gFexLRJetPhi;h_gFexLRJetEtaPhiMap', title="gFexLRJe #eta vs #phi;gFexLRJet #eta;gFexLRJet #phi",
                            type='TH2F',path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    # Eta/phi maps with pt cuts
    myGroup.defineHistogram('gFexLRJetEtaCutPt0,gFexLRJetPhiCutPt0;h_gFexLRJetEtaPhiMapCutPt0_weighted', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>10;gFexLRJet #eta;gFexLRJet #phi;gFexLRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gLJ/",weight="gFexLRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt0,gFexLRJetPhiCutPt0;h_gFexLRJetEtaPhiMapCutPt0', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>0;gFexLRJet #eta;gFexLRJet #phi",
                            type='TH2F',path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt10,gFexLRJetPhiCutPt10;h_gFexLRJetEtaPhiMapCutPt10_weighted', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>10;gFexLRJet #eta;gFexLRJet #phi;gFexLRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gLJ/",weight="gFexLRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt10,gFexLRJetPhiCutPt10;h_gFexLRJetEtaPhiMapCutPt10', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>10;gFexLRJet #eta;gFexLRJet #phi",
                            type='TH2F',path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt50,gFexLRJetPhiCutPt50;h_gFexLRJetEtaPhiMapCutPt50_weighted', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>50;gFexLRJet #eta;gFexLRJet #phi;gFexLRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gLJ/",weight="gFexLRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt50,gFexLRJetPhiCutPt50;h_gFexLRJetEtaPhiMapCutPt50', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>50;gFexLRJet #eta;gFexLRJet #phi",
                            type='TH2F',path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt100,gFexLRJetPhiCutPt100;h_gFexLRJetEtaPhiMapCutPt100_weighted', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>100;gFexLRJet #eta;gFexLRJet #phi;gFexLRJet TransverseEnergy",
                            type='TH2F',path=trigPath+"gLJ/",weight="gFexLRJetTransverseEnergy", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)
    myGroup.defineHistogram('gFexLRJetEtaCutPt100,gFexLRJetPhiCutPt100;h_gFexLRJetEtaPhiMapCutPt100', title="gFexLRJet #eta vs #phi - tobEt [200 MeV Scale]>100;gFexLRJet #eta;gFexLRJet #phi",
                            type='TH2F',path=trigPath+"gLJ/", xbins=etabins,xmin=etamin,xmax=etamax,ybins=32,ymin=-math.pi,ymax=math.pi)

                            
    ######  gXE  ######
    myGroup.defineHistogram('gFexMET;h_gFexMET', title='gFex MET obtained with jets without jets algorithm;MET [MeV];counts',
                            type='TH1F', path=trigPath+"gXE/", xbins=100,xmin=0,xmax=600000)    
    
    myGroup.defineHistogram('gFexMETx;h_gFexMETx', title='gFex MET x obtained with jets without jets algorithm ; MET x [MeV];counts',
                            type='TH1F', path=trigPath+"gXE/", xbins=100,xmin=-300000,xmax=300000)

    myGroup.defineHistogram('gFexMETy;h_gFexMETy', title='gFex MET y obtained with jets without jets algorithm ; MET y [MeV];counts',
                            type='TH1F', path=trigPath+"gXE/", xbins=100,xmin=-300000,xmax=300000)
    
    ######  gTE  ######
    myGroup.defineHistogram('gFexSumET;h_gFexSumET', title='gFex SumET obtained with jets without jets algorithm ; SumET [MeV];counts',
                            type='TH1F', path=trigPath+"gTE/", xbins=100,xmin=0,xmax=5000000)    
    
    ######  gMHT  ######
    myGroup.defineHistogram('gFexMHTx;h_gFexMHTx', title='gFex MHT x obtained with jets without jets algorithm; MHT x [MeV];counts',
                            type='TH1F', path=trigPath+"gMHT/", xbins=100,xmin=-300000,xmax=300000)

    myGroup.defineHistogram('gFexMHTy;h_gFexMHTy', title='gFex MHT y obtained with jets without jets algorithm;MHT y [MeV];counts',
                            type='TH1F', path=trigPath+"gMHT/", xbins=100,xmin=-300000,xmax=300000)    
    
    ######  gMST  ######
    myGroup.defineHistogram('gFexMSTx;h_gFexMSTx', title='gFex MST x obtained with jets without jets algorithm;MST x [MeV];counts',
                            type='TH1F', path=trigPath+"gMST/", xbins=100,xmin=-200000,xmax=200000)

    myGroup.defineHistogram('gFexMSTy;h_gFexMSTy', title='gFex MST y obtained with jets without jets algorithm;MST y [MeV];counts',
                            type='TH1F', path=trigPath+"gMST/", xbins=100,xmin=-200000,xmax=200000)    
    
    ######  gXE_NoiseCut  ######
    myGroup.defineHistogram('gFexMETx_NoiseCut;h_gFexMETx_NoiseCut', title='gFex MET x obtained with noise cut algorithm;MET x [MeV]; counts',
                            type='TH1F', path=trigPath+"gXE_NoiseCut/", xbins=100,xmin=-600000,xmax=600000)

    myGroup.defineHistogram('gFexMETy_NoiseCut;h_gFexMETy_NoiseCut', title='gFex MET y obtained with noise cut algorithm;MET y [MeV]; counts',
                            type='TH1F', path=trigPath+"gXE_NoiseCut/", xbins=100,xmin=-600000,xmax=600000)    
                            
    myGroup.defineHistogram('gFexMET_NoiseCut;h_gFexMET_NoiseCut', title='gFex MET [MeV] obtained with noise cut algorithm;MET [MeV]; counts',
                            type='TH1F', path=trigPath+"gXE_NoiseCut/", xbins=100,xmin=-10000,xmax=1000000)
                            
                                
    ######  gXE_RMS  ######

    myGroup.defineHistogram('gFexMETx_ComponentsRms;h_gFexMETx_ComponentsRms', title='gFex MET x obtained with Rho+RMS algorithm;MET x [MeV]; counts',
                            type='TH1F', path=trigPath+"gXE_RMS/", xbins=100,xmin=-600000,xmax=600000)    
                            
    myGroup.defineHistogram('gFexMETy_ComponentsRms;h_gFexMETy_ComponentsRms', title='gFex MET y obtained with Rho+RMS algorithm;MET y [MeV]; counts',
                            type='TH1F', path=trigPath+"gXE_RMS/", xbins=100,xmin=-600000,xmax=600000)    
                            
                            
    ######  gTE_NoiseCut  ######
    myGroup.defineHistogram('gFexSumET_NoiseCut;h_gFexSumET_NoiseCut', title='gFex SumET [MeV] obtained with noise cut algorithm;Sum ET [MeV]; counts',
                            type='TH1F', path=trigPath+"gTE_NoiseCut/", xbins=100,xmin=0,xmax=2500000)    
    
    
    ######  gTE_RMS  ######
    myGroup.defineHistogram('gFexMET_Rms;h_gFexMET_Rms', title='gFex MET [MeV] obtained with Rho+RMS algorithm;MET [MeV]; counts',
                            type='TH1F', path=trigPath+"gTE_RMS/", xbins=100,xmin=-100000,xmax=1000000)

    myGroup.defineHistogram('gFexSumET_Rms;h_gFexSumET_Rms', title='gFex SumET [MeV] obtained with Rho+RMS algorithm;Sum ET [MeV]; counts',
                            type='TH1F', path=trigPath+"gTE_RMS/", xbins=100,xmin=0,xmax=2000000)




    ######  gRHO  ######
    myGroup.defineHistogram('gFexRhoTransverseEnergy;h_gFexRhoTransverseEnergy', title='gFex Rho Transverse Energy; gFexRho Et [MeV]; counts',
                            type='TH1F', path=trigPath+"gRHO", xbins=100,xmin=-300000,xmax=300000)


    ######  GENERAL HISTOS  ######
    myGroup.defineHistogram('gFexType;h_gFexType', title='gFex Type for all Trigger Objects; gFex Types',
                            xlabels=["gRho", "gBlockLead", "gBlockSub", "gJet"], type='TH1F', path=trigPath, xbins=4,xmin=0,xmax=4)


    acc = helper.result()
    result.merge(acc)
    return result


if __name__=='__main__':
    # set input file and config options
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import glob

    # MCs with SCells
    #inputs = glob.glob('/eos/user/t/thompson/ATLAS/LVL1_mon/valid1.361021.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ1W.recon.ESD.e3569_s3126_d1623_r12488/ESD.24607652._000025.pool.root.1')
    #inputs = glob.glob('/eos/atlas/atlascerngroupdisk/det-l1calo/OfflineSoftware/mc16_13TeV.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.recon.ESD.e3601_s3126_r12406/ESD.24368002._000045.pool.root.1')
    #
    # Above MCs processed adding L1_eEMRoI
    inputs = glob.glob('/afs/cern.ch/user/t/thompson/work/public/LVL1_monbatch/run_sim/l1calo.361106.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zee.ESD.eFex_2021-05-16T2101.root')
    
    flags = initConfigFlags()
    flags.Input.Files = inputs
    flags.Output.HISTFileName = 'ExampleMonitorOutput.root'

    flags.lock()
    flags.dump() # print all the configs

    from AthenaCommon.AppMgr import ServiceMgr
    ServiceMgr.Dump = False

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg  
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    GfexMonitorCfg = GfexMonitoringConfig(flags)
    cfg.merge(GfexMonitorCfg)

    # options - print all details of algorithms, very short summary 
    cfg.printConfig(withDetails=False, summariseProps = True)

    nevents=100
    cfg.run(nevents)

