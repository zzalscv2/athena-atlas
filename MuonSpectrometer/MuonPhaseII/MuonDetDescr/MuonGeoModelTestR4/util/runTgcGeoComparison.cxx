
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @brief Helper macro to compare the output from the readout geometry dumps:
 *        python -m MuonGeoModelTest.runGeoModelTest
 *        python -m MuonGeoModelTestR4.runGeoModelTest 
 *  
*/
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <MuonReadoutGeometryR4/StripDesign.h>

#include <GeoPrimitives/GeoPrimitives.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>

#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>

constexpr double tolerance = 100 * Gaudi::Units::micrometer;

using namespace MuonGMR4;
/// Helper struct to represent a full Rpc chamber
struct TgcChamber{
    /// Default constructor
    TgcChamber() = default;

    /// Identifier fields stationIndex / stationEta / stationPhi
    unsigned stIdx{0};
    int eta{0};
    unsigned phi{0};
    ///
    float shortWidth{0.f};
    float longWidth{0.f};
    float height{0.f};
    float thickness{0.f};

    /// Sorting operator to insert the object into std::set
    bool operator<(const TgcChamber& other) const {
        if (stIdx != other.stIdx) return stIdx < other.stIdx;
        if (eta != other.eta) return eta < other.eta;
        return phi < other.phi;
    }
    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};

    struct WireGang {
        unsigned int numWires{0};
        unsigned int gasGap{0};
        unsigned int number{0};
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        Amg::Vector2D localPos{Amg::Vector2D::Zero()};
        float length{0.f};

        bool operator<(const WireGang& other) const {
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            return number < other.number;
        }
    };

    struct LayerTrans{
        unsigned int gasGap{0};
        bool measPhi{false};
        Amg::Transform3D trans{Amg::Transform3D::Identity()};
        
        float shortWidth{0.f};
        float longWidth{0.f};
        float height{0.f};

        unsigned int numWires{0};

        bool operator<(const LayerTrans& other) const{
             if (gasGap!= other.gasGap) return gasGap < other.gasGap;
             return measPhi < other.measPhi;
        }
    };
    std::set<WireGang> etaWires{};
    std::set<LayerTrans> transforms{};

};

class ChamberWidthTest : public StripDesign {
    public:
        double widthAtPos(const Amg::Vector2D& pos) const{
            Amg::Vector2D delta  = leftInterSect(pos) - rightInterSect(pos);
            return std::hypot (delta.x(), delta.y());
        }


};

std::ostream& operator<<(std::ostream& ostr, const TgcChamber::WireGang& gang) {
    ostr<<"wire gang (gap/number/n-Wires/length): "<<gang.gasGap<<"/"<<gang.number<<"/"<<gang.numWires<<"/"<<gang.length;
    // ostr<<" position: "<<Amg::toString(gang.position,2 );
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const TgcChamber::LayerTrans& lTrans) {
    ostr<<"layer transform (gap/measPhi) "<<lTrans.gasGap<<"/"<<(lTrans.measPhi ? "si" : "no");
    // ostr<<"  "<<to_string(lTrans.trans);
    return ostr;
}
std::ostream& operator<<(std::ostream& ostr, const TgcChamber& chamb) {
     static const std::map<int, std::string> stationDict{
        {41, "T1F"}, {42, "T1E"}, 
        {43, "T2F"}, {44, "T2E"},
        {45, "T3F"}, {46, "T3E"},
        {47, "T4F"}, {48, "T4E"},
    };
    ostr<<"stName: "<<stationDict.at(chamb.stIdx)<<" ";
    ostr<<"eta: "<<chamb.eta<<" ";
    ostr<<"phi: "<<chamb.phi<<" ";
    return ostr;    
}


