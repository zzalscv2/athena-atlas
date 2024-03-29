/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ITrackingVolumesSvc.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRINTERFACES_ITRACKINGVOLUMESSERVICE_H
#define TRKDETDESCRINTERFACES_ITRACKINGVOLUMESSERVICE_H

// Include Files
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/StatusCode.h"
#include "AthenaKernel/IOVSvcDefs.h"


namespace Trk {
    
    class Volume;

    /** @class ITrackingVolumesSvc 
    
        Service which is used to create volumes enclosing various parts of ATLAS, and which can be used e.g.
        to surfaces at the entrance to the Muon Spectrometer, on which to compare Trk::TrackParameters.
    
        The interface implemented by the TrackingVolumesSvc.
    
        @author Edward.Moyse@cern.ch
    */
    class ITrackingVolumesSvc : virtual public IInterface {

       public:
      /// Creates the InterfaceID and interfaceID() method
      DeclareInterfaceID(ITrackingVolumesSvc, 1, 0);

        /** Enum to identify the various "volumes" which exist. The names of the enum variables are 
           intended to be as close as possible to the terminology already in use*/
        enum TrackingVolumeIdentifier {
            CalorimeterEntryLayer=0, //!< Tracking Volume which defines the entrance srufaces of the calorimeter
            MuonSpectrometerEntryLayer=1, //!< Tracking Volume which defines the entrance surfaces of the MS
            MuonSpectrometerExitLayer=2, //!< Tracking Volume which defines the outer surfaces of the MS
            NumIdentifiers=3
        };

        /** Provide the TrackingVolumes */
        virtual const Trk::Volume& volume(const TrackingVolumeIdentifier& volumeId) const = 0;
    
        /** Returns the name of the TrackingVolumes built with this Svc */
        virtual const std::string& volumeName(const TrackingVolumeIdentifier& volumeId) const = 0;
    
    
    };

}

#endif 

