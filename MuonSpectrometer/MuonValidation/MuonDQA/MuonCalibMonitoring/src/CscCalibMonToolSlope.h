/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCALIBMONITORING_CSCCALIBMONTOOLSLOPE_H
#define MUONCALIBMONITORING_CSCCALIBMONTOOLSLOPE_H

#include "CscCalibMonToolBase.h"

#include "CxxUtils/checker_macros.h"
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/MsgStream.h"

/////////////////////////////////////////////////////////////////////////////
class TH1F;
class TH1I;
class TProfile;
class CscCalibReportSlope;

/**
  @class CscCalibMonToolSlope
  @brief Class for monitoring CSC relative gain calibration results

  This class monitors the slope determined to be the relative gain between
  CSC chanels, as produced by CscCalcSlope.cxx in the CscCalibAlgs package. 
  It produces numerous histograms to illustrate  the results. It runs during 
  the finalize stage of the job. The info it 
  looks at includes:
  -The slope and intercept and chi^2 of the fit
  -COOL slope.
  -Difference between slope and COOL slope

  CscCalibMonToolBase provides the basis for much of the operation of this class.
  Generally, the program flow osciallates between base class functions and
  CscCalibMonToolSlope functions.
*/
class ATLAS_NOT_THREAD_SAFE CscCalibMonToolSlope: public CscCalibMonToolBase
//    ^ modifying const histograms in StoreGate
{

    public:
        /**Constructor*/
        CscCalibMonToolSlope (const std::string & type, const std::string & name, 
                const IInterface* parent);

        /**Destructor*/
        ~CscCalibMonToolSlope() {};
      
        /**Initialize slope sepcific values. Also calls CscCalibMonToolBase::intialize*/
        StatusCode initialize();
        
        /**Delete slope sepcific values. Also calls CscCalibMonToolBase::finalize*/
        StatusCode finalize(); 
        
        
        /**Books all histograms not retrieved driectly from CscCalcSlope*/
        StatusCode bookHistograms();

    private:
        /**handleParameter sets things up for the base class procParameter. It makes
          decisions based on the parameter being passed in parVals on how to handle the data
          @param parVals Values for a particular parameter*/
        virtual StatusCode handleParameter(const CscCalibResultCollection* parVals);

    
        /**postProc retrieves the plots fit to produce the gain*/
        virtual StatusCode postProc();

        /**Make fractional deviation plots between the gain plot points and the fit*/
        StatusCode makeFracGraphs(const CscCalibReportSlope & slopeReport);
       
        /**Look for dead channels*/
        StatusCode findDeadChannels(const CscCalibReportSlope & slopeReport);
        
        //Leftover from when I was using more arrays. I should get rid of this or replace it with something more vector friendly
        //Maybe when I start using the slope program again more.. 2009-09-24
        std::vector<float> & setArray(std::vector<float> & array , const float &value, const int &numEntries);

        /**Generated next neighbor channel ratios of the slopes*/
        void genNeighborRatios(const std::vector<float> & source, std::vector<float> & ratios) const;

    private:
        /**Do neighbor ratios*/
        bool m_doNeighborRatios{};

        /**Histogram all values for all histograms*/
        bool m_histAttenLevels{};

        /**Maximum deviation from expected values allowed before m_h_numBad is incremented*/
        float m_slopeMaxDiff{},m_interceptMax{},m_chi2Max{}, m_fracDevMax{}, m_peaktMaxDiff{};

        /**Bin corresponding to each category in m_h_numbad*/
        int m_slopeBadBin{1}, m_interceptBadBin{2}, m_chi2BadBin{3}, m_peaktBadBin{4}, m_fracBadBin{5}, 
            m_deadBadBin{6},m_missingBadBin{7};

        int m_totalLiveBin{1}, m_totalDeadBin{2}, m_newLiveBin{3}, m_newDeadBin{4};
        /**Stores number of channels that are bad in several categories*/
        TH1I *m_h_numBad{};
        
        /**Histograms that simply histogram all entries for a value*/
        TH1F *m_h_slopeCompareOverview{}, *m_h_interceptOverview{},*m_h_chi2Overview{}, 
             *m_h_slopeMissingChans{};
        //TH1F* m_h_peaktCompareOverview;
        
        /**Overview of dead channels and changes*/
        TH1F *m_h_deadOverview{};
        
       /**Holds fractional deviation TProfiles*/
        std::vector<TProfile*> m_fracProfs;
        
        /**HistCollections. See CscCalibMonToolBase for definition*/ 
        HistCollection *m_slopeNewColl{}, *m_slopeOldColl{}, *m_slopeDiffColl{};
        HistCollection *m_peaktNewColl{}, *m_peaktOldColl{}, *m_peaktDiffColl{};
        HistCollection *m_interceptColl{};
        HistCollection *m_chi2Coll{};       
        HistCollection *m_deadNewColl{}, *m_deadDiffColl{};
        HistCollection *m_slopeRatioColl{};


        HistCollection * m_fitResColl{};

        DataVector<HistCollection> m_ampColls;

        /**Storegate key for pedestal amplitude histograms*/
        std::string m_histKey; 

        /**Name of subdirectory under EXPERT or SHIFT that the histograms will be stored*/
        std::string m_subDir;
        
        /**Dead channel finding user defined parameters*/
        int m_deadPulserLevelCutoff, m_deadADCCutoff;
        
        /**Hash Ids one would expect to be in calibration data*/
        //std::set<int> m_expectedHashIdsAll, m_expectedHashIdsPrec;
};


#endif
