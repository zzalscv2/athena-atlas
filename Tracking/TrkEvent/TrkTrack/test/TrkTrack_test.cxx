/*
 *  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "TrkTrack/Track.h"

#include "TrkTrack/MultiComponentStateOnSurface.h"
#include "TrkTrack/TrackStateOnSurface.h"

#include "TestTools/leakcheck.h"

#include <iostream>

int
main()
{
  std::cout << "Trk::Track test" << std::endl;

  Athena_test::Leakcheck check;

  Trk::FitQuality fq(10, 20);
  Trk::TrackInfo info{};
  // TSOS
  Trk::TrackStateOnSurface TSOS(fq, nullptr, nullptr, nullptr);
  DataVector<const Trk::TrackStateOnSurface> tsvec1(SG::VIEW_ELEMENTS);
  tsvec1.push_back(&TSOS);
  // MTSOS
  Trk::MultiComponentStateOnSurface MTSOS(fq, nullptr, nullptr, {}, nullptr, 0);
  DataVector<const Trk::MultiComponentStateOnSurface> tsvec2(SG::VIEW_ELEMENTS);
  tsvec2.push_back(&MTSOS);

  // Track from TSOS
  Trk::Track track1(info,
                    DataVector<const Trk::TrackStateOnSurface>(tsvec1),
                    new Trk::FitQuality(fq));

  std::cout << "Track from TSOS" << '\n';
  std::cout << track1.isValid() << '\n';
  std::cout << track1.trackStateOnSurfaces()->at(0)->variety() << '\n';
  std::cout << (dynamic_cast<const Trk::TrackStateOnSurface*>(
                  track1.trackStateOnSurfaces()->at(0)) != nullptr)
            << '\n';

  // Track from MTSOS
  Trk::Track track2(info,
                    DataVector<const Trk::MultiComponentStateOnSurface>(tsvec2),
                    new Trk::FitQuality(fq));

  std::cout << "Track from MTSOS" << '\n';
  std::cout << track2.isValid() << '\n';
  std::cout << track2.trackStateOnSurfaces()->at(0)->variety() << '\n';
  std::cout << (dynamic_cast<const Trk::MultiComponentStateOnSurface*>(
                  track2.trackStateOnSurfaces()->at(0)) != nullptr)
            << '\n';

}

