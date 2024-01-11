/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetTagTools/SVForIPTool.h"

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

namespace Analysis {


  SVForIPTool::SVForIPTool(const std::string& name,
                           const std::string& n, const IInterface* p):
    AthAlgTool(name, n,p)
  {
    declareInterface<SVForIPTool>(this);
  }  

  void SVForIPTool::getDirectionFromSecondaryVertexInfo(Amg::Vector3D & SvxDirection,
                                                        bool & canUseSvxDirection,
                                                        xAOD::BTagging* BTag,
                                                        const std::string & secVxFinderName,
                                                        const xAOD::Vertex & priVtx) const
  {
    std::vector< ElementLink< xAOD::VertexContainer > > myVertices;
    BTag->variable<std::vector<ElementLink<xAOD::VertexContainer> > >(secVxFinderName, "vertices", myVertices);
    
    if (myVertices.empty()) {
      ATH_MSG_DEBUG(" No secondary vertex found for getting the B flight direction (for the IP sign calculation)");
    } else {
      if (myVertices[0].isValid()) {
	canUseSvxDirection=true;
	SvxDirection=(*myVertices[0])->position()-priVtx.position();
	ATH_MSG_VERBOSE(" Get direction from InDetVKalVertex: phi: " << SvxDirection.phi() <<
			" theta: " << SvxDirection.theta() );
      } else {
	ATH_MSG_WARNING("SVX info seems usable, but no SVX available !!!");
      }
    }
  }


  void SVForIPTool::getTrkFromV0FromSecondaryVertexInfo(std::vector<const xAOD::TrackParticle*> & TrkFromV0,
							xAOD::BTagging* BTag,
                                                        const std::string & secVxFinderName) const
  {
    std::vector<ElementLink<xAOD::TrackParticleContainer> > TrkFromV0_ELs;
    BTag->variable<std::vector<ElementLink<xAOD::TrackParticleContainer> > >(secVxFinderName, "badTracksIP", TrkFromV0_ELs);
    for (const auto& link : TrkFromV0_ELs) {
      if (link.isValid()) TrkFromV0.push_back(*link);
    }
  }
  
}//end namespace

