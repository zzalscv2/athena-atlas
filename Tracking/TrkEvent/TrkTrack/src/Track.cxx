/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
 */

#include "TrkTrack/Track.h"

#include <cassert>
#include <iostream>
#include <string>

#include "GaudiKernel/MsgStream.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkTrack/AlignmentEffectsOnTrack.h"

Trk::Track::Track(const TrackInfo& info, TrackStates&& trackStateOnSurfaces,
                  std::unique_ptr<FitQuality> fitQuality)
    : Trk::ObjectCounter<Trk::Track>(),
      m_trackStateVector(std::move(trackStateOnSurfaces)),
      m_cachedParameterVector{},
      m_cachedMeasurementVector{},
      m_cachedOutlierVector{},
      m_perigeeParameters{},
      m_fitQuality(std::move(fitQuality)),
      m_trackInfo(info) {
  // find the Perigee params they will become valid given the outcome
  findPerigeeImpl();
}

Trk::Track::Track(const Trk::Track& rhs)
    : Trk::ObjectCounter<Trk::Track>(rhs),
      m_cachedParameterVector{},
      m_cachedMeasurementVector{},
      m_cachedOutlierVector{},
      m_perigeeParameters{},
      m_fitQuality(nullptr) {
  // Do the actual payload copy
  copyHelper(rhs);
}

Trk::Track& Trk::Track::operator=(const Trk::Track& rhs) {
  if (this != &rhs) {
    // First clear this object
    m_fitQuality.reset(nullptr);
    m_trackSummary.reset(nullptr);
    // Invalidate the caches
    resetCaches();
    // The following is a DataVector and so will delete
    // the contained objects automatically.
    m_trackStateVector.clear();
    // copy payload of rhs to this
    copyHelper(rhs);
  }
  return *this;
}

void Trk::Track::copyHelper(const Trk::Track& rhs) {
  // set the author to be that of the Track being copied.
  m_trackInfo = rhs.m_trackInfo;

  // create & copy other variables if available
  if (rhs.fitQuality() != nullptr) {
    m_fitQuality = std::make_unique<Trk::FitQuality>(*(rhs.m_fitQuality));
  }
  // create & copy other variables
  if (rhs.trackSummary() != nullptr) {
    m_trackSummary = std::make_unique<Trk::TrackSummary>(*(rhs.m_trackSummary));
  }
  // Create the TrackStateVector and the perigeeParameters
  m_trackStateVector.reserve(rhs.m_trackStateVector.size());

  TSoS_iterator itTSoSEnd = rhs.m_trackStateVector.end();
  for (TSoS_iterator itTSoS = rhs.m_trackStateVector.begin();
       itTSoS != itTSoSEnd; ++itTSoS) {
    assert(*itTSoS != nullptr);  // check that is defined.
    // clone and store
    TrackStateOnSurface* tsos = (**itTSoS).clone();
    m_trackStateVector.push_back(tsos);
    // Check if this a perigee so we can already cache it
    if (tsos != nullptr && tsos->type(TrackStateOnSurface::Perigee)) {
      const Trk::Perigee* perigee = nullptr;
      const Trk::TrackParameters* tp = tsos->trackParameters();
      if (tp && tp->type() == Trk::AtaSurface &&
          tp->surfaceType() == Trk::SurfaceType::Perigee) {
        perigee = static_cast<const Trk::Perigee*>(tp);
      }
      if (perigee != nullptr) {
        m_perigeeParameters.store(perigee);  // Now they will be valid
      }
    }
  }
}

const DataVector<const Trk::TrackParameters>* Trk::Track::trackParameters()
    const {

  if (m_trackStateVector.empty()) {
    return nullptr;
  }
  // Do work only if it is not valid.
  if (!m_cachedParameterVector.isValid()) {
    // create cached parameter vector (which DOES NOT OWN ELEMENTS)
    DataVector<const Trk::TrackParameters> tmp_ParameterVector(
        SG::VIEW_ELEMENTS);
    tmp_ParameterVector.reserve(m_trackStateVector.size());
    TSoS_iterator itTSoSEnd = m_trackStateVector.end();
    for (TSoS_iterator itTSoS = m_trackStateVector.begin(); itTSoS != itTSoSEnd;
         ++itTSoS) {
      const TrackParameters* trackParameters = (*itTSoS)->trackParameters();
      // check to make sure that the TrackParameters exists first
      if (trackParameters != nullptr) {
        tmp_ParameterVector.push_back(trackParameters);
      }
    }
    m_cachedParameterVector.set(std::move(tmp_ParameterVector));
  }
  return m_cachedParameterVector.ptr();
}

void Trk::Track::findPerigee() const {
  if (!m_perigeeParameters.isValid()) {
    findPerigeeImpl();
  }
}

void Trk::Track::findPerigeeImpl() const {
  // loop through all passed parameters and, if there is a at Perigee in there,
  // assign it to Perigee parameters. There should never be more
  // than one perigee type.
  // Note that there can be other objects, like VertexOnTrack measurements, with
  // params at a Perigee surface, thus the  TSoS check.

  const Trk::Perigee* tmpPerigeeParameters = nullptr;
  DataVector<const TrackStateOnSurface>::const_iterator it =
      m_trackStateVector.begin();
  DataVector<const TrackStateOnSurface>::const_iterator itEnd =
      m_trackStateVector.end();
  for (; it != itEnd; ++it) {
    if ((*it)->type(TrackStateOnSurface::Perigee)) {
      const Trk::TrackParameters* tp = (*it)->trackParameters();
      if (tp && tp->type() == Trk::AtaSurface &&
          tp->surfaceType() == Trk::SurfaceType::Perigee) {
        tmpPerigeeParameters = static_cast<const Trk::Perigee*>(tp);
      }

      if (tmpPerigeeParameters != nullptr) {
        break;  // found perigee so stop loop.
      }
    }
  }
  // set to value and valid
  if (tmpPerigeeParameters) {
    m_perigeeParameters.set(tmpPerigeeParameters);
  }
}

