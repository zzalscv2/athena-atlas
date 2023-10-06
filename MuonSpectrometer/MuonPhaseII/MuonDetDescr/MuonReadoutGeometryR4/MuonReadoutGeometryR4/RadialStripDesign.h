/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_RADIALSTRIPDESIGN_H
#define MUONREADOUTGEOMETRYR4_RADIALSTRIPDESIGN_H

#include <MuonReadoutGeometryR4/StripDesign.h>

namespace MuonGMR4{

/* The phi readout channels of the Tgc chambers are an essamble of wires that are readout 
 * together. The wires are equi-distant, but not every wire group in a gas gap contains the
 * same number of wires. Hence, the channel width & pitch vary across the board. The wire group 
 * design accounts for this detector design feature. It inherits from the StripDesign class and 
 * overloads the stripPosition feature to place the measurement onto the middle wire of each group.
 * Nevertheless, it's important to know about the position and the lenghts of each wire seperately
 * as these quantities are fed into the digitization. Therefore, extra methods are added to the design
 * to provide this information as well.
*/
class RadialStripDesign;
using RadialStripDesignPtr = GeoModel::TransientConstSharedPtr<RadialStripDesign>;
    
class RadialStripDesign: public StripDesign {
    public:
        RadialStripDesign() = default;
        /// set sorting operator
        bool operator<(const RadialStripDesign& other) const;
        
        void addStrip(const double posOnBottom,
                      const double posOnTop);
        
        Amg::Vector2D stripDir(int stripNumber) const;

    private:
        Amg::Vector2D stripPosition(int stripNum) const override final;
        struct stripEdges{
            double distOnBottom{0.};
            double distOnTop{0.};
        };
        std::vector<stripEdges> m_strips{};
};

struct RadialDesignSorter{
    bool operator()(const RadialStripDesignPtr& a, const RadialStripDesignPtr& b) const {
            return (*a) < (*b);
    }
    bool operator()(const RadialStripDesign&a ,const RadialStripDesign& b) const {
        return a < b;
    }
};

}
#endif