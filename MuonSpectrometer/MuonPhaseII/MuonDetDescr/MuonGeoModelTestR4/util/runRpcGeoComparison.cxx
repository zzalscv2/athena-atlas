
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
#include <MuonCablingData/NrpcCablingData.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>


#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>

constexpr double tolerance = 10 * Gaudi::Units::micrometer;

/// Helper struct to represent a full Rpc chamber
struct RpcChamber{
    /// Default constructor
    RpcChamber() = default;
    
    /// Identifier of the Rpc chamber
    using  chamberIdentifier = NrpcCablingOfflineID;
    chamberIdentifier id{};

    /// Sorting operator to insert the object into std::set
    bool operator<(const RpcChamber& other) const {
        return id < other.id;
    }
    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};

    float stripPitchEta{0.f};
    float stripPitchPhi{0.f};
    float stripWidthEta{0.f};
    float stripWidthPhi{0.f};

    float stripLengthEta{0.f};
    float stripLengthPhi{0.f};

    unsigned int numStripsEta{0};
    unsigned int numStripsPhi{0};

    unsigned int numGasGapsEta{0};
    unsigned int numGasGapsPhi{0};

    struct RpcStrip{
        Amg::Vector3D position{Amg::Vector3D::Zero()};
        /// @brief  Strip number
        unsigned int strip{0};
        /// @brief  Gas gap of the strip
        unsigned int gasGap{0};
        /// @brief  Phi panel of the strip
        unsigned int doubletPhi{0};
        /// @brief flag whether the strip measures phi
        bool measPhi{false};

        /// @brief Odering operator to use the strip with set
        bool operator<(const RpcStrip& other) const {
            if (measPhi != other.measPhi) return !measPhi;
            if (doubletPhi != other.doubletPhi) return doubletPhi < other.doubletPhi;
            if (gasGap != other.gasGap) return gasGap < other.gasGap;
            return strip < other.strip;
        }       
    };
    std::set<RpcStrip> strips{};
 
};

/// Translation of the station Index -> station Name. Dictionary taken from
/// https://gitlab.cern.ch/atlas/athena/-/blob/main/DetectorDescription/IdDictParser/data/IdDictMuonSpectrometer_R.09.03.xml
std::ostream& operator<<(std::ostream& ostr, const RpcChamber& chamb) {
    static const std::map<int, std::string> stationDict{
        {0, "BIL"}, {1, "BIS"}, {7, "BIR"},
        {2, "BML"}, {3, "BMS"}, {8, "BMF"}, {53, "BME"}, {54, "BMG"}, {52, "BIM"},
        {4, "BOL"}, {5, "BOS"}, {9, "BOF"}, {10, "BOG"},
        {6, "BEE"}, {14, "EEL"}, {15, "EES"}, 
        {13, "EIL"}, 
        {17, "EML"}, {18, "EMS"}, 
        {20, "EOL"}, {21, "EOS"}
    };
    ostr<<"Rpc chamber "<<stationDict.at(chamb.id.stationIndex)<<" "<<chamb.id;
    return ostr;
}

 std::ostream& operator<<(std::ostream& ostr,const RpcChamber::RpcStrip & strip) {
    ostr<<"strip (gasGap/phiPanel/isPhiStrip/number): ";
    ostr<<strip.gasGap<<"/"<<strip.doubletPhi<<"/";
    ostr<<(strip.measPhi ? "si" : "no")<<"/"<<strip.strip<<", ";
    ostr<<"position: "<<Amg::toString(strip.position, 2);
    return ostr;
}





