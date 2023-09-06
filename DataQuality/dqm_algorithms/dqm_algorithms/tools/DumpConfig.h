/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*! \file AlgorithmHelper.cpp does basic functions to get dqm_core::Results from algorithms 
 * \author Haleh Hadavand
 */
#ifndef DQM_ALGORITHMS_TEST_DUMPCONFIG_H
#define DQM_ALGORITHMS_TEST_DUMPCONFIG_H

#include <map>
#include <iostream>
#include <fstream>
#include <dqm_core/test/DummyAlgorithmConfig.h>

namespace dqm_algorithms
{
  namespace tools
    {
      class DumpConfig {
	public:
	DumpConfig(const std::string& ParameterName,
                   dqm_core::test::DummyAlgorithmConfig & config,
                   const std::string& algorithmname,
                   const std::string& histogramname,
                   const std::string& reffilename="",
                   const std::string& refhistogramname="",
                   float weight=1.,
                   const std::string& regionname="" );
	~DumpConfig();
	
	void DumpOnlineConfig(const std::string& filename,bool dumpAgent=true);
        void DumpOfflineConfig(const std::string& filename);

	private:
	void WriteThresholdFromMap(const std::map<std::string,double>& object,
                                   const std::string& ParameterName,
                                   const std::string& Name);
	void DumpThresholds();
	void DumpParams();
	void DumpRegion();
	void DumpAgent();

        std::ofstream m_myfile;
	std::map<std::string,double> m_params;
	std::map<std::string,std::string> m_strParams;
	std::map<std::string,double> m_gthresh;
	std::map<std::string,double> m_rthresh;
	std::vector<std::string> m_red_id;
	std::vector<std::string> m_green_id;
	std::vector<std::string> m_param_id;
     
	std::string m_ParameterName;
	dqm_core::test::DummyAlgorithmConfig  m_config;
	std::string m_regionname;
	std::string m_algorithmname;
	std::string m_histogramname;
	std::string m_refhistogramname;
	std::string m_reffilename;
	float m_weight;
	
    };
  }
}

#endif
