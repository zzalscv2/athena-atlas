/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRACKING_VERSIONS_TRACKSTATE_V1_H
#define XAODTRACKING_VERSIONS_TRACKSTATE_V1_H
#include <cstdint>
#include "AthContainers/AuxElement.h"
#include "AthLinks/ElementLink.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODTracking/SurfaceBackendContainer.h"

namespace xAOD {
    using TrackStateIndexType=uint32_t; // TODO move this declaration to one location
    /**
     * @brief class describing TrackState - backend of navigation object of Acts::MultiTrajectory
     */
    class TrackState_v1 : public SG::AuxElement {
      public:
        /**
         * @brief get chi2 of this track state
         */
        double chi2() const;

        /**
         * @brief Set the Chi2 value of this track state
         */
        void setChi2(double);

        /**
         * @brief pointers API needed by MTJ
         */
        const double* chi2Ptr() const;
        double* chi2Ptr();


        /**
         * @brief get path length of this track state
         */
        double pathLength() const;
        /**
         * @brief set path length of this track state
         */
        void setPathLength(double);

        /**
         * @brief pointers API needed by MTJ
         */
        const double* pathLengthPtr() const;
        double* pathLengthPtr();


        /**
         * @brief set the index of preceding TrackState 
         * TrackStates in the collection from multitude of short one-directional linked lists pointing from state to an earlier state.
         * This index allows to traverse backwards a list to which this TrackState belongs.
         * Maximum possible value indicates that this is the inital TrackState (terminal node of linked-list)
         * @return index, invalid if identical to max possible value of this type
         */
        TrackStateIndexType previous() const;

        /**
         * @brief Set the Previous index
         * @see previous() for explanation
         */
        void setPrevious(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* previousPtr() const;
        TrackStateIndexType* previousPtr();
      

        /**
         * @brief index in the TrackParametersContainer corresponding to this TrackStates container
         * It points to the parameter predicted parameters value (by KF) 
         * @return index, invalid if identical to max possible value of this type
         */
        TrackStateIndexType predicted() const; 

        /**
         * @brief Set the Predicted index
         * @see predicted() method for explanation
         */
        void setPredicted(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* predictedPtr() const;
        TrackStateIndexType* predictedPtr();

        /**
         * @brief index in the TrackParametersContainer corresponding to this TrackStates container
         * Points to filtered parameters
         * @return index, invalid if identical to max possible value of this type
         */
        TrackStateIndexType filtered() const; 

        /**
         * @brief Set the Filtered index
         * @see filtered() method for explanation
         */
        void setFiltered(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* filteredPtr() const;
        TrackStateIndexType* filteredPtr();

        /**
         * @brief index in the TrackParametersContainer corresponding to this TrackStates container
         * Points to the smoothed parameters
         * @return index, invalid if identical to max possible value of this type
         */
        TrackStateIndexType smoothed() const; 

        /**
         * @brief Set the Smoothed index
         * @see smoothed() method for explanation
         */
        void setSmoothed(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* smoothedPtr() const;
        TrackStateIndexType* smoothedPtr();

        /**
         * @brief index in the TrackJacobianContainer corresponding to this TrackStates container
         * 
         * @return index, invalid if identical to max possible value of this type
         */
        TrackStateIndexType jacobian() const; 

        /**
         * @brief Set the Jacobian index
         * @see jacobian() method for explanation
         */
        void setJacobian(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* jacobianPtr() const;
        TrackStateIndexType* jacobianPtr();

        /**
         * @brief index in TrackMeasurementContainer corresponding to this TracksTate
         * 
         * @return index, invalid if identical to max possible value of this type
         */
        
        /**
         * @brief index in TrackMeasurementContainer corresponding to this TracksTate
         * Points to calibrated measurement
         * @return index, invalid if identical to max possible value of this type
         */
        TrackStateIndexType calibrated() const; 
        /**
         * @brief Set the Calibrated index
         * @see calibrated() method
         */
        void setCalibrated(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* calibratedPtr() const;
        TrackStateIndexType* calibratedPtr();

        /**
         * @brief measurement dimensions
         * 
         * @return TrackStateIndexType 
         */
        TrackStateIndexType measDim() const; 
        /**
         * @brief Set the measurement dimensions
         */
        void setMeasDim(TrackStateIndexType);

        /**
         * @brief pointers API needed by MTJ
         */
        const TrackStateIndexType* measDimPtr() const;
        TrackStateIndexType* measDimPtr();

        /**
         * @brief EL to uncalibrated measurement
         * Can be invalid sometimes!
         */
        ElementLink<xAOD::UncalibratedMeasurementContainer> uncalibratedMeasurementLink() const;

        /**
         * @brief Set EL to uncalibrated measurement
         */
        void setUncalibratedMeasurementLink( ElementLink<xAOD::UncalibratedMeasurementContainer> );

        /**
         * @brief geometry ID associated with uncalibrated measurement
         */
         uint64_t geometryId() const;

        /**
         * @brief set geometry ID associated with uncalibrated measurement
         */
        void setGeometryId(uint64_t);

        /**
         * @brief obtains link to the surface representation
         */
        ElementLink<xAOD::SurfaceBackendContainer> surfaceLink() const;

        /**
         * @brief sets link to the surface representation
         */
        void setSurfaceLink(ElementLink<xAOD::SurfaceBackendContainer>);


    };
}


#endif