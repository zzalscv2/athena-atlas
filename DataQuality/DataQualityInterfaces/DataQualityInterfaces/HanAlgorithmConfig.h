/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef dqiHanAlgorithmConfig_h
#define dqiHanAlgorithmConfig_h

#include "dqm_core/AlgorithmConfig.h"
#include "dqm_core/exceptions.h"

class TDirectory;
class TFile;
class TKey;


namespace dqi {

class HanConfigAssessor;


class HanAlgorithmConfig : public dqm_core::AlgorithmConfig {
public:

  HanAlgorithmConfig( const HanConfigAssessor& hca, TFile* config );

  HanAlgorithmConfig( TObject* reference, 
                      const std::map< std::string, double >& parameters,
                      const std::map< std::string, std::string>& stringParameters,
                      const std::map< std::string, double >& greenThresholds,
                      const std::map< std::string, double >& redThresholds,
                      const HanConfigAssessor* hca);

  virtual ~HanAlgorithmConfig();

  virtual TObject* getReference() const override;

protected:

  void CopyAlgConfig( const HanConfigAssessor& hca );

  void CopyRuntimeConfig();

  TFile* m_file;
  
  TObject*  m_ref;
  const HanConfigAssessor* m_hca;

private:
};

} // namespace dqi

#endif

