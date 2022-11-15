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
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackMeasurement.h"

#include "ActsTrkEvent/SourceLink.h"

namespace ActsTrk {

    namespace detail { struct Decoration; }

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
            using TrackMeasurementContainerBackendPtr = typename std::conditional<RWState, const xAOD::TrackMeasurementContainer*, xAOD::TrackMeasurementContainer*>::type;
            
            using IndexType = std::uint32_t;

            using TrackStateProxy = typename Acts::MultiTrajectory<ActsTrk::MultiTrajectory<RWState>>::TrackStateProxy;
            using ConstTrackStateProxy = typename Acts::MultiTrajectory<ActsTrk::MultiTrajectory<RWState>>::ConstTrackStateProxy;

            /**
             * @brief Construct a new Multi Trajectory object given backends
             * @note the MTJ does claim ownership over the data in the backend
             * @param state - track state (indices) backend             
             */
            MultiTrajectory( TrackStateContainerBackendPtr states, TrackParametersContainerBackendPtr parameters,
                             TrackJacobianContainerBackendPtr jacobians, TrackMeasurementContainerBackendPtr measurements );


            /**
             * @brief Add state with stograge for data that depends on the mask
             * 
             * @param mask - bitmask deciding which backends are extended 
             * @param istate - previous state
             * @return index of just added state
             */
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, IndexType) addTrackState_impl(
                Acts::TrackStatePropMask mask, IndexType iprevious);

            /**
             * @brief Access component by key
             * 
             * @param key 
             * @param istate 
             * @return std::any - that needs to be cast to a const ptr (non const for the nonconst variant)
             */

            const std::any component_impl(Acts::HashedString key, IndexType istate) const;
            std::any component_impl(Acts::HashedString key, IndexType istate); 
 
           
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

            typename ConstTrackStateProxy::Parameters parameters_impl(IndexType index) const {
                return trackParameters().at(index)-> paramsEigen();
            };
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, typename TrackStateProxy::Parameters) parameters_impl(IndexType index){
                return trackParameters().at(index)-> paramsEigen();
            }

            /**
             * @brief obtain covariances for a state at given index
             * 
             * @param index 
             * @return TrackStateProxy::Covariance 
             */            
            typename ConstTrackStateProxy::Covariance covariance_impl(IndexType index) const {
                return trackParameters().at(index)-> covMatrixEigen();
            };        
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, typename TrackStateProxy::Covariance) covariance_impl(IndexType index) {
                return trackParameters().at(index)-> covMatrixEigen();
            }

            /**
             * @brief obtain jacobian for a state at given index
             * 
             * @param index 
             * @return TrackStateProxy::Covariance 
             */
            
            inline typename ConstTrackStateProxy::Covariance jacobian_impl(IndexType index) const {
                return trackJacobians().at(index)-> jacEigen();
            };
            ATH_MEMBER_REQUIRES(RWState==IsReadWrite, typename TrackStateProxy::Covariance) jacobian_impl(IndexType index) {
                return trackJacobians().at(index)-> jacEigen();
            }
            
            /**
             * @brief size of the MTJ
             * 
             * @return size_t 
             */

            inline size_t size_impl() const {
            	return m_trackStates->size();
            };

            /**
             * @brief clears backends
             * 
             */
            inline void clear_impl(){   
                m_trackStates->clear();
                m_trackParameters->clear();
                m_trackJacobians->clear();
                m_trackMeasurements->clear();
            };              

            /**
             * @brief checks if the backends are connected (i.e. is safe to use, else any other call will cause segfaults)
             */
            bool has_backends() const;

            /**
             * @brief enables particular decoration, type & name need to be specified
             * 
             * @tparam T type of decoration (usually POD) 
             * @param name name of the decoration
             */
            template<typename T>
            void enableDecoration(const std::string& name);

        private:
            // bare pointers to the backend (need to be fast and we do not claim ownership anyways)
            TrackStateContainerBackendPtr m_trackStates = nullptr;

            // inline const xAOD::TrackStateContainer& trackStates() const { return *m_trackStates; }
            // inline xAOD::TrackStateContainer& trackStates() { return *m_trackStates; }
            
            TrackParametersContainerBackendPtr m_trackParameters = nullptr;

            inline const xAOD::TrackParametersContainer& trackParameters() const { return *m_trackParameters; }
            inline xAOD::TrackParametersContainer& trackParameters() { return *m_trackParameters; }
            
            TrackJacobianContainerBackendPtr m_trackJacobians = nullptr;

            inline const xAOD::TrackJacobianContainer& trackJacobians() const { return *m_trackJacobians; }
            inline xAOD::TrackJacobianContainer& trackJacobians() { return *m_trackJacobians; }

            TrackMeasurementContainerBackendPtr m_trackMeasurements = nullptr;

            inline const xAOD::TrackMeasurementContainer& trackMeasurements() const { return *m_trackMeasurements; }
            inline xAOD::TrackMeasurementContainer& trackMeasurements() { return *m_trackMeasurements; }


            friend class ActsTrk::MultiTrajectory<IsReadWrite>;
            friend class ActsTrk::MultiTrajectory<IsReadOnly>;

            std::vector<detail::Decoration> m_decorations;
            //!< decoration accessors, one per type
            template<typename T>
            std::any decorationSetter(IndexType, const std::string&);
            template<typename T>
            const std::any decorationGetter(IndexType, const std::string&) const;

            ///!< cache of source links for uncalibrated measurements
            using SourceLinkType = Acts::SourceLink;
            std::vector<SourceLinkType*> m_sourceLinks;

    };

    typedef ActsTrk::MultiTrajectory<ActsTrk::IsReadOnly>  ConstMultiTrajectory;
    typedef ActsTrk::MultiTrajectory<ActsTrk::IsReadWrite> MutableMultiTrajectory;


    namespace detail {
        struct Decoration {
            using SetterType = std::function<std::any(MultiTrajectory<true>::IndexType, const std::string&)>;
            using GetterType = std::function<const std::any(MultiTrajectory<true>::IndexType, const std::string&)>;

            Decoration(const std::string& n, SetterType s, GetterType g)
                : name(n), hash(Acts::hashString(name)), setter(s), getter(g) {}
            std::string name; // xAOD API needs this
            uint32_t hash; // Acts API comes with this
            SetterType setter;// type aware accessors
            GetterType getter;
    };
    }



} // EOF namespace ActsTrk

               
                
#include "MultiTrajectory.icc"


#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::ConstMultiTrajectory , 237752966 , 1 )


#endif
