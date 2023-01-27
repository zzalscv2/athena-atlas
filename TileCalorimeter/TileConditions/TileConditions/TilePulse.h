/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILEPULSEFORM_H
#define TILECONDITIONS_TILEPULSEFORM_H

// Tile includes
#include "TileConditions/TileCalibData.h"

/**
 * @class TilePulse
 * @brief Condition object to keep and provide Tile pulse shape
 */
class TilePulse {
  public:

    TilePulse(std::unique_ptr<TileCalibDataFlt> pulseShape) : m_pulseShape{std::move(pulseShape)} {};
    virtual ~TilePulse() = default;

    bool getPulseShapeYDY(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                          float time, float &y, float &dy) const;

    TilePulse(const TilePulse&) = delete;
    TilePulse& operator= (const TilePulse&) = delete;

  private:

    //=== TileCalibData
    std::unique_ptr<TileCalibDataFlt> m_pulseShape;
};

// inlines
inline
bool TilePulse::getPulseShapeYDY(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                                     float time, float &y, float &dy) const {
  return m_pulseShape->getCalibDrawer(drawerIdx)->getYDY(channel, adc, time, y, dy);
}

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

CLASS_DEF(TilePulse, 239991006, 0)
CONDCONT_DEF(TilePulse, 228652188);

#endif