const Trk::Perigee* Trk::Track::perigeeParameters() const {
  if (!m_perigeeParameters.isValid()) {
    // findPerigee performs the setting of the parameters
    // i.e does the CachedValue set
    findPerigeeImpl();
  }

  // Return payload if valid
  if (m_perigeeParameters.isValid()) {
    return *(m_perigeeParameters.ptr());
  }

  return nullptr;
}

const DataVector<const Trk::MeasurementBase>* Trk::Track::measurementsOnTrack()
    const {
  if (m_trackStateVector.empty()) {
    return nullptr;
  }

  // We only need to do work if not valid.
  if (!m_cachedMeasurementVector.isValid()) {
    // create new DataVector which DOES NOT OWN ELEMENTS .
    DataVector<const Trk::MeasurementBase> tmpMeasurementVector(
        SG::VIEW_ELEMENTS);
    // for measurements on track it is very likely that #(meas) ~ #(TSOS)->
    // reserve(#(TSOS))
    tmpMeasurementVector.reserve(m_trackStateVector.size());

    TSoS_iterator itTSoSEnd = m_trackStateVector.end();
    for (TSoS_iterator itTSoS = m_trackStateVector.begin(); itTSoS != itTSoSEnd;
         ++itTSoS) {
      if (!(*itTSoS)->type(TrackStateOnSurface::Outlier)) {
        const Trk::MeasurementBase* rot = (*itTSoS)->measurementOnTrack();
        // does it have a measurement ?
        if (rot != nullptr) {
          tmpMeasurementVector.push_back(rot);
        }
      }
    }
    m_cachedMeasurementVector.set(std::move(tmpMeasurementVector));
  }

  return m_cachedMeasurementVector.ptr();
}

const DataVector<const Trk::MeasurementBase>* Trk::Track::outliersOnTrack()
    const {
  if (m_trackStateVector.empty()) {
    return nullptr;
  }
  // We only need to do work if not valid
  if (!m_cachedOutlierVector.isValid()) {
    // create new DataVector which DOES NOT OWN ELEMENTS .
    DataVector<const Trk::MeasurementBase> tmpOutlierVector(SG::VIEW_ELEMENTS);
    TSoS_iterator itTSoSEnd = m_trackStateVector.end();
    for (TSoS_iterator itTSoS = m_trackStateVector.begin(); itTSoS != itTSoSEnd;
         ++itTSoS) {
      if ((*itTSoS)->type(TrackStateOnSurface::Outlier)) {
        const Trk::MeasurementBase* rot = (*itTSoS)->measurementOnTrack();
        assert(rot != nullptr);
        tmpOutlierVector.push_back(rot);
      }
    }
    m_cachedOutlierVector.set(std::move(tmpOutlierVector));
  }
  return m_cachedOutlierVector.ptr();
}

/**Overload of << operator for both, MsgStream and std::ostream for debug
 * output*/
MsgStream& Trk::operator<<(MsgStream& sl, const Trk::Track& track) {
  std::string name("Track ");
  sl << name << "Author = " << track.info().dumpInfo() << endmsg;
  if (track.fitQuality() != nullptr) {
    sl << *(track.fitQuality()) << endmsg;
  }
  if (track.trackSummary() != nullptr) {
    sl << *(track.trackSummary()) << endmsg;
  } else {
    sl << "No TrackSummary available in this track." << endmsg;
  }
  if (track.trackStateOnSurfaces() != nullptr) {
    sl << name << "has " << (track.trackStateOnSurfaces()->size())
       << " trackStateOnSurface(s)" << endmsg;

    // level() shows the output level, currentLevel()
    // shows what the stream is set to
    if (sl.level() < MSG::INFO) {
      // loop over TrackStateOnSurfaces if verbose turned on
      DataVector<const TrackStateOnSurface>::const_iterator it =
          track.trackStateOnSurfaces()->begin();
      int num = 0;
      for (; it != track.trackStateOnSurfaces()->end(); ++it) {
        sl << " --------- Start of TrackStateOnSurface \t" << num << "\t-------"
           << endmsg;
        sl << (**it);
        sl << " ---------   End of TrackStateOnSurface \t" << num++
           << "\t-------" << endmsg;
      }
    }
  }
  return sl;
}

std::ostream& Trk::operator<<(std::ostream& sl, const Trk::Track& track) {
  std::string name("Track ");
  sl << name << "Author = " << track.info().dumpInfo() << std::endl;
  if (track.fitQuality() != nullptr) {
    sl << *(track.fitQuality()) << std::endl;
  }
  if (track.trackSummary() != nullptr) {
    sl << *(track.trackSummary()) << std::endl;
  } else {
    sl << "No TrackSummary available in this track." << std::endl;
  }

  if (track.trackStateOnSurfaces() != nullptr) {
    sl << name << "has " << (track.trackStateOnSurfaces()->size())
       << " trackStateOnSurface(s)" << std::endl;
    DataVector<const TrackStateOnSurface>::const_iterator it =
        track.trackStateOnSurfaces()->begin();
    int num = 0;
    for (; it != track.trackStateOnSurfaces()->end(); ++it) {
      sl << " --------- Start of TrackStateOnSurface \t" << num << "\t-------"
         << std::endl;
      sl << (**it);
      sl << " ---------   End of TrackStateOnSurface \t" << num++ << "\t-------"
         << std::endl;
    }
  }
  return sl;
}