std::set<RpcChamber> readTreeDump(const std::string& inputFile) {
    std::set<RpcChamber> to_ret{};
    std::cout<<"Read the Rpc geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("RpcGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'RpcGeoModelTree'"<<std::endl;
        return to_ret;
    }

    TTreeReaderValue<unsigned short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};
    TTreeReaderValue<uint8_t> stationDoubletR{treeReader, "stationDoubletR"};
    TTreeReaderValue<uint8_t> stationDoubletZ{treeReader,"stationDoubletZ"};
    TTreeReaderValue<uint8_t> stationDoubletPhi{treeReader,"stationDoubletPhi"};

     /// Number of strips, strip pitch in eta & phi direction
    TTreeReaderValue<uint8_t> numStripsEta{treeReader, "numEtaStrips"};
    TTreeReaderValue<uint8_t> numStripsPhi{treeReader, "numPhiStrips"};
    /// Number of eta & phi gas gaps
    TTreeReaderValue<uint8_t> numGasGapsEta{treeReader, "numEtaGasGaps"};
    TTreeReaderValue<uint8_t> numGasGapsPhi{treeReader, "numPhiGasGaps"};
       
    /// Strip dimensions 
    TTreeReaderValue<float> stripEtaPitch{treeReader, "stripEtaPitch"};
    TTreeReaderValue<float> stripPhiPitch{treeReader, "stripPhiPitch"};
    TTreeReaderValue<float> stripEtaWidth{treeReader, "stripEtaWidth"};
    TTreeReaderValue<float> stripPhiWidth{treeReader, "stripPhiWidth"};
    TTreeReaderValue<float> stripEtaLength{treeReader, "stripEtaLength"};
    TTreeReaderValue<float> stripPhiLength{treeReader, "stripPhiLength"};
 
    /// Geo Model transformation
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};

    TTreeReaderValue<std::vector<float>> stripPosX{treeReader, "stripPosX"};
    TTreeReaderValue<std::vector<float>> stripPosY{treeReader, "stripPosY"};
    TTreeReaderValue<std::vector<float>> stripPosZ{treeReader, "stripPosZ"};
    
    TTreeReaderValue<std::vector<bool>> stripPosMeasPhi{treeReader, "stripPosMeasPhi"};
    TTreeReaderValue<std::vector<uint8_t>> stripPosGasGap{treeReader, "stripPosGasGap"};
    TTreeReaderValue<std::vector<uint8_t>> stripPosNum{treeReader, "stripPosNum"};
    TTreeReaderValue<std::vector<uint8_t>> stripDblPhi{treeReader, "stripPosDoubletPhi"};
    while (treeReader.Next()) {
        RpcChamber newchamber{};

        newchamber.id.stationIndex = (*stationIndex);
        newchamber.id.eta = (*stationEta);
        newchamber.id.phi = (*stationPhi);
        newchamber.id.doubletPhi = (*stationDoubletPhi);
        newchamber.id.doubletR = (*stationDoubletR);
        newchamber.id.doubletZ = (*stationDoubletZ);
        
        newchamber.stripPitchEta = (*stripEtaPitch);
        newchamber.stripPitchPhi = (*stripPhiPitch);
        newchamber.stripWidthEta = (*stripEtaWidth);
        newchamber.stripWidthPhi = (*stripPhiWidth);
        newchamber.stripLengthEta = (*stripEtaLength);
        newchamber.stripLengthPhi = (*stripPhiLength);

        newchamber.numStripsEta = (*numStripsEta);
        newchamber.numStripsPhi = (*numStripsPhi);
        newchamber.numGasGapsEta = (*numGasGapsEta);
        newchamber.numGasGapsPhi = (*numGasGapsPhi);
        

        Amg::Vector3D geoTrans{(*geoModelTransformX)[0], (*geoModelTransformY)[0], (*geoModelTransformZ)[0]};
        Amg::RotationMatrix3D geoRot{Amg::RotationMatrix3D::Identity()};
        geoRot.col(0) = Amg::Vector3D((*geoModelTransformX)[1], (*geoModelTransformY)[1], (*geoModelTransformZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*geoModelTransformX)[2], (*geoModelTransformY)[2], (*geoModelTransformZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*geoModelTransformX)[3], (*geoModelTransformY)[3], (*geoModelTransformZ)[3]);       
        newchamber.geoModelTransform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans));       
        
        //strips
        for (size_t s = 0; s < stripPosX->size(); ++s){
            using RpcStrip = RpcChamber::RpcStrip;
            RpcStrip newStrip{};
            newStrip.position = Amg::Vector3D{(*stripPosX)[s], (*stripPosY)[s], (*stripPosZ)[s]};
            newStrip.gasGap = (*stripPosGasGap)[s];
            newStrip.doubletPhi = (*stripDblPhi)[s];
            newStrip.measPhi = (*stripPosMeasPhi)[s];
            newStrip.strip = (*stripPosNum)[s];
            newchamber.strips.insert(std::move(newStrip));
        }
        
        auto insert_itr = to_ret.insert(std::move(newchamber));
        if (!insert_itr.second) {
            std::stringstream err{};
            err<<__FILE__<<":"<<__LINE__<<" The chamber "<<(*insert_itr.first).id
               <<" has already been inserted. "<<std::endl;
            throw std::runtime_error(err.str());
        }
    }
    std::cout<<"File parsing is finished. Found in total "<<to_ret.size()<<" readout element dumps "<<std::endl;
    return to_ret;
}

