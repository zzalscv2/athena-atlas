/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETTAGINFOTPCNV_MSVVtxInfoCnv_P1_H
#define JETTAGINFOTPCNV_MSVVtxInfoCnv_P1_H

#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

#include "JetTagInfoTPCnv/MSVVtxInfo_p1.h"
#include "JetTagInfo/MSVVtxInfo.h"
#include "VxVertex/RecVertex.h"
#include "DataModelAthenaPool/ElementLinkVectorCnv_p1.h"

class MsgStream;


namespace Analysis {
  class MSVVtxInfoCnv_p1
    : public T_AthenaPoolTPCnvBase<MSVVtxInfo, MSVVtxInfo_p1>
    {
    public:
      inline MSVVtxInfoCnv_p1 (void)
	: m_recoVertexCnv(0)
	{}

      virtual void persToTrans(const MSVVtxInfo_p1 *persObj,
			       MSVVtxInfo *transObj,
			       MsgStream &log);
      virtual void transToPers(const MSVVtxInfo *transObj,
			       MSVVtxInfo_p1 *persObj,
			       MsgStream &log);

    private:

      ITPConverterFor<Trk::Vertex> *m_recoVertexCnv;
      ElementLinkVectorCnv_p1<ElementLinkVector<Rec::TrackParticleContainer> > m_trackVecCnv;
    };
}


#endif
