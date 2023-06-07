//======================================================================
// PixelCalib : Read calibration file, redo the fit.
//
// Note : this program only runs on Pixel (barrel and endcap) but not IBL.
//
// ~% root -b -q PixelCalib.C
//
// Mapping convention for FE in the calibration.
//
//     ^
//  phi|
//  320|
//     | FE15 FE14 FE13 ... FE9 FE8
//  160|
//     | FE0  FE1  FE2  ... FE6 FE7
//     |
//    0+------------------------------>
//     0    18   36     ...   126 144
//                                  eta
//======================================================================

#include "../common/PixelMapping.h"
#include "TROOT.h"
#include "TKey.h"
#include "TString.h"
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TH2F.h"
#include "TDirectoryFile.h"
#include "TGraphErrors.h"



#include <array>
#include <iostream>
#include <string>
#include <map>
#include <filesystem>

#ifdef ATLAS_GCC_CHECKERS
  #include "CxxUtils/checker_macros.h"
  ATLAS_NO_CHECK_FILE_THREAD_SAFETY;
#endif

namespace fs = std::filesystem;
using pix::PixelMapping; 

enum ChipSettings{THR_NORM, THR_RMS_NORM, THR_SIG_NORM, TIM_NORM, 
                  THR_LONG, THR_RMS_LONG, THR_SIG_LONG, TIM_LONG,
                  THR_GANG, THR_RMS_GANG, THR_SIG_GANG, TIM_GANG,
                  NSETTINGS};
typedef std::array<float, NSETTINGS> OneChipSettings;
typedef std::array<OneChipSettings, 16> OneModuleSettings;
//obviously inefficient; will rethink this later
typedef std::map<std::string,OneModuleSettings > ValueMap_t;


double funcTot(double* x, double* par) {
  double ret = 9.9e10;
  double num = x[0]+par[1];
  double denom = x[0]+par[2];
  if (denom != 0.0) ret = par[0]*(num/denom);
  return ret;
}

double funcDisp(double* x, double* par) {
  double ret = 9.9e10;
  ret = par[0]+par[1]*x[0];
  return ret;
}

std::string getDatetodayStr() {
  time_t t = time(nullptr);
  const tm* localTime = localtime(&t);
  std::stringstream s;
  s << "20" << localTime->tm_year - 100;
  s << std::setw(2) << std::setfill('0') << localTime->tm_mon + 1;
  s << std::setw(2) << std::setfill('0') << localTime->tm_mday;
  return s.str();
}

bool 
moduleNotInPart(int whichPart, const std::string & modName){
  if (modName == "DSP_ERRORS"){
    return true;
  }
  auto notStartsWith=[&modName](const std::string & startsWith)->bool{
    const auto s=startsWith.size();
    return (modName.substr(0,s) != startsWith);
  };
  if (whichPart == 0){
    if (notStartsWith("LI")) return true;
  }
  if (whichPart == 1){
    if (notStartsWith("L0")) return true;
  }
  if (whichPart == 2){
    if (notStartsWith("L1") and notStartsWith("L2")) return true;
  }
  if (whichPart == 3){
    if (notStartsWith("D")) return true;
  }
  if (whichPart == 4){
    if (notStartsWith("L1")) return true;
   
  }
  if (whichPart == 5){
    if (notStartsWith("L2")) return true;
  }
  return false;
}

int 
chipId(int whichPart, int iphi, int ieta){
  int circ = -1;
  if (whichPart != 0){
    if (iphi < 160){
      circ = (int)(ieta / 18);
    } else {
      circ = 15 - (int)(ieta / 18);
    } // FE15, FE14, ... FE8
  } else {
    if (ieta < 80){
      circ = 0;
    } else {
      circ = 1;
    }
  }
  if (circ>15) circ = -1;//error
  return circ;
}

int
pixelType(int whichPart, int iphi, int ieta){
  int pixtype = 0;
  if (whichPart != 0)
  {
    if (ieta % 18 == 0 || ieta % 18 == 17)
    {
      pixtype = 1;
    } // define long pixels
    if (iphi > 152 && iphi < 160 && iphi % 2 == 1)
    {
      pixtype = 2;
    } // define ganged pixels
    if (iphi > 159 && iphi < 167 && iphi % 2 == 0)
    {
      pixtype = 2;
    }
  }
  // IBL
  else
  {
    if (ieta == 0 || ieta == 79 || ieta == 80 || ieta == 159)
    {
      pixtype = 1;
    } // define long pixels
  }
  return pixtype;
}

TIter
getRodIterator(const TFile & inputFile){
  TDirectoryFile* scanDir = (TDirectoryFile*)((TKey*)inputFile.GetListOfKeys()->First())->ReadObj();
  TList* rodKeyList = (TList*)scanDir->GetListOfKeys();
  return TIter(rodKeyList);
}

