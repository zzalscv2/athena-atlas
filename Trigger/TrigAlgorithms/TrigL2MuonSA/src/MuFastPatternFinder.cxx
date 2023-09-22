/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuFastPatternFinder.h"
#include "xAODTrigMuon/TrigMuonDefs.h"
#include "MdtRegionDefiner.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigL2MuonSA::MuFastPatternFinder::MuFastPatternFinder(const std::string& type,
						     const std::string& name,
						     const IInterface*  parent):
   AthAlgTool(type,name,parent)
{
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigL2MuonSA::MuFastPatternFinder::initialize()
{
   ATH_CHECK( m_nswPatternFinder.retrieve() );
   ATH_CHECK( m_idHelperSvc.retrieve() );
   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void TrigL2MuonSA::MuFastPatternFinder::doMdtCalibration(TrigL2MuonSA::MdtHitData& mdtHit, double track_phi, double phi0, bool isEndcap) const
{
   int StationName  = mdtHit.name;
   int StationEta   = mdtHit.StationEta;
   int StationPhi   = mdtHit.StationPhi;
   int Multilayer   = mdtHit.Multilayer;
   int Layer        = mdtHit.TubeLayer;
   int Tube         = mdtHit.Tube;

   ATH_MSG_DEBUG("...StationName/StationEta/StationPhi/Multilayer/Layer/Tube="
		 << StationName << "/" << StationEta << "/" << StationPhi << "/" << Multilayer << "/"
		 << Layer << "/" << Tube);

   Identifier id = ( mdtHit.Id.is_valid() ) ? mdtHit.Id : m_idHelperSvc->mdtIdHelper().channelID(StationName,StationEta,
       StationPhi,Multilayer,Layer,Tube);

   int tdcCounts    = (int)mdtHit.DriftTime;
   int adcCounts    = mdtHit.Adc;

   double R    = mdtHit.R;
   //   double InCo = mdtHit.cInCo;
   const double cosDphi = std::cos(std::abs(track_phi - phi0)); 
   double InCo = cosDphi ? 1./ cosDphi: 0;
   double X    = (isEndcap)? R*std::cos(track_phi): R*InCo*std::cos(track_phi);
   double Y    = (isEndcap)? R*std::sin(track_phi): R*InCo*std::sin(track_phi);
   double Z    = mdtHit.Z;
   const Amg::Vector3D point(X,Y,Z);
   
   MdtCalibInput calHit{id, adcCounts,tdcCounts, point};
   ATH_MSG_DEBUG("... MDT hit raw digit tdcCounts/adcCounts=" << tdcCounts << "/" << adcCounts);

   ATH_MSG_DEBUG("... MDT hit position X/Y/Z/track_phi/Multilayer/Layer/Tube="
		 << Amg::toString(point, 2) << "/" << track_phi << "/" << Multilayer << "/" << Layer << "/" << Tube);

   MdtCalibOutput calibOut = m_mdtCalibrationTool->calibrate(Gaudi::Hive::currentContext(), calHit,  false);
   double driftSpace = calibOut.driftRadius();
   double driftSigma = calibOut.driftRadiusUncert();

   ATH_MSG_DEBUG("... MDT hit calibrated driftSpace/driftSigma=" << driftSpace << "/" << driftSigma);

   const double ZERO_LIMIT = 1e-4;

   if( std::abs(driftSpace) > ZERO_LIMIT ) {
      mdtHit.DriftSpace = driftSpace;
      mdtHit.DriftSigma = driftSigma;
   }
   else {
      mdtHit.DriftSpace = 0;
      mdtHit.DriftSigma = 0;
   }

}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------


StatusCode TrigL2MuonSA::MuFastPatternFinder::findPatterns(const TrigL2MuonSA::MuonRoad&            muonRoad,
							   TrigL2MuonSA::MdtHits&                   mdtHits,
							   TrigL2MuonSA::StgcHits&                  stgcHits,
							   TrigL2MuonSA::MmHits&                    mmHits,
							   std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns) const
{
  ATH_CHECK( findPatterns(muonRoad, mdtHits, v_trackPatterns) );
  ATH_CHECK( m_nswPatternFinder->findPatterns(muonRoad, stgcHits, mmHits, v_trackPatterns.back()) );

  for(unsigned int i_hit=0; i_hit<v_trackPatterns.back().stgcSegment.size(); i_hit++) {
    ATH_MSG_DEBUG("PatternFinder: output sTGC hits global eta/phi/r/z/layer/channelType/isOutlier " << v_trackPatterns.back().stgcSegment[i_hit].eta << "/" << v_trackPatterns.back().stgcSegment[i_hit].phi << "/" << v_trackPatterns.back().stgcSegment[i_hit].r << "/" << v_trackPatterns.back().stgcSegment[i_hit].z << "/" << v_trackPatterns.back().stgcSegment[i_hit].layerNumber << "/" << v_trackPatterns.back().stgcSegment[i_hit].channelType << "/" << v_trackPatterns.back().stgcSegment[i_hit].isOutlier);
  }

  for(unsigned int i_hit=0; i_hit<v_trackPatterns.back().mmSegment.size(); i_hit++) {
    ATH_MSG_DEBUG("PatternFinder: output MM hits global eta/phi/r/z/layer/isOutlier " << v_trackPatterns.back().mmSegment[i_hit].eta << "/" << v_trackPatterns.back().mmSegment[i_hit].phi << "/" << v_trackPatterns.back().mmSegment[i_hit].r << "/" << v_trackPatterns.back().mmSegment[i_hit].z << "/" << v_trackPatterns.back().mmSegment[i_hit].layerNumber << "/" << v_trackPatterns.back().mmSegment[i_hit].isOutlier);
  }

  return StatusCode::SUCCESS;

}


StatusCode TrigL2MuonSA::MuFastPatternFinder::findPatterns(const TrigL2MuonSA::MuonRoad&            muonRoad,
							   TrigL2MuonSA::MdtHits&                   mdtHits,
							   std::vector<TrigL2MuonSA::TrackPattern>& v_trackPatterns) const
{

   // find only 1 track pattern
   v_trackPatterns.clear();
   TrigL2MuonSA::TrackPattern trackPattern;
   for (int i=0; i<xAOD::L2MuonParameters::Chamber::MaxChamber; i++) trackPattern.mdtSegments[i].clear();

   // Saddress = pos1/3 = 0:Large, 1:Large-SP, 2:Small, 3:Small-SP
   trackPattern.s_address = (muonRoad.isEndcap)? -1: muonRoad.Special + 2*muonRoad.LargeSmall;

   const unsigned int MAX_STATION =  11;
   const unsigned int MAX_LAYER   =  12;

   TrigL2MuonSA::MdtLayerHits v_mdtLayerHits[MAX_STATION][MAX_LAYER];

   for(unsigned int i_station=0; i_station<MAX_STATION; i_station++) {
      for(unsigned int i_layer=0; i_layer<MAX_LAYER; i_layer++) {
	 v_mdtLayerHits[i_station][i_layer].ntot       = 0;
	 v_mdtLayerHits[i_station][i_layer].ntot_all   = 0;
	 v_mdtLayerHits[i_station][i_layer].ndigi      = 0;
	 v_mdtLayerHits[i_station][i_layer].ndigi_all  = 0;
	 v_mdtLayerHits[i_station][i_layer].ResSum     = 0.;
      }
   }

   unsigned int i_layer_max   = 0;
   unsigned int chamber_max = 0;

   double trigger_phi = muonRoad.phiMiddle; // phi fit result to be used
   double phi0; // phi at the first layer to be used

   for(unsigned int i_hit=0; i_hit<mdtHits.size(); i_hit++) {

     unsigned int chamber = mdtHits[i_hit].Chamber;

     if( chamber > chamber_max ) chamber_max = chamber;
     if (chamber >= xAOD::L2MuonParameters::Chamber::MaxChamber) continue;

     double aw = muonRoad.aw[chamber][0];
     double bw = muonRoad.bw[chamber][0];
     double Z = mdtHits[i_hit].Z;
     double R = mdtHits[i_hit].R;
     //
     double residual = calc_residual(aw,bw,Z,R);
     mdtHits[i_hit].R        = R;
     mdtHits[i_hit].Residual = residual;
     mdtHits[i_hit].isOutlier = 0;
     unsigned int i_layer = mdtHits[i_hit].Layer;
     double rWidth = muonRoad.rWidth[chamber][i_layer];

     ATH_MSG_DEBUG("... chamber/Z/R/aw/bw/residual/rWidth="
		   << chamber << "/" << Z << "/" << R << "/" << aw << "/" << bw << "/" << residual << "/" << rWidth);

     int stationPhi = mdtHits[i_hit].StationPhi;
     std::string name = m_idHelperSvc->mdtIdHelper().stationNameString(mdtHits[i_hit].name);
     int chamber_this = 99;
     int sector_this = 99;
     bool isEndcap;
     MdtRegionDefiner::find_station_sector(name, stationPhi, isEndcap, chamber_this, sector_this);
     if( !isEndcap && sector_this != muonRoad.MDT_sector_trigger ) {
       mdtHits[i_hit].isOutlier   = 3;
       continue;
     }

     if( std::abs(residual) > rWidth ) {
       mdtHits[i_hit].isOutlier   = 2;
       continue;
     }

     ATH_MSG_DEBUG(" --> included at i_layer=" << i_layer);
     if( mdtHits[i_hit].TubeLayer < 1 || i_layer > (MAX_LAYER-1) ) {
       ATH_MSG_WARNING("strange i_layer=" << i_layer);
       return StatusCode::FAILURE;
     }
     if( i_layer > i_layer_max ) i_layer_max = i_layer;
     //      v_mdtLayerHits[i_station][i_layer].indexes[v_mdtLayerHits[i_station][i_layer].ndigi] = i_hit;
     v_mdtLayerHits[chamber][i_layer].indexes.push_back(i_hit);
     v_mdtLayerHits[chamber][i_layer].ndigi++;
     v_mdtLayerHits[chamber][i_layer].ndigi_all++;
     v_mdtLayerHits[chamber][0].ntot++;
     v_mdtLayerHits[chamber][0].ntot_all++;
     v_mdtLayerHits[chamber][0].ResSum += residual;
   }


   const double DeltaMin = 0.025;

   for(unsigned int chamber=0; chamber<=chamber_max; chamber++) {

     ATH_MSG_DEBUG(" --- chamber=" << chamber);

     double ResMed = 0;

     ATH_MSG_DEBUG("removing outliers...");

     // remove outlier
     while(1) {
       if (chamber==9) break;//BME skips this loop
       if (chamber==10) break;//BMG skips this loop
       unsigned int layer = 999999;
       double DistMax  = 0.;
       double Residual = 0.;
       unsigned int i_hit_max = 999999;
       ResMed = (v_mdtLayerHits[chamber][0].ntot!=0)?
	 v_mdtLayerHits[chamber][0].ResSum/v_mdtLayerHits[chamber][0].ntot : 0.;
       for(unsigned int i_layer=0; i_layer<=i_layer_max; i_layer++) {
	 for(unsigned int idigi=0; idigi<v_mdtLayerHits[chamber][i_layer].ndigi_all;idigi++) {
	
	   unsigned int i_hit = v_mdtLayerHits[chamber][i_layer].indexes[idigi];
	   if(mdtHits[i_hit].isOutlier > 0) continue;
	   double DistMed = std::abs(mdtHits[i_hit].Residual - ResMed);
	   if(DistMed>=DistMax) {
	     DistMax   = DistMed;
	     Residual  = mdtHits[i_hit].Residual;
	     layer     = i_layer;
	     i_hit_max = i_hit;
	   }
	 }
       }
       ATH_MSG_DEBUG("ResMed=" << ResMed << ": DistMax/layer/i_hit_max/ntot="
		     << DistMax << "/" << layer << "/" << i_hit_max << "/" << v_mdtLayerHits[chamber][0].ntot);
       // break conditions
       if(layer == 999999) break;
       if(v_mdtLayerHits[chamber][layer].ndigi==1) break;
       double Mednew = (v_mdtLayerHits[chamber][0].ResSum - Residual)/(v_mdtLayerHits[chamber][0].ntot - 1);
       double Delta = 2.*std::abs((ResMed - Mednew)/(ResMed + Mednew));
       ATH_MSG_DEBUG("Mednew/Delta/DeltaMin=" << Mednew << "/" << Delta << "/" << DeltaMin);
       if(Delta<=DeltaMin) break;

       // if not, delete the maxRes and continue;
       v_mdtLayerHits[chamber][0].ResSum = v_mdtLayerHits[chamber][0].ResSum - Residual;
       v_mdtLayerHits[chamber][0].ntot--;
       v_mdtLayerHits[chamber][layer].ndigi--;
       mdtHits[i_hit_max].isOutlier = 2;
     }

     ATH_MSG_DEBUG("choosing one at each layer...");

     // choose one at each layer, and record it in segment
      TrigL2MuonSA::MdtHits mdtSegment;
      mdtSegment.clear();

      phi0 = 0;
      for(unsigned int i_layer=0; i_layer<=i_layer_max; i_layer++) {
	
	ATH_MSG_DEBUG("i_layer=" << i_layer << ": ndigi=" << v_mdtLayerHits[chamber][i_layer].ndigi);
	
	// choose one at each layer
	while( v_mdtLayerHits[chamber][i_layer].ndigi>=2 ) {
	  double ResMax = 0.;
	  unsigned int i_hit_max = 999999;
	  for(unsigned int idigi=0;idigi<v_mdtLayerHits[chamber][i_layer].ndigi_all;idigi++) {
	    unsigned int i_hit = v_mdtLayerHits[chamber][i_layer].indexes[idigi];
	    if( mdtHits[i_hit].isOutlier > 0 ) continue;
	    if( std::abs(mdtHits[i_hit].Residual) >= ResMax ) {
	      ResMax = std::abs(mdtHits[i_hit].Residual);
	      i_hit_max = i_hit;
	    }
	  }
	  ATH_MSG_DEBUG("ResMax=" << ResMax << ": i_hit_max=" << i_hit_max);
	  if( i_hit_max == 999999 ) break;
	  v_mdtLayerHits[chamber][0].ResSum = v_mdtLayerHits[chamber][0].ResSum - ResMax;
	  v_mdtLayerHits[chamber][0].ntot--;
	  v_mdtLayerHits[chamber][i_layer].ndigi--;
	  mdtHits[i_hit_max].isOutlier = 1;
	}
	
	// record it into segement
	for(unsigned int i_digi=0; i_digi< v_mdtLayerHits[chamber][i_layer].ndigi_all; i_digi++) {
	  unsigned int i_hit = v_mdtLayerHits[chamber][i_layer].indexes[i_digi];
	  if (i_layer==0) phi0 = mdtHits[i_hit].cPhi0;
	  doMdtCalibration(mdtHits[i_hit], trigger_phi, phi0, muonRoad.isEndcap);
	  if (mdtHits[i_hit].isOutlier > 1) continue;
	  mdtSegment.push_back(mdtHits[i_hit]);
	}
      }

      //
      ATH_MSG_DEBUG("nr of hits in segment=" << mdtSegment.size());
      trackPattern.mdtSegments[chamber] = mdtSegment;

   } // end loop on stations.

   v_trackPatterns.push_back(trackPattern);

   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

double TrigL2MuonSA::MuFastPatternFinder::calc_residual(double aw,double bw,double x,double y) const
{
   const double ZERO_LIMIT = 1e-4;
   if( std::abs(aw) < ZERO_LIMIT ) return y-bw;
   double ia  = 1/aw;
   double iaq = ia*ia;
   double dz  = x - (y-bw)*ia;
   return dz/std::sqrt(1.+iaq);
}
