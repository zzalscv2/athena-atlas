/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ActsTrkEvent_MultiTrajectory_h
#define ActsTrkEvent_MultiTrajectory_h
#include <type_traits>
#include "CxxUtils/concepts.h"
#include "Acts/EventData/MultiTrajectory.hpp"
#include "Acts/EventData/TrackStatePropMask.hpp"
#include "Acts/Utilities/HashedString.hpp"

#include "xAODTracking/TrackStateContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackMeasurementsContainer.h"

namespace ActsTrk {
    constexpr static bool IsReadOnly = true;
    constexpr static bool IsReadWrite = false;

    /**
     * @brief Athena implementation of ACTS::MultiTrajectory
     * 
     * @tparam MOD - generates variant of the class that allows ro (if == CANMODIFY) or rw (when not CANMODIFY)
     */
    template<bool RWState>
    class MultiTrajectory final 
    : public Acts::MultiTrajectory<ActsTrk::MultiTrajectory<RWState>>{ // TODO select other variant of the interface for UNMODIFIABLE (needs next ACTS version)
        public:

            using TrackStateContainerBackendPtr = typename std::conditional<RWState, const xAOD::TrackStateContainer*, xAOD::TrackStateContainer*>::type;
            using TrackParametersContainerBackendPtr = typename std::conditional<RWState, const xAOD::TrackParametersContainer*, xAOD::TrackParametersContainer*>::type;
            using TrackJacobianContainerBackendPtr = typename std::conditional<RWState, const xAOD::TrackJacobianContainer*, xAOD::TrackJacobianContainer*>::type;
            using TrackMeasurementsContainerBackendPtr = typename std::conditional<RWState, const xAOD::TrackMeasurementsContainer*, xAOD::TrackMeasurementsContainer*>::type;
            
            using IndexType = std::uint32_t;

            using TrackStateProxy = typename Acts::MultiTrajectory<ActsTrk::MultiTrajectory<RWState>>::TrackStateProxy;
            using ConstTrackStateProxy = typename Acts::MultiTrajectory<ActsTrk::MultiTrajectory<RWState>>::ConstTrackStateProxy;

            /**
             * @brief Construct a new Multi Trajectory object given backends
             * @note the MTJ does claim ownership over the data in the backend
             * @param state - track state (indices) backend             
             */
            MultiTrajectory( TrackStateContainerBackendPtr states, TrackParametersContainerBackendPtr parameters,
                             TrackJacobianContainerBackendPtr jacobians, TrackMeasurementsContainerBackendPtr measurements );

            /**
             * @brief Construct a new Multi Trajectory object by moving from a modifiable version
             * @warning - this constructor truly moves, i.e. RHS object becomes unusable
             * @param rhs - the MTJ to move from
             */
            MultiTrajectory( ActsTrk::MultiTrajectory<IsReadWrite>&& rhs);



            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, IndexType) addTrackState_impl(
                Acts::TrackStatePropMask mask,
                IndexType iprevious);

            /**
             * @brief Access component by key
             * 
             * @param key 
             * @param istate 
             * @return std::any - that needs to be cast to a const ptr (non const for the nonconst variant)
             */
            std::any component_impl(Acts::HashedString key, IndexType istate) const;
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, std::any) component_impl(Acts::HashedString key, IndexType istate); // TODO disable for UNMODIFIABLE variant

            /**
             * @brief checks if given state has requested component
             * 
             * @param key - name (const char*)
             * @param istate - index in the 
             * @return true 
             * @return false 
             */
            constexpr bool has_impl(Acts::HashedString key, IndexType istate) const;

            /**
             * @brief obtains proxy to the track state under given index
             * 
             * @param index 
             * @return TrackStateProxy::Parameters 
             */
            typename ConstTrackStateProxy::Parameters parameters_impl(IndexType index) const;
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, typename TrackStateProxy::Parameters) parameters_impl(IndexType index);

            /**
             * @brief obtain covariances for a state at given index
             * 
             * @param index 
             * @return TrackStateProxy::Covariance 
             */
            typename ConstTrackStateProxy::Covariance covariance_impl(IndexType index) const;
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, typename TrackStateProxy::Covariance) covariance_impl(IndexType index);

            /**
             * @brief size of the MTJ
             * 
             * @return size_t 
             */
            inline size_t size_impl() const;

            /**
             * @brief clears backends
             * 
             */
            inline void clear_impl();            

            /**
             * @brief checks if the backends are connected (i.e. is safe to use, else any other call will cause segfaults)
             */
            bool has_backends() const;
        private:
            // bare pointers to the backend (need to be fast and we do not claim ownership anyways)
            TrackStateContainerBackendPtr m_trackStates = nullptr;
            TrackParametersContainerBackendPtr m_trackParameters = nullptr;
            TrackJacobianContainerBackendPtr m_jacobians = nullptr;
            TrackMeasurementsContainerBackendPtr m_measurements = nullptr;
            friend class ActsTrk::MultiTrajectory<IsReadWrite>;
            friend class ActsTrk::MultiTrajectory<IsReadOnly>;

    };
    template<bool RWState>
    bool MultiTrajectory<RWState>::has_backends() const { 
        return m_trackStates != nullptr 
            and m_trackParameters != nullptr;
    }

    typedef ActsTrk::MultiTrajectory<ActsTrk::IsReadOnly> ConstMultiTrajectory;
    typedef ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite> MutableMultiTrajectory;

} // EOF namespace ActsTrk


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::ConstMultiTrajectory , 237752966 , 1 )


#endif