/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

// Dear emacs, this is -*-c++-*-
//----------------------------------------------------------------------------------------

#ifndef TFORWARDELECTRONLIKELIHOODTOOL_H
#define TFORWARDELECTRONLIKELIHOODTOOL_H


#include <fstream>
#include <iostream>

// ROOT includes
#include "TFile.h"
#include "TH1.h"

// Include the return objects and the base class
#include "PATCore/TAccept.h"
#include "PATCore/TResult.h"
#include "PATCore/TCalculatorToolBase.h"
#include "PATCore/TSelectorToolBase.h"
#include "AsgTools/AsgMessaging.h"
#include "SafeTH1.h"

namespace LikeEnumForward {
  enum Menu {
    Loose,
    Medium,
    Tight,
    CustomOperatingPoint
  };

  struct LHAcceptVars_t{
    double likelihood;
    double eta;
    double eT;
    double ip;
  };

  struct LHCalcVars_t{
    double eta;
    double eT;
    double secondLambda;
    double lateral; 
    double longitudinal; 
    double fracMax; 
    double centerLambda;
    double secondR;
    double secondDensity;
    double significance;
    double ip;
  };

  struct ROOT6_NamespaceAutoloadHook{};
}

namespace Root {
  class TForwardElectronLikelihoodTool : public TCalculatorToolBase, public TSelectorToolBase, public asg::AsgMessaging
  {

  public: 
    /// Standard constructor

    TForwardElectronLikelihoodTool(const char* name = "TForwardElectronLikelihoodTool");
    
    /// Standard destructor
    ~TForwardElectronLikelihoodTool();

  public:
    /// Initialize this class
    int initialize();

    /// Finalize this class; everything that should be done after the event loop should go here
    int finalize() {return 1;}

    /// The main accept method: the actual cuts are applied here
    const Root::TAccept& accept(LikeEnumForward::LHAcceptVars_t& vars_struct) const;
    const Root::TAccept& accept( double likelihood,
				 double eta, 
				 double eT,
				 double ip ) const;
    const Root::TResult& calculate(LikeEnumForward::LHCalcVars_t& vars_struct) const ;
    const Root::TResult& calculate(  
				   double eta,
				   double eT,
				   double secondLambda, 
				   double lateral, 
				   double longitudinal, 
				   double centerLambda,
				   double fracMax,
				   double secondR,
				   double significance,
				   double secondDensity,
				   double ip ) const;
    
    ///Reset the internal (TSelectorToolBase) m_accept and return it 
    const Root::TAccept& cleanTAccept() const{
      m_accept.clear();
      return m_accept;
    }

    /// Add an input file that holds the PDFs
    inline void setPDFFileName ( const std::string& val ) { PdfFileName = val; }

    /// Load the variable histograms from the pdf file.
    int LoadVarHistograms(std::string vstr, unsigned int varIndex);


    /// Set the prefix of the result name
    inline void setResultPrefix ( const std::string& val ) { m_resultPrefix = val; }
   
    /// The string for the result
    inline void setResultName ( const std::string& val ) { m_resultName = val; }

    // Private methods
  private:
   
    // For every input "varVector", make sure elements of vector are
    // in the same order as prescribed in fVariables
    /// Internal methods to calculate the LH discriminant from a set of variables
    double EvaluateLikelihood(std::vector<double> varVector,double et,double eta,double ip=0) const;


    ////Mask out the variables ,out of all possible ones, 
    ///that are not employed in the current configuration
    ///as read from the input config file 
    unsigned int GetLikelihoodBitmask(std::string vars) const;

    
    double InterpolateCuts(const std::vector<double>& cuts,const std::vector<double>& cuts_4gev,double et,double eta) const;
    double InterpolatePdfs(unsigned int s_or_b,unsigned int ipbin,double et,double eta,int bin,unsigned int var) const;
    /// Apply a transform to zoom into the LH output peaks.
    double TransformLikelihoodOutput(double ps,double pb) const;
    /// Eta binning for pdfs and discriminant cuts.
    unsigned int getLikelihoodEtaBin(double eta) const ;
    /// Et binning for for the likelihood pdfs and discriminants.
    unsigned int getLikelihoodEtHistBin(double et)const ;
    //Pile-up binning    
    unsigned int getIpBin(double ip) const;
    //get the bin names as is in the input file
    void getBinName(char* buffer, int etbin,int etabin) const;
  
  public:
    /** @brief range of eta bins for e-ID*/
    std::vector<float> CutBinEta_ForwardElectron;
    /** @brief cut on secondlambda*/
    std::vector<float> CutSECONDLAMBDA_ForwardElectron;
    /** @brief cut on lateral*/
    std::vector<float> CutLATERAL_ForwardElectron;
    /** @brief cut on longitudinal*/
    std::vector<float> CutLONGITUDINAL_ForwardElectron;
    /** @brief cut on maxFrac*/
    std::vector<float> CutCELLMAXFRAC_ForwardElectron;
    /** @brief cut values for cut on secondR */
    std::vector<float> CutSECONDR_ForwardElectron;
    /** @brief cut on centerlambda*/
    std::vector<float> CutCENTERLAMBDA_ForwardElectron;
    std::vector<float> CutSECONDDENSITY_ForwardElectron;
    /** @brief do pileup-dependent correction on discriminant value*/
    bool doPileupCorrection;
    /** @brief cut on likelihood output*/
    std::vector<double> CutLikelihood;
    /** @brief pileup correction factor for cut on likelihood output*/
    std::vector<double> CutLikelihoodPileupCorrection;
     /** @brief pileup slope factor for cut on likelihood output*/
    std::vector<double> CutLikelihoodPileupCorrectionA;
     /** @brief pileup constant factor for cut on likelihood output*/
    std::vector<double> CutLikelihoodPileupCorrectionB;
    /** @brief variables to use in the LH*/
    std::string VariableNames;
    /** The operating point for the final cuts*/
    LikeEnumForward::Menu OperatingPoint;
    /** Name of the pdf file*/
    std::string PdfFileName;

     // Private member variables
  private:

    /// Pointer to the opened TFile that holds the PDFs
    TFile*  m_pdfFile;

    /// The prefix string for the result
    std::string m_resultPrefix;

    /// The string for the result
    std::string m_resultName;

    /// The position of the kinematic cuts bit in the TAccept return object, separate for eta/Et
    int m_cutPosition_kinematicEta;
    int m_cutPosition_kinematicEt ;
    /// The position of the likelihood cut bit in the TAccept return object
    int m_cutPosition_LH;
    /// The position of the likelihood value bit in the TResult return object
   
    int m_resultPosition_LH;

    static const unsigned int  s_IP_FBINS=1;
    static const double s_fIpBounds[s_IP_FBINS+1];
    static const unsigned int  s_fnEtBinsHist     = 4;  // number of hists stored for nominal LH
    static const unsigned int  s_fnDiscEtBins     = 4;  // number of discs stored for original LH
    static const unsigned int  s_fnEtaBins        = 10;
    static const unsigned int  s_fnVariables      = 8;
    static const char*  s_fVariables  [s_fnVariables]; 
    EGSelectors::SafeTH1* m_fPDFbins [2][s_IP_FBINS][s_fnEtBinsHist][s_fnEtaBins][s_fnVariables]; // [sig(0)/bkg(1)][ip][et][eta][variable]
  };

} // End: namespace Root

#endif
