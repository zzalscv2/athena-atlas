/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILECONDTOOLPULSESHAPE_H
#define TILECONDITIONS_TILECONDTOOLPULSESHAPE_H

// Tile includes
#include "TileConditions/TilePulse.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"


#include <vector>

class TileCondToolPulseShape: public AthAlgTool {
  public:

    static const InterfaceID& interfaceID();
    TileCondToolPulseShape(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~TileCondToolPulseShape();

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    bool getPulseShapeYDY(unsigned int drawerIdx, unsigned int channel, unsigned int adc,
                          float time, float &y, float &dy,
                          const EventContext &ctx) const;

  private:

   /**
    * @brief Name of TilePulse in condition store
    */
    SG::ReadCondHandleKey<TilePulse> m_pulseShapeKey{this,
        "TilePulseShape", "TilePulseShape", "Input Tile pulse shape"};

};

#endif