bool 
fillThresholdCalibration(const PixelMapping &pm, ValueMap_t & pcdMap, const std::string & inThrFile, const int WhichPart, const int ncol, const int nrow){
  if (inThrFile.empty()) return false;
  TFile riThrFile(inThrFile.c_str(),"READ");
  if (not riThrFile.IsOpen()){
    std::cout<<"File "<<inThrFile<<" could not be opened."<<std::endl;
    return false;
  } else {
    std::cout<<"File "<<inThrFile<<" opened."<<std::endl;
  }
    
  TString chi2HistName = "SCURVE_CHI2";
  TString thrHistName = "SCURVE_MEAN";
  TString sigHistName = "SCURVE_SIGMA";
  TH1F h1dChi2("h1dChi2","",200,0,1);
  TH1F h1dThr("h1dThr","",200,0,6000);
  TH1F h1dSig("h1dSig","",200,0,500);

  TIter rodItr = getRodIterator(riThrFile);
  TKey* rodKey;
 
  while ((rodKey=(TKey*)rodItr())) {
    TString rodName(rodKey->GetName());
    TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
    TList* modKeyList = (TList*)rodDir->GetListOfKeys();
    TIter modItr(modKeyList);
    TKey* modKey;
    while ((modKey=(TKey*)modItr())) {
      TString modName(modKey->GetName());
      std::string modStr(modKey->GetName());
      if (moduleNotInPart(WhichPart, modStr)) continue;
      if (not pm.contains(modStr)) continue;
      TString chi2HistDirPath = modName + "/" + chi2HistName + "/A0/B0";
      TDirectoryFile *chi2HistDir = (TDirectoryFile *)rodDir->Get(chi2HistDirPath);
      TH2F *h2dChi2 = (TH2F *)((TKey *)chi2HistDir->GetListOfKeys()->First())->ReadObj();
      // chi2HistMap[modName] = h2dChi2;
      TString thrHistDirPath = modName + "/" + thrHistName + "/A0/B0";
      TDirectoryFile *thrHistDir = (TDirectoryFile *)rodDir->Get(thrHistDirPath);
      TH2F *h2dThr = (TH2F *)((TKey *)thrHistDir->GetListOfKeys()->First())->ReadObj();
      // thrHistMap[modName] = h2dThr;
      TString sigHistDirPath = modName + "/" + sigHistName + "/A0/B0";
      TDirectoryFile *sigHistDir = (TDirectoryFile *)rodDir->Get(sigHistDirPath);
      TH2F *h2dSig = (TH2F *)((TKey *)sigHistDir->GetListOfKeys()->First())->ReadObj();
      // sigHistMap[modName] = h2dSig;
      std::array<TH1F, 16> h1dThrNormArr;
      int nbins=200;
      float thrLo=0.;
      float thrHi=6000.;
      const std::string  thrNormHistoPrefix="h1dThrNormI";
      int idx=0;
      for (auto & thisHisto: h1dThrNormArr){
        const TString histoName(thrNormHistoPrefix+std::to_string(idx++));
        thisHisto = TH1F(histoName,"",nbins, thrLo, thrHi);
      }
      //
      std::array<TH1F, 16> h1dSigNormArr;
      float sigLo=0.;
      float sigHi=500.;
      const std::string  sigNormHistoPrefix="h1dSigNormI";
      idx=0;
      for (auto & thisHisto: h1dSigNormArr){
        const TString histoName(sigNormHistoPrefix+std::to_string(idx++));
        thisHisto = TH1F(histoName,"",nbins, sigLo, sigHi);
      }
      //
      std::array<TH1F, 16> h1dThrLongArr;
      const std::string  thrLongHistoPrefix="h1dThrLongI";
      idx=0;
      for (auto & thisHisto: h1dThrLongArr){
        const TString histoName(thrLongHistoPrefix+std::to_string(idx++));
        thisHisto = TH1F(histoName,"",nbins, thrLo, thrHi);
      }
      //
      std::array<TH1F, 16> h1dSigLongArr;
      const std::string  sigLongHistoPrefix="h1dSigLongI";
      idx=0;
      for (auto & thisHisto: h1dSigLongArr){
        const TString histoName (sigLongHistoPrefix+std::to_string(idx++));
        thisHisto = TH1F(histoName,"",nbins, sigLo, sigHi);
      }
      //
      std::array<TH1F, 16> h1dThrGangArr;
      const std::string  thrGangHistoPrefix="h1dThrGangI";
      idx=0;
      for (auto & thisHisto: h1dThrGangArr){
        const TString histoName(thrGangHistoPrefix+std::to_string(idx++));
        thisHisto = TH1F(histoName,"",nbins, thrLo, thrHi);
      }
      //
      std::array<TH1F, 16> h1dSigGangArr;
      const std::string  sigGangHistoPrefix="h1dSigGangI";
      idx=0;
      for (auto & thisHisto: h1dSigGangArr){
        const TString histoName(sigGangHistoPrefix+std::to_string(idx++));
        thisHisto = TH1F(histoName,"",nbins, sigLo, sigHi);
      }
      

      for (int ieta = 0; ieta < ncol; ieta++){
        for (int iphi = 0; iphi < nrow; iphi++){
          float chi2 = h2dChi2->GetBinContent(ieta + 1, iphi + 1);
          float thr = h2dThr->GetBinContent(ieta + 1, iphi + 1);
          float sig = h2dSig->GetBinContent(ieta + 1, iphi + 1);
          h1dChi2.Fill(chi2);
          h1dThr.Fill(thr);
          h1dSig.Fill(sig);

          if (thr == 0 || thr > 10000 || sig == 0 || sig > 1000)
            continue; // || chi2 > 0.5 || chi2 <= 0) continue;

          // Identify FE chip ID
          // Simply, in the calibration, the FEs are aligned from bottom left corner (0,0) with anti-clock-wise direction.
          int circ = chipId(WhichPart, iphi, ieta);
          if (circ < 0) continue;

          // normal pixels
          int pixtype = pixelType(WhichPart, iphi, ieta);
          //==================
          // Fill information
          //==================
          const std::array<TH1F*,3> possibleThrArrays{&h1dThrNormArr[circ], &h1dThrLongArr[circ], &h1dThrGangArr[circ]};
          const std::array<TH1F*,3> possibleSigArrays{&h1dSigNormArr[circ], &h1dSigLongArr[circ], &h1dSigGangArr[circ]};
          possibleThrArrays[pixtype]->Fill(thr);
          possibleSigArrays[pixtype]->Fill(sig);
        }
      }
     
      for (int idx=0;idx!=16;++idx){
        pcdMap[modStr][idx][THR_NORM] = h1dThrNormArr[idx].GetMean();
        pcdMap[modStr][idx][THR_RMS_NORM] = h1dThrNormArr[idx].GetRMS();
        pcdMap[modStr][idx][THR_SIG_NORM] = h1dSigNormArr[idx].GetMean();

        pcdMap[modStr][idx][THR_LONG] = h1dThrLongArr[idx].GetMean();
        pcdMap[modStr][idx][THR_RMS_LONG] = h1dThrLongArr[idx].GetRMS();
        pcdMap[modStr][idx][THR_SIG_LONG] = h1dSigLongArr[idx].GetMean();
        
        pcdMap[modStr][idx][THR_GANG] = h1dThrGangArr[idx].GetMean();
        pcdMap[modStr][idx][THR_RMS_GANG] = h1dThrGangArr[idx].GetRMS();
        pcdMap[modStr][idx][THR_SIG_GANG] = h1dSigGangArr[idx].GetMean();
      }
    }
  }
  return true;
}