std::set<TgcChamber> readTreeDump(const std::string& inputFile) {
    std::set<TgcChamber> to_ret{};
    std::cout<<"Read the Tgc geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<"runTgcComparison() "<<__LINE__<<": Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("TgcGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()) {
        std::cerr<<"runTgcComparison() "<<__LINE__<<": The file "<<inputFile<<" does not contain the 'TgcGeoModelTree'"<<std::endl;
        return to_ret;
    }
    /// Identifier
    TTreeReaderValue<unsigned short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};

    TTreeReaderValue<float> shortWidth{treeReader, "ChamberWidthS"};
    TTreeReaderValue<float> longWidth{treeReader, "ChamberWidthL"};
    TTreeReaderValue<float> height{treeReader, "ChamberHeight"};
    TTreeReaderValue<float> thickness{treeReader, "ChamberThickness"};
    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};


    TTreeReaderValue<std::vector<float>>gangCenterX{treeReader, "gangCenterX"};
    TTreeReaderValue<std::vector<float>>gangCenterY{treeReader, "gangCenterY"};
    TTreeReaderValue<std::vector<float>>gangCenterZ{treeReader, "gangCenterZ"};
    
    TTreeReaderValue<std::vector<float>>gangLocalPosX{treeReader, "gangLocalPosX"};
    TTreeReaderValue<std::vector<float>>gangLocalPosY{treeReader, "gangLocalPosY"};

    TTreeReaderValue<std::vector<uint8_t>> gangGasGap{treeReader, "gangGasGap"};
    TTreeReaderValue<std::vector<unsigned int>> gangNum{treeReader, "gangNumber"};
    TTreeReaderValue<std::vector<uint8_t>> gangNumWires{treeReader, "gangNumWires"};
    TTreeReaderValue<std::vector<float>> gangLength{treeReader, "gangLength"};

    TTreeReaderValue<std::vector<float>> layerCol1X{treeReader, "layerLinearCol1X"};
    TTreeReaderValue<std::vector<float>> layerCol1Y{treeReader, "layerLinearCol1Y"};
    TTreeReaderValue<std::vector<float>> layerCol1Z{treeReader, "layerLinearCol1Z"};

    TTreeReaderValue<std::vector<float>> layerCol2X{treeReader, "layerLinearCol2X"};
    TTreeReaderValue<std::vector<float>> layerCol2Y{treeReader, "layerLinearCol2Y"};
    TTreeReaderValue<std::vector<float>> layerCol2Z{treeReader, "layerLinearCol2Z"};

    TTreeReaderValue<std::vector<float>> layerCol3X{treeReader, "layerLinearCol3X"};
    TTreeReaderValue<std::vector<float>> layerCol3Y{treeReader, "layerLinearCol3Y"};
    TTreeReaderValue<std::vector<float>> layerCol3Z{treeReader, "layerLinearCol3Z"};

    TTreeReaderValue<std::vector<float>> layerTransX{treeReader, "layerTranslationX"};
    TTreeReaderValue<std::vector<float>> layerTransY{treeReader, "layerTranslationY"};
    TTreeReaderValue<std::vector<float>> layerTransZ{treeReader, "layerTranslationZ"};
    
    TTreeReaderValue<std::vector<bool>> layerMeasPhi{treeReader,"layerMeasPhi"};
    TTreeReaderValue<std::vector<uint8_t>> layerNumber{treeReader,"layerNumber"};
    
    TTreeReaderValue<std::vector<float>> layShortWidth{treeReader,"layerWidthS"};
    TTreeReaderValue<std::vector<float>> layLongWidth{treeReader,"layerWidthL"};
    TTreeReaderValue<std::vector<float>> layHeight{treeReader, "layerHeight"};
    TTreeReaderValue<std::vector<uint16_t>> layerNumWires{treeReader, "layerNumWires"};

    while (treeReader.Next()) {
        TgcChamber newchamber{};

        newchamber.stIdx = (*stationIndex);
        newchamber.eta = (*stationEta);
        newchamber.phi = (*stationPhi);
        newchamber.shortWidth = (*shortWidth);
        newchamber.longWidth= (*longWidth);
        newchamber.height = (*height);
        newchamber.thickness = (*thickness);
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
            newGang.localPos = Amg::Vector2D{(*gangLocalPosX)[g], (*gangLocalPosY)[g]};
            newGang.length = (*gangLength)[g];
            auto insert_itr = newchamber.etaWires.insert(std::move(newGang));
            if (!insert_itr.second) {
                std::stringstream err{};
                err<<"runTgcComparison() "<<__LINE__<<": The chamber "<<(*insert_itr.first)
                    <<" has already been inserted. "<<std::endl;
                throw std::runtime_error(err.str());
            }
        }

        for (size_t l = 0 ; l < layerMeasPhi->size(); ++l) {
            Amg::RotationMatrix3D layRot{Amg::RotationMatrix3D::Identity()};
            layRot.col(0) = Amg::Vector3D{(*layerCol1X)[l], (*layerCol1Y)[l], (*layerCol1Z)[l]};
            layRot.col(1) = Amg::Vector3D{(*layerCol2X)[l], (*layerCol2Y)[l], (*layerCol2Z)[l]};
            layRot.col(2) = Amg::Vector3D{(*layerCol3X)[l], (*layerCol3Y)[l], (*layerCol3Z)[l]};
            Amg::Vector3D trans{(*layerTransX)[l],(*layerTransY)[l],(*layerTransZ)[l]};
            TgcChamber::LayerTrans layTrans{};
            layTrans.trans = Amg::getTransformFromRotTransl(std::move(layRot), std::move(trans));
            layTrans.gasGap = (*layerNumber)[l];
            layTrans.measPhi = (*layerMeasPhi)[l];
            layTrans.shortWidth = (*layShortWidth)[l];
            layTrans.longWidth = (*layLongWidth)[l];
            layTrans.height = (*layHeight)[l];
            layTrans.numWires = (*layerNumWires)[l];
            auto insert_itr = newchamber.transforms.insert(std::move(layTrans));
            if (!insert_itr.second) {
                std::stringstream err{};
                err<<"runTgcComparison() "<<__LINE__<<": The chamber "<<(*insert_itr.first)
                    <<" has already been inserted. "<<std::endl;
                throw std::runtime_error(err.str());
            }
        }
        
        auto insert_itr = to_ret.insert(std::move(newchamber));
        if (!insert_itr.second) {
            std::stringstream err{};
            err<<"runTgcComparison() "<<__LINE__<<": The chamber "<<(*insert_itr.first)
               <<" has already been inserted. "<<std::endl;
            throw std::runtime_error(err.str());
        }
    }
    return to_ret;
}

