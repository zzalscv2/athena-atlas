/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class  SCT_ClusterOnTrackTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Interface for SCT_ClusterOnTrack production
///////////////////////////////////////////////////////////////////
// started 1/05/2004 I.Gavrilenko - see ChangeLog for details
///////////////////////////////////////////////////////////////////

#ifndef SCT_ClusterOnTrackTool_H
#define SCT_ClusterOnTrackTool_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

#include "InDetCondTools/ISiLorentzAngleTool.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCTRIO_OnTrackErrorScaling.h"
#include "SCT_ModuleDistortions/ISCT_ModuleDistortionsTool.h"
#include "TrkParameters/TrackParameters.h"

namespace InDet {


/** @brief creates SCT_ClusterOnTrack objects allowing to
    calibrate cluster position and error using a given track hypothesis. 

    See doxygen of Trk::RIO_OnTrackCreator for details.
    Different strategies to calibrate the cluster error can be chosen
    by job Option. Also the handle to the general hit-error scaling
    is implemented.
*/
  class SCT_ClusterOnTrackTool final: 
    public AthAlgTool,virtual public Trk::IRIO_OnTrackCreator
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:

  //! AlgTool constructor 
  SCT_ClusterOnTrackTool(const std::string&,const std::string&,const IInterface*);
  virtual ~SCT_ClusterOnTrackTool () = default;
  //! AlgTool initialisation
  virtual StatusCode initialize() override;
  //! AlgTool termination
  virtual StatusCode finalize  () override;

  
/** @brief produces an SCT_ClusterOnTrack using the measured
    SCT_Cluster and the track prediction. 

    This method is a factory, so the client has to take care
     of management/deletion of the  SCT_ClusterOnTrack.
 */
  virtual InDet::SCT_ClusterOnTrack* correct(
      const Trk::PrepRawData&, const Trk::TrackParameters&) const override;

  /** @brief Returns a correction to be applied to the SCT cluster local x
     position in simulated events to remove a position bias introduced by the
     SCT digitisation.

        @param [in] phi     angle of track relative to Lorentz drift direction,
     in transverse plane
        @param [in] nstrip  SCT cluster size (number of strips)
   */
  static double getCorrection(double phi, int nstrip) ;
  

/** @brief Returns the resolution on the reconstructed position (local x) of SCT clusters
    in simulated events.

      @param [in] phi     angle of track relative to Lorentz drift direction, in transverse plane
      @param [in] nstrip  SCT cluster size (number of strips)

    The parameterisation of the resolution contained in getError() was derived from SCT
    barrel clusters (80 micron pitch).  It can be applied also to endcap clusters, after
    rescaling to the appropriate pitch.
 */
  static double getError(double phi, int nstrip) ;

 private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////

   //! toolhandle for central error scaling
  //   SG::ReadCondHandleKey<SCTRIO_OnTrackErrorScaling> m_sctErrorScalingKey
  //     {this,"SCTErrorScalingKey", "/Indet/TrkErrorScalingSCT", "Key for SCT error scaling conditions data."};
   SG::ReadCondHandleKey<RIO_OnTrackErrorScaling> m_sctErrorScalingKey
     {this,"SCTErrorScalingKey", "/Indet/TrkErrorScalingSCT", "Key for SCT error scaling conditions data."};

   ToolHandle<ISCT_ModuleDistortionsTool> m_distortionsTool;
   ToolHandle<ISiLorentzAngleTool> m_lorentzAngleTool{this, "LorentzAngleTool", "SiLorentzAngleTool", "Tool to retreive Lorentz angle"};
   //! flag storing if errors need scaling or should be kept nominal

   //! job options
   bool                               m_option_make2dimBarrelClusters;
   bool                               m_doDistortions ;//!< Flag to set Distortions
   int                                m_option_errorStrategy;
   int                                m_option_correctionStrategy;
};

} // end of namespace InDet

#endif // SCT_ClusterOnTrackTool_H