bool
fillTimingCalibration(const PixelMapping & pm, ValueMap_t & timMap, const std::string & inTimFile, const int WhichPart, const int ncol, const int nrow){
  if (inTimFile.empty()) return false;
  TFile riTimFile(inTimFile.c_str(),"READ");
  if (not riTimFile.IsOpen()){
    std::cout<<"File "<<inTimFile<<" could not be opened."<<std::endl;
    return false;
  } else {
    std::cout<<"File "<<inTimFile<<" opened."<<std::endl;
  }
  TString timHistName = "SCURVE_MEAN";
 
  TIter rodItr=getRodIterator(riTimFile);
  TKey* rodKey;
 
  const int nbins=300;
  const float timLo = 1000.;
  const float timHi =7000.;
  const std::array<std::string,3> pixTypeNames={"Norm", "Long", "Gang"};
  auto formName=[](const std::string &prefix, const std::string & pixType, int idx)->TString{
    TString histoName(prefix+pixType+"I"+std::to_string(idx));
    return histoName;
  };
  while ((rodKey=(TKey*)rodItr())) {
    TString rodName(rodKey->GetName());
    TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
    TList* modKeyList = (TList*)rodDir->GetListOfKeys();
    TIter modItr(modKeyList);
    TKey* modKey;
    while ((modKey=(TKey*)modItr())) {
      TString modName(modKey->GetName());
      std::string modStr(modKey->GetName());
      if (moduleNotInPart(WhichPart, modStr)) continue;
      if (modName=="DSP_ERRORS") { continue; }
      //
      if (not pm.contains(modStr)) continue;
      //
      TString timHistDirPath = modName + "/" + timHistName + "/A0/B0";
      TDirectoryFile* timHistDir = (TDirectoryFile*)rodDir->Get(timHistDirPath);
      TH2F* h2dTim = (TH2F*)((TKey*)timHistDir->GetListOfKeys()->First())->ReadObj();
      std::array<TH1F, 16> h1dTimNormArr;
      std::array<TH1F, 16> h1dTimLongArr;
      std::array<TH1F, 16> h1dTimGangArr;
      for (int i=0;i!=16;++i){
        h1dTimNormArr[i] = TH1F(formName("h1dTim", "Norm", i), "",nbins, timLo, timHi);
        h1dTimLongArr[i] = TH1F(formName("h1dTim", "Long", i), "",nbins, timLo, timHi);
        h1dTimGangArr[i] = TH1F(formName("h1dTim", "Gang", i), "", nbins, timLo, timHi);
      }
      for (int ieta=0; ieta<ncol; ieta++) {
        for (int iphi=0; iphi<nrow; iphi++) {
          float tim = h2dTim->GetBinContent(ieta+1,iphi+1);
          if (tim<0.5) { continue; }

          // Identify FE chip ID
          // Simply, in the calibration, the FEs are aligned from bottom left corner (0,0) with anti-clock-wise direction.
          int circ = chipId(WhichPart, iphi, ieta);
          if (circ < 0) continue;
          // normal pixels
          int pixtype = pixelType(WhichPart, iphi, ieta);

          //==================
          // Fill information
          //==================
          if (pixtype==1) {
            h1dTimLongArr[circ].Fill(tim);
          } else if (pixtype==0) {
            h1dTimNormArr[circ].Fill(tim);
          } else if (pixtype==2) {
            h1dTimGangArr[circ].Fill(tim);
          }
        }
      }
      for (int i=0;i!=16;++i){
        timMap[modStr][i][TIM_NORM] = h1dTimNormArr[i].GetMean();
        timMap[modStr][i][TIM_LONG] = h1dTimLongArr[i].GetMean();
        timMap[modStr][i][TIM_GANG] = h1dTimGangArr[i].GetMean();
      }

    }
  }
  return true;
}

