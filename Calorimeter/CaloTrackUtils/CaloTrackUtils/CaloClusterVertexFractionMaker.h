/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCLUSTERVERTEXFRACTIONMAKER_H
#define CALOCLUSTERVERTEXFRACTIONMAKER_H

#include "GaudiKernel/ToolHandle.h"
#include "CaloUtils/CaloClusterCollectionProcessor.h"
#include <string>
#include <vector>

class CaloClusterContainer;
namespace Trk { class IExtrapolator; class Surface; }

class CaloClusterVertexFractionMaker: public AthAlgTool, virtual public CaloClusterCollectionProcessor
{
 public:
  CaloClusterVertexFractionMaker(const std::string& type, const std::string& name, const IInterface* parent);
  ~CaloClusterVertexFractionMaker();

  using CaloClusterCollectionProcessor::execute;
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx,
                             xAOD::CaloClusterContainer* theClusColl) const override;

 private:
  static double calculateDPhi(double phi1, double phi2) ;

    const double m_CALO_INNER_R;        // inner radius of the EM barrel envelope in mm
    const double m_CALO_INNER_Z;
//     const double INV_CALO_INNER_R; // the inverse for error calculation
    double m_dRMatchMax;  // maximum value in dR for the track to be matched to the cluster, can be set by jobOptions
    double m_dR2MatchMax; // m_dRMatchMax * m_dRMatchMax, this is used to avoid taking the square root all the time for each track and cluster
    double m_maxClusterEta; // maximum |eta| of clusters for which a CVF is calculated

    // Pre-configured extrapolator
    ToolHandle< Trk::IExtrapolator > m_extrapolator;

    // name of VxContainer
    std::string m_vxContainerName;

    std::unique_ptr<Trk::Surface> m_cylinderSurface_atCaloEntrance; // to extrapolate to clusters in barrel
    std::unique_ptr<Trk::Surface> m_discSurface_atCaloEntrance_positiveZ; // to extrapolate to clusters in endcap, this surface has to be created new for each cluster ...
    std::unique_ptr<Trk::Surface> m_discSurface_atCaloEntrance_negativeZ; // to extrapolate to clusters in endcap, this surface has to be created new for each cluster ...
};

#endif // CALOCLUSTERVERTEXFRACTIONMAKER_H






