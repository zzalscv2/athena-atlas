// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

/**
 * @file FPGATrackSimRegionSlices.h
 * @author Riley Xu - riley.xu@cern.ch
 * @date Janurary 7th, 2021
 * @brief Stores slice definitions for FPGATrackSim regions
 *
 * See header.
 */

#include "FPGATrackSimConfTools/FPGATrackSimRegionSlices.h"

#include <AsgMessaging/MessageCheck.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace asg::msgUserCode;

///////////////////////////////////////////////////////////////////////////////
// Constructor/Desctructor
///////////////////////////////////////////////////////////////////////////////


FPGATrackSimRegionSlices::FPGATrackSimRegionSlices(std::string const & filepath) 
{
  // Open the file
  std::ifstream fin(filepath);
  if (!fin.is_open())
    {
      ANA_MSG_ERROR("Couldn't open " << filepath);
      throw ("FPGATrackSimRegionSlices couldn't open " + filepath);
    }

  // Variables to fill
  unsigned region;
  std::string line, key;
  FPGATrackSimTrackPars min, max;

  // Parse the file
  bool ok = true;
  while (getline(fin, line))
    {
      if (line.empty() || line[0] == '#') continue;
      std::istringstream sline(line);

      ok = ok && (sline >> key);
      if (!ok) break;
      if (key == "region")
	{
	  ok = ok && (sline >> region);
	  if (ok && region > 0) m_regions.push_back({ min, max });
	  ok = ok && (region == m_regions.size());
	  min = FPGATrackSimTrackPars(); // reset
	  max = FPGATrackSimTrackPars(); // reset
	}
      else if (key == "phi") ok = ok && (sline >> min.phi >> max.phi);
      else if (key == "eta") ok = ok && (sline >> min.eta >> max.eta);
      else if (key == "qpt") ok = ok && (sline >> min.qOverPt >> max.qOverPt);
      else if (key == "d0") ok = ok && (sline >> min.d0 >> max.d0);
      else if (key == "z0") ok = ok && (sline >> min.z0 >> max.z0);
      else ok = false;

      if (!ok) break;
    }

  if (!ok)
    {
      ANA_MSG_ERROR("Found error reading file at line: " << line);
      throw "FPGATrackSimRegionSlices read error";
    }

  m_regions.push_back({ min, max }); // last region still needs to be added
}



///////////////////////////////////////////////////////////////////////////////
// Interface Functions
///////////////////////////////////////////////////////////////////////////////


bool FPGATrackSimRegionSlices::inRegion(unsigned region, FPGATrackSimTruthTrack const & t) const
{
  if (region >= m_regions.size())
    {
      ANA_MSG_WARNING("inRegion() region " << region << " out-of-bounds " << m_regions.size());
      return false;
    }
  FPGATrackSimTrackPars min = m_regions[region].first;
  FPGATrackSimTrackPars max = m_regions[region].second;
  FPGATrackSimTrackPars cur = t.getPars();

  for (unsigned i = 0; i < FPGATrackSimTrackPars::NPARS; i++)
    {
      if (cur[i] < min[i]) return false;
      if (cur[i] > max[i]) return false;
    }

  return true;
}
