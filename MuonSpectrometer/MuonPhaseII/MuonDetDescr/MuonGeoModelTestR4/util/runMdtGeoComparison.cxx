
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @brief Helper macro to compare the output from the readout geometry dumps:
 *          python -m MuonGeoModelTest.runGeoModelTest
 *          python -m MuonGeoModelTestR4.runGeoModelTest 
 *        for the Mdt subdetectors
 *  
*/
#include <GeoPrimitives/GeoPrimitives.h>
#include <GeoPrimitives/GeoPrimitivesHelpers.h>
#include <GeoPrimitives/GeoPrimitivesToStringConverter.h>
#include <MuonCablingData/MdtCablingData.h>
#include <MuonReadoutGeometryR4/StringUtils.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <string>
#include <set>
#include <vector>
#include <map>
#include <iostream>


#include <PathResolver/PathResolver.h>
#include <TFile.h>
#include <TTreeReader.h>



/// Helper struct to represent a full Mdt chamber
struct MdtChamber{
    /// Default constructor
    MdtChamber() = default;
    
    /// Identifier of the mdt chamber
    using  chamberIdentifier = MdtCablingOffData;
    chamberIdentifier id{};

    /// Sorting operator to insert the object into std::set
    bool operator<(const MdtChamber& other) const {
        return id < other.id;
    }

    /// Transformation of the underlying GeoModel element
    Amg::Transform3D geoModelTransform{Amg::Transform3D::Identity()};
    /// Number of tube layers
    unsigned int numLayers{0};
    /// Number of tubes
    unsigned int numTubes{0};
    /// Pitch between two tubes
    double tubePitch{0.};
    /// Inner  tube radius
    double tubeRadius{0.};
    
    struct TubePositioning{
        /// Layer to which the tube belongs
        unsigned int layerNum{0};
        /// Number of the tube
        unsigned int tubeNum{0};
        /// local -> global transformation of the tube
        Amg::Transform3D localToGlobal{Amg::Transform3D::Identity()};
        /// Position of the readout frame in global coordinates
        Amg::Vector3D readoutPos{Amg::Vector3D::Zero()};
        /// Tube length
        double tubeLength{0.};
        /// wire length
        double wireLength{0.};
        /// actiive tube length
        double activeLength{0.};
    };
    std::vector<TubePositioning> tubeInfo{};
    /// Returns the access to the full tube information 
    ///  Layer : (1 -nLayers), Tube : (1-nTubes)
    const TubePositioning& getTube (unsigned int layer, unsigned  int tube) const{
        static const TubePositioning dummy{};
        unsigned int idx = (layer -1)*numTubes + (tube -1);
        return idx < tubeInfo.size() ? tubeInfo[idx] :  dummy;
    }
    void insertTube(TubePositioning&& newTube) {
        unsigned int idx = (newTube.layerNum - 1)*numTubes + (newTube.tubeNum -1);
        if (tubeInfo.size() <= idx) tubeInfo.resize(idx+1);
        tubeInfo[idx] = std::move(newTube);
    }
};


/// Translation of the station Index -> station Name. Dictionary taken from
/// https://gitlab.cern.ch/atlas/athena/-/blob/main/DetectorDescription/IdDictParser/data/IdDictMuonSpectrometer_R.09.03.xml
std::ostream& operator<<(std::ostream& ostr, const MdtChamber& chamb) {
    static const std::map<int, std::string> stationDict{
        {0, "BIL"}, {1, "BIS"}, {7, "BIR"},
        {2, "BML"}, {3, "BMS"}, {8, "BMF"}, {53, "BME"}, {54, "BMG"}, {52, "BIM"},
        {4, "BOL"}, {5, "BOS"}, {9, "BOF"}, {10, "BOG"},
        {6, "BEE"}, {14, "EEL"}, {15, "EES"}, 
        {13, "EIL"}, 
        {17, "EML"}, {18, "EMS"}, 
        {20, "EOL"}, {21, "EOS"}
    };
    ostr<<"Mdt chamber "<<stationDict.at(chamb.id.stationIndex)<<" "<<chamb.id;    
    return ostr;
} 





