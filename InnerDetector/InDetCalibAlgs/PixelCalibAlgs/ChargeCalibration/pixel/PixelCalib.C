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
#include <stdexcept>
#include <functional>
#include <tuple>

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
typedef std::map<TString,OneModuleSettings > ValueMap_t;
//module histograms
typedef std::array<TH1F, 16> OneModuleHistograms_t;
typedef std::array<OneModuleHistograms_t *,3> PixelTypeHistograms_t;


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
moduleInPart(int whichPart, const TString & modName){
  if (modName == "DSP_ERRORS"){
    return false;
  }
  static const std::array<TString, 6> prefixes{"LI", "L0", "L1", "D", "L1", "L2"};
  bool result = modName.BeginsWith(prefixes[whichPart]);
  if (whichPart == 2) result = (result or modName.BeginsWith("L2"));
  return result;
}

// Identify FE chip ID
// Simply, in the calibration, the FEs are aligned from bottom left corner (0,0) with anti-clock-wise direction.
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
  if (whichPart != 0){
    if (ieta % 18 == 0 || ieta % 18 == 17){
      pixtype = 1;
    } // define long pixels
    if (iphi > 152 && iphi < 160 && iphi % 2 == 1){
      pixtype = 2;
    } // define ganged pixels
    if (iphi > 159 && iphi < 167 && iphi % 2 == 0){
      pixtype = 2;
    }
  } else /*IBL */{
    if (ieta == 0 || ieta == 79 || ieta == 80 || ieta == 159){
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

TIter
getModuleIterator( TDirectoryFile* rodDir){
    TList* modKeyList = (TList*)rodDir->GetListOfKeys();
    return TIter(modKeyList);
}

TH2F *
get2DHistogramFromPath( TDirectoryFile* rodDir, const TString & moduleName, const TString & histName, int charge=-1){
  TString suffix = (charge<0)?(""):(TString("/C") + charge);
  TString fullHistoPath = moduleName + "/" + histName + "/A0/B0" + suffix;
  TDirectoryFile *histDir = (TDirectoryFile *)rodDir->Get(fullHistoPath);
  TH2F *pTH2 = (TH2F *)((TKey *)histDir->GetListOfKeys()->First())->ReadObj();
  return pTH2;
}

OneModuleHistograms_t
sixteenHistograms(const TString & titlePrefix, int nBins, float lo, float hi){
  OneModuleHistograms_t result;
  for (int i=0;i!=16;++i){
    TString title = titlePrefix + i;
    result[i] = TH1F(title, "", nBins, lo, hi);
  }
  return result;
}
void
resetHistograms(std::array<TH1F,16> & histogramArray){
  for (auto & thisHisto: histogramArray){
    thisHisto.Reset("ICESM");
  }
}

struct LegacyFitParameters{
  float A{};
  float C{};
  float E{};
  //
  void
  extractFromFit(const TF1 & fit){
    A=fit.GetParameter(0);
    E=fit.GetParameter(1);
    C=fit.GetParameter(2);
  }
  // 
  float
  predictedCharge(float tot) const {
    return (E * A - tot * C) / (tot - A);
  }
  //
  template <typename ValType>
  float anomaly(ValType tot, ValType charge) const{
    return std::abs( 1.f - predictedCharge(tot) / charge );
  }
  //
  void
  invalidate(){
    A = 0;
    E = -28284.3;
    C = 0;
  }
};

struct LinearFitParameters{
  float P0{};
  float P1{};
  void
   extractFromFit(const TF1 & fit){
     P0 = fit.GetParameter(0);
     P1 = fit.GetParameter(1);
   }
};

template <size_t N>
std::pair<float, int>
findMaxAnomaly(const LegacyFitParameters & fitParams, const std::array<float, N> & totArray, const std::array<float, N> & chargeArray, size_t qthresh, bool inFirstTen=false){
  std::array<float, N> badcalI{};
  for (size_t c=qthresh; c<N; ++c){
    badcalI[c] = fitParams.anomaly(totArray[c], chargeArray[c]);
  }
  const auto endIterator = inFirstTen ? badcalI.begin() + 10 : badcalI.end();
  auto startIterator = badcalI.begin() + qthresh;
  const auto maxIt = std::max_element(startIterator, endIterator);
  const int n = static_cast<int>(std::distance(badcalI.begin(), maxIt));
  return {*maxIt, n};
}

std::pair<float, int> 
findMaxAnomaly(const LegacyFitParameters & fitParams, const std::vector<double> & totArray, const std::vector<double> & chargeArray, size_t qthresh, bool inFirstTen=false){
  const auto N=chargeArray.size();
  std::vector<float> badcalI(chargeArray.size(), 0.f);
  for (size_t c=qthresh; c<N; ++c){
    badcalI[c] = fitParams.anomaly( totArray[c], chargeArray[c]);
  }
  const auto endIterator = inFirstTen ? badcalI.begin() + 10: badcalI.end();
  auto startIterator = badcalI.begin() + qthresh;
  if ((endIterator > badcalI.end()) or (startIterator > endIterator)){
    throw std::runtime_error("iterator anomaly in findMaxAnomaly (vector version)");
  }
  const auto maxIt = std::max_element(startIterator, endIterator);
  const int n = static_cast<int>(std::distance(badcalI.begin(), maxIt));
  return {*maxIt, n};
}

std::pair<int,int>
numRowsAndColumns(int whichPart){
  typedef std::pair<int, int> RowCol_t;
  static constexpr std::array<RowCol_t,2> validAnswers{RowCol_t{336, 160}, RowCol_t{320, 144}};
  return (whichPart == 0) ? validAnswers[0] : validAnswers[1];
}

//get data from 2D histogram on file into local 1D histogram, with sigma histo
void
getData(TH2F * source, TH2F* sigmaSource, PixelTypeHistograms_t possibleHistograms, 
    PixelTypeHistograms_t possibleSigmaHistograms, int whichPart, 
    std::function<bool(float)> cut, 
    std::function<bool(float)> sigCut = [](float){return false;}){
  const auto [nrow,ncol] =  numRowsAndColumns(whichPart);
  for (int ieta = 0; ieta < ncol; ieta++){
    for (int iphi = 0; iphi < nrow; iphi++){
      float thr = source->GetBinContent(ieta + 1, iphi + 1);
      float sig = sigmaSource->GetBinContent(ieta + 1, iphi + 1);
      if (cut(thr) or sigCut(sig)) continue; 
      int circ = chipId(whichPart, iphi, ieta);
      if (circ < 0) continue;
      int pixtype = pixelType(whichPart, iphi, ieta);
      //possibleHistograms[pixType] returns a pointer to one module's worth of TH1F histograms
      (*possibleHistograms[pixtype])[circ].Fill(thr);
      (*possibleSigmaHistograms[pixtype])[circ].Fill(sig);
    }
  }
  return;
}

//get data from 2D histogram on file into local 1D histogram, with no sigma histo
void
getData(TH2F * source, PixelTypeHistograms_t possibleHistograms,  int whichPart, std::function<bool(float)> cut){
  const auto [nrow,ncol] =  numRowsAndColumns(whichPart);
  for (int ieta = 0; ieta < ncol; ieta++){
    for (int iphi = 0; iphi < nrow; iphi++){
      float thr = source->GetBinContent(ieta + 1, iphi + 1);
      if (cut(thr)) continue; 
      int circ = chipId(whichPart, iphi, ieta);
      if (circ < 0) continue;
      int pixtype = pixelType(whichPart, iphi, ieta);
      //possibleHistograms[pixType] returns a pointer to one module's worth of TH1F histograms
      (*possibleHistograms[pixtype])[circ].Fill(thr);
    }
  }
  return;
}

bool 
fillThresholdCalibration(const PixelMapping &pm, ValueMap_t & pcdMap, const std::string & inThrFile, const int whichPart){
  if (inThrFile.empty()) return false;
  TFile riThrFile(inThrFile.c_str(),"READ");
  if (not riThrFile.IsOpen()){
    std::cout<<"File "<<inThrFile<<" could not be opened."<<std::endl;
    return false;
  } else {
    std::cout<<"File "<<inThrFile<<" opened."<<std::endl;
  }
    
  //unused, here for documentation of file content:
  //const TString chi2HistName = "SCURVE_CHI2";
  const TString thrHistName = "SCURVE_MEAN";
  const TString sigHistName = "SCURVE_SIGMA";
  
  const int nbins=200;
  const float thrLo=0.;
  const float thrHi=6000.;
  const float sigLo=0.;
  const float sigHi=500;
  //
  OneModuleHistograms_t h1dThrNormArr = sixteenHistograms("h1dThrNormI", nbins, thrLo, thrHi);
  OneModuleHistograms_t h1dSigNormArr = sixteenHistograms("h1dSigNormI", nbins, sigLo, sigHi);
  OneModuleHistograms_t h1dThrLongArr = sixteenHistograms("h1dThrLongI", nbins, thrLo, thrHi);
  OneModuleHistograms_t h1dSigLongArr = sixteenHistograms("h1dSigLongI", nbins, sigLo, sigHi);
  OneModuleHistograms_t h1dThrGangArr = sixteenHistograms("h1dThrGangI", nbins, thrLo, thrHi);
  OneModuleHistograms_t h1dSigGangArr = sixteenHistograms("h1dSigGangI", nbins, sigLo, sigHi);
  //
  PixelTypeHistograms_t possibleHistograms{&h1dThrNormArr, &h1dThrLongArr, &h1dThrGangArr};
  PixelTypeHistograms_t possibleSigmaHistograms{&h1dSigNormArr, &h1dSigLongArr, &h1dSigGangArr};
  //
  TIter rodItr = getRodIterator(riThrFile);
  TKey* rodKey;
  while ((rodKey=(TKey*)rodItr())) {
    TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
    TKey* modKey;
    TIter modItr=getModuleIterator(rodDir);
    
    while ((modKey=(TKey*)modItr())) {
      TString modName(modKey->GetName());
      if ( not moduleInPart(whichPart, modName)) continue;
      if (not pm.contains(std::string(modName))) continue;
      TH2F *h2dThr = get2DHistogramFromPath(rodDir,modName, "SCURVE_MEAN");
      TH2F *h2dSig = get2DHistogramFromPath(rodDir,modName, "SCURVE_SIGMA");
      //
      resetHistograms(h1dThrNormArr);
      resetHistograms(h1dSigNormArr);
      resetHistograms(h1dThrLongArr);
      resetHistograms(h1dSigLongArr);
      resetHistograms(h1dThrGangArr);
      resetHistograms(h1dSigGangArr);
      //
      auto cut = [](float thr)->bool{
        return (thr == 0.f || thr > 10000.f);
      };
      auto sigCut = [](float sig)->bool{
        return (sig == 0.f || sig > 1000.f);
      };
      getData(h2dThr, h2dSig, possibleHistograms, possibleSigmaHistograms, whichPart, cut, sigCut);
      //
      for (int idx=0;idx!=16;++idx){
        pcdMap[modName][idx][THR_NORM] = h1dThrNormArr[idx].GetMean();
        pcdMap[modName][idx][THR_RMS_NORM] = h1dThrNormArr[idx].GetRMS();
        pcdMap[modName][idx][THR_SIG_NORM] = h1dSigNormArr[idx].GetMean();

        pcdMap[modName][idx][THR_LONG] = h1dThrLongArr[idx].GetMean();
        pcdMap[modName][idx][THR_RMS_LONG] = h1dThrLongArr[idx].GetRMS();
        pcdMap[modName][idx][THR_SIG_LONG] = h1dSigLongArr[idx].GetMean();
        
        pcdMap[modName][idx][THR_GANG] = h1dThrGangArr[idx].GetMean();
        pcdMap[modName][idx][THR_RMS_GANG] = h1dThrGangArr[idx].GetRMS();
        pcdMap[modName][idx][THR_SIG_GANG] = h1dSigGangArr[idx].GetMean();
      }
    }
  }
  return true;
}

bool
fillTimingCalibration(const PixelMapping & pm, ValueMap_t & timMap, const std::string & inTimFile, const int whichPart){
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
 
  while ((rodKey=(TKey*)rodItr())) {
    TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
    TIter modItr = getModuleIterator(rodDir);
    TKey* modKey;
    while ((modKey=(TKey*)modItr())) {
      const TString modName(modKey->GetName());
      if ( not moduleInPart(whichPart, modName)) continue;
      if (not pm.contains(std::string(modName))) continue;
      //
      TH2F* h2dTim = get2DHistogramFromPath(rodDir,modName, timHistName);
      OneModuleHistograms_t h1dTimNormArr = sixteenHistograms("h1dTimNormI", nbins, timLo, timHi);
      OneModuleHistograms_t h1dTimLongArr = sixteenHistograms("h1dTimLongI", nbins, timLo, timHi);
      OneModuleHistograms_t h1dTimGangArr = sixteenHistograms("h1dTimGangI", nbins, timLo, timHi);
      PixelTypeHistograms_t possibleHistograms{&h1dTimNormArr, &h1dTimLongArr, &h1dTimGangArr};
      auto cut = [](float tim)->bool{
        return (tim < 0.5f);
      };
      getData(h2dTim, possibleHistograms, whichPart, cut);
      for (int i=0;i!=16;++i){
        timMap[modName][i][TIM_NORM] = h1dTimNormArr[i].GetMean();
        timMap[modName][i][TIM_LONG] = h1dTimLongArr[i].GetMean();
        timMap[modName][i][TIM_GANG] = h1dTimGangArr[i].GetMean();
      }
    }
  }
  return true;
}

void PixelCalib() {
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
  static constexpr int whichPart = 1; // 0=IBL, 1=BLayer, 2=L1L2, 3=disk, 4=L1, 5=L2;
 //******************************************************************
  if ((whichPart<0) or (whichPart>5)) {
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
  // selecting inputs file
  const std::string filepath{"/eos/atlas/atlascerngroupdisk/det-ibl/charge-calibration/202207_ALL/"};
  const std::string suffix{".root"};
  const std::array<std::string, 6> 
  thresholdFiles{"SCAN_S000087723","SCAN_S000087719","SCAN_S000087719", "SCAN_S000087719","SCAN_S000087719","SCAN_S000087719"};
  //
  const std::array<std::string, 6> 
  timingFiles{"SCAN_S000087726", "SCAN_S000087717", "SCAN_S000087717", "SCAN_S000087717", "SCAN_S000087717", "SCAN_S000087717"};
  //
  const std::array<std::string, 6> 
  totFiles{"SCAN_S000087714", "SCAN_S000087710", "SCAN_S000087710", "SCAN_S000087710", "SCAN_S000087710", "SCAN_S000087710"};
  //
  const std::array<std::string, 6> 
  messages{"Running on IBL", "Running on BLayer", "Running on L1L2", "Running on Disk", "Running on L1", "Running on L2"};
  std::cout<<messages[whichPart]<<std::endl;
  const std::string inThrFile = filepath + thresholdFiles[whichPart] + suffix;
  const std::string inTimFile = filepath + timingFiles[whichPart] + suffix;
  const std::string inTotFile = filepath + totFiles[whichPart] + suffix;

  // Set 2-parameter mode
  bool Fit2Parameter = false;
  double fixAparam = 1000.0;    // fix parameter A as 1000.0 (default)

  // selecting Q threshold
  static constexpr std::array<size_t, 6> possibleThresholds={8,5,5,5,5,5};
  static constexpr size_t qthresh = possibleThresholds[whichPart];
    
  // PIXEL
  static constexpr int ncharge = 21;  // injected charges
  static constexpr std::array<float, ncharge> chargeArr{   3000, 3500, 4000, 4500, 5000, 5500, 6000, 6500, 7000, 7500, 8000, 8500, 9000, 9500, 10000, 12000, 14000, 16000, 18000, 20000, 25000};
  static constexpr std::array<float, ncharge> chargeErrArr{   0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,     0,     0,     0,     0,     0,     0,     0};
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
  if (not fillThresholdCalibration(pixmap, pcdMap, inThrFile, whichPart)){
    std::cout<<"The threshold calibration from file "<<inThrFile<<" was not successful"<<std::endl;
    fclose(outputfile);
    return;
  }
  //
  if (not fillTimingCalibration(pixmap, timMap, inTimFile, whichPart)){
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
   
    TIter rodItr=getRodIterator(riTotFile);
    TKey* rodKey;
    while ((rodKey=(TKey*)rodItr())) {
      TDirectoryFile* rodDir = (TDirectoryFile*)rodKey->ReadObj();
      TIter modItr = getModuleIterator(rodDir);
      TKey* modKey;
      while ((modKey=(TKey*)modItr())) {
        const TString modName(modKey->GetName());
        if (not moduleInPart(whichPart, modName)) continue;
        // get module map
        if (not pixmap.contains(std::string(modName))) continue;
        constexpr int nChips = 16;
        std::array<std::array<float, ncharge>, nChips> totArrI{};
        std::array<std::array<float, ncharge>, nChips> totErrArrI{};
        std::array<std::array<float, ncharge>, nChips> totSigArrI{};
        std::array<std::array<float, ncharge>, nChips> totSigErrArrI{};
        std::array<std::array<float, ncharge>, nChips> totLongArrI{};
        std::array<std::array<float, ncharge>, nChips> totErrLongArrI{};
        //
        constexpr int nbins = 255;
        constexpr float totLo = 0.;
        constexpr float totHi = 255.;
        constexpr int sigNBins = 100;
        constexpr float sigLo = 0.;
        constexpr float sigHi = 1.;
        
        for (int c=0; c<ncharge; ++c) {
          TH2F* h2dTot = get2DHistogramFromPath(rodDir,modName, totHistName, c);
          TH2F* h2dTotSig = get2DHistogramFromPath(rodDir,modName, totSigHistName, c);
          OneModuleHistograms_t h1dTotI = sixteenHistograms("h1dTotI",nbins, totLo, totLo);
          OneModuleHistograms_t h1dTotSigI = sixteenHistograms("h1dTotSigI", sigNBins, sigLo, sigHi);
          OneModuleHistograms_t h1dTotLongI = sixteenHistograms("h1dTotLongI", nbins, totLo, totHi);
          OneModuleHistograms_t h1dTotSigLongI = sixteenHistograms("h1dTotSigLongI", sigNBins, sigLo, sigHi);
          //as in original code, only distinguish between 'normal' and not 'normal'
          PixelTypeHistograms_t possibleHistograms{&h1dTotI, &h1dTotLongI, &h1dTotLongI};
          PixelTypeHistograms_t possibleSigmaHistograms{&h1dTotSigI, &h1dTotSigLongI, &h1dTotSigLongI};
          auto cut = [](float tot)->bool{
            return (tot < 0.1f);
          };
          getData(h2dTot, h2dTotSig, possibleHistograms, possibleSigmaHistograms, whichPart, cut);
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
        std::array<LegacyFitParameters, nChips> fitParams{};
        std::array<LinearFitParameters, nChips> linearFitParams{};
        //
        const TString totGraphPrefix = modName + "__grTotI";
        const TString sigmaGraphPrefix = modName + "__grTotSigI";
        const TString f1TotNamePrefix("f1TotI");
        const TString f1DispNamePrefix("f1DispI");
        for (int idx=0;idx!=nChips;++idx){
          //
          grTotI[idx]=TGraphErrors(ncharge, chargeArr.data(), totArrI[idx].data(), chargeErrArr.data(), totErrArrI[idx].data());
          grTotI[idx].SetName(totGraphPrefix + idx);
          grTotSigI[idx]= TGraphErrors(ncharge, chargeArr.data(), totSigArrI[idx].data(), chargeErrArr.data(), totSigErrArrI[idx].data());
          grTotSigI[idx].SetName(sigmaGraphPrefix + idx);
          //
          f1TotI[idx] = TF1(f1TotNamePrefix + idx,funcTot, FitStartingPoint, chargeArr[ncharge-1]+100, 3);
          f1DispI[idx] = TF1(f1DispNamePrefix + idx,funcDisp, FitStartingPoint, chargeArr[ncharge-1]+100, 2);
          gROOT->GetListOfFunctions()->Add(&f1TotI[idx]);
          gROOT->GetListOfFunctions()->Add(&f1DispI[idx]);
          if (Fit2Parameter) { f1TotI[idx].FixParameter(0,fixAparam); }  // 2-parameter fitting mode
          //
          grTotI[idx].Fit(&(f1TotI[idx]),"MRQ");
          grTotSigI[idx].Fit(&(f1DispI[idx]),"MRQ");
          //
          fitParams[idx].extractFromFit(f1TotI[idx]);
          linearFitParams[idx].extractFromFit(f1DispI[idx]);
        }
        //
        std::array<TGraphErrors, nChips> grTotLongI;
        std::array<TGraphErrors, nChips> grTotLongSigI;
        std::array<TF1, nChips> f1TotLongI;
        std::array<TF1, nChips> f1DispLongI;
        std::array<LegacyFitParameters, nChips> longFitParams{};
        std::array<LinearFitParameters, nChips> longLinearFitparams{};
        //
        const TString totLongGraphPrefix = modName + "__grTotLongI";
        const TString sigmaLongGraphPrefix = modName + "__grTotLongSigI";
        const TString f1TotLongPrefix("f1TotLongI");
        const TString f1DispLongPrefix("f1DispLongI");
        for (int idx=0;idx!=nChips;++idx){
          grTotLongI[idx]=TGraphErrors(ncharge, chargeArr.data(), totLongArrI[idx].data(), chargeErrArr.data(), totErrLongArrI[idx].data());
          grTotLongI[idx].SetName(totLongGraphPrefix + idx);
          //NB original code not using 'Long' version of totSigArrI
          grTotLongSigI[idx] = TGraphErrors(ncharge, chargeArr.data(), totSigArrI[idx].data(), chargeErrArr.data(), totSigErrArrI[idx].data());
          grTotSigI[idx].SetName(sigmaLongGraphPrefix + idx);
          //
          f1TotLongI[idx]=TF1(f1TotLongPrefix + idx,funcTot, FitStartingPoint, chargeArr[ncharge-1]+100, 3);
          f1DispLongI[idx]=TF1(f1DispLongPrefix +idx,funcDisp, FitStartingPoint, chargeArr[ncharge-1]+100, 2);
          gROOT->GetListOfFunctions()->Add(&f1TotLongI[idx]);
          gROOT->GetListOfFunctions()->Add(&f1DispLongI[idx]);
          if (Fit2Parameter) { f1TotLongI[idx].FixParameter(0,fixAparam); }  // 2-parameter fitting mode
          grTotLongI[idx].Fit(&f1TotLongI[idx],"MRQ");
          grTotLongSigI[idx].Fit(&f1DispLongI[idx],"MRQ");
          //
          longFitParams[idx].extractFromFit(f1TotLongI[idx]);
          longLinearFitparams[idx].extractFromFit(f1DispLongI[idx]);
        }
        std::array<float, nChips>  badcalI_max{};
        for (int idx=0;idx !=nChips;++idx){
          std::tie(badcalI_max[idx], std::ignore) = findMaxAnomaly(fitParams[idx], totArrI[idx], chargeArr, qthresh);
        }
        std::array<std::vector<Double_t>, nChips> chargeArrI_re;
        std::array<std::vector<Double_t>, nChips> chargeErrArrI_re;
        std::array<std::vector<Double_t>, nChips> totArrI_re;
        std::array<std::vector<Double_t>, nChips> totErrArrI_re;
        std::array<std::vector<Double_t>, nChips> totSigArrI_re;
        std::array<std::vector<Double_t>, nChips> totSigErrArrI_re;
       
        for (int idx=0;idx != nChips;++idx){
          chargeArrI_re[idx].assign(chargeArr.begin(), chargeArr.end());
          chargeErrArrI_re[idx].assign(chargeErrArr.begin(), chargeErrArr.end());
          totArrI_re[idx].assign(totArrI[idx].begin(), totArrI[idx].end());
          totErrArrI_re[idx].assign(totErrArrI[idx].begin(), totErrArrI[idx].end());
          totSigArrI_re[idx].assign(totSigArrI[idx].begin(), totSigArrI[idx].end());
          totSigErrArrI_re[idx].assign(totSigErrArrI[idx].begin(), totSigErrArrI[idx].end());
        }

        const double chi2_error = 0.05;
        fprintf(outputfile, "%s", modName.Data());
        
        for (int idx=0;idx!=nChips;++idx){
          fprintf(outputfile, "\n");
          int ncharge_re = ncharge;
          fprintf(outputfile, "[ ");
          while(badcalI_max[idx] > chi2_error){
            if (ncharge_re < ncharge/2.0){
              fitParams[idx].invalidate();
              break;
            }else{
              int n_max{};
              std::tie(std::ignore, n_max) = findMaxAnomaly(fitParams[idx], totArrI_re[idx], chargeArrI_re[idx], qthresh);
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
              grTotI[idx].SetName(totGraphPrefix + idx);
              grTotSigI[idx] = TGraphErrors (ncharge_re, chargeArrI_re[idx].data(), totSigArrI_re[idx].data(), chargeErrArrI_re[idx].data(), totSigErrArrI_re[idx].data());
              grTotSigI[idx].SetName(sigmaGraphPrefix + idx);
              f1TotI[idx] = TF1 (f1TotNamePrefix + idx,funcTot, FitStartingPoint, chargeArrI_re[idx][ncharge_re-1]+100, 3);
              f1DispI[idx] = TF1(f1DispNamePrefix + idx,funcDisp, FitStartingPoint, chargeArrI_re[idx][ncharge_re-1]+100, 2);
              if (Fit2Parameter) { f1TotI[idx].FixParameter(0,fixAparam); }  // 2-parameter fitting mode
              gROOT->GetListOfFunctions()->Add(&f1TotI[idx]);
              gROOT->GetListOfFunctions()->Add(&f1DispI[idx]);
              grTotI[idx].Fit(&f1TotI[idx],"MRQ");
              grTotSigI[idx].Fit(&f1DispI[idx],"MRQ");
              fitParams[idx].extractFromFit(f1TotI[idx]);
              linearFitParams[idx].extractFromFit(f1DispI[idx]);
              constexpr bool inFirstTen = true;
              std::tie(badcalI_max[idx], std::ignore) = findMaxAnomaly(fitParams[idx], totArrI_re[idx], chargeArrI_re[idx], qthresh, inFirstTen);
            }
          }
          fprintf(outputfile, "]");
        }
        fprintf(outputfile, "\n");
        const int n = (whichPart == 0)? 2:nChips;
        std::cout << modName << std::endl;
        for (int idx=0;idx!=n;++idx){
          const std::string chipName="I"+std::to_string(idx);
          std::cout << chipName <<" "
          << int(pcdMap[modName][idx][THR_NORM]) << " " << int(pcdMap[modName][idx][THR_RMS_NORM]) << " "
          << int(pcdMap[modName][idx][THR_SIG_NORM]) << " " << int(timMap[modName][idx][TIM_NORM]) << " "
          << int(pcdMap[modName][idx][THR_LONG]) << " " << int(pcdMap[modName][idx][THR_RMS_LONG]) << " "
          << int(pcdMap[modName][idx][THR_SIG_LONG]) << " " << int(timMap[modName][idx][TIM_LONG]) << " "
          << int(pcdMap[modName][idx][THR_GANG]) << " " << int(pcdMap[modName][idx][THR_RMS_GANG]) << " "
          << int(pcdMap[modName][idx][THR_SIG_GANG]) << " " << int(timMap[modName][idx][TIM_GANG]) << " "
          << fitParams[idx].A << " " << fitParams[idx].E << " " << fitParams[idx].C << " "
          << longFitParams[idx].A << " " << longFitParams[idx].E << " " << longFitParams[idx].C << " "
          << linearFitParams[idx].P0<< " " << linearFitParams[idx].P1
          << std::endl;
        }
      }
    }
  }
  fclose(outputfile);
}

int main(){
  PixelCalib();
  return 0;
}
