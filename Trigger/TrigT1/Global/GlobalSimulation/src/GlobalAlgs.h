//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBSIM_GLOBALALGS_H
#define GLOBSIM_GLOBALALGS_H

#include "IGlobalAlg.h"
#include "TrigConfData/L1Connector.h" // for TriggerLine
#include "L1TopoEvent/TOBArray.h"
#include <vector>
#include <memory>

/*
 * This file provides implementaitons for GlobalSim algs.
 *
 * The implementation classes are currently used by GobalL1TopoSimulation
 *
 */

namespace TCS {
  class TopoInputEvent;
  class CountingAlg;
  class SortingAlg;
  class DecisionAlg;
  class ConfigurableAlg;
  class TriggerLine;
}

namespace GlobalSim {
  class DataRepository;

  class Alg_gsRoot : public IGlobalAlg {
  public:
    Alg_gsRoot(std::size_t out_sn);
    virtual ~Alg_gsRoot();
    virtual void run(DataRepository&,
		     const TCS::TopoInputEvent&) override;
    virtual std::ostream& print(std::ostream& os) const override;
  private:
    std::size_t m_out_sn;
  };

  class Alg_gsInput : public IGlobalAlg{
  public:
    Alg_gsInput(std::size_t out_sn,
		const std::string& inputSelStr);

    virtual ~Alg_gsInput();
    virtual void run(DataRepository& repo,
		     const TCS::TopoInputEvent& event) override;
    virtual std::ostream& print(std::ostream& os) const override;
    
  private:
    std::size_t m_out_sn;
    std::string m_inputSelStr;
  };


  class Alg_gsCounting : public IGlobalAlg{
  public:
    Alg_gsCounting(std::size_t in_sn,
		   std::size_t out_sn,
		   TCS::ConfigurableAlg* alg);

    virtual ~Alg_gsCounting();
    virtual void run(DataRepository& repo,
		     const TCS::TopoInputEvent&) override;
    virtual std::ostream& print(std::ostream& os) const override;
  private:
    std::size_t m_in_sn;
    std::size_t m_out_sn;
    TCS::CountingAlg* m_alg;
  };
  
  
  class Alg_gsSorting : public IGlobalAlg{
  public:
    Alg_gsSorting(std::size_t in_sn,
		  std::size_t out_sn,
		  TCS::ConfigurableAlg* alg);

    virtual ~Alg_gsSorting();
    virtual void run(DataRepository& repo,
		     const TCS::TopoInputEvent&) override;
    virtual std::ostream& print(std::ostream& os) const override;
    
  private:
    std::size_t m_in_sn;
    std::size_t m_out_sn;
    TCS::SortingAlg* m_alg;
    
  };
  
   class Alg_gsDecision : public IGlobalAlg{
   public:
     Alg_gsDecision(const std::vector<std::size_t>& in_sns,
		    const std::size_t out_sn,
		    const std::vector<TrigConf::TriggerLine>&,
		    TCS::ConfigurableAlg* alg);
     
     virtual ~Alg_gsDecision();
     virtual void run(DataRepository& repo,
		      const TCS::TopoInputEvent&) override;
     virtual std::ostream& print(std::ostream& os) const override;
   private:
     std::vector<std::size_t> m_in_sns;
     
     std::size_t m_out_sn;
     std::vector<TrigConf::TriggerLine> m_triggerLines;
     TCS::DecisionAlg* m_alg;
     using TOBArrayVec =std::vector<TCS::TOBArray*>;
     TOBArrayVec m_outputVec;
   };
   
 }


#endif
