/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETPHYSVALMONITORING_TRACKTRUTHSELECTORTOOL_H
#define INDETPHYSVALMONITORING_TRACKTRUTHSELECTORTOOL_H 1

#include "AthenaBaseComps/AthAlgTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "xAODTruth/TruthParticle.h"
#include "AsgTools/AsgTool.h"

#include <atomic>
#include <mutex>

class TrackTruthSelectionTool:
  public virtual ::IAsgSelectionTool,
  public asg::AsgTool  {
  ASG_TOOL_CLASS1(TrackTruthSelectionTool, IAsgSelectionTool);
public:
  TrackTruthSelectionTool(const std::string& name);
  // TrackTruthSelectionTool(const std::string& type,const std::string& name,const IInterface* parent);
  virtual
  ~TrackTruthSelectionTool();

  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual const asg::AcceptInfo& getAcceptInfo( ) const override;
  virtual asg::AcceptData accept(const xAOD::IParticle* p) const override;
  virtual asg::AcceptData accept(const xAOD::TruthParticle* p) const;
private:
  asg::AcceptInfo m_accept;
  std::vector<std::pair<std::string, std::string> > m_cuts;
  mutable std::atomic<ULong64_t> m_numTruthProcessed; // !< a counter of the number of tracks proccessed
  mutable std::atomic<ULong64_t> m_numTruthPassed; // !< a counter of the number of tracks that passed all cuts
  mutable std::vector<ULong64_t> m_numTruthPassedCuts ATLAS_THREAD_SAFE; // !< tracks the number of tracks that passed each cut family. Guarded by m_mutex
  mutable std::mutex m_mutex; // !< To guard m_numTruthPassedCuts

  // Cut values;
  float m_maxEta;
  float m_maxPt;
  float m_minPt;
  bool m_requireOnlyPrimary;
  bool m_requireCharged;
  bool m_requireStatus1;
  // max decay radius for secondaries [mm];
  // set to within (Run2) pixel by default; set to <0 for no cut
  double m_maxProdVertRadius;
  int m_pdgId;
};

#endif // > !INDETPHYSVALMONITORING_TRACKTRUTHSELECTORTOOL_H
