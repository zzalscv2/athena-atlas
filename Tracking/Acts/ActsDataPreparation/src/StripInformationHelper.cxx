/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "StripInformationHelper.h"

namespace ActsTrk {

    StripInformationHelper::StripInformationHelper (const unsigned int& idHash,
						    const Amg::Vector3D& stripStart,
                                                    const Amg::Vector3D& stripEnd,
                                                    const Amg::Vector3D& beamSpotVertex,
                                                    const float& locx,
                                                    const size_t& clusterIndex,
                                                    const size_t& stripIndex)
    {
        this->set(idHash, stripStart, stripEnd, beamSpotVertex, locx, clusterIndex, stripIndex);
    }

    void StripInformationHelper::set(const unsigned int& idHash,
				     const Amg::Vector3D& stripStart,
                                     const Amg::Vector3D& stripEnd,
                                     const Amg::Vector3D& beamSpotVertex,
                                     const float& locx,
                                     const size_t& clusterIndex,
                                     const size_t& stripIndex)
    {
        m_stripCenter  = 0.5*(stripStart+stripEnd);
        m_stripDir     = stripStart-stripEnd;
        m_trajDir      = 2.*(m_stripCenter-beamSpotVertex);
        m_normal       = m_stripDir.cross(m_trajDir);
        m_oneOverStrip = 1./m_stripDir.mag();
        m_locX         = locx;
        m_clusterIndex = clusterIndex;
        m_stripIndex   = stripIndex;
	m_idHash       = idHash;
    }

    Amg::Vector3D StripInformationHelper::position(const double& shift) const
    {
        return (m_stripCenter+(0.5*shift)*m_stripDir);
    }
}
