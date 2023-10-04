/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_FUTURETRACKCONTAINER_H
#define ACTSTRKEVENT_FUTURETRACKCONTAINER_H 1

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackBackendContainer.h"

namespace ActsTrk {

// temporary definition
namespace future {
// Holder referring to data stored in StoreGate
template <typename T>
struct ConstDataLinkHolder {
  DataLink<T> m_link;
  ConstDataLinkHolder(const DataLink<T>& link) : m_link{link} {}

  const T& operator*() const { return *(m_link.cptr()); }
  const T* operator->() const { return m_link.cptr(); }
};

using ConstTrackContainer =
    Acts::TrackContainer<ActsTrk::ConstTrackBackendContainer,
                         ActsTrk::ConstMultiTrajectory,
                         future::ConstDataLinkHolder>;
using TrackContainer =
    Acts::TrackContainer<ActsTrk::MutableTrackBackendContainer,
                         ActsTrk::MutableMultiTrajectory,
                         Acts::detail::RefHolder>;

}
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::future::ConstTrackContainer , 1231467572 , 1 )

#endif