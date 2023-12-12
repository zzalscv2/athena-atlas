//  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#include "GlobalAlgs.h"
#include "GlobalData.h"
#include "GlobalData.tpl"

#include "DataRepository.h"

#include "L1TopoEvent/TopoInputEvent.h"
#include "L1TopoCommon/Types.h"
#include "L1TopoInterfaces/Count.h"
#include "L1TopoInterfaces/Decision.h"
#include "L1TopoInterfaces/CountingAlg.h"
#include "L1TopoInterfaces/SortingAlg.h"
#include "L1TopoInterfaces/DecisionAlg.h"

#include "L1TopoCoreSim/DecisionConnector.h"

#include "L1TopoInterfaces/ConfigurableAlg.h"

/*
 * This file provides a PABC IGS_Alg for GlobalSim algs.
 * It also provides implementation classes to run Algorithms
 * taken from L1TopoInterfaces.
 *
 * The implementation classes are currently used by GobalL1TopoSimulation
 *
 */

namespace GlobalSim {

  Alg_gsRoot::Alg_gsRoot(std::size_t out_sn) : m_out_sn{out_sn} {
  }

  Alg_gsRoot::~Alg_gsRoot() = default;

  void Alg_gsRoot::run(DataRepository&,
		     const TCS::TopoInputEvent&) {
  }

  std::ostream& Alg_gsRoot::print(std::ostream& os) const {
    os << "Alg_gsRoot. output_sn " << m_out_sn << " does nothing (yet)\n";
    return os;
  }

  
  Alg_gsInput::Alg_gsInput(std::size_t out_sn,
			   const std::string& inputSelStr) :
    m_out_sn{out_sn},
    m_inputSelStr{inputSelStr}{    
  }
  
  Alg_gsInput::~Alg_gsInput() = default;
  
  void Alg_gsInput::run(DataRepository& repo,
			const TCS::TopoInputEvent& event) {
    
    const TCS::InputTOBArray* result =
      event.inputTOBs(TCS::inputType(m_inputSelStr));

    
    repo.write(GlobalData<const TCS::InputTOBArray*>(m_out_sn,
						     result));
  }
  
  std::ostream& Alg_gsInput::print(std::ostream& os) const {
    os << "Alg_gsInput. output_sn " << m_out_sn << " sel string "
       << m_inputSelStr;
    return os;
  }

  Alg_gsCounting::Alg_gsCounting(std::size_t in_sn,
				 std::size_t out_sn,
				 TCS::ConfigurableAlg* alg) :
    m_in_sn{in_sn}, m_out_sn(out_sn)
  {
    // why no virtual 'run' method in the base class?
    m_alg = dynamic_cast<TCS::CountingAlg*>(alg);
    if (!m_alg) {
      throw std::runtime_error("Error casting ConfigurableAlgorithm to CountingAlg");
    }
    
  }
    
  Alg_gsCounting::~Alg_gsCounting() = default;
    
  void Alg_gsCounting::run(DataRepository& repo,
			   const TCS::TopoInputEvent&)
  {
    auto result = TCS::Count();
    auto nbits = m_alg->numberOutputBits();
    result.setNBits(nbits);

    const auto& gsData = repo.read<const TCS::InputTOBArray*>(m_in_sn);
    const TCS::InputTOBArray* pdata = gsData.data();
    const TCS::InputTOBArray& data = *pdata;

    m_alg->process(data, result);

    repo.write(GlobalData<TCS::Count>(m_out_sn,
				      result));
  }

  std::ostream& Alg_gsCounting::print(std::ostream& os) const {
    os << "Alg_gsCounting. "
       << " input_sn " << m_in_sn
       << " output_sn " << m_out_sn;
    return os;
  }
  
  Alg_gsSorting::Alg_gsSorting(std::size_t in_sn,
			       std::size_t out_sn,
			       TCS::ConfigurableAlg* alg) :
    m_in_sn{in_sn}, m_out_sn(out_sn) {
    m_alg = dynamic_cast<TCS::SortingAlg*>(alg);
    if (!m_alg) {
      throw std::runtime_error("Error casting ConfigurableAlgorithm to SortingAlg");
    }
  }
  
