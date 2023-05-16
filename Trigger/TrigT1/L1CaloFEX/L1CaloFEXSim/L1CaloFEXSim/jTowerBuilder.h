/*
    Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#ifndef JTOWERBUILDER_H
#define JTOWERBUILDER_H

// STL
#include <string>

// Athena/Gaudi
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IjTowerBuilder.h"
#include "CaloEvent/CaloCellContainer.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"

#include "TH1F.h"
#include "TH1I.h"
#include "TFile.h"

class CaloIdManager;

namespace LVL1 {

class jTowerBuilder: public AthAlgTool, virtual public IjTowerBuilder {

    public:
        jTowerBuilder(const std::string& type,const std::string& name,const IInterface* parent);
        virtual ~jTowerBuilder() = default;
        virtual StatusCode initialize() override;

        virtual void init(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) override ;
        virtual void execute(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) override ;
        virtual void reset() override ;



    private:
        void BuildEMBjTowers  (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildTRANSjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildEMEjTowers  (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildEMIEjTowers (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildFCALjTowers (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildHECjTowers  (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildAllTowers   (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;
        void BuildSingleTower (std::unique_ptr<jTowerContainer> & jTowerContainerRaw,float eta, float phi, int key_eta, float keybase, int posneg, float centre_eta = 0.0, float centre_phi = 0.0, int fcal_layer = -1) const;

        void AssignPileupAndNoiseValues (std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const;

        static constexpr float m_TT_Size_phi = M_PI/32;
        static constexpr float m_TT_Size_phi_FCAL = M_PI/16;


        //property for jFEX mapping
        Gaudi::Property<std::string> m_PileupWeigthFile {this, "PileupWeigthFile", "Run3L1CaloSimulation/Noise/jTowerCorrection.20210308.r12406.root", "Root file for the pileup weight"};
        Gaudi::Property<std::string> m_PileupHelperFile {this, "PileupHelperFile", "Run3L1CaloSimulation/Calibrations/jFEX_MatchedMapping.2022Mar10.r12406.root", "Root file to set the jTower coordinated (float eta/phi)"};

        //histograms need to set coordinates and noise subtraction
        TH1F* m_jTowerArea_hist;
        TH1I* m_Firmware2BitwiseID;
        TH1I* m_BinLayer;
        TH1F* m_EtaCoords;
        TH1F* m_PhiCoords;

};

} // end of LVL1 namespace
#endif