void PixelCalib(bool test=false) {
  std::cout<<"Starting calibration macro"<<std::endl;
  /*  Calibration during TS2 September 19, 2017
  IBL:
    THR: 67422 (Note that LI_S06_C_M1_C1 does not have its final tuning here.
                We did not do a full threshold after tuning but I can pass
                the short (1 in 32 pixels) threshold if that would suffice.)
    TOT Calib: 67489

  BARREL:
    THR: 67465 (Note that this is only a SHORT threshold.)
    TOT CALIB: 67502 & 67504 (Inner and outer halves of the detector.)

  DISK:
    THR: 67482
    TOT CALIB: 67531 (Disk & Blayer Together)

  BLAYER:
    THR: 67530
    TOT CALIB: 67531 (Disk & Blayer Together, saved in DISK directory)

  analog thresholds:
     b-layer : 5000e
     L1,L2   : 3500e
     Endcap : 4500e

  */
  
  
  

 //******************************************************************
  int WhichPart = 1; // 0=IBL, 1=BLayer, 2=L1L2, 3=disk, 4=L1, 5=L2;
 //******************************************************************
  if ((WhichPart<0) or (WhichPart>5)) {
    std::cout<<"Unrecognised part"<<std::endl;
    return;
  }
  //
  PixelMapping pixmap("../common/mapping.csv");
  if (pixmap.nModules() == 2048){
    std::cout<< "Mapping file opened ok"<<std::endl;
  } else {
    std::cout<< "Mapping problem!"<<std::endl;
    return;
  }
  //
  std::string inThrFile = "";
  std::string inTimFile = "";
  std::string inTotFile = "";

  std::string Output = "";
  std::string modHead = "";

  // selecting inputs file
  if (WhichPart==0) {
     std::cout << "Running on IBL" << std::endl;
     inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087723.root";
     inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087726.root";
     inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087714.root";
     Output = "Out_IBL";
  }
  if (WhichPart==1) {
     std::cout << "Running on BLayer" << std::endl;
     inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087719.root";
     inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087717.root";
     inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087710.root";
     if (test){
       std::cout<<"Selected test files with small samples of input are used"<<std::endl;
       inThrFile = "../../test/SAMPLE_S000087719.root";
       inTimFile = "../../test/SAMPLE_S000087717.root";
       inTotFile = "../../test/SAMPLE_S000087710.root";
     }
     Output = "Out_BLayer";
  }
  if (WhichPart==2) {
     std::cout << "Running on L1L2" << std::endl; //usally 2 scan, not merging the root life but the txt file
     inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087719.root";
     inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087717.root";
     inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087710.root";
     Output = "Out_L1L2";
  }
  if (WhichPart==3) {
     std::cout << "Running on Disk" << std::endl;
     inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087719.root";
     inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087717.root";
     inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087710.root";
     Output = "Out_Disk";
  }
  if (WhichPart==4) {
     std::cout << "Running on L1" << std::endl;
     inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087719.root";
     inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087717.root";
     inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087710.root";
     Output = "Out_L1";
  }
  if (WhichPart==5) {
     std::cout << "Running on L2" << std::endl;
     inThrFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087719.root";
     inTimFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087717.root";
     inTotFile = "/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/SCAN_S000087710.root";
     Output = "Out_L2";
  }

  // Set 2-parameter mode
  bool Fit2Parameter = false;
  double fixAparam = 1000.0;    // fix parameter A as 1000.0 (default)

  // selecting Q threshold
  std::array<int, 6> possibleThresholds={8,5,5,5,5,5};
  int qthresh = possibleThresholds[WhichPart];
 

  int nrow = -1;     // y-axis
  int ncol = -1; // x-axis
  if (WhichPart == 0)
  {
    nrow = 336;
    ncol = 160;
  }else{
    nrow = 320; // y-axis
    ncol = 144; // x-axis
  }

  // PIXEL
  constexpr int ncharge = 21;  // injected charges
  float chargeArr[ncharge]    = {3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 12000, 14000, 16000, 18000, 20000, 25000};
  float chargeErrArr[ncharge] = {   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,     0,     0,     0,     0,     0,     0};
  // IBL
//  const int ncharge = 22;  // # of injected charges
//  float chargeArr[ncharge] = {1400, 1500, 1750, 2000, 2500, 3000, 3500, 4000, 5000, 6000, 8000, 10000, 12000, 14000, 16000, 18000, 20000, 22000, 24000, 26000, 28000, 30000};
//  float chargeErrArr[ncharge] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


  ValueMap_t pcdMap;
  ValueMap_t timMap;


  FILE *outputfile;
  std::string summaryName = getDatetodayStr();
  summaryName += "_CalibSummary.txt";

  outputfile = fopen(summaryName.c_str(), "w");
  if (outputfile == NULL) {
    printf("cannot open\n");
    exit(1);
  }
  
  //pcd map passed by reference and will be updated with the result
  if (not fillThresholdCalibration(pixmap, pcdMap, inThrFile, WhichPart, ncol, nrow)){
    std::cout<<"The threshold calibration from file "<<inThrFile<<" was not successful"<<std::endl;
    fclose(outputfile);
    return;
  }
  //
  if (not fillTimingCalibration(pixmap, timMap, inTimFile, WhichPart, ncol, nrow)){
    std::cout<<"The timing calibration from file "<<inTimFile<<" was not successful"<<std::endl;
    fclose(outputfile);
    return;
  }
 
  if (!inTotFile.empty()) {
    std::cout << std::endl << "INFO =>> tot calib analysis..." << std::endl;

    TFile riTotFile(inTotFile.c_str(),"READ");
    if (not riTotFile.IsOpen()){
      std::cout<<"File "<<inTotFile<<" could not be opened."<<std::endl;
      fclose(outputfile);
      return;
    } else {
      std::cout<<"File "<<inTotFile<<" opened."<<std::endl;
    }
    TString totHistName = "TOT_MEAN";
    TString totSigHistName = "TOT_SIGMA";
    TH1F h1dTot7("h1dTot7","",64,0,16);
    TH1F h1dTotSig7("h1dTotSig7","",100,0,1);

    TIter rodItr=getRodIterator(riTotFile);
    TKey* rodKey;
    while ((rodKey=(TKey*)rodItr())) {
      TString rodName(rodKey->GetName());
      TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
      TList* modKeyList = (TList*)rodDir->GetListOfKeys();
      TIter modItr(modKeyList);
      TKey* modKey;

      while ((modKey=(TKey*)modItr())) {
        TString modName(modKey->GetName());
        std::string modStr(modKey->GetName());
        if (moduleNotInPart(WhichPart, modStr)) continue;
        // get module map
        if (not pixmap.contains(modStr)) continue;
        constexpr int nChips = 16;
        std::array<std::array<float, ncharge>, nChips> totArrI{};
        std::array<std::array<float, ncharge>, nChips> totErrArrI{};
        std::array<std::array<float, ncharge>, nChips> totSigArrI{};
        std::array<std::array<float, ncharge>, nChips> totSigErrArrI{};
        std::array<std::array<float, ncharge>, nChips> totLongArrI{};
        std::array<std::array<float, ncharge>, nChips> totErrLongArrI{};
        //
        int nbins = 255;
        float totLo = 0.;
        float totHi = 255.;
        int sigNBins = 100;
        float sigLo = 0.;
        float sigHi = 1.;
        for (int c=0; c<ncharge; ++c) {
          
          //std::cout<<"DEBUG: Proceeding with charge "<<c<<std::endl;
          TString totHistDirPath = modName + "/" + totHistName + "/A0/B0/C";
          totHistDirPath += c;
          TDirectoryFile* totHistDir = (TDirectoryFile*)rodDir->Get(totHistDirPath);
          if (not totHistDir) continue;
          TH2F* h2dTot = (TH2F*)((TKey*)totHistDir->GetListOfKeys()->First())->ReadObj();
          TString totSigHistDirPath = modName + "/" + totSigHistName + "/A0/B0/C";
          totSigHistDirPath += c;
          TDirectoryFile* totSigHistDir = (TDirectoryFile*)rodDir->Get(totSigHistDirPath);
          TH2F* h2dTotSig = (TH2F*)((TKey*)totSigHistDir->GetListOfKeys()->First())->ReadObj();
          
          std::array<TH1F, 16> h1dTotI;
          std::array<TH1F, 16> h1dTotSigI;
          std::array<TH1F, 16> h1dTotLongI;
          std::array<TH1F, 16> h1dTotSigLongI;
          for (int i=0;i!=16;++i){
            const auto iStr = std::to_string(i);
            TString histoName("h1dTotI" + iStr);
            h1dTotI[i] = TH1F(histoName,"", nbins, totLo, totHi);
            histoName = "h1dTotSigI" + iStr;
            h1dTotSigI[i] = TH1F(histoName,"", sigNBins, sigLo, sigHi);
            histoName  = "h1dTotLongI" + iStr;
            h1dTotLongI[i] = TH1F(histoName,"", nbins, totLo, totHi);
            histoName = "h1dTotSigLongI" + iStr;
            h1dTotSigLongI[i] = TH1F(histoName,"", sigNBins, sigLo, sigHi);
          }
          for (int ieta=0; ieta<ncol; ieta++) {
            for (int iphi=0; iphi<nrow; iphi++) {
              float tot = h2dTot->GetBinContent(ieta+1,iphi+1);
              float totSig = h2dTotSig->GetBinContent(ieta+1,iphi+1);
              if (tot<0.1) { continue; }
              // monitoring
              if (c == 7) {
                h1dTot7.Fill(tot);
                h1dTotSig7.Fill(totSig);
              }

              // Identify FE chip ID
              // Simply, in the calibration, the FEs are aligned from bottom left corner (0,0) with anti-clock-wise direction.
              int circ = chipId(WhichPart, iphi, ieta);
              if (circ < 0) continue;

              // normal pixels
              int pixtype = pixelType(WhichPart, iphi, ieta);

              //==================
              // Fill information
              //==================

              if (pixtype>0) {
                h1dTotLongI[circ].Fill(tot);
                h1dTotSigLongI[circ].Fill(totSig);
              } else {
                h1dTotI[circ].Fill(tot);
                h1dTotSigI[circ].Fill(totSig);
              }
            }
          }
          for (int i=0;i!=16;++i){
            totArrI[i][c] = h1dTotI[i].GetMean();
            totErrArrI[i][c] = h1dTotI[i].GetMeanError();
            totSigArrI[i][c] = std::sqrt(h1dTotSigI[i].GetMean()*h1dTotSigI[i].GetMean()+h1dTotI[i].GetRMS()*h1dTotI[i].GetRMS());
            totSigErrArrI[i][c] = std::sqrt(h1dTotSigI[i].GetMeanError()*h1dTotSigI[i].GetMeanError()+h1dTotI[i].GetRMSError()*h1dTotI[i].GetRMSError());
            if (totSigErrArrI[i][c] >1.0)  totArrI[i][c] = 0.;
            //
            totLongArrI[i][c] = h1dTotLongI[i].GetMean();
            totErrLongArrI[i][c] = h1dTotLongI[i].GetMeanError();
            if (totErrLongArrI[i][c] > 1.0) totLongArrI[i][c] = 0;
          }
        }
        float FitStartingPoint = chargeArr[qthresh]-100;
        
        
        std::array<TGraphErrors, nChips> grTotI;
        std::array<TGraphErrors, nChips> grTotSigI;
        std::array<TF1, nChips> f1TotI;
        std::array<TF1, nChips> f1DispI;
        std::array<float, nChips> parAI{};
        std::array<float, nChips> parEI{};
        std::array<float, nChips> parCI{};
        std::array<float, nChips> parP0I{};
        std::array<float, nChips> parP1I{};
        //
        for (int idx=0;idx!=nChips;++idx){
          std::string idxStr=std::to_string(idx);
          TString grTotSuffix("__grTotI" + idxStr);
          TString grTotSigSuffix ("__grTotSigI" + idxStr);
          TString grTotName = modName + grTotSuffix;
          TString grTotSigName = modName + grTotSigSuffix;
          //
          
          grTotI[idx]=TGraphErrors(ncharge, chargeArr, totArrI[idx].data(), chargeErrArr, totErrArrI[idx].data());
          grTotI[idx].SetName(grTotName);
          grTotSigI[idx]= TGraphErrors(ncharge, chargeArr, totSigArrI[idx].data(), chargeErrArr, totSigErrArrI[idx].data());
          grTotSigI[idx].SetName(grTotSigName);
          //
          TString f1TotName("f1TotI" + idxStr);
          TString f1DispName("f1DispI" + idxStr);
          f1TotI[idx] = TF1(f1TotName,funcTot, FitStartingPoint, chargeArr[ncharge-1]+100, 3);
          f1DispI[idx] = TF1(f1DispName,funcDisp, FitStartingPoint, chargeArr[ncharge-1]+100, 2);
          gROOT->GetListOfFunctions()->Add(&f1TotI[idx]);
          gROOT->GetListOfFunctions()->Add(&f1DispI[idx]);
          if (Fit2Parameter) { f1TotI[idx].FixParameter(0,fixAparam); }  // 2-parameter fitting mode
          grTotI[idx].Fit(&(f1TotI[idx]),"MRQ");
          grTotSigI[idx].Fit(&(f1DispI[idx]),"MRQ");
          //
          parAI[idx] = f1TotI[idx].GetParameter(0);
          parEI[idx] = f1TotI[idx].GetParameter(1);
          parCI[idx] = f1TotI[idx].GetParameter(2);
          parP0I[idx] = f1DispI[idx].GetParameter(0);
          parP1I[idx] = f1DispI[idx].GetParameter(1);
        }
        //
        std::array<TGraphErrors, nChips> grTotLongI;
        std::array<TGraphErrors, nChips> grTotLongSigI;
        std::array<TF1, nChips> f1TotLongI;
        std::array<TF1, nChips> f1DispLongI;
        std::array<float, nChips> parLongAI{};
        std::array<float, nChips> parLongEI{};
        std::array<float, nChips> parLongCI{};
        std::array<float, nChips> parLongP0I{};
        std::array<float, nChips> parLongP1I{};
        for (int idx=0;idx!=nChips;++idx){
          const std::string idxStr = std::to_string(idx);
          TString grTotSuffix("__grTotLongI" + idxStr);
          TString grTotSigSuffix ("__grTotLongSigI" + idxStr);
          TString grTotName = modName + grTotSuffix;
          TString grTotSigName = modName + grTotSigSuffix;
          //
          grTotLongI[idx]=TGraphErrors(ncharge, chargeArr, totLongArrI[idx].data(), chargeErrArr, totErrLongArrI[idx].data());
          grTotLongI[idx].SetName(grTotName);
          //NB original code not using 'Long' version of totSigArrI
          grTotLongSigI[idx] = TGraphErrors(ncharge, chargeArr, totSigArrI[idx].data(), chargeErrArr, totSigErrArrI[idx].data());
          grTotSigI[idx].SetName(grTotSigName);
          //
          TString f1TotName("f1TotLongI" + idxStr);
          TString f1DispName("f1DispLongI" + idxStr);
          f1TotLongI[idx]=TF1(f1TotName,funcTot, FitStartingPoint, chargeArr[ncharge-1]+100, 3);
          f1DispLongI[idx]=TF1(f1DispName,funcDisp, FitStartingPoint, chargeArr[ncharge-1]+100, 2);
          gROOT->GetListOfFunctions()->Add(&f1TotLongI[idx]);
          gROOT->GetListOfFunctions()->Add(&f1DispLongI[idx]);
          if (Fit2Parameter) { f1TotLongI[idx].FixParameter(0,fixAparam); }  // 2-parameter fitting mode
          grTotLongI[idx].Fit(&f1TotLongI[idx],"MRQ");
          grTotLongSigI[idx].Fit(&f1DispLongI[idx],"MRQ");
          //
          parLongAI[idx] = f1TotLongI[idx].GetParameter(0);
          parLongEI[idx] = f1TotLongI[idx].GetParameter(1);
          parLongCI[idx] = f1TotLongI[idx].GetParameter(2);
          parLongP0I[idx] = f1DispLongI[idx].GetParameter(0);
          parLongP1I[idx] = f1DispLongI[idx].GetParameter(1);
        }
        std::array<std::array<float, ncharge>, nChips> badcalI{};
        std::array<float, nChips>  badcalI_max{};
        for (int c=qthresh; c<ncharge; ++c){
          for (int idx=0;idx !=nChips;++idx){
             badcalI[idx][c] = std::abs( 1 - ( (parAI[idx] * parEI[idx] - parCI[idx] * totArrI[idx][c]) / (totArrI[idx][c] - parAI[idx]) ) / chargeArr[c] );
          }
        }
        for (int idx=0;idx !=nChips;++idx){
           const auto &thisArray = badcalI[idx];
           badcalI_max[idx] = *(std::max_element(thisArray.begin(), thisArray.end()));
        }
        std::array<std::vector<Double_t>, nChips> chargeArrI_re;
        std::array<std::vector<Double_t>, nChips> chargeErrArrI_re;
        std::array<std::vector<Double_t>, nChips> totArrI_re;
        std::array<std::vector<Double_t>, nChips> totErrArrI_re;
        std::array<std::vector<Double_t>, nChips> totSigArrI_re;
        std::array<std::vector<Double_t>, nChips> totSigErrArrI_re;
       
        for (int idx=0;idx != nChips;++idx){
          chargeArrI_re[idx].assign(chargeArr, chargeArr + ncharge);
          chargeErrArrI_re[idx].assign(chargeErrArr, chargeErrArr + ncharge);
          totArrI_re[idx].assign(totArrI[idx].begin(), totArrI[idx].end());
          totErrArrI_re[idx].assign(totErrArrI[idx].begin(), totErrArrI[idx].end());
          totSigArrI_re[idx].assign(totSigArrI[idx].begin(), totSigArrI[idx].end());
          totSigErrArrI_re[idx].assign(totSigErrArrI[idx].begin(), totSigErrArrI[idx].end());
        }

        const double chi2_error = 0.05;
        fprintf(outputfile, "%s", modStr.c_str());
        
        for (int idx=0;idx!=nChips;++idx){
          fprintf(outputfile, "\n");
          int ncharge_re = ncharge;
          fprintf(outputfile, "[ ");
          while(badcalI_max[idx] > chi2_error){
            if (ncharge_re < ncharge/2.0){
              parAI[idx] = 0;
              parEI[idx] = -28284.3;
              parCI[idx] = 0;
              break;
            }else{
              std::vector<Double_t> find_max(ncharge_re, 0.);//fills vector with zeroes
              for(int i = qthresh; i < ncharge_re; i++){
                find_max[i]=(std::abs(1 - ((parAI[idx] * parEI[idx] - parCI[idx] * totArrI_re[idx][i]) / (totArrI_re[idx][i] - parAI[idx])) / chargeArrI_re[idx][i]));
              }
              std::vector<Double_t>::iterator iter = std::max_element(find_max.begin(), find_max.end());
              int n_max = static_cast<int>(std::distance(find_max.begin(), iter));
            
              ncharge_re = ncharge_re - 1;
              auto numToErase = n_max;
              if ((ncharge_re - n_max)<=2) numToErase = ncharge_re;
              //
              fprintf(outputfile, "%d", (int) chargeArrI_re[idx][numToErase]);
              fprintf(outputfile, " ");
              chargeArrI_re[idx].erase(chargeArrI_re[idx].begin() + numToErase);
              totArrI_re[idx].erase(totArrI_re[idx].begin() + numToErase);
              totSigArrI_re[idx].erase(totSigArrI_re[idx].begin() + numToErase);
              chargeErrArrI_re[idx].erase(chargeErrArrI_re[idx].begin() + numToErase);
              totErrArrI_re[idx].erase(totErrArrI_re[idx].begin() + numToErase);
              totSigErrArrI_re[idx].erase(totSigErrArrI_re[idx].begin() + numToErase);
              //
              grTotI[idx]= TGraphErrors(ncharge_re, chargeArrI_re[idx].data(), totArrI_re[idx].data(), chargeErrArrI_re[idx].data(), totErrArrI_re[idx].data());
              std::string idxStr = std::to_string(idx);
              TString suffix("__grTotI" + idxStr);
              TString histoName(modName + suffix);
              grTotI[idx].SetName(histoName);
              grTotSigI[idx] = TGraphErrors (ncharge_re, chargeArrI_re[idx].data(), totSigArrI_re[idx].data(), chargeErrArrI_re[idx].data(), totSigErrArrI_re[idx].data());
              TString sigSuffix ( "__grTotSigI" + idxStr);
              TString sigHistoName(modName+sigSuffix);
              grTotSigI[idx].SetName(sigHistoName);
              TString f1TotName("f1TotI" + idxStr);
              f1TotI[idx] = TF1 (f1TotName,funcTot, FitStartingPoint, chargeArrI_re[idx][ncharge_re-1]+100, 3);
              TString f1DispName("f1DispI" + idxStr);
              f1DispI[idx] = TF1(f1DispName,funcDisp, FitStartingPoint, chargeArrI_re[idx][ncharge_re-1]+100, 2);
              if (Fit2Parameter) { f1TotI[idx].FixParameter(0,fixAparam); }  // 2-parameter fitting mode
              gROOT->GetListOfFunctions()->Add(&f1TotI[idx]);
              gROOT->GetListOfFunctions()->Add(&f1DispI[idx]);
              grTotI[idx].Fit(&f1TotI[idx],"MRQ");
              grTotSigI[idx].Fit(&f1DispI[idx],"MRQ");
              parAI[idx] = f1TotI[idx].GetParameter(0);
              parEI[idx] = f1TotI[idx].GetParameter(1);
              parCI[idx] = f1TotI[idx].GetParameter(2);
              parP0I[idx] = f1DispI[idx].GetParameter(0);
              parP1I[idx] = f1DispI[idx].GetParameter(1);
              //
              badcalI[idx].fill(0.);
              for(int i = qthresh; i < ncharge_re; i++){
                badcalI[idx][i] = std::abs( 1 - ( (parAI[idx] * parEI[idx] - parCI[idx] * totArrI_re[idx][i]) / (totArrI_re[idx][i] - parAI[idx]) ) / chargeArrI_re[idx][i] );
              }
              badcalI_max[idx] = *std::max_element(badcalI[idx].begin(), badcalI[idx].begin() + 10);
            }
          }
          fprintf(outputfile, "]");
        }
        fprintf(outputfile, "\n");
        const int n = (WhichPart == 0)? 2:nChips;
        std::cout << modStr << std::endl;
        for (int idx=0;idx!=n;++idx){
          const std::string chipName="I"+std::to_string(idx);
          std::cout << chipName <<" "
          << int(pcdMap[modStr][idx][THR_NORM]) << " " << int(pcdMap[modStr][idx][THR_RMS_NORM]) << " "
          << int(pcdMap[modStr][idx][THR_SIG_NORM]) << " " << int(timMap[modStr][idx][TIM_NORM]) << " "
          << int(pcdMap[modStr][idx][THR_LONG]) << " " << int(pcdMap[modStr][idx][THR_RMS_LONG]) << " "
          << int(pcdMap[modStr][idx][THR_SIG_LONG]) << " " << int(timMap[modStr][idx][TIM_LONG]) << " "
          << int(pcdMap[modStr][idx][THR_GANG]) << " " << int(pcdMap[modStr][idx][THR_RMS_GANG]) << " "
          << int(pcdMap[modStr][idx][THR_SIG_GANG]) << " " << int(timMap[modStr][idx][TIM_GANG]) << " "
          << parAI[idx] << " " << parEI[idx] << " " << parCI[idx] << " "
          << parLongAI[idx] << " " << parLongEI[idx] << " " << parLongCI[idx] << " "
          << parP0I[idx]<< " " << parP1I[idx]
          << std::endl;
        }
      }
    }
  }
  fclose(outputfile);
}


int main(){
  PixelCalib(false);
  return 0;
}
