/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRKEVENT_FUTURETRACKCONTAINER_H
#define ACTSTRKEVENT_FUTURETRACKCONTAINER_H 1

#include "ActsEvent/MultiTrajectory.h"
#include "ActsEvent/TrackStorageContainer.h"

namespace ActsTrk {

// temporary definition
namespace future {
// Holder referring to data stored in StoreGate
template <typename T>
struct DataLinkHolder {
  DataLink<T> m_link;
  DataLinkHolder(const DataLink<T>& link) : m_link{link} {}

  const T& operator*() const { return *(m_link.cptr()); }
  const T* operator->() const { return m_link.cptr(); }
};

using TrackContainer =
    Acts::TrackContainer<ActsTrk::TrackStorageContainer,
                         ActsTrk::MultiTrajectory,
                         future::DataLinkHolder>;
using MutableTrackContainer =
    Acts::TrackContainer<ActsTrk::MutableTrackStorageContainer,
                         ActsTrk::MutableMultiTrajectory,
                         Acts::detail::RefHolder>;

}
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( ActsTrk::future::TrackContainer , 1100705412 , 1 )

#endif