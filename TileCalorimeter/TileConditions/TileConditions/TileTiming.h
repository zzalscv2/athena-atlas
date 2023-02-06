/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILETIMING_H
#define TILECONDITIONS_TILETIMING_H

// Tile includes
#include "TileConditions/TileCalibData.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

/**
 * @class TileTiming
 * @brief Condition object to keep and provide Tile timing
 */
class TileTiming {
  public:

    TileTiming(std::unique_ptr<TileCalibDataFlt> timing) : m_timing{std::move(timing)} {};
    virtual ~TileTiming() = default;

    float getSignalPhase(unsigned int drawerIdx, unsigned int channel, unsigned int adc) const;

    TileTiming(const TileTiming&) = delete;
    TileTiming& operator= (const TileTiming&) = delete;

  private:

    //=== TileCalibData
    std::unique_ptr<TileCalibDataFlt> m_timing;
};

// inlines
inline
float TileTiming::getSignalPhase(unsigned int drawerIdx, unsigned int channel, unsigned int adc) const {
  return m_timing->getCalibDrawer(drawerIdx)->getData(channel, adc, 0);
}

// Set up the ClassID of this class (obtained using 'clid -s TileTiming')
CLASS_DEF(TileTiming, 45959584, 0)
// Set up the ClassID of the container (obtained using 'clid -cs TileTiming')
CONDCONT_DEF(TileTiming, 72660768);

#endif
