/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeantFollowerHelper.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
#ifndef GeantFollowerHelper_H
#define GeantFollowerHelper_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkParameters/TrackParameters.h" //typedef, can't fwd declare
#include "TrkG4UserActions/IGeantFollowerHelper.h"
#include "G4ThreeVector.hh" //typedef, can't fwd declare

class TTree;

namespace Trk
{

  class IExtrapolator;

  class GeantFollowerHelper : public extends<AthAlgTool, IGeantFollowerHelper>
  {

    public:

      static constexpr int MAXPROBES{50000};

      // constructor, destructor
      GeantFollowerHelper(const std::string&,const std::string&,const IInterface*);
      virtual ~GeantFollowerHelper ();

      // Athena hooks
      virtual StatusCode initialize() override;
      virtual StatusCode finalize  () override;

      // Follower interface
      // a) begin event - initialize follower process
      virtual void beginEvent() override;
      // b) track the particle
      virtual void trackParticle(const G4ThreeVector& pos, const G4ThreeVector& mom, int pdg, double charge, float t, float X0) override;
      // c) end event - ntuple writing
      virtual void endEvent() override;

    private:

      ToolHandle<IExtrapolator>      m_extrapolator;
      bool                           m_extrapolateDirectly;
      bool                           m_extrapolateIncrementally;

      const TrackParameters* m_parameterCache;
      float                  m_tX0Cache;

      // put some validation code is
      std::string                    m_validationTreeName;        //!< validation tree name - to be acessed by this from root
      std::string                    m_validationTreeDescription; //!< validation tree description - second argument in TTree
      std::string                    m_validationTreeFolder;      //!< stream/folder to for the TTree to be written out

      TTree*                         m_validationTree;            //!< Root Validation Tree
      /** Ntuple variables : initial parameters
          Split this out into a separate, dynamically-allocated block.
          Otherwise, the CaloCellNoiseAlg is so large that it violates
          the ubsan sanity checks. **/
      struct TreeData {
          float                  m_t_x {0};
          float                  m_t_y {0};
          float                  m_t_z {0};
          float                  m_t_theta {0};
          float                  m_t_eta {0};
          float                  m_t_phi {0};
          float                  m_t_p {0};
          float                  m_t_charge {0};
          int                    m_t_pdg {0};
          /** Ntuple variables : g4 step parameters */
          int                    m_g4_steps {0};
          float                  m_g4_p[MAXPROBES] {0};
          float                  m_g4_eta[MAXPROBES] {0};
          float                  m_g4_theta[MAXPROBES] {0};
          float                  m_g4_phi[MAXPROBES] {0};
          float                  m_g4_x[MAXPROBES] {0};
          float                  m_g4_y[MAXPROBES] {0};
          float                  m_g4_z[MAXPROBES] {0};
          float                  m_g4_tX0[MAXPROBES] {0};
          float                  m_g4_t[MAXPROBES] {0};
          float                  m_g4_X0[MAXPROBES] {0};
          /** Ntuple variables : trk follow up parameters */
          int                    m_trk_status[MAXPROBES] {0};
          float                  m_trk_p[MAXPROBES] {0};
          float                  m_trk_eta[MAXPROBES] {0};
          float                  m_trk_theta[MAXPROBES] {0};
          float                  m_trk_phi[MAXPROBES] {0};
          float                  m_trk_x[MAXPROBES] {0};
          float                  m_trk_y[MAXPROBES] {0};
          float                  m_trk_z[MAXPROBES] {0};
          float                  m_trk_lx[MAXPROBES] {0};
          float                  m_trk_ly[MAXPROBES] {0};
      };
      std::unique_ptr<TreeData> m_treeData;
  };

}

#endif