/// Returns true if the linear part of the transformation does not
/// Apply a rotation or elongation
bool doesNotDeform(const Amg::Transform3D& trans) {
    for (unsigned int d = 0; d < 3 ; ++d) {
        const double defLength = Amg::Vector3D::Unit(d).dot(trans.linear() * Amg::Vector3D::Unit(d));
        if (std::abs(defLength - 1.) > std::numeric_limits<float>::epsilon()) {
            return false;
        }
    }
    return true;
}

#define TEST_BASICPROP(attribute, propName) \
    if (std::abs(1.*test.attribute - 1.*reference.attribute) > tolerance) {           \
        std::cerr<<"RpcGeoModelComparison() "<<__LINE__<<": The chamber "<<reference  \
                 <<" differs w.r.t "<<propName<<" "<< reference.attribute             \
                 <<" (ref) vs. " <<test.attribute << " (test)" << std::endl;          \
        chamberOkay = false;                                                          \
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
    /// Parse the tree dump
    std::set<RpcChamber> refChambers = readTreeDump(refFile);
    if (refChambers.empty()) {
        std::cerr<<"The file "<<refFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::set<RpcChamber> testChambers = readTreeDump(testFile);
    if (testChambers.empty()) {
        std::cerr<<"The file "<<testFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    int return_code = EXIT_SUCCESS;
    /// Start to loop over the chambers
    for (const RpcChamber& reference : refChambers) {
        std::set<RpcChamber>::const_iterator test_itr = testChambers.find(reference);
        
        if (test_itr == testChambers.end()) {
            std::cerr<<"The chamber "<<reference<<" is not part of the testing "<<std::endl;
            return_code = EXIT_FAILURE;
            continue;
        }
        bool chamberOkay = true;
        const RpcChamber& test = {*test_itr};
        
        TEST_BASICPROP(numGasGapsEta, "numer of eta gas gaps");
        TEST_BASICPROP(numGasGapsPhi, "numer of phi gas gaps");
        
        TEST_BASICPROP(numStripsEta, "numer of eta strips");
        TEST_BASICPROP(numStripsPhi, "numer of phi strips");
        
        TEST_BASICPROP(stripPitchEta, "eta strip pitch");
        TEST_BASICPROP(stripPitchPhi, "phi strip pitch");
        
        TEST_BASICPROP(stripWidthEta, "eta strip width");
        TEST_BASICPROP(stripWidthPhi, "phi strip width");

        TEST_BASICPROP(stripLengthEta, "eta strip length");
        TEST_BASICPROP(stripLengthPhi, "phi strip length");
        
        using RpcStrip = RpcChamber::RpcStrip;
            
        for (const RpcStrip& refStrip : reference.strips) {
            std::set<RpcStrip>::const_iterator strip_itr = test.strips.find(refStrip);
            if (strip_itr == test.strips.end()) {
                std::cerr<<refStrip<<" is not found in "<<test<<std::endl;
                chamberOkay = false;
            }
            const RpcStrip& testStrip{*strip_itr};
            if ( (testStrip.position - refStrip.position).mag() > tolerance) {
                std::cerr<<testStrip<<" should be located at "
                         <<Amg::toString(refStrip.position, 2)<<" in chamber "<<test<<std::endl;
                chamberOkay = false;
            }

        }
        if (!chamberOkay) {
            return_code = EXIT_FAILURE;
        }
    }
    return return_code;

}


