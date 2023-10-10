
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @brief Helper macro to compare the output from the readout geometry dumps:
 *        python -m MuonGeoModelTest.runGeoModelTest
 *        python -m MuonGeoModelTestR4.runGeoModelTest 
 *  
*/
#include <GeoPrimitives/GeoPrimitives.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <MuonCablingData/MdtCablingData.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>


#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>


/// Helper struct to represent a full Rpc chamber
struct TgcChamber{
    /// Default constructor
    TgcChamber() = default;
    
    /// Identifier of the Rpc chamber
    using  chamberIdentifier = MdtCablingOffData;
    chamberIdentifier id{};

    /// Sorting operator to insert the object into std::set
    bool operator<(const TgcChamber& other) const {
        return id < other.id;
    }
    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};

    struct WireGang {
        unsigned int numWires{0};
        unsigned int gasGap{0};
        unsigned int number{0};
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        float length{0.f};

        bool operator<(const WireGang& other) const {
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            return number < other.number;
        }

    };
    std::set<WireGang> phiWires{};

};

std::ostream& operator<<(std::ostream& ostr, const TgcChamber::WireGang& gang) {
    ostr<<"wire gang (gap/number/n-Wires): "<<gang.gasGap<<"/"<<gang.number<<"/"<<gang.numWires;
    ostr<<" position: "<<Amg::toString(gang.position,2 );
    ostr<<" length: "<<gang.length;
    return ostr;
}

std::set<TgcChamber> readTreeDump(const std::string& inputFile) {
    std::set<TgcChamber> to_ret{};
    std::cout<<"Read the Rpc geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("TgcGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'TgcGeoModelTree'"<<std::endl;
        return to_ret;
    }
    /// Identifier
    TTreeReaderValue<unsigned short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};    
    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};


    TTreeReaderValue<std::vector<float>>gangCenterX{treeReader, "gangCenterX"};
    TTreeReaderValue<std::vector<float>>gangCenterY{treeReader, "gangCenterY"};
    TTreeReaderValue<std::vector<float>>gangCenterZ{treeReader, "gangCenterZ"};

    TTreeReaderValue<std::vector<uint8_t>> gangGasGap{treeReader, "gangGasGap"};
    TTreeReaderValue<std::vector<unsigned int>> gangNum{treeReader, "gangNumber"};
    TTreeReaderValue<std::vector<uint8_t>> gangNumWires{treeReader, "gangNumWires"};
    TTreeReaderValue<std::vector<float>> gangLength{treeReader, "gangLength"};

    while (treeReader.Next()) {
        TgcChamber newchamber{};

        newchamber.id.stationIndex = (*stationIndex);
        newchamber.id.eta = (*stationEta);
        newchamber.id.phi = (*stationPhi);

        Amg::Vector3D geoTrans{(*geoModelTransformX)[0], (*geoModelTransformY)[0], (*geoModelTransformZ)[0]};
        Amg::RotationMatrix3D geoRot{Amg::RotationMatrix3D::Identity()};
        geoRot.col(0) = Amg::Vector3D((*geoModelTransformX)[1], (*geoModelTransformY)[1], (*geoModelTransformZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*geoModelTransformX)[2], (*geoModelTransformY)[2], (*geoModelTransformZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*geoModelTransformX)[3], (*geoModelTransformY)[3], (*geoModelTransformZ)[3]);       
        newchamber.geoModelTransform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans)); 
        
        for (size_t g  = 0; g < gangGasGap->size(); ++g) {
            
            TgcChamber::WireGang newGang{};
            newGang.gasGap = (*gangGasGap)[g];
            newGang.number = (*gangNum)[g];
            newGang.numWires =(*gangNumWires)[g];
            newGang.position = Amg::Vector3D{(*gangCenterX)[g], (*gangCenterY)[g], (*gangCenterZ)[g]};
            newGang.length = (*gangLength)[g];
            auto insert_itr = newchamber.phiWires.insert(std::move(newGang));
            if (!insert_itr.second) {
                std::stringstream err{};
                err<<__FILE__<<":"<<__LINE__<<" The chamber "<<(*insert_itr.first)
                    <<" has already been inserted. "<<std::endl;
            }
        }
        
        auto insert_itr = to_ret.insert(std::move(newchamber));
        if (!insert_itr.second) {
            std::stringstream err{};
            err<<__FILE__<<":"<<__LINE__<<" The wire gang "<<(*insert_itr.first).id
               <<" has already been inserted. "<<std::endl;
            throw std::runtime_error(err.str());
        }
    }
    return to_ret;
}


int main( int argc, char** argv ) {
    std::string refFile{}, testFile{};
    
    for (int arg = 1; arg < argc; ++arg) {
       std::string the_arg{argv[arg]};
       if (the_arg == "--refFile" && arg +1 < argc) {
          refFile = std::string{argv[arg+1]};
          ++arg;
       } else if (the_arg == "--testFile" && arg + 1 < argc) {
            testFile = std::string{argv[arg+1]};
            ++arg;
       }
    }
    if (refFile.empty()) {
        std::cerr<<"Please parse the path of the reference file via --refFile "<<std::endl;
        return EXIT_FAILURE;
    }
    if (testFile.empty()) {
        std::cerr<<"Please parse the path of the test file via --testFile "<<std::endl;
        return EXIT_FAILURE;
    }
    /// check whether the files are xroot d -> otherwise call path resovler
    if (refFile.find("root://") != 0) refFile = PathResolver::FindCalibFile(refFile);
    if (testFile.find("root://") != 0) testFile = PathResolver::FindCalibFile(testFile);
    return EXIT_SUCCESS;
}