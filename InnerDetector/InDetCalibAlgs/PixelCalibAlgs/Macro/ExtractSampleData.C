/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <iostream>


void ExtractSampleData(){
  //selection of part corresponds to original code, i.e. BLayer, indicated by WhichPart=1
  std::string inThrFile;
  std::string inTimFile;
  std::string inTotFile;
  //
  std::string exThrFile;
  std::string exTimFile;
  std::string exTotFile;


 std::cout << "Running on BLayer" << std::endl;
 inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087719.root";
 inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087717.root";
 inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087710.root";
 exThrFile = "SAMPLE_S000087719.root";
 exTimFile = "SAMPLE_S000087717.root";
 exTotFile = "SAMPLE_S000087710.root";

  //
  const std::array<std::string,2> rodNames{"ROD_B2_S10", "ROD_B2_S11"};
  //
  //
  TFile riThrFile(inThrFile.c_str(),"READ");
  if (not riThrFile.IsOpen()){
    std::cout<<"File "<<inThrFile<<" could not be opened."<<std::endl;
    return;
  } else {
    std::cout<<"File "<<inThrFile<<" opened."<<std::endl;
  }
  TFile fhThrFile(exThrFile.c_str(),"RECREATE");
  //
  TDirectoryFile* scanDir = (TDirectoryFile*)((TKey*)riThrFile.GetListOfKeys()->First())->ReadObj();
  TString topDirName = scanDir->GetName();
  std::cout<<"Top Directory name "<<topDirName<<std::endl;
  auto pTopDir= fhThrFile.mkdir(topDirName);
  pTopDir->cd();
  TList* rodKeyList = (TList*)scanDir->GetListOfKeys();
  TIter rodItr(rodKeyList);
  auto getKey = [&rodKeyList](const std::string & s)-> TKey *{
    return (TKey *) rodKeyList->FindObject(s.c_str());
  };
  std::array<TKey*, 2> rodKeys{getKey(rodNames[0]),getKey(rodNames[1])};
  TKey* pRodKey{};
  //Same for Thr and Tim files
  TString chi2HistName = "SCURVE_CHI2";
  TString thrHistName = "SCURVE_MEAN";
  TString sigHistName = "SCURVE_SIGMA";
  TString subdir = "/A0/B0";
  //
  
  for (auto pRodKey:rodKeys) {  //iterate over two rods
    TString thisRodName = pRodKey->GetName();
    std::cout<<"Writing threshold scan data for "<<thisRodName<<std::endl;
    auto pNewRodDir=pTopDir->mkdir(thisRodName);
    pNewRodDir->cd();
    auto pRodDir = (TDirectoryFile*)pRodKey->ReadObj();
    auto pModKeyList = pRodDir->GetListOfKeys();
    TIter modItr(pModKeyList);
    for (auto j:{0,1,2}) { //iterate over first three modules in rods
      auto modKey=(TKey*)modItr(); 
      std::string modStr(modKey->GetName());
      
      std::cout<<"Module: "<<modStr<<std::endl;
      TString modName(modStr);
      TString modSubdir = "/"+chi2HistName + subdir;
      TString chi2HistDirPath = modName +  modSubdir;
      auto pNewHistoDir=pNewRodDir->mkdir(chi2HistDirPath);
      pNewHistoDir->cd();
      gDirectory->cd(chi2HistName+subdir);
      TDirectoryFile *chi2HistDir = (TDirectoryFile *)pRodDir->Get(chi2HistDirPath);
      TH2F *h2dChi2 = (TH2F *)((TKey *)chi2HistDir->GetListOfKeys()->First())->ReadObj();
      TString n=h2dChi2->GetName();
      gDirectory->WriteObject(h2dChi2, n);
      // chi2HistMap[modName] = h2dChi2;
      TString thrHistDirPath = modName + "/" + thrHistName + subdir;
      pNewHistoDir=pNewRodDir->mkdir(thrHistDirPath);
      pNewHistoDir->cd();
      gDirectory->cd(thrHistName+subdir);
      TDirectoryFile *thrHistDir = (TDirectoryFile *)pRodDir->Get(thrHistDirPath);
      TH2F *h2dThr = (TH2F *)((TKey *)thrHistDir->GetListOfKeys()->First())->ReadObj();
      n=h2dThr->GetName();
      gDirectory->WriteObject(h2dThr, n);
        // thrHistMap[modName] = h2dThr;
      TString sigHistDirPath = modName + "/" + sigHistName + subdir;
      pNewHistoDir=pNewRodDir->mkdir(sigHistDirPath);
      pNewHistoDir->cd();
      gDirectory->cd(sigHistName+subdir);
      TDirectoryFile *sigHistDir = (TDirectoryFile *)pRodDir->Get(sigHistDirPath);
      TH2F *h2dSig = (TH2F *)((TKey *)sigHistDir->GetListOfKeys()->First())->ReadObj();
      n=h2dSig->GetName();
      gDirectory->WriteObject(h2dSig, n);
    }
  }
  riThrFile.Close();
  fhThrFile.Close();
  //
  //Now the 'Tim' file...
  TFile riTimFile(inTimFile.c_str(),"READ");
  if (not riTimFile.IsOpen()){
    std::cout<<"File "<<inTimFile<<" could not be opened."<<std::endl;
    return;
  } else {
    std::cout<<"File "<<inTimFile<<" opened."<<std::endl;
  }
  TFile fhTimFile(exTimFile.c_str(),"RECREATE");
  scanDir = (TDirectoryFile*)((TKey*)riTimFile.GetListOfKeys()->First())->ReadObj();
  topDirName = scanDir->GetName();
  std::cout<<"Top Directory name "<<topDirName<<std::endl;
  pTopDir= fhTimFile.mkdir(topDirName);
  pTopDir->cd();
  rodKeyList = (TList*)scanDir->GetListOfKeys();
  rodItr=TIter(rodKeyList);
  //
  rodKeys={getKey(rodNames[0]),getKey(rodNames[1])};
  for (auto pRodKey:rodKeys) {  //iterate over two rods
    TString thisRodName = pRodKey->GetName();
    std::cout<<"Writing timing scan data for "<<thisRodName<<std::endl;
    auto pNewRodDir=pTopDir->mkdir(thisRodName);
    pNewRodDir->cd();
    auto pRodDir = (TDirectoryFile*)pRodKey->ReadObj();
    auto pModKeyList = pRodDir->GetListOfKeys();
    TIter modItr(pModKeyList);
    
    for (auto j:{0,1,2}) { //iterate over first three modules in rods
      auto modKey=(TKey*)modItr(); 
      std::string modStr(modKey->GetName());
      
      std::cout<<"Module: "<<modStr<<std::endl;
      TString modName(modStr);
      TString modSubdir = "/"+chi2HistName + subdir;
      TString chi2HistDirPath = modName +  modSubdir;
      auto pNewHistoDir=pNewRodDir->mkdir(chi2HistDirPath);
      pNewHistoDir->cd();
      gDirectory->cd(chi2HistName+subdir);
      TDirectoryFile *chi2HistDir = (TDirectoryFile *)pRodDir->Get(chi2HistDirPath);
      TH2F *h2dChi2 = (TH2F *)((TKey *)chi2HistDir->GetListOfKeys()->First())->ReadObj();
      TString n=h2dChi2->GetName();
      gDirectory->WriteObject(h2dChi2, n);
      // chi2HistMap[modName] = h2dChi2;
      TString thrHistDirPath = modName + "/" + thrHistName + subdir;
      pNewHistoDir=pNewRodDir->mkdir(thrHistDirPath);
      pNewHistoDir->cd();
      gDirectory->cd(thrHistName+subdir);
      TDirectoryFile *thrHistDir = (TDirectoryFile *)pRodDir->Get(thrHistDirPath);
      TH2F *h2dThr = (TH2F *)((TKey *)thrHistDir->GetListOfKeys()->First())->ReadObj();
      n=h2dThr->GetName();
      gDirectory->WriteObject(h2dThr, n);
        // thrHistMap[modName] = h2dThr;
      TString sigHistDirPath = modName + "/" + sigHistName + subdir;
      pNewHistoDir=pNewRodDir->mkdir(sigHistDirPath);
      pNewHistoDir->cd();
      gDirectory->cd(sigHistName+subdir);
      TDirectoryFile *sigHistDir = (TDirectoryFile *)pRodDir->Get(sigHistDirPath);
      TH2F *h2dSig = (TH2F *)((TKey *)sigHistDir->GetListOfKeys()->First())->ReadObj();
      n=h2dSig->GetName();
      gDirectory->WriteObject(h2dSig, n);
    }
  }
  riTimFile.Close();
  fhTimFile.Close();
  
  //Now the 'Tot' file...
  TFile riTotFile(inTotFile.c_str(),"READ");
  if (not riTotFile.IsOpen()){
    std::cout<<"File "<<inTotFile<<" could not be opened."<<std::endl;
    return;
  } else {
    std::cout<<"File "<<inTotFile<<" opened."<<std::endl;
  }
  TFile fhTotFile(exTotFile.c_str(),"RECREATE");
  scanDir = (TDirectoryFile*)((TKey*)riTotFile.GetListOfKeys()->First())->ReadObj();
  topDirName = scanDir->GetName();
  std::cout<<"Top Directory name "<<topDirName<<std::endl;
  pTopDir= fhTotFile.mkdir(topDirName);
  pTopDir->cd();
  rodKeyList = (TList*)scanDir->GetListOfKeys();
  rodItr=TIter(rodKeyList);
  //
  TString totHistName="TOT_MEAN";
  sigHistName = "TOT_SIGMA";
  rodKeys={getKey(rodNames[0]),getKey(rodNames[1])};
  for (auto pRodKey:rodKeys) {  //iterate over two rods
    TString thisRodName = pRodKey->GetName();
    std::cout<<"Writing Tot scan data for "<<thisRodName<<std::endl;
    auto pNewRodDir=pTopDir->mkdir(thisRodName);
    pNewRodDir->cd();
    auto pRodDir = (TDirectoryFile*)pRodKey->ReadObj();
    auto pModKeyList = pRodDir->GetListOfKeys();
    TIter modItr(pModKeyList);
        
    for (auto j:{0,1,2}) { //iterate over first three modules in rods
      auto modKey=(TKey*)modItr(); 
      TString modStr(modKey->GetName());
      std::cout<<"Module: "<<modStr<<std::endl;
      auto pNewModDir = pNewRodDir->mkdir(modStr);
      auto pModDir = (TDirectoryFile*)modKey->ReadObj();
      pModDir->cd();
      auto pTotMean = (TDirectoryFile*)((TKey*)pModDir->GetListOfKeys()->First())->ReadObj();
      TString tStr(pTotMean->GetName());
      auto pA = (TDirectoryFile*)((TKey*)pTotMean->GetListOfKeys()->First())->ReadObj();
      TString aStr(pA->GetName());
      auto pB = (TDirectoryFile*)((TKey*)pA->GetListOfKeys()->First())->ReadObj();
      TString bStr(pB->GetName());
      TString modName(modStr);
      auto pNewTotMean=pNewModDir->mkdir("TOT_MEAN");
      auto pNewA = pNewTotMean->mkdir("A0");
      auto pNewB = pNewA->mkdir("B0");
      if (not pNewTotMean){
        std::cout<<"Failed to make directory TOT_MEAN"<<std::endl;
      }
      
      pB->cd();
      //gDirectory->cd(bSubdir);
      auto here = gDirectory->GetName();
      //now need to iterate over the 20 'CX' directories
      auto pCDirList=gDirectory->GetListOfKeys();
      
      TIter cItr(pCDirList);
      
      for (auto k(0);k<20;++k){
        auto thisKey=(TKey*)cItr(); 
        TString cStr(thisKey->GetName());
        auto pNewCDir=pNewB->mkdir(cStr);
        auto pC = (TDirectoryFile*)thisKey->ReadObj();
        TH2F *h2dTot = (TH2F *)((TKey *)pC->GetListOfKeys()->First())->ReadObj();
        TString n=h2dTot->GetName();
        pNewCDir->WriteObject(h2dTot, n);
      }
      
      //
      auto pTotSigma = (TDirectoryFile*)((TKey*)pModDir->GetListOfKeys()->Last())->ReadObj();
      tStr=(pTotSigma->GetName());
      pA = (TDirectoryFile*)((TKey*)pTotSigma->GetListOfKeys()->First())->ReadObj();
      aStr = (pA->GetName());
      pB = (TDirectoryFile*)((TKey*)pA->GetListOfKeys()->First())->ReadObj();
      bStr = (pB->GetName());
      modName = modStr;
      auto pNewTotSigma=pNewModDir->mkdir("TOT_SIGMA");
      pNewA = pNewTotSigma->mkdir("A0");
      pNewB = pNewA->mkdir("B0");
      if (not pNewTotMean){
        std::cout<<"Failed to make directory TOT_SIGMA"<<std::endl;
      }
      
      pB->cd();
      //gDirectory->cd(bSubdir);
      here = gDirectory->GetName();
      //now need to iterate over the 20 'CX' directories
      auto pCDirListS=gDirectory->GetListOfKeys();
      
      TIter cItrS(pCDirListS);
      
      for (auto k(0);k<20;++k){
        auto thisKey=(TKey*)cItrS(); 
        TString cStr(thisKey->GetName());
        auto pNewCDir=pNewB->mkdir(cStr);
        auto pC = (TDirectoryFile*)thisKey->ReadObj();
        TH2F *h2dTot = (TH2F *)((TKey *)pC->GetListOfKeys()->First())->ReadObj();
        TString n=h2dTot->GetName();
        pNewCDir->WriteObject(h2dTot, n);
      }
    }
  }
  riTotFile.Close();
  fhTotFile.Close();

  
  
  
  
  

  return;
}
