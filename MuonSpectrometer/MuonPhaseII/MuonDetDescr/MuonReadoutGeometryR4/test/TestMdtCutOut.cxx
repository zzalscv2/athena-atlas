/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <iostream>
#include <MuonReadoutGeometryR4/MdtCutOut.h>
#include <MuonReadoutGeometryR4/MdtReadoutElement.h>
#include <Identifier/IdentifierHash.h>



int main (int, char**){
    /// Let's define a set with a couple of cutouts
    using namespace MuonGMR4;
    
    MdtCutOuts cutOutSet{};

    MdtCutOut cutLay1{};
    /// Define a first cut out
    const IdentifierHash hash1Start = MdtReadoutElement::measurementHash(1, 45);
    const IdentifierHash hash1End = MdtReadoutElement::measurementHash(1, 77);
    cutLay1.layer = MdtReadoutElement::layerNumber(hash1Start);
    cutLay1.firstTube = MdtReadoutElement::tubeNumber(hash1Start);
    cutLay1.lastTube =  MdtReadoutElement::tubeNumber(hash1End);
    cutLay1.leftX = 123.;
    cutLay1.rightX = 456;
    /// Insert the cutout into the set
    cutOutSet.insert(cutLay1);
    /// Define a cutout for the second layer
    MdtCutOut cutLay2{};
    const IdentifierHash hash2Start = MdtReadoutElement::measurementHash(2, 23);
    const IdentifierHash hash2End = MdtReadoutElement::measurementHash(1, 27);            
    
    cutLay2.layer = MdtReadoutElement::layerNumber(hash2Start);
    cutLay2.firstTube = MdtReadoutElement::tubeNumber(hash2Start);
    cutLay2.lastTube = MdtReadoutElement::tubeNumber(hash2End);
    cutLay2.leftX = 562;
    cutLay2.rightX = 421;
    /// Insert the cutout
    if (!cutOutSet.insert(cutLay2).second){
        std::cerr<<"Failed to add tube cutout "<<cutLay2<<std::endl;
        return EXIT_FAILURE;
    }
    MdtCutOut cutLay1a{};
    const IdentifierHash hash1aStart = MdtReadoutElement::measurementHash(1, 12);
    const IdentifierHash hash1aEnd = MdtReadoutElement::measurementHash(1, 33);
    cutLay1a.layer = MdtReadoutElement::layerNumber(hash1aStart);
    cutLay1a.firstTube = MdtReadoutElement::tubeNumber(hash1aStart);
    cutLay1a.lastTube =  MdtReadoutElement::tubeNumber(hash1aEnd);
    cutLay1a.rightX = 211.;
    if (!cutOutSet.insert(cutLay1a).second){
        std::cerr<<"Failed to add tube cutout "<<cutLay1a<<std::endl;
        return EXIT_FAILURE;
    }
    ++cutLay1a.firstTube;
    if (cutOutSet.insert(cutLay1a).second) {
        std::cerr<<"Should not add overlapping cutout "<<cutLay1a<<std::endl;
    }
    std::cout<<"Filled cutouts "<<cutOutSet.size()<<std::endl;
    /// Test the look up scheme
    for (unsigned int tube = 1; tube < 90; ++tube){
        const IdentifierHash hash = MdtReadoutElement::measurementHash(1,tube);
        MdtCutOuts::const_iterator itr = cutOutSet.find(hash);
        
        const unsigned int tubeHash = MdtReadoutElement::tubeNumber(hash);
        if (itr != cutOutSet.end()) {
            if (itr->layer != MdtReadoutElement::layerNumber(hash) || itr->firstTube > tubeHash || itr->lastTube < tubeHash){
                std::cerr<<"Wrong cutout catched for 1,"<<tube<<". Got "<<(*itr)<<std::endl;
                return EXIT_FAILURE;
            }
        } else if ((tubeHash >=
                    MdtReadoutElement::tubeNumber(hash1Start) && 
                    tubeHash <=
                    MdtReadoutElement::tubeNumber(hash1End)) || 
                    
                    (tubeHash >=
                    MdtReadoutElement::tubeNumber(hash1aStart) && 
                    tubeHash <=
                    MdtReadoutElement::tubeNumber(hash1aEnd))
                    
                    ) {
            std::cerr<<"Expected to find a cut out for 1,"<<tube<<". Got nothing"<<std::endl;
            return EXIT_FAILURE;
        }        
    }
    /// Ensure that layer 3 is not returning anything
    if (cutOutSet.find(MdtReadoutElement::measurementHash(3,1)) !=cutOutSet.end()){
        std::cerr<<"There should be no cut out registered for 3,1"<<std::endl;
        return EXIT_FAILURE;
    }
    std::cout<<"Mdt cutout test is a great success "<<std::endl;
    return EXIT_SUCCESS;
}