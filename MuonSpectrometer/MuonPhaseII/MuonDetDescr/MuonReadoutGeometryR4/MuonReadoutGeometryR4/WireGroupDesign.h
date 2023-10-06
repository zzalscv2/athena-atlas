/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRYR4_WIREGROUPDESIGN_H
#define MUONREADOUTGEOMETRYR4_WIREGROUPDESIGN_H

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
class WireGroupDesign;
using WireDesignPtr = GeoModel::TransientConstSharedPtr<WireGroupDesign>;
    
class WireGroupDesign: public StripDesign {
    public:
        WireGroupDesign() = default;
        /// set sorting operator
        bool operator<(const WireGroupDesign& other) const;
        /// Adds a new group of wires to the design.
        void declareGroup(const unsigned int numWires);
        /// Returns the number of wires in a given group.
        unsigned int numWiresInGroup(unsigned int groupNum) const;
        /// Returns the positition of the i-th wire in the g-th group
        /// groupNum    [1; numStrips()
        /// wire number [1; numWiresInGroup()
        Amg::Vector2D wirePosition(unsigned int groupNum, 
                                   unsigned int wireNum) const;
    
        /// @brief Returns the edge point at negative y
        /// @param groupNum [1; numStrips()]
        /// @param wireNum  [1; numWiresInGroup()]
        /// @return  
        Amg::Vector2D leftWireEdge(unsigned int groupNum,
                                   unsigned int wireNum) const;
        
        /// @brief Returns the edge point at positive y
        /// @param groupNum [1; numStrips()]
        /// @param wireNum  [1; numWiresInGroup()]
        /// @return  
        Amg::Vector2D rightWireEdge(unsigned int groupNum,
                                    unsigned int wireNum) const;
    private:
        Amg::Vector2D stripPosition(int stripNum) const override final;
        /// @brief helper construct to cache the number of wires in each group as well
        ///        as the accumulated number of wires from the previous groups.
        struct wireGroup{
            wireGroup(unsigned int nWires, unsigned int accWires):
                numWires{nWires}, accumlWires{accWires} {} 
            /// Number of wires in this group
            unsigned int numWires{0};
            /// Number of all wires in the previous groups
            unsigned int accumlWires{0};
        };
        std::vector<wireGroup> m_groups{};
};

struct WireDesignSorter{
    bool operator()(const WireDesignPtr&a, const WireDesignPtr& b) const {
            return (*a) < (*b);
    }
    bool operator()(const WireGroupDesign&a ,const WireGroupDesign& b) const {
        return a < b;
    }
};

}
#endif