  Alg_gsSorting::~Alg_gsSorting() = default;
    
  void Alg_gsSorting::run(DataRepository& repo,
			  const TCS::TopoInputEvent&) {
    
    auto result = TCS::TOBArray(); 
    
    m_alg->sort(*(repo.read<const TCS::InputTOBArray*>(m_in_sn).data()),
		result);

    
    repo.write(GlobalData<TCS::TOBArray>(m_out_sn,
					 result));
  }
  
  
  std::ostream& Alg_gsSorting::print(std::ostream& os) const {
    os << "Alg_gsSorting. "
       << " input_sn " << m_in_sn
       << " output_sn " << m_out_sn
       << '\n';
    return os;
  }

  
  Alg_gsDecision::Alg_gsDecision(const std::vector<std::size_t>& in_sns,
				 const std::size_t out_sn,
				 const std::vector<TrigConf::TriggerLine>& tls,
				 TCS::ConfigurableAlg* alg
				 ) :
    m_in_sns{in_sns}, m_out_sn{out_sn}, m_triggerLines{tls},
    m_outputVec(tls.size(), nullptr){

    m_alg = dynamic_cast<TCS::DecisionAlg*>(alg);
    if (!m_alg) {
      throw std::runtime_error("Error casting ConfigurableAlgorithm to DecisionAlg");
    }

    // from TopoSteeringStructure
    auto nbits = tls.size();
    m_alg->setNumberOutputBits(nbits);
    
  }
     
  void Alg_gsDecision::run(DataRepository& repo,
			   const TCS::TopoInputEvent&) {
       using ConstTOBArrayVec =std::vector<const TCS::TOBArray*>;
       
       auto input_vec = ConstTOBArrayVec();
       input_vec.reserve(m_in_sns.size());
       
       // obtain a number of previously created TOBArrays for the input
       std::transform(m_in_sns.cbegin(),
		      m_in_sns.cend(),
		      std::back_inserter(input_vec),
		      [&repo](const auto& sn) {
			return &(repo.read<TCS::TOBArray>(sn).data());});

	      
       // from TriggerSteering
       using TOBArrayVec =std::vector<TCS::TOBArray*>;
       auto nbits = m_alg->numberOutputBits();
       auto decision = TCS::Decision();
       decision.setNBits(nbits);

       if (m_triggerLines.size() != nbits) {
	 std::stringstream ss;
	 ss << "Error running Decision Alg: "
	    << m_alg->fullname()
	    << " alg nbits: "
	    << nbits
	    << " no of triggerLines " 
	    << m_triggerLines.size();
	 throw std::runtime_error(ss.str());
       }

       // in case the destructor has not been run since the last call to
       // this function:
       for(const auto& ptr : m_outputVec) {delete ptr;}
       
       for (std::size_t i = 0; i != nbits; ++i){
	 m_outputVec[i] = new TCS::TOBArray(m_triggerLines[i].name());
	 // not yet
	 // output_vec[i]->setAmbiguity(m_abiguities[i]);
       }	 

       m_alg->process(input_vec, m_outputVec,  decision);
       
       repo.write(GlobalData<TOBArrayVec>(m_out_sn,
					  m_outputVec));
       
       repo.write(GlobalData<TCS::Decision>(m_out_sn,
					    decision));
  }

  Alg_gsDecision::~Alg_gsDecision() {
    for(const auto& ptr : m_outputVec) {delete ptr;}
  }
  
  std::ostream& Alg_gsDecision::print(std::ostream& os) const {
    os << "Alg_gsDecision "
       << " input_sns [" << m_in_sns.size() << "] ";
    for (const auto& i : m_in_sns) {
      os << i << ' ';
    }
    
    os << " output_sn " << m_out_sn
       << " alg name " << m_alg->fullname()
       << '\n';
    return os;
  }
   
 }
