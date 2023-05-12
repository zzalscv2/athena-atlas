/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           eFEXtauAlgo.h  -  
//                              -------------------
//     begin                : 06 05 2020
//     email                : nicholas.andrew.luongo@cern.ch
//  ***************************************************************************/


#ifndef eFEXtauAlgo_H
#define eFEXtauAlgo_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "L1CaloFEXSim/eFEXtauAlgoBase.h"
#include "L1CaloFEXSim/eFEXtauTOB.h"
#include "L1CaloFEXSim/eTowerContainer.h"

namespace LVL1 {
  
  //Doxygen class description below:
  /** The eFEXtauAlgo class calculates the tau TOB variables
  */
  
  class eFEXtauAlgo : public eFEXtauAlgoBase{
    
  public:
    /** Constructors */
    eFEXtauAlgo(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    
    /** Destructor */
    virtual ~eFEXtauAlgo();

    virtual void setup(int inputTable[3][3], int efex_id, int fpga_id, int central_eta) override;

    virtual std::unique_ptr<eFEXtauTOB> getTauTOB() override;
    virtual unsigned int rCoreCore() override;
    virtual unsigned int rCoreEnv() override;
    virtual unsigned int rHadCore() override;
    virtual unsigned int rHadEnv() override;
    virtual void getSums(unsigned int seed, bool UnD, 
                       std::vector<unsigned int> & RcoreSums, 
                       std::vector<unsigned int> & Remums) override;
    virtual unsigned int getEt() override;
    virtual unsigned int getBitwiseEt() override;

  protected:

  private:
    virtual void setSupercellSeed() override;
    virtual void setUnDAndOffPhi() override;
    virtual bool getUnD() override;
    virtual unsigned int getSeed() override;
	
    unsigned int m_seed;
    bool m_und;
    unsigned int m_offPhi;

  };
  
} // end of namespace

//CLASS_DEF( LVL1::eFEXtauAlgo , 140708609 , 1 )

#endif
