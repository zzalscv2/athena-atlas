/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*! \file SimpleAlgorithmConfig  Defines the class SimpleAlgorithmConfig a concrete simple implementation of dqm_core::AlgorithmConfig
 *  \author andrea.dotti@cern.ch
 */

#ifndef DQM_ALGORITHMS_TOOLS_SIMPLEALGORITHMCONFIG
#define DQM_ALGORITHMS_TOOLS_SIMPLEALGORITHMCONFIG
#include <TObject.h>
#include <dqm_core/AlgorithmConfig.h>
namespace dqm_algorithms 
{
  namespace tools 
    {
    /*! \brief This class provides a simple implementation of the DQMF abstract AlgorithmConfig interface
    	which can be used in dqm_algorithms (see AddReference algorithms).
       \ingroup public
      */
    class SimpleAlgorithmConfig : public dqm_core::AlgorithmConfig 
      {
      public:
	///Default constructor
	SimpleAlgorithmConfig();
	SimpleAlgorithmConfig(TObject *ref);
#ifndef __MAKECINT__
	///Copy constructor
	SimpleAlgorithmConfig(const AlgorithmConfig& conf);
#endif

		///Getters, interface defines in AlgorithmConfig
	virtual TObject * getReference() const override;
	///Setters
	void setReference(TObject* ref);
	void addParameter(std::string,double);//< Adds a new parameter to the list of current params
	void addGenericParameter(std::string,std::string);//< Adds a new string parameter to the list of current generic params
	void addGreenThreshold(std::string,double);//< Adds a new threshold to the list of current params
	void addRedThreshold(std::string,double);//< Adds a new threshold to the list of current params
      private:
	TObject* m_ref;
      };
  }

}

#endif
