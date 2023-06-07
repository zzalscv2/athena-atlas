/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#include "G4DebuggingTools/G4DebuggingHelper.h"

#include "G4Electron.hh"
#include "G4Positron.hh"
#include "G4Gamma.hh"
#include "G4Neutron.hh"
#include "G4Proton.hh"
#include "G4PionPlus.hh"
#include "G4PionMinus.hh"
#include "G4PionZero.hh"
#include "G4Version.hh"
#include <string_view>

namespace G4DebuggingHelpers {
  bool G4StrContains(const std::string_view& s, const char* v)
  {
    return s.find(v) != std::string_view::npos;
  }
  
  const G4String ClassifyParticle( const G4ParticleDefinition* def ) {
    if (def == G4Electron::Electron())
      return "e-";
    else if (def == G4Positron::Positron())
      return "e+";
    else if (def == G4Gamma::Gamma())
      return "gamma";
    else if (def == G4Neutron::Neutron())
      return "neutron";
    else if (def == G4Proton::Proton())
      return "proton";
    else if (def == G4PionPlus::PionPlus() || def == G4PionMinus::PionMinus() || def == G4PionZero::PionZero())
      return "pion";
    return "other";
  }

  const G4String ClassifyMaterial( const G4String &nom ) {
    if (nom == "FCal1Absorber"
    ||  nom == "LiquidArgon"
    ||  nom == "Copper"
    ||  nom == "Lead"
    ||  nom == "Aluminum"
    ||  nom == "FCal23Absorber"
    ||  nom == "Iron"
    ||  nom == "Air"
    ||  nom == "myLead"
    ||  nom == "shieldIron"
    ||  nom == "FCal23Slugs"
    ||  nom == "Glue"
    ||  nom == "KaptonC"
    ||  nom == "Kapton"
    ||  nom == "ShieldSteel"
    ||  nom == "myIron"
    ||  nom == "ShieldBrass"
    ||  nom == "Straw"
    ||  nom == "XeCO2O2"
    ||  nom == "CO2"
    ||  nom == "Valmat"
    ||  nom == "BoratedPolyethelyne"
    ||  nom == "FoilRadiatorB"
    ||  nom == "G10"
    ||  nom == "FoilRadiatorAC"
    ||  nom == "PyrogelXT"
    ||  nom == "Vacuum")
      return nom;
    else if (nom.compare(0,12,"pix::IBL_Fwd")==0)
      return "IBL_Fwd";
    return "other";
  }

  const G4String ClassifyVolume( const G4String &nomstr ) {
    std::string_view nom(nomstr); //Avoid copying characters during comparison
    if ( nom.length() >= 17 && nom.substr(13, 4) == "EMEC" ) {
      return "EMEC";
    }
    else if ( nom.length() >= 16 && nom.substr(13, 3) == "EMB" ) {
      return "EMB";
    }
    else if ( nom.length() >= 25 && nom.substr(21, 4) == "Cryo" ) {
      return "Cryo";
    }
    else if ( nom.length() >= 26 && nom.substr(13, 13) == "FCAL::Module1" ) {
      return "FC1";
    }
    else if ( nom.length() >= 25 && nom.substr(13, 12) == "FCAL::Module" ) {
      return "FC23";
    }
    else if ( nom.length() >= 17 && nom.substr(13, 4) == "FCAL" ) {
      return "FCOther";
    }
    else if ( nom.length() >= 16 && nom.substr(13, 3) == "HEC" ) {
      return "HEC";
    }
    else if ( nom.length() >= 31 && nom.substr(21, 10) == "Presampler" ) {
      return "Presampler";
    }
    else if ( nom.length() >= 3 && nom.substr(0, 3) == "LAr" ) {
      return "LAr";
    }
    else if ( ( nom.substr(0, 4) == "MUON" )
         ||   ( nom.length() >= 4  && nom.substr(0, 4)  == "Muon" )
         ||   ( nom.length() >= 9  && nom.substr(0, 9)  == "DriftTube" )
         ||   ( nom.length() >= 12 && nom.substr(0, 12) == "SensitiveGas" )
#if G4VERSION_NUMBER < 1100
         ||     nomstr.contains("MDT")
         ||     nomstr.contains("station") ) {
#else
         ||     G4StrUtil::contains(nom, "MDT")
         ||     G4StrUtil::contains(nom, "station") ) {
#endif
      return "Muon";
    }
    else if ( nom.length() >= 8 && nom.substr(0, 8) == "ITkPixel" ) {
      return "ITkPixel";
    }
    else if ( nom.length() >= 8 && nom.substr(0, 8) == "ITkStrip" ) {
      return "ITkStrip";
    }
    else if ( ( nom.length() >= 5 && nom.substr(0, 5) == "Pixel" )
         ||     nom == "Outside Barrel Service") {
      return "Pixel";
    }
    else if ( nom.length() >= 3 && nom.substr(0, 3) == "SCT" ) {
      return "SCT";
    }
    else if ( ( nom.length() >= 3 && nom.substr(0, 3) == "TRT" )
         ||     nom == "GasMANeg" ) {
      return "TRT";
    }
    else if ( nom.length() >= 4 && nom.substr(0, 4) == "Tile" ) {
      return "Tile";
    }
    else if (nom.length() >= 7 && nom.substr(0, 7) == "Section" )
      return "Section";
    else if ( ( nom.length() >= 12 && nom.substr(0, 12) == "InDetServMat" )
         ||   ( nom.length() >= 4  && nom.substr(0, 4)  == "IDET" )
         ||   ( nom.length() >= 3  && nom.substr(0, 3)  == "ITK" )
         ||   ( nom.length() >= 8  && nom.substr(0, 8)  == "BeamPipe" )
         ||   ( nom.length() >= 3  &&
                ( nom.substr(0, 3) == "BLM" || nom.substr(0, 3) == "BCM" || nom.substr(0, 3) == "PLR" ) )
         ||   ( nom.length() >= 8  && nom.substr(0, 8)  == "BCMPrime") ) {
      return "Service";
    }
    return "other";
  }

} // end namespace G4DebuggingHelpers
