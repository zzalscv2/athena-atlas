/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// PositionMomentumWriter.h
///////////////////////////////////////////////////////////////////

#ifndef TRK_POSITIONMOMENTUMWRITER_H
#define TRK_POSITIONMOMENTUMWRITER_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "TrkValInterfaces/IPositionMomentumWriter.h"

class TTree;

namespace Trk {

/** @class PositionMomentumWriter

    Position momentum writer

    @author Andreas.Salzburger -at- cern.ch
*/

  class PositionMomentumWriter : public AthAlgTool, virtual public IPositionMomentumWriter  {
    public:
      /** standard AlgTool constructor / destructor */
      PositionMomentumWriter(const std::string&,const std::string&,const IInterface*);
      ~PositionMomentumWriter(){}
      
      /** standard Athena methods */
      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      
      /** Initialize State */
      virtual void initializeTrack(const Amg::Vector3D& pos,
                                   const Amg::Vector3D& mom,
                                   double m, int pdg) override;
      
      
      /** Record a single TrackState */
      virtual void recordTrackState(const Amg::Vector3D& pos, const Amg::Vector3D& mom) override;
      
      /** Finalization State */
      virtual void finalizeTrack() override;
    
  private: 
         
      std::string                       m_treeName;
      std::string                       m_treeFolder;  
      std::string                       m_treeDescription;
      TTree*                    m_tree;

      float                     m_pM;
      float                     m_pEta;
      float                     m_pPhi;
      float                     m_pE;
      float                     m_eEta;
      float                     m_ePhi;
      float                     m_eE;
      int                       m_pPdg;
      std::vector< float >*     m_pPositionX;
      std::vector< float >*     m_pPositionY;      
      std::vector< float >*     m_pPositionZ;
      std::vector< float >*     m_pPositionR;
      std::vector< float >*     m_pMomentumX;
      std::vector< float >*     m_pMomentumY;      
      std::vector< float >*     m_pMomentumZ;
      std::vector< float >*     m_pMomentumMag;
      std::vector< float >*     m_pMomentumEta;
      std::vector< float >*     m_pMomentumPhi;


};


} // end of namespace

#endif // TRK_POSITIONMOMENTUMWRITER_H
