/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "CalibrationDataInterface/CDIReader.h"

#include <string>
#include <iomanip>
#include <iostream>


///////////////////////////////////////////////////////////////////////////////////////////////////
//// This executable demonstrates the usage of the CDIReader helper class                       //
//// Set up the object with the path of the CDI file, and play around with the metadata reader //
////////////////////////////////////////////////////////////////////////////////////////////////

using Analysis::CDIReader;

int main() {
    std::cout << " Launching the CDIReaderTester . . ." << std::endl;
    CDIReader Reader("/cvmfs/atlas.cern.ch/repo/sw/database/GroupData/xAODBTaggingEfficiency/13TeV/2020-21-13TeV-MC16-CDI-2021-04-16_v1.root");
    
    Reader.printTaggers();
    std::cout << " " << std::endl;
    Reader.printJetCollections();
    std::cout << " " << std::endl;
    Reader.printWorkingPoints();
    std::cout << " " << std::endl;
    Reader.printLabels();
    std::cout << " " << std::endl;
    
    // Check that a hypothetical configuration works, given your CDI file
    Reader.checkConfig("DL1", "AntiKt4EMPFlowJets_BTagging201810", "FixedCutBEff_60");

    std::cout << " ------------------------------------------------------ " << std::endl;
    Reader.printDSIDs();
    std::cout << " -----------------Finish CDIReader Test---------------- " << std::endl;

    return EXIT_SUCCESS;
}