std::set<MdtChamber> readTreeDump(const std::string& inputFile) {
    std::set<MdtChamber> to_ret{};
    std::cout<<"Read the Mdt geometry tree dump from "<<inputFile<<std::endl;
    std::unique_ptr<TFile> inFile{TFile::Open(inputFile.c_str())};
    if (!inFile || !inFile->IsOpen()) {
        std::cerr<<__FILE__<<":"<<__LINE__<<" Failed to open "<<inputFile<<std::endl;
        return to_ret;
    }
    TTreeReader treeReader("MdtGeoModelTree", inFile.get());
    if (treeReader.IsInvalid()){
        std::cerr<<__FILE__<<":"<<__LINE__<<" The file "<<inputFile<<" does not contain the 'MdtGeoModelTree'"<<std::endl;
        return to_ret;
    }

    TTreeReaderValue<unsigned short> stationIndex{treeReader, "stationIndex"};
    TTreeReaderValue<short> stationEta{treeReader, "stationEta"};
    TTreeReaderValue<short> stationPhi{treeReader, "stationPhi"};
    TTreeReaderValue<short> stationML{treeReader, "stationMultiLayer"};
    
    TTreeReaderValue<double> tubeRadius{treeReader, "tubeRadius"};
    TTreeReaderValue<double> tubePitch{treeReader, "tubePitch"}; 
    TTreeReaderValue<unsigned short> numTubes{treeReader, "numTubes"};
    TTreeReaderValue<unsigned short> numLayers{treeReader, "numLayers"};
    
    TTreeReaderValue<std::vector<float>> geoModelTransformX{treeReader, "GeoModelTransformX"};
    TTreeReaderValue<std::vector<float>> geoModelTransformY{treeReader, "GeoModelTransformY"};
    TTreeReaderValue<std::vector<float>> geoModelTransformZ{treeReader, "GeoModelTransformZ"};

    /// Information to access each tube individually
    TTreeReaderValue<std::vector<unsigned short>> tubeLayer{treeReader, "tubeLayer"}; 
    TTreeReaderValue<std::vector<unsigned short>> tubeNumber{treeReader, "tubeNumber"};

    TTreeReaderValue<std::vector<double>> tubeLength{treeReader, "tubeLength"};
    TTreeReaderValue<std::vector<double>> activeTubeLength{treeReader, "activeTubeLength"};
    TTreeReaderValue<std::vector<double>> wireLength{treeReader, "wireLength"};

    TTreeReaderValue<std::vector<float>> tubeTransformTransX{treeReader, "tubeTransformTranslationX"};
    TTreeReaderValue<std::vector<float>> tubeTransformTransY{treeReader, "tubeTransformTranslationY"};
    TTreeReaderValue<std::vector<float>> tubeTransformTransZ{treeReader, "tubeTransformTranslationZ"};
    
    TTreeReaderValue<std::vector<float>> tubeTransformCol0X{treeReader, "tubeTransformCol0X"};
    TTreeReaderValue<std::vector<float>> tubeTransformCol0Y{treeReader, "tubeTransformCol0Y"};
    TTreeReaderValue<std::vector<float>> tubeTransformCol0Z{treeReader, "tubeTransformCol0Z"};
 
    TTreeReaderValue<std::vector<float>> tubeTransformCol1X{treeReader, "tubeTransformCol1X"};
    TTreeReaderValue<std::vector<float>> tubeTransformCol1Y{treeReader, "tubeTransformCol1Y"};
    TTreeReaderValue<std::vector<float>> tubeTransformCol1Z{treeReader, "tubeTransformCol1Z"};
 
    TTreeReaderValue<std::vector<float>> tubeTransformCol2X{treeReader, "tubeTransformCol2X"};
    TTreeReaderValue<std::vector<float>> tubeTransformCol2Y{treeReader, "tubeTransformCol2Y"};
    TTreeReaderValue<std::vector<float>> tubeTransformCol2Z{treeReader, "tubeTransformCol2Z"};
 
    TTreeReaderValue<std::vector<float>> readOutPosX{treeReader, "readOutPosX"};
    TTreeReaderValue<std::vector<float>> readOutPosY{treeReader, "readOutPosY"};
    TTreeReaderValue<std::vector<float>> readOutPosZ{treeReader, "readOutPosZ"};

    while (treeReader.Next()) {
        MdtChamber newchamber{};

        newchamber.id.stationIndex = (*stationIndex);
        newchamber.id.eta = (*stationEta);
        newchamber.id.phi = (*stationPhi);
        newchamber.id.multilayer = (*stationML);

        newchamber.tubeRadius = (*tubeRadius);
        newchamber.tubePitch = (*tubePitch);
        
        newchamber.numTubes = (*numTubes);
        newchamber.numLayers = (*numLayers);

        Amg::RotationMatrix3D geoRot{Amg::RotationMatrix3D::Identity()};
        geoRot.col(0) = Amg::Vector3D((*geoModelTransformX)[1], (*geoModelTransformY)[1], (*geoModelTransformZ)[1]);
        geoRot.col(1) = Amg::Vector3D((*geoModelTransformX)[2], (*geoModelTransformY)[2], (*geoModelTransformZ)[2]);
        geoRot.col(2) = Amg::Vector3D((*geoModelTransformX)[3], (*geoModelTransformY)[3], (*geoModelTransformZ)[3]);       
        Amg::Vector3D geoTrans{(*geoModelTransformX)[0], (*geoModelTransformY)[0], (*geoModelTransformZ)[0]};
        newchamber.geoModelTransform = Amg::getTransformFromRotTransl(std::move(geoRot), std::move(geoTrans));       
        /// Readout the information of each tube specifically
        for (size_t t = 0; t < tubeLayer->size(); ++t){
            using TubePositioning = MdtChamber::TubePositioning;
            TubePositioning newTube{};
            newTube.layerNum = (*tubeLayer)[t];
            newTube.tubeNum = (*tubeNumber)[t];
            newTube.tubeLength = (*tubeLength)[t];
            newTube.wireLength = (*wireLength)[t];
            Amg::RotationMatrix3D tubeRot{Amg::RotationMatrix3D::Identity()};
            tubeRot.col(0) = Amg::Vector3D((*tubeTransformCol0X)[t],(*tubeTransformCol0Y)[t], (*tubeTransformCol0Z)[t]);
            tubeRot.col(1) = Amg::Vector3D((*tubeTransformCol1X)[t],(*tubeTransformCol1Y)[t], (*tubeTransformCol1Z)[t]);
            tubeRot.col(2) = Amg::Vector3D((*tubeTransformCol2X)[t],(*tubeTransformCol2Y)[t], (*tubeTransformCol2Z)[t]);
            Amg::Vector3D tubeTrans{(*tubeTransformTransX)[t],(*tubeTransformTransY)[t], (*tubeTransformTransZ)[t]};
            newTube.localToGlobal = Amg::getTransformFromRotTransl(std::move(tubeRot), std::move(tubeTrans));
            newTube.readoutPos = Amg::Vector3D{(*readOutPosX)[t],(*readOutPosY)[t],(*readOutPosZ)[t]};
            newchamber.insertTube(std::move(newTube));
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
    std::set<MdtChamber> refChambers = readTreeDump(refFile);
    if (refChambers.empty()) {
        std::cerr<<"The file "<<refFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    std::set<MdtChamber> testChambers = readTreeDump(testFile);
    if (testChambers.empty()) {
        std::cerr<<"The file "<<testFile<<" should contain at least one chamber "<<std::endl;
        return EXIT_FAILURE;
    }
    int return_code = EXIT_SUCCESS;
    /// Start to loop over the chambers
    for (const MdtChamber& reference : refChambers) {
        std::set<MdtChamber>::const_iterator test_itr = testChambers.find(reference);
        
        if (test_itr == testChambers.end()) {
            std::cerr<<"The chamber "<<reference<<" is not part of the testing "<<std::endl;
            return_code = EXIT_FAILURE;
            continue;
        }
        bool chamberOkay = true;
        const MdtChamber& test = {*test_itr};
        /// 
        if (test.numLayers != reference.numLayers) {
            std::cerr<<"The number of layers differs between reference "
                     <<reference.numLayers<<" and test "<<test.numLayers
                     <<" for "<<reference<<std::endl;
            chamberOkay = false; 
        }
        if (test.numTubes != reference.numTubes) {
            std::cerr<<"The number of tubes differs between reference "
                     <<reference.numTubes<<" and test "<<test.numTubes
                     <<" for "<<reference<<std::endl;
            chamberOkay = false; 
        }
        if (std::abs(test.tubePitch - reference.tubePitch) > std::numeric_limits<float>::epsilon()) {
            std::cerr<<"The tube pitch differ between reference "<<reference.tubePitch
                     <<" and test "<<test.tubePitch<<" for "<<reference<<std::endl;
            chamberOkay = false;
        }
        const Amg::Transform3D distortion = test.geoModelTransform.inverse() * reference.geoModelTransform;
        /// We do not care whether the orientation of the coordinate system along the wire flips for negative
        /// chambers or not
        if (!doesNotDeform(distortion) && 
            !(reference.id.eta < 0 && doesNotDeform(distortion* Amg::getRotateX3D(M_PI)))) {   
            std::cerr<<"The chamber coordinate systems rotate differently for  "
                     <<reference<<". Difference in the coordinate transformation: "
                     <<MuonGMR4::to_string(distortion)<<std::endl;
            chamberOkay = false;            
        }
        constexpr double tolerance = 10 * Gaudi::Units::micrometer;
        /// The ultimate goal is to have the tube positioned at the same place. 
        /// We maybe need the origin position later when we are adding the alignable transforms...
        if (false && distortion.translation().mag() > tolerance) {
            std::cout<<"The origins of the chamber coordinate systems are not exactly at the same point for "
                     <<reference<<". Translation shift: "<<Amg::toString(distortion.translation(), 2)<<std::endl;
        }
        /// Check the tube transformations
        bool stagFailure{false}, alignFailure{false}, readoutOrient{false};            
        for (unsigned int layer = 1; layer<= std::min(reference.numLayers, test.numLayers) ; ++layer) {
            for (unsigned int tube = 1; tube <= std::min(reference.numTubes, test.numTubes); ++tube) {
                using TubePositioning = MdtChamber::TubePositioning;
                const TubePositioning& refTube = reference.getTube(layer, tube);
                const TubePositioning& testTube = test.getTube(layer, tube);
                const Amg::Transform3D tubeDistortion = testTube.localToGlobal.inverse() * refTube.localToGlobal;
                if (!alignFailure && !(doesNotDeform(tubeDistortion)  || 
                                       (reference.id.eta < 0 && doesNotDeform(tubeDistortion* Amg::getRotateX3D(M_PI))) )) {
                    std::cerr<<"In chamber "<<reference<<" the tube reference systems for ("<<layer<<", "<<tube
                             <<") are not exactly aligned. "<<MuonGMR4::to_string(tubeDistortion)<<std::endl;                   
                    alignFailure = true;
                }
                /// Remember the tube staggering is in the (x-y) plane. Allow for
                /// deviations in the z-axis due to different cutouts
                if (!stagFailure && tubeDistortion.translation().perp() > tolerance) {
                    std::cerr<<"Misplaced staggering found in chamber "<<reference<<" the tube ("<<layer<<", "<<tube<<") "
                             << Amg::toString(tubeDistortion.translation(), 2)<<std::endl;                    
                    stagFailure = true;
                }
                
                if (std::abs(refTube.tubeLength - testTube.tubeLength) >= tolerance) {
                    std::cerr<<"Different tube lengths found in chamber "
                             <<reference<<" for the tubes ("<<layer<<", "<<tube<<") "
                             <<refTube.tubeLength<<" vs. "<<testTube.tubeLength<<"."<<std::endl;
                    chamberOkay = false;
                }
                /// In cases where the tube coordinate systems are not aligned, 
                /// there's no point in checking the position of the reaodout
                if (alignFailure || readoutOrient) continue;
                const Amg::Transform3D refSystem = refTube.localToGlobal.inverse();

                const Amg::Vector3D refRO = refSystem * refTube.readoutPos;
                const Amg::Vector3D testRO = refSystem* testTube.readoutPos;
                if (refRO.z()* testRO.z() < 0.){
                    std::cerr<<"The readout is on different sites for chamber: "<<reference<<". "
                              <<Amg::toString(refRO, 2)<<" vs. "<<Amg::toString(testRO)<<std::endl;
                    readoutOrient = true;
                }

            }
            if (stagFailure || alignFailure) {
                chamberOkay = false;
            }
        }
        if (!chamberOkay) return_code = EXIT_FAILURE;
        else std::cout<<"Found perfect agreement between new & old geometry for "<<reference<<std::endl;       
    }
    return return_code;

}


