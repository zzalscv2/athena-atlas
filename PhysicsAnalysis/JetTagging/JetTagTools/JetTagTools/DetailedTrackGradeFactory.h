/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGTOOLS_DetailedTrackGradeFactory_H
#define JETTAGTOOLS_DetailedTrackGradeFactory_H
/**
   @class DetailedTrackGradeFactory
   Package : JetTagTools
   @author  Marc Lehmacher & Giacinto Piacquadio
   Created : 20 May 2008

   DetailedTrackGradeFactory provides grades in addition
   to the BasicTrackGradeFactory "Good" and "Shared" grades,
   namely grades in pT(Track), |eta|(Track), ptFrac=pT(Track)/pT(Jet)
   and HitBLayer

*/

#include "AthenaBaseComps/AthAlgTool.h"
#include "JetTagTools/ITrackGradeFactory.h"
#include <string>
#include <vector>

#include "GaudiKernel/ToolHandle.h"
#include "xAODTracking/TrackParticle.h"

#include "JetTagInfo/TrackGradesDefinition.h"

//namespace xAOD { class TrackParticle; }
  

namespace Analysis
{

  class TrackGrade;

class DetailedTrackGradeFactory : public AthAlgTool, virtual public ITrackGradeFactory
{
 public:

  DetailedTrackGradeFactory(const std::string&,const std::string&,const IInterface*);
  virtual ~DetailedTrackGradeFactory();

  /** AlgTool initailize method */
  StatusCode initialize();
  /** AlgTool finalize method */
  StatusCode finalize();

  TrackGrade* getGrade(const xAOD::TrackParticle & track,
		       const xAOD::IParticle::FourMom_t & jetMomentum) const;

  virtual const TrackGradesDefinition & getTrackGradesDefinition() const;

private:


  TrackGradesDefinition m_trackGradesDefinition;

  bool m_hitBLayerGrade;   // grade for tracks without hit in Blayer

  bool m_useSharedHitInfo; /// if false the following cuts are ignored
  bool m_useDetailSharedHitInfo; //blayer, pixel or sct shared hits
  int m_nSharedBLayer;/// max. number of shared hits in B layer
  int m_nSharedPix;   /// max. number of shared hits in pixels
  int m_nSharedSct;   /// max. number of shared hits in SCT
  int m_nSharedSi;    /// max. number of shared hits in pixels+SCT

  bool m_ptFracGrade;   // grade for tracks with ptFrac < m_ptFracCut
  double m_ptFracCut;    /// cut on ptFrac

  bool m_ptEtaGrades;    /// grades in pt and/or |eta| of tracks
  std::vector<double> m_ptLowerCuts;


  std::vector<double> m_etaLowerCuts;

};

}
#endif /// JETTAGTOOLS_DetailedTrackGradeFactory_H