#define TEST_BASICPROP(attribute, propName)                                      \
    if (std::abs(1.*test.attribute - 1.*ref.attribute) > tolerance) {            \
        std::cerr<<"TgcGeoModelComparison() "<<__LINE__<<": The chamber "<<test  \
                 <<" differs w.r.t "<<propName<<" "<< ref.attribute              \
                 <<" (ref) vs. " <<test.attribute << " (test)" << std::endl;     \
        chambOk = false;                                                         \
    }

#define TEST_LAYPROP(attribute, propName)                                          \
    if (std::abs(1.*refTrans.attribute - 1.*testTrans.attribute) > tolerance) {    \
        std::cerr<<"TgcGeoModelComparison() "<<__LINE__<<": The chamber "<<test    \
                 <<" differs in "<<refTrans<<" w.r.t. "<<propName<<". "            \
                 <<refTrans.attribute<<" (ref) vs. "<<testTrans.attribute          \
                 <<" (test), delta: "<<(refTrans.attribute - testTrans.attribute)  \
                 <<std::endl;                                                      \
        chambOk = false;                                                           \
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

    const std::set<TgcChamber> refChambers = readTreeDump(refFile);
    const std::set<TgcChamber> testChambers = readTreeDump(testFile);
    int retCode{EXIT_SUCCESS};
    for (const TgcChamber& ref : refChambers) {
        std::set<TgcChamber>::const_iterator test_itr = testChambers.find(ref);
        if (test_itr == testChambers.end()) {
            std::cerr<<"runTgcComparison() "<<__LINE__<<": The chamber "<<ref<<" is not in the test geometry. "<<std::endl;
            retCode = EXIT_FAILURE;
            continue;
        }
        const TgcChamber& test{*test_itr};
        bool chambOk{true};
        /// Check the chamber dimensions
        TEST_BASICPROP(thickness, "chamber thickness");
        TEST_BASICPROP(shortWidth, "chamber short width");
        TEST_BASICPROP(longWidth, "chamber long width");
        TEST_BASICPROP(height, "chamber height");        
        /// Check the orientation of the layers
        for (const TgcChamber::LayerTrans& refTrans : ref.transforms) {
            std::set<TgcChamber::LayerTrans>::const_iterator l_test_itr = test.transforms.find(refTrans);
            if (l_test_itr == test.transforms.end()) {
                std::cerr<<"runTgcComparison() "<<__LINE__<<": The layer "<<refTrans
                          <<" in chamber "<<ref
                          <<" is not part of the test geometry. "<<std::endl;
                chambOk = false;
                break;
            }
            const TgcChamber::LayerTrans& testTrans{*l_test_itr};
            /// check whether the transformations lead to the same point
            const Amg::Transform3D layTest = testTrans.trans.inverse() * refTrans.trans;
            if (!doesNotDeform(layTest)) {
                std::cerr<<"runTgcComparison() "<<__LINE__<<": In "<<ref<<" "<<refTrans
                         <<", the transformations are orientated diffrently "<<to_string(layTest)<<std::endl;
                chambOk = false;  
                break;              
            }
            if (layTest.translation().mag() > tolerance) {
                 std::cerr<<"runTgcComparison() "<<__LINE__<<": The transformations in layer "<<refTrans<<", "
                          <<ref<<" are pointing to different reference points "<<Amg::toString(layTest.translation(),2)<<std::endl;
                chambOk = false;
                break;
            }
            TEST_LAYPROP(numWires , "number of wires");
            TEST_LAYPROP(shortWidth, "short width");
            TEST_LAYPROP(longWidth, "long width"); 
            TEST_LAYPROP(height, "height");
            if (!chambOk) break;
        }
        /// Let's take a look at the wire gang positions
        for (const TgcChamber::WireGang& refGang : ref.etaWires) {
            std::set<TgcChamber::WireGang>::const_iterator test_itr = test.etaWires.find(refGang);
            if (test_itr == test.etaWires.end()) {
                std::cerr<<"runTgcComparison() "<<__LINE__<<": "<<refGang<<" is not part of "
                            <<ref<<std::endl;
                retCode = EXIT_FAILURE;
                continue;
            }
            const TgcChamber::WireGang& testGang{*test_itr};
            if (testGang.numWires != refGang.numWires) {
                std::cerr<<"runTgcComparison() "<<__LINE__<<": "<<refGang<<" in "<<ref
                            <<" has different  wires. "<<refGang.numWires<<" vs. "
                            <<testGang.numWires<<std::endl;
                chambOk = false;
            }                
            const Amg::Vector3D diffPos = refGang.position - testGang.position;
            constexpr double halfPitch = 0.9 * Gaudi::Units::mm;
            if (diffPos.mag() - halfPitch > tolerance) {
                std::cerr<<"runTgcComparison() "<<__LINE__<<": In "<<ref<<" "<<testGang
                         <<" is displaced by "<<Amg::toString(diffPos, 2)<<", mag="<<diffPos.mag()<<std::endl;
                chambOk = false;
            }
            TgcChamber::LayerTrans findLayer{};
            findLayer.gasGap = testGang.gasGap;
            const TgcChamber::LayerTrans& refLayer{*ref.transforms.find(findLayer)};
            const TgcChamber::LayerTrans& testLayer{*test.transforms.find(findLayer)};
            
            ChamberWidthTest wTest{};
            wTest.defineTrapezoid(refLayer.shortWidth/2, refLayer.longWidth /2 , refLayer.height /2);
            
            if (std::abs(refGang.length - testGang.length) > tolerance) {
                std::cerr<<"runTgcComparison() "<<__LINE__<<": In "<<ref<<" "<<testGang<<" different length detected "
                         <<refGang.length<<" (ref) vs. "<<testGang.length<<" (test). Delta: "
                         <<(refGang.length - testGang.length)
                         <<" edge intersection: "<<(wTest.widthAtPos(refGang.localPos) - std::abs(refLayer.shortWidth - testLayer.shortWidth))
                         <<" hand calculation: "<<(wTest.shortHalfHeight()*2. + (wTest.halfWidth() + refGang.localPos.x())* 
                                (wTest.longHalfHeight() - wTest.shortHalfHeight()) / wTest.halfWidth() - std::abs(refLayer.shortWidth - testLayer.shortWidth))<<std::endl;      
                chambOk = false;
            }
        }
        if (!chambOk) {
            retCode = EXIT_FAILURE;
            continue;
        }
    }
    return retCode;
}