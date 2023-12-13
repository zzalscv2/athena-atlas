//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef GLOBALSIM_ALGDATAFETCHER_H
#define GLOBALSIM_ALGDATAFETCHER_H

/*
 * AlgDataFetcher constructs a sequence of AlgData instances,
 * The execOrder method builds a call graph, orders the sequence
 * in order of excution. Execution order respects the Algorithm
 * IO object dependencies - that is it ensures that an Algorithm
 * which provides an output that is an input of a later Algorthm
 * that the output is available when the latter Algorithm is run.
 *
 */
 

#include "IAlgDataFetcher.h"
#include <vector>
#include <map>
#include <string>

namespace TrigConf {
  class L1Menu;
  class L1Board;
  class L1Connector;
  class L1TopoAlgorithm;
  class TriggerLine;
}

namespace GlobalSim {
  enum class AlgDataDesc;
  class AlgDataFetcher : public IAlgDataFetcher {
    using ADP = std::shared_ptr<AlgData>;
    
  public:
    AlgDataFetcher(bool do_eConn,
		   bool do_oConn,
		   const TrigConf::L1Menu* l1menu);
    virtual ~AlgDataFetcher() {}
    
    virtual const std::vector<ADP>&
    algData() const override;

    // in execution order
    virtual std::vector<ADP>
    execOrderedAlgData() const override;
   
    virtual std::string toString() const override;
    virtual bool isValid () const override {return m_errMsgs.empty();}
    virtual std::vector<std::string>  errMsgs () const override {return m_errMsgs;}
    
  private:

    std::vector<const TrigConf::L1Board*>
    getBoards(const TrigConf::L1Menu* l1menu) const;

    template<typename Iter>
    std::vector<ADP>
    uniqueConfDecisionAlgs(Iter begin,
			   Iter end,
			   const TrigConf::L1Menu*) const;

    template<typename Iter>
    std::vector<ADP>
    uniqueConfCountingAlgs(Iter begin,
			       Iter end,
			       const TrigConf::L1Menu*) const;

    std::vector<ADP>
    uniqueConfSortingAlgs(const std::vector<ADP>&,
			  const TrigConf::L1Menu*) const;
 
    std::vector<ADP>
    uniqueConfInputAlgs(const std::vector<ADP>&) const;

    std::vector<ADP>
    uniqueConfRootAlg() const;

    std::vector<const TrigConf::L1Connector*>
      conf_connectors_(const TrigConf::L1Menu*,
		       const TrigConf::L1Board*) const;

    std::vector<ADP>
    adpsFromTriggerLines(std::vector<TrigConf::TriggerLine>&,
			 AlgDataDesc desc,
			 const TrigConf::L1Menu* l1menu
			 ) const;
    

    std::vector<ADP> m_algData;
    
    std::map<std::string, std::size_t> m_name2sn;
    std::map<std::size_t, std::string> m_sn2name;

    std::vector<std::string>  m_errMsgs;
  };
}
#endif
