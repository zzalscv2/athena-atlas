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
        
        /** @brief: Defines a new radial strip.
         *  @param: Intersection between the left strip edge & the bottom panel edge measured from the panel edge center
         *  @param: Intersection between the left strip edge & the top panel edge measured from the panel edge center
        */
        void addStrip(const double posOnBottom,
                      const double posOnTop);
        
        /** @brief: Returns the direction of the radial strip (Pointing from the bottom edge to the top edge)
          * @param: Strip number in the global scheme [1- nStrips()] 
        */
        Amg::Vector2D stripDir(int stripNumber) const;
        /** @brief: Returns the intersection of the left strip edge at the bottom panel's edge*/
        Amg::Vector2D stripLeftEdgeBottom(int stripNumber) const;
        /** @brief: Returns the intersecton of the strip right edge at the bottom panel's edge*/
        Amg::Vector2D stripRightEdgeBottom(int stripNumber) const;
        /** @brief: Returns the intersection of the left strip edge at the top panel's edge */
        Amg::Vector2D stripLeftEdgeTop(int stripNumber) const;
        /** @brief: Returns the intersecetion fo the right strip edge at the top panel's edge */
        Amg::Vector2D stripRightEdgeTop(int stripNumber) const;

        /// Returns the number of defined strips
        int numStrips() const override;

    private:        
        Amg::Vector2D panelEdgeCenter() const;
        Amg::Vector2D leftInterSect(int stripNum, bool uncapped = false) const override final;
        Amg::Vector2D rightInterSect(int stripNum, bool uncapped = false) const override final;        
        /// Helper struct to 
        struct stripEdges{
            stripEdges(double dBot, double dTop):
                distOnBottom{dBot},
                distOnTop{dTop} {}

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

using RadialStripDesignSet = std::set<RadialStripDesignPtr, RadialDesignSorter>;

}
#include <MuonReadoutGeometryR4/RadialStripDesign.icc>
#endif