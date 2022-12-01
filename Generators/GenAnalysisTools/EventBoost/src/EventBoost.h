/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////
//                                             //
//  File:  EventBoost.h                        //
//                                             //
//  Algorithm designed to                      //
//  boost the HepMC record according           //
//  beam inclination.                          //
//                                             //
//  Author: Esben Klinkby <klinkby@nbi.dk>     //
//                                             //
/////////////////////////////////////////////////

#ifndef EVENTBOOST_H
#define EVENTBOOST_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"
#include "CxxUtils/checker_macros.h"

#include <string>


class ATLAS_NOT_THREAD_SAFE EventBoost: public AthAlgorithm {
//    ^ const_cast in AnalyseGenEvent
public:

  //Standard algorithm methods:

  EventBoost( const std::string& name,
			ISvcLocator* pSvcLocator) ;
			 
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
  virtual StatusCode finalize() override;

private:
  StatusCode Analyse_BeginEvent();
  StatusCode Analyse_EndEvent();
  StatusCode GenAnalysis_initialize();
  StatusCode GenAnalysis_finalize();
  StatusCode AnalyseGenEvent ATLAS_ARGUMENT_NOT_CONST_THREAD_SAFE (const HepMC::GenEvent*);
  StatusCode EventCopy(const HepMC::GenEvent* evt) const;

  bool doModification(HepMC::GenParticlePtr part, double& pxsum);
  bool doVertexModification(HepMC::GenVertexPtr ver, double rand_x, double rand_y, double rand_z); 

  int m_nModifiedEvent;
  int m_nFailedEvent;
  int m_nModifiedTotal;
  int m_nFailedTotal;

  double m_beam_inclination;

  std::string m_genEvtInKey;
  std::string m_genEvtOutKey;
  bool m_gaussian_vertex_smearing;
  bool m_flat_vertex_smearing;

  double m_flat_rand_x;
  double m_flat_rand_y;
  double m_flat_rand_z;
  double m_gauss_rand_x;
  double m_gauss_rand_y;
  double m_gauss_rand_z;

  double m_pxsum;

  std::vector<double> m_gaussian_width;
  std::vector<double> m_gaussian_mean;
  std::vector<double> m_flat_smearing_boundary_min;
  std::vector<double> m_flat_smearing_boundary_max;

};

#endif
