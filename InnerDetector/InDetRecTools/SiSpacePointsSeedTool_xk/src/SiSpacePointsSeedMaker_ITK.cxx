/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class SiSpacePointsSeedMaker_ITK
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used for TRT_DriftCircleOnTrack object production
///////////////////////////////////////////////////////////////////
// Version 1.0 21/04/2004 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#include "SiSpacePointsSeedTool_xk/SiSpacePointsSeedMaker_ITK.h"

#include "InDetPrepRawData/SiCluster.h"

#include <iomanip>
#include <ostream>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointsSeedMaker_ITK::SiSpacePointsSeedMaker_ITK
(const std::string& t, const std::string& n, const IInterface* p)
  : base_class(t, n, p)
{
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_ITK::initialize()
{
  StatusCode sc = AlgTool::initialize();

  ATH_CHECK(m_spacepointsPixel.initialize(m_pixel));
  ATH_CHECK(m_spacepointsSCT.initialize(m_sct));
  ATH_CHECK(m_spacepointsOverlap.initialize(m_useOverlap));

  // Get beam geometry
  //
  ATH_CHECK(m_beamSpotKey.initialize());

  // Get magnetic field service
  //
  if ( !m_fieldServiceHandle.retrieve() ){
    ATH_MSG_FATAL("Failed to retrieve " << m_fieldServiceHandle );
    return StatusCode::FAILURE;
  }    
  ATH_MSG_DEBUG("Retrieved " << m_fieldServiceHandle );

  // PRD-to-track association (optional)
  ATH_CHECK( m_prdToTrackMap.initialize( !m_prdToTrackMap.key().empty()));

  if (m_r_rmax < 1100.) m_r_rmax = 1100.;
  
  // Build framework
  //
  buildFrameWork();

  // Get output print level
  //
  m_outputlevel = msg().level()-MSG::DEBUG;
  if (m_outputlevel<=0) {
    EventData data;
    initializeEventData(data);
    data.nprint=0;
    dump(data, msg(MSG::DEBUG));
  }
  m_umax = 100.-fabs(m_umax)*300.;

  m_initialized = true;

  return sc;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_ITK::finalize()
{
  return AlgTool::finalize();
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new event 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newEvent(EventData& data, int iteration) const
{
  if (not data.initialized) initializeEventData(data);

  data.iteration0 = iteration;
  data.trigger = false;
  if (!m_pixel && !m_sct) return;

  iteration <=0 ? data.iteration = 0 : data.iteration = iteration;
  erase(data);
  data.dzdrmin = m_dzdrmin0;
  data.dzdrmax = m_dzdrmax0;

  if (!data.iteration) {
    buildBeamFrameWork(data);

    double f[3], gP[3] ={10.,10.,0.};
    if (m_fieldServiceHandle->solenoidOn()) {
      m_fieldServiceHandle->getFieldZR(gP, f);
      data.K = 2./(300.*f[2]);
    } else {
      data.K = 2./(300.* 5. );
    }

    data.ipt2K = m_ipt2/(data.K*data.K);
    data.ipt2C = m_ipt2*m_COF;
    data.COFK  = m_COF*(data.K*data.K);
    data.i_spforseed_ITK = data.l_spforseed_ITK.begin();
  } else {
    data.r_first = 0;
    fillLists(data);
    return;
  }

  data.checketa = data.dzdrmin > 1.;

  float irstep = 1./m_r_rstep;
  int   irmax  = m_r_size-1;
  for (int i=0; i<data.nr; ++i) {
    int n = data.r_index[i];
    data.r_map[n] = 0;
    data.r_Sorted_ITK[n].clear();
  }
  data.ns = data.nr = 0;

  SG::ReadHandle<Trk::PRDtoTrackMap>  prd_to_track_map;
  if (!m_prdToTrackMap.key().empty()) {
    prd_to_track_map=SG::ReadHandle<Trk::PRDtoTrackMap>(m_prdToTrackMap);
    if (!prd_to_track_map.isValid()) {
      ATH_MSG_ERROR("Failed to read PRD to track association map.");
    }
  }

  // Get pixels space points containers from store gate 
  //
  data.r_first = 0;
  if (m_pixel) {

    SG::ReadHandle<SpacePointContainer> spacepointsPixel{m_spacepointsPixel};
    if (spacepointsPixel.isValid()) {

      for (const SpacePointCollection* spc: *spacepointsPixel) {
        for (const Trk::SpacePoint* sp: *spc) {

	  if ((prd_to_track_map.cptr() && isUsed(sp,*prd_to_track_map)) || sp->r() > m_r_rmax || sp->r() < m_r_rmin ) continue;

	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(data, sp);
          if (!sps) continue;

	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  data.r_Sorted_ITK[ir].push_back(sps);
          ++data.r_map[ir];
	  if (data.r_map[ir]==1) data.r_index[data.nr++] = ir;
	  if (ir > data.r_first) data.r_first = ir;
	  ++data.ns;
	}
      }
    }
    ++data.r_first;
  }

  // Get sct space points containers from store gate
  //
  if (m_sct) {

    SG::ReadHandle<SpacePointContainer> spacepointsSCT{m_spacepointsSCT};
    if (spacepointsSCT.isValid()) {

      for (const SpacePointCollection* spc: *spacepointsSCT) {
        for (const Trk::SpacePoint* sp: *spc) {

	  if ((prd_to_track_map.cptr() && isUsed(sp,*prd_to_track_map)) || sp->r() > m_r_rmax || sp->r() < m_r_rmin ) continue;

	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(data, sp);
          if (!sps) continue;

	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  data.r_Sorted_ITK[ir].push_back(sps);
          ++data.r_map[ir];
	  if (data.r_map[ir]==1) data.r_index[data.nr++] = ir;
	  ++data.ns;
	}
      }
    }

    // Get sct overlap space points containers from store gate 
    //
    if (m_useOverlap) {

      SG::ReadHandle<SpacePointOverlapCollection> spacepointsOverlap{m_spacepointsOverlap};
      if (spacepointsOverlap.isValid()) {
	
        for (const Trk::SpacePoint* sp: *spacepointsOverlap) {

	  if ((prd_to_track_map.cptr() && isUsed(sp,*prd_to_track_map)) || sp->r() > m_r_rmax || sp->r() < m_r_rmin) continue;

	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(data, sp);
          if (!sps) continue;

	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  data.r_Sorted_ITK[ir].push_back(sps);
          ++data.r_map[ir];
	  if (data.r_map[ir]==1) data.r_index[data.nr++] = ir;
	  ++data.ns;
	}
      }
    }
  }

  if (iteration < 0) data.r_first = 0;
  fillLists(data);
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new region
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newRegion
(EventData& data,
 const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT) const
{
  if (not data.initialized) initializeEventData(data);

  data.iteration = 0;
  data.trigger = false;
  erase(data);
  if (!m_pixel && !m_sct) return;

  data.dzdrmin = m_dzdrmin0;
  data.dzdrmax = m_dzdrmax0;

  buildBeamFrameWork(data);

  double f[3], gP[3] ={10.,10.,0.};

  if (m_fieldServiceHandle->solenoidOn()) {
    m_fieldServiceHandle->getFieldZR(gP, f);
    data.K = 2./(300.*f[2]);
  } else {
    data.K = 2./(300.* 5. );
  }

  data.ipt2K = m_ipt2/(data.K*data.K);
  data.ipt2C = m_ipt2*m_COF;
  data.COFK = m_COF*(data.K*data.K);

  data.i_spforseed_ITK = data.l_spforseed_ITK.begin();

  float irstep = 1./m_r_rstep;
  int   irmax  = m_r_size-1;

  data.r_first = 0;
  data.checketa = false;

  for (int i=0; i<data.nr; ++i) {
    int n = data.r_index[i];
    data.r_map[n] = 0;
    data.r_Sorted_ITK[n].clear();
  }
  data.ns = data.nr = 0;

  // Get pixels space points containers from store gate 
  //
  if (m_pixel && vPixel.size()) {

    SG::ReadHandle<SpacePointContainer> spacepointsPixel{m_spacepointsPixel};
    if (spacepointsPixel.isValid()) {
      SpacePointContainer::const_iterator spce = spacepointsPixel->end();

      // Loop through all trigger collections
      //
      for (const IdentifierHash& l: vPixel) {
	SpacePointContainer::const_iterator w = spacepointsPixel->indexFind(l);
	if (w==spce) continue;
        for (const Trk::SpacePoint* sp: **w) {
	  float r = sp->r();
          if (r > m_r_rmax || r < m_r_rmin) continue;
	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(data, sp);
	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  data.r_Sorted_ITK[ir].push_back(sps);
          ++data.r_map[ir];
	  if (data.r_map[ir]==1) data.r_index[data.nr++] = ir;
	  ++data.ns;
	}
      }
    }
  }

  // Get sct space points containers from store gate 
  //
  if (m_sct && vSCT.size()) {

    SG::ReadHandle<SpacePointContainer> spacepointsSCT{m_spacepointsSCT};
    if (spacepointsSCT.isValid()) {
      SpacePointContainer::const_iterator spce = spacepointsSCT->end();

      // Loop through all trigger collections
      //
      for (const IdentifierHash& l: vSCT) {
	SpacePointContainer::const_iterator w = spacepointsSCT->indexFind(l);
	if (w==spce) continue;
        for (const Trk::SpacePoint* sp: **w) {
	  float r = sp->r();
          if (r > m_r_rmax || r < m_r_rmin) continue;
	  InDet::SiSpacePointForSeedITK* sps = newSpacePoint(data, sp);
	  int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
	  data.r_Sorted_ITK[ir].push_back(sps);
          ++data.r_map[ir];
	  if (data.r_map[ir]==1) data.r_index[data.nr++] = ir;
	  ++data.ns;
	}
      }
    }
  }
  fillLists(data);
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new region
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newRegion
(EventData& data,
 const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT, const IRoiDescriptor& IRD) const
{
  if (not data.initialized) initializeEventData(data);

  newRegion(data, vPixel, vSCT);
  data.trigger = true;

  double dzdrmin = 1./tan(2.*atan(exp(-IRD.etaMinus())));
  double dzdrmax = 1./tan(2.*atan(exp(-IRD.etaPlus ())));
 
  data.zminB        = IRD.zedMinus()-data.zbeam[0]; // min bottom Z
  data.zmaxB        = IRD.zedPlus ()-data.zbeam[0]; // max bottom Z
  data.zminU        = data.zminB+550.*dzdrmin;
  data.zmaxU        = data.zmaxB+550.*dzdrmax;
  double fmax    = IRD.phiPlus ();
  double fmin    = IRD.phiMinus();
  if (fmin > fmax) fmin-=(2.*M_PI);
  data.ftrig        = (fmin+fmax)*.5;
  data.ftrigW       = (fmax-fmin)*.5;
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with two space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::find2Sp(EventData& data, const std::list<Trk::Vertex>& lv) const
{
  if (not data.initialized) initializeEventData(data);

  data.zminU = m_zmin;
  data.zmaxU = m_zmax;

  int mode = 0;
  if (lv.begin()!=lv.end()) mode = 1;
  bool newv = newVertices(data, lv);
  
  if (newv || !data.state || data.nspoint!=2 || data.mode!=mode || data.nlist) {

    data.i_seede_ITK = data.l_seeds_ITK.begin();
    data.state   = 1;
    data.nspoint = 2;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fvNmin  = 0;
    data.fNmin   = 0;
    data.zMin    = 0;
    production2Sp(data);
  }
  data.i_seed_ITK = data.l_seeds_ITK.begin();
  
  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with three space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::find3Sp(EventData& data, const std::list<Trk::Vertex>& lv) const
{
  if (not data.initialized) initializeEventData(data);

  data.zminU = m_zmin;
  data.zmaxU = m_zmax;

  int mode = 2;
  if (lv.begin()!=lv.end()) mode = 3;
  bool newv = newVertices(data, lv);

  if (newv || !data.state || data.nspoint!=3 || data.mode!=mode || data.nlist) {
    data.i_seede_ITK = data.l_seeds_ITK.begin();
    data.state   = 1;
    data.nspoint = 3;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fvNmin  = 0;
    data.fNmin   = 0;
    data.zMin    = 0;
    production3Sp(data);
  }
  data.i_seed_ITK = data.l_seeds_ITK.begin();
  data.seed_ITK = data.seeds_ITK.begin();

  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with three space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::find3Sp(EventData& data, const std::list<Trk::Vertex>& lv, const double* ZVertex) const
{
  if (not data.initialized) initializeEventData(data);

  data.zminU = ZVertex[0];
  if (data.zminU < m_zmin) data.zminU = m_zmin;
  data.zmaxU = ZVertex[1];
  if (data.zmaxU > m_zmax) data.zmaxU = m_zmax;

  int mode = 2;
  if (lv.begin()!=lv.end()) mode = 3;
  bool newv = newVertices(data, lv);

  if (newv || !data.state || data.nspoint!=3 || data.mode!=mode || data.nlist) {
    data.i_seede_ITK = data.l_seeds_ITK.begin();
    data.state   = 1;
    data.nspoint = 3;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fvNmin  = 0;
    data.fNmin   = 0;
    data.zMin    = 0;
    production3Sp(data);
  }
  data.i_seed_ITK = data.l_seeds_ITK.begin();
  data.seed_ITK = data.seeds_ITK.begin();

  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with variable number space points with or without vertex constraint
// Variable means (2,3,4,....) any number space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::findVSp(EventData& data, const std::list<Trk::Vertex>& lv) const
{
  if (not data.initialized) initializeEventData(data);

  data.zminU = m_zmin;
  data.zmaxU = m_zmax;

  int mode = 5;
  if (lv.begin()!=lv.end()) mode = 6;
  bool newv = newVertices(data, lv);
  
  if (newv || !data.state || data.nspoint!=4 || data.mode!=mode || data.nlist) {

    data.i_seede_ITK = data.l_seeds_ITK.begin();
    data.state   = 1;
    data.nspoint = 4;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fvNmin  = 0;
    data.fNmin   = 0;
    data.zMin    = 0;
    production3Sp(data);
  }
  data.i_seed_ITK = data.l_seeds_ITK.begin();
  data.seed_ITK = data.seeds_ITK.begin();

  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_ITK::dump(EventData& data, MsgStream& out) const
{
  if (not data.initialized) initializeEventData(data);

  if (data.nprint) return dumpEvent(data, out);
  return dumpConditions(data, out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_ITK::dumpConditions(EventData& data, MsgStream& out) const
{
  int n = 42-m_spacepointsPixel.key().size();
  std::string s2; for (int i=0; i<n; ++i) s2.append(" "); s2.append("|");
  n     = 42-m_spacepointsSCT.key().size();
  std::string s3; for (int i=0; i<n; ++i) s3.append(" "); s3.append("|");
  n     = 42-m_spacepointsOverlap.key().size();
  std::string s4; for (int i=0; i<n; ++i) s4.append(" "); s4.append("|");
  n     = 42-m_beamSpotKey.key().size();
  std::string s5; for (int i=0; i<n; ++i) s5.append(" "); s5.append("|");


  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| Pixel    space points   | "<<m_spacepointsPixel.key() <<s2
     <<endmsg;
  out<<"| SCT      space points   | "<<m_spacepointsSCT.key()<<s3
     <<endmsg;
  out<<"| Overlap  space points   | "<<m_spacepointsOverlap.key()<<s4
     <<endmsg;
  out<<"| BeamConditionsService   | "<<m_beamSpotKey.key()<<s5
     <<endmsg;
  out<<"| usePixel                | "
     <<std::setw(12)<<m_pixel 
     <<"                              |"<<endmsg;
  out<<"| useSCT                  | "
     <<std::setw(12)<<m_sct 
     <<"                              |"<<endmsg;
  out<<"| maxSize                 | "
     <<std::setw(12)<<m_maxsize 
     <<"                              |"<<endmsg;
  out<<"| maxSizeSP               | "
     <<std::setw(12)<<m_maxsizeSP
     <<"                              |"<<endmsg;
  out<<"| pTmin  (mev)            | "
     <<std::setw(12)<<std::setprecision(5)<<m_ptmin
     <<"                              |"<<endmsg;
  out<<"| |rapidity|          <=  | " 
     <<std::setw(12)<<std::setprecision(5)<<m_rapcut
     <<"                              |"<<endmsg;
  out<<"| max radius SP           | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rmax 
     <<"                              |"<<endmsg;
  out<<"| min radius SP           | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rmin 
     <<"                              |"<<endmsg;
  out<<"| radius step             | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rstep
     <<"                              |"<<endmsg;
  out<<"| min Z-vertex position   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zmin
     <<"                              |"<<endmsg;
  out<<"| max Z-vertex position   | "
     <<std::setw(12)<<std::setprecision(5)<<m_zmax
     <<"                              |"<<endmsg;
  out<<"| min radius first  SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1min
     <<"                              |"<<endmsg;
  out<<"| min radius second SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2min
     <<"                              |"<<endmsg;
  out<<"| min radius last   SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r3min
     <<"                              |"<<endmsg;
  out<<"| max radius first  SP(3) | "
     <<std::setw(12)<<std::setprecision(4)<<m_r1max
     <<"                              |"<<endmsg;
  out<<"| max radius second SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2max
     <<"                              |"<<endmsg;
  out<<"| max radius last   SP(3) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r3max
     <<"                              |"<<endmsg;
  out<<"| min radius first  SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1minv
     <<"                              |"<<endmsg;
  out<<"| min radius second SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2minv
     <<"                              |"<<endmsg;
  out<<"| max radius first  SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r1maxv
     <<"                              |"<<endmsg;
  out<<"| max radius second SP(2) | "
     <<std::setw(12)<<std::setprecision(5)<<m_r2maxv
     <<"                              |"<<endmsg;
  out<<"| min space points dR     | "
     <<std::setw(12)<<std::setprecision(5)<<m_drmin
     <<"                              |"<<endmsg;
  out<<"| max space points dR     | "
     <<std::setw(12)<<std::setprecision(5)<<m_drmax
     <<"                              |"<<endmsg;
  out<<"| max dZ    impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_dzver 
     <<"                              |"<<endmsg;
  out<<"| max dZ/dR impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_dzdrver 
     <<"                              |"<<endmsg;
  out<<"| max       impact        | "
     <<std::setw(12)<<std::setprecision(5)<<m_diver
     <<"                              |"<<endmsg;
  out<<"| max       impact pps    | "
     <<std::setw(12)<<std::setprecision(5)<<m_diverpps
     <<"                              |"<<endmsg;
  out<<"| max       impact sss    | "
     <<std::setw(12)<<std::setprecision(5)<<m_diversss
     <<"                              |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| Beam X center           | "
     <<std::setw(12)<<std::setprecision(5)<<data.xbeam[0]
     <<"                              |"<<endmsg;
  out<<"| Beam Y center           | "
     <<std::setw(12)<<std::setprecision(5)<<data.ybeam[0]
     <<"                              |"<<endmsg;
  out<<"| Beam Z center           | "
     <<std::setw(12)<<std::setprecision(5)<<data.zbeam[0]
     <<"                              |"<<endmsg;
  out<<"| Beam X-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<data.xbeam[1]
     <<std::setw(12)<<std::setprecision(5)<<data.xbeam[2]
     <<std::setw(12)<<std::setprecision(5)<<data.xbeam[3]
     <<"      |"<<endmsg;
  out<<"| Beam Y-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<data.ybeam[1]
     <<std::setw(12)<<std::setprecision(5)<<data.ybeam[2]
     <<std::setw(12)<<std::setprecision(5)<<data.ybeam[3]
     <<"      |"<<endmsg;
  out<<"| Beam Z-axis direction   | "
     <<std::setw(12)<<std::setprecision(5)<<data.zbeam[1]
     <<std::setw(12)<<std::setprecision(5)<<data.zbeam[2]
     <<std::setw(12)<<std::setprecision(5)<<data.zbeam[3]
     <<"      |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  return out;
}

///////////////////////////////////////////////////////////////////
// Dumps event information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_ITK::dumpEvent(EventData& data, MsgStream& out) const
{
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| data.ns                    | "
     <<std::setw(12)<<data.ns
     <<"                              |"<<endmsg;
  out<<"| data.nsaz                  | "
     <<std::setw(12)<<data.nsaz
     <<"                              |"<<endmsg;
  out<<"| data.nsazv                 | "
     <<std::setw(12)<<data.nsazv
     <<"                              |"<<endmsg;
  out<<"| seeds                   | "
     <<std::setw(12)<<data.l_seeds_ITK.size()
     <<"                              |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  return out;
}

///////////////////////////////////////////////////////////////////
// Find next set space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::findNext(EventData& data) const
{
  if (data.endlist) return;

  data.i_seede_ITK = data.l_seeds_ITK.begin();

  if      (data.mode==0 || data.mode==1) production2Sp(data);
  else if (data.mode==2 || data.mode==3) production3Sp(data);
  else if (data.mode==5 || data.mode==6) production3Sp(data);

  data.i_seed_ITK = data.l_seeds_ITK.begin();
  data.seed_ITK = data.seeds_ITK.begin();
  ++data.nlist;
}                       

///////////////////////////////////////////////////////////////////
// New and old list vertices comparison
///////////////////////////////////////////////////////////////////

bool InDet::SiSpacePointsSeedMaker_ITK::newVertices(EventData& data, const std::list<Trk::Vertex>& lV) const
{
  unsigned int s1 = data.l_vertex.size();
  unsigned int s2 = lV.size();

  data.isvertex = false;
  if (s1==0 && s2==0) return false;

  data.l_vertex.clear();
  if (s2 == 0) return false;

  data.isvertex = true;
  for (const Trk::Vertex& v: lV) {
    data.l_vertex.insert(static_cast<float>(v.position().z()));
  }

  data.zminU = (*data.l_vertex. begin())-20.;
  if ( data.zminU < m_zmin) data.zminU = m_zmin;
  data.zmaxU = (*data.l_vertex.rbegin())+20.;
  if ( data.zmaxU > m_zmax) data.zmaxU = m_zmax;

  return false;
}

///////////////////////////////////////////////////////////////////
// Initiate frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::buildFrameWork() 
{
  m_ptmin = fabs(m_ptmin);
  
  if (m_ptmin < 100.) m_ptmin = 100.;

  if (m_diversss < m_diver   ) m_diversss = m_diver;
  if (m_divermax < m_diversss) m_divermax = m_diversss;

  if (fabs(m_etamin) < .1) m_etamin = -m_etamax;
  m_dzdrmax0 = 1./tan(2.*atan(exp(-m_etamax)));
  m_dzdrmin0 = 1./tan(2.*atan(exp(-m_etamin)));
  
  m_COF = 134*.05*9.;
  m_ipt = 1./fabs(.9*m_ptmin);
  m_ipt2 = m_ipt*m_ipt;

  // Build radius sorted containers
  //
  m_r_size = static_cast<int>((m_r_rmax+.1)/m_r_rstep);

  // Build radius-azimuthal sorted containers
  //
  constexpr float pi2 = 2.*M_PI;
  const int   NFmax = SizeRF;
  const float sFmax = static_cast<float>(NFmax)/pi2;
  const float sFmin = 100./60.;

  float ptm = 400.;
  if (m_ptmin < ptm) ptm = m_ptmin;

  m_sF = ptm /60.;
  if (m_sF > sFmax) m_sF = sFmax;
  else if (m_sF < sFmin) m_sF = sFmin;
  m_fNmax = static_cast<int>(pi2*m_sF);
  if (m_fNmax >=NFmax) m_fNmax = NFmax-1;

  // Build radius-azimuthal-Z sorted containers for Z-vertices
  //
  const int   NFtmax = SizeRFV;
  const float sFvmax = static_cast<float>(NFtmax)/pi2;
  m_sFv = m_ptmin/120.;
  if (m_sFv>sFvmax)  m_sFv = sFvmax; 
  m_fvNmax = static_cast<int>(pi2*m_sFv);
  if (m_fvNmax>=NFtmax) m_fvNmax = NFtmax-1;

  // Build maps for radius-azimuthal-Z sorted collections 
  //
  for (int f=0; f<=m_fNmax; ++f) {

    int fb = f-1; if (fb<0      ) fb=m_fNmax;
    int ft = f+1; if (ft>m_fNmax) ft=0;
    
    // For each azimuthal region loop through all Z regions
    //
    for (int z=0; z<SizeZ; ++z) {
 
      int a        = f *SizeZ+z;
      int b        = fb*SizeZ+z;
      int c        = ft*SizeZ+z;
      m_rfz_b [a]    = 3; m_rfz_t [a]    = 3;
      m_rfz_ib[a][0] = a; m_rfz_it[a][0] = a;
      m_rfz_ib[a][1] = b; m_rfz_it[a][1] = b;
      m_rfz_ib[a][2] = c; m_rfz_it[a][2] = c;
      if (z==5) {

	m_rfz_t [a]    = 9;
	m_rfz_it[a][3] = a+1;
	m_rfz_it[a][4] = b+1;
	m_rfz_it[a][5] = c+1;
	m_rfz_it[a][6] = a-1;
	m_rfz_it[a][7] = b-1;
	m_rfz_it[a][8] = c-1;
      } else if (z> 5) {

	m_rfz_b [a]    = 6;
	m_rfz_ib[a][3] = a-1;
	m_rfz_ib[a][4] = b-1;
	m_rfz_ib[a][5] = c-1;

	if (z<10) {

	  m_rfz_t [a]    = 6;
	  m_rfz_it[a][3] = a+1;
	  m_rfz_it[a][4] = b+1;
	  m_rfz_it[a][5] = c+1;
	}
      } else {

	m_rfz_b [a]    = 6;
	m_rfz_ib[a][3] = a+1;
	m_rfz_ib[a][4] = b+1;
	m_rfz_ib[a][5] = c+1;

	if (z>0) {

	  m_rfz_t [a]    = 6;
	  m_rfz_it[a][3] = a-1;
	  m_rfz_it[a][4] = b-1;
	  m_rfz_it[a][5] = c-1;
	}
      }

      if (z==3) {
	m_rfz_b[a]      = 9;
	m_rfz_ib[a][6] = a+2;
	m_rfz_ib[a][7] = b+2;
	m_rfz_ib[a][8] = c+2;
      } else if (z==7) {
	m_rfz_b[a]      = 9;
	m_rfz_ib[a][6] = a-2;
	m_rfz_ib[a][7] = b-2;
	m_rfz_ib[a][8] = c-2;
      }
    }
  }

  // Build maps for radius-azimuthal-Z sorted collections for Z
  //
  for (int f=0; f<=m_fvNmax; ++f) {

    int fb = f-1;
    if (fb<0) fb=m_fvNmax;
    int ft = f+1;
    if (ft>m_fvNmax) ft=0;
    
    // For each azimuthal region loop through central Z regions
    //
    for (int z=0; z<SizeZV; ++z) {
      
      int a  = f *SizeZV+z;
      int b  = fb*SizeZV+z;
      int c  = ft*SizeZV+z;
      m_rfzv_n[a]    = 3;
      m_rfzv_i[a][0] = a;
      m_rfzv_i[a][1] = b;
      m_rfzv_i[a][2] = c;
      if (z>1) {
	m_rfzv_n[a]    = 6;
	m_rfzv_i[a][3] = a-1;
	m_rfzv_i[a][4] = b-1;
	m_rfzv_i[a][5] = c-1;
      } else if (z<1) {
	m_rfzv_n[a]    = 6;
	m_rfzv_i[a][3] = a+1;
	m_rfzv_i[a][4] = b+1;
	m_rfzv_i[a][5] = c+1;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::buildBeamFrameWork(EventData& data) const
{ 
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };

  const Amg::Vector3D &cb = beamSpotHandle->beamPos();
  double tx = tan(beamSpotHandle->beamTilt(0));
  double ty = tan(beamSpotHandle->beamTilt(1));

  double ph   = atan2(ty,tx);
  double th   = acos(1./sqrt(1.+tx*tx+ty*ty));
  double sint = sin(th);
  double cost = cos(th);
  double sinp = sin(ph);
  double cosp = cos(ph);
  
  data.xbeam[0] =  static_cast<float>(cb.x());
  data.xbeam[1] =  static_cast<float>(cost*cosp*cosp+sinp*sinp);
  data.xbeam[2] =  static_cast<float>(cost*sinp*cosp-sinp*cosp);
  data.xbeam[3] = -static_cast<float>(sint*cosp               );
  
  data.ybeam[0] =  static_cast<float>(cb.y());
  data.ybeam[1] =  static_cast<float>(cost*cosp*sinp-sinp*cosp);
  data.ybeam[2] =  static_cast<float>(cost*sinp*sinp+cosp*cosp);
  data.ybeam[3] = -static_cast<float>(sint*sinp               );
  
  data.zbeam[0] =  static_cast<float>(cb.z());
  data.zbeam[1] =  static_cast<float>(sint*cosp);
  data.zbeam[2] =  static_cast<float>(sint*sinp);
  data.zbeam[3] =  static_cast<float>(cost);
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////
void  InDet::SiSpacePointsSeedMaker_ITK::convertToBeamFrameWork
(EventData& data, const Trk::SpacePoint*const& sp,float* r) const
{
  r[0] = static_cast<float>(sp->globalPosition().x())-data.xbeam[0];
  r[1] = static_cast<float>(sp->globalPosition().y())-data.ybeam[0];
  r[2] = static_cast<float>(sp->globalPosition().z())-data.zbeam[0];

  if (!sp->clusterList().second) return;

  // Only for SCT space points
  //
  const InDet::SiCluster* c0 = static_cast<const InDet::SiCluster*>(sp->clusterList().first );
  const InDet::SiCluster* c1 = static_cast<const InDet::SiCluster*>(sp->clusterList().second);
  
  Amg::Vector2D lc0 = c0->localPosition();
  Amg::Vector2D lc1 = c1->localPosition();
  
  std::pair<Amg::Vector3D, Amg::Vector3D > e0 =
    (c0->detectorElement()->endsOfStrip(InDetDD::SiLocalPosition(lc0.y(),lc0.x(),0.)));
  std::pair<Amg::Vector3D, Amg::Vector3D > e1 =
    (c1->detectorElement()->endsOfStrip(InDetDD::SiLocalPosition(lc1.y(),lc1.x(),0.)));

  Amg::Vector3D b0 (e0.second-e0.first);
  Amg::Vector3D b1 (e1.second-e1.first);
  Amg::Vector3D d02(e0.first -e1.first);

  // b0
  r[ 3] = static_cast<float>(b0[0]);
  r[ 4] = static_cast<float>(b0[1]);
  r[ 5] = static_cast<float>(b0[2]);
  
  // b1
  r[ 6] = static_cast<float>(b1[0]);
  r[ 7] = static_cast<float>(b1[1]);
  r[ 8] = static_cast<float>(b1[2]);

  // r0-r2
  r[ 9] = static_cast<float>(d02[0]);
  r[10] = static_cast<float>(d02[1]);
  r[11] = static_cast<float>(d02[2]);

  // r0
  r[12] = static_cast<float>(e0.first[0])-data.xbeam[0];
  r[13] = static_cast<float>(e0.first[1])-data.ybeam[0];
  r[14] = static_cast<float>(e0.first[2])-data.zbeam[0];
}
   
///////////////////////////////////////////////////////////////////
// Initiate space points seed maker
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::fillLists(EventData& data) const
{
  constexpr float pi2 = 2.*M_PI;
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r, re;

  int  ir0 =0;
  
  for (int i=data.r_first; i!=m_r_size; ++i) {

    if (!data.r_map[i]) continue;
    r = data.r_Sorted_ITK[i].begin();
    re = data.r_Sorted_ITK[i].end();
    if (!ir0) ir0 = i;

    if (data.iteration && (*r)->spacepoint->clusterList().second) break;

    for (; r!=re; ++r) {
      
      // Azimuthal angle sort
      //
      float F = (*r)->phi();
      if (F<0.) F+=pi2;

      int   f = static_cast<int>(F*m_sF);
      if (f < 0) f = m_fNmax;
      else if (f > m_fNmax) f = 0;

      int z;
      float Z = (*r)->z();

      // Azimuthal angle and Z-coordinate sort
      //
      if (Z>0.) {
	Z< 250.?z=5:Z< 450.?z=6:Z< 925.?z=7:Z< 1400.?z=8:Z< 2500.?z=9:z=10;
      } else {
	Z>-250.?z=5:Z>-450.?z=4:Z>-925.?z=3:Z>-1400.?z=2:Z>-2500.?z=1:z= 0;
      }

      int n = f*SizeZ+z;
      ++data.nsaz;
      data.rfz_Sorted_ITK[n].push_back(*r);
      if (!data.rfz_map[n]++) data.rfz_index[data.nrfz++] = n;
      
      if (!data.iteration && (*r)->spacepoint->clusterList().second == 0 && z>=3 && z<=7) { 
	z<=4 ? z=0 : z>=6 ? z=2 : z=1;

	// Azimuthal angle and Z-coordinate sort for fast vertex search
	//
	f = static_cast<int>(F*m_sFv);
        if (f < 0) f += m_fvNmax;
        else if (f> m_fvNmax) f -= m_fvNmax;

        n = f*3+z;
        ++data.nsazv;
	data.rfzv_Sorted_ITK[n].push_back(*r);
        if (!data.rfzv_map[n]++) data.rfzv_index[data.nrfzv++] = n;
      }
    }
  }
  data.state = 0;
}
   
///////////////////////////////////////////////////////////////////
// Erase space point information
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::erase(EventData& data) const
{
  for (int i=0; i<data.nrfz; ++i) {
    int n = data.rfz_index[i];
    data.rfz_map[n] = 0;
    data.rfz_Sorted_ITK[n].clear();
  }
  
  for (int i=0; i<data.nrfzv; ++i) {
    int n = data.rfzv_index[i];
    data.rfzv_map[n] = 0;
    data.rfzv_Sorted_ITK[n].clear();
  }
  data.state = 0;
  data.nsaz  = 0;
  data.nsazv = 0;
  data.nrfz  = 0;
  data.nrfzv = 0;
}

///////////////////////////////////////////////////////////////////
// 2 space points seeds production
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production2Sp(EventData& data) const
{
  if (data.nsazv<2) return;

  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0,r0e,r,re;
  int nseed = 0;

  // Loop thorugh all azimuthal regions
  //
  for (int f=data.fvNmin; f<=m_fvNmax; ++f) {

    // For each azimuthal region loop through Z regions
    //
    int z = 0;
    if (!data.endlist) z = data.zMin;
    for (; z<SizeZV; ++z) {
      
      int a = f*SizeZV+z;
      if (!data.rfzv_map[a]) continue;
      r0  = data.rfzv_Sorted_ITK[a].begin();
      r0e = data.rfzv_Sorted_ITK[a].end  ();

      if (!data.endlist) {
        r0 = data.rMin_ITK;
        data.endlist = true;
      }

      // Loop through trigger space points
      //
      for (; r0!=r0e; ++r0) {
	float X  = (*r0)->x();
	float Y  = (*r0)->y();
	float R  = (*r0)->radius();
	if (R<m_r2minv) continue;
        if (R>m_r2maxv) break;
	float Z  = (*r0)->z();
	float ax = X/R;
	float ay = Y/R;

	// Bottom links production
	//
	int NB = m_rfzv_n[a];
	for (int i=0; i<NB; ++i) {	  
	  int an = m_rfzv_i[a][i];
	  if (!data.rfzv_map[an]) continue;

	  r  =  data.rfzv_Sorted_ITK[an].begin();
	  re =  data.rfzv_Sorted_ITK[an].end  ();
	  
	  for (; r!=re; ++r) {
	    float Rb =(*r)->radius();
	    if (Rb<m_r1minv) continue;
            if (Rb>m_r1maxv) break;
	    float dR = R-Rb;
	    if (dR<m_drminv) break;
            if (dR>m_drmax) continue;
	    float dZ = Z-(*r)->z();
	    float Tz = dZ/dR;
            if (Tz<data.dzdrmin || Tz>data.dzdrmax) continue;
	    float Zo = Z-R*Tz;

	    // Comparison with vertices Z coordinates
	    //
	    if (!isZCompatible(data, Zo, Rb, Tz)) continue;

	    // Momentum cut
	    //
	    float dx =(*r)->x()-X;
	    float dy =(*r)->y()-Y;
	    float x  = dx*ax+dy*ay;
	    float y  =-dx*ay+dy*ax;
	    float xy = x*x+y*y; if (xy == 0.) continue;
	    float r2 = 1./xy;
	    float Ut = x*r2;
	    float Vt = y*r2;
	    float UR = Ut*R+1.; if (UR == 0.) continue;
	    float A  = Vt*R/UR;
	    float B  = Vt-A*Ut;
	    if (fabs(B*data.K) > m_ipt*sqrt(1.+A*A)) continue;
            ++nseed;
	    newSeed(data, (*r), (*r0), Zo);
	  }
	}
	if (nseed < m_maxsize) continue;
	data.endlist=false;
        data.rMin_ITK = (++r0);
        data.fvNmin=f;
        data.zMin=z;
	return;
      }
    }
  }
  data.endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production3Sp(EventData& data) const
{ 
  if (data.nsaz<3) return;
  data.seeds_ITK.clear();

  const int   ZI[SizeZ]= {5,6,7,8,9,10,4,3,2,1,0};
  std::list<InDet::SiSpacePointForSeedITK*>::iterator rt[9],rte[9],rb[9],rbe[9];
  int nseed = 0;

  // Loop thorugh all azimuthal regions
  //
  for (int f=data.fNmin; f<=m_fNmax; ++f) {
    
    // For each azimuthal region loop through all Z regions
    //
    int z = 0;
    if (!data.endlist) z = data.zMin;

    for (; z<SizeZ; ++z) {

      int a  = f *SizeZ+ZI[z];
      if (!data.rfz_map[a]) continue;
      int NB = 0, NT = 0;
      for (int i=0; i<m_rfz_b[a]; ++i) {
	int an =  m_rfz_ib[a][i];
	if (!data.rfz_map[an]) continue;
	rb [NB] = data.rfz_Sorted_ITK[an].begin();
        rbe[NB++] = data.rfz_Sorted_ITK[an].end();
      } 
      for (int i=0; i<m_rfz_t[a]; ++i) {
	int an = m_rfz_it[a][i];
	if (!data.rfz_map[an]) continue;
	rt [NT] = data.rfz_Sorted_ITK[an].begin();
        rte[NT++] = data.rfz_Sorted_ITK[an].end();
      } 

      if (data.iteration == 0  && data.iteration0 ==0) production3SpSSS(data, rb, rbe, rt, rte, NB, NT, nseed);
      else                                       production3SpPPP(data, rb, rbe, rt, rte, NB, NT, nseed);

      if (!data.endlist) {
        data.fNmin = f;
        data.zMin = z;
        return;
      }
    }
  }
  data.endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 pixel space points seeds for full scan
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production3SpPPP
(EventData& data,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rb ,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rbe,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rt ,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rte,
 int NB, int NT, int& nseed) const
{
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0=rb[0], r;
  if (!data.endlist) {
    r0 = data.rMin_ITK;
    data.endlist = true;
  }

  float ipt2K = data.ipt2K;
  float ipt2C = data.ipt2C;
  float COFK  = data.COFK;
  float imaxp = m_diver;
  float imaxs = m_divermax;

  data.CmSp_ITK.clear();

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {

    data.nOneSeeds = 0;
    data.mapOneSeeds_ITK.clear();

    float R = (*r0)->radius();

    const Trk::Surface* sur0 = (*r0)->sur();
    const Trk::Surface* surn = (*r0)->sun();
    float               X    = (*r0)->x();
    float               Y    = (*r0)->y();
    float               Z    = (*r0)->z();
    int                 Nb   = 0;

    // Bottom links production
    //
    for (int i=0; i<NB; ++i) {
      for (r=rb[i]; r!=rbe[i]; ++r) {	
	float Rb =(*r)->radius();
	float dR = R-Rb;

	if (dR > m_drmax) {
          rb[i]=r;
          continue;
        }
	if (dR < m_drmin) break;
	if ((*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;

	float Tz = (Z-(*r)->z())/dR, aTz =fabs(Tz);

	if (aTz < data.dzdrmin || aTz > data.dzdrmax) continue;
	
	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(data, Zo, Rb, Tz)) continue;
	data.SP_ITK[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i<NT; ++i) {
      for (r=rt[i]; r!=rte[i]; ++r) {
	float Rt =(*r)->radius();
	float dR = Rt-R;
	
	if (dR<m_drmin) {
          rt[i]=r;
          continue;
        }
	if (dR>m_drmax) break;

	if ( (*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;

	float Tz = ((*r)->z()-Z)/dR, aTz =fabs(Tz);

	if (aTz < data.dzdrmin || aTz > data.dzdrmax) continue;

	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(data, Zo, R, Tz)) continue;
  	data.SP_ITK[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;
    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();
    float ax    = X/R;
    float ay    = Y/R;

    for (int i=0; i<Nt; ++i) {
      InDet::SiSpacePointForSeedITK* sp = data.SP_ITK[i];

      float dx  = sp->x()-X;
      float dy  = sp->y()-Y;
      float dz  = sp->z()-Z;
      float x   = dx*ax+dy*ay;
      float y   = dy*ax-dx*ay;
      float r2  = 1./(x*x+y*y);
      float dr  = sqrt(r2);
      float tz  = dz*dr;
      if (i < Nb) tz = -tz;

      data.Tz[i]   = tz;
      data.Zo[i]   = Z-R*tz;
      data.R [i]   = dr;
      data.U [i]   = x*r2;
      data.V [i]   = y*r2;
      data.Er[i]   = ((covz0+sp->covz())+(tz*tz)*(covr0+sp->covr()))*r2;
    }
    covr0 *= .5;
    covz0 *= 2.;
   
    // Three space points comparison
    //
    for (int b=0; b<Nb; ++b) {    
      float  Zob  = data.Zo[b];
      float  Tzb  = data.Tz[b];
      float  Rb2r = data.R [b]*covr0;
      float  Rb2z = data.R [b]*covz0;
      float  Erb  = data.Er[b];
      float  Vb   = data.V [b];
      float  Ub   = data.U [b];
      float  Tzb2 = (1.+Tzb*Tzb);
      float sTzb2 = sqrt(Tzb2);
      float  CSA  = Tzb2*COFK;
      float ICSA  = Tzb2*ipt2C;
      float imax  = imaxp;
      if (data.SP_ITK[b]->spacepoint->clusterList().second) imax = imaxs;
  
      for (int t=Nb; t<Nt; ++t) {
	float dT  = ((Tzb-data.Tz[t])*(Tzb-data.Tz[t])-data.R[t]*Rb2z-(Erb+data.Er[t]))-(data.R[t]*Rb2r)*((Tzb+data.Tz[t])*(Tzb+data.Tz[t]));
	if (dT > ICSA) continue;

	float dU  = data.U[t]-Ub;
        if (dU == 0.) continue;
	float A   = (data.V[t]-Vb)/dU;
	float S2  = 1.+A*A;
	float B   = Vb-A*Ub;
	float B2  = B*B;
	if (B2  > ipt2K*S2 || dT*S2 > B2*CSA) continue;

	float Im  = fabs((A-B*R)*R);

	if (Im <= imax) {
	  float dr = data.R[b];
          if (data.R[t] < data.R[b]) dr = data.R[t];
          Im+=fabs((Tzb-data.Tz[t])/(dr*sTzb2));
	  data.CmSp_ITK.emplace_back(std::make_pair(B/sqrt(S2), data.SP_ITK[t]));
          data.SP_ITK[t]->setParam(Im);
	}
      }
      if (!data.CmSp_ITK.empty()) {
        newOneSeedWithCurvaturesComparison(data, data.SP_ITK[b], (*r0), Zob);
      }
    }
    fillSeeds(data);
    nseed += data.fillOneSeeds;
    if (nseed>=m_maxsize) {
      data.endlist=false;
      ++r0;
      data.rMin_ITK = r0;
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////
// Production 3 SCT space points seeds for full scan
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::production3SpSSS
(EventData& data,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rb ,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rbe,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rt ,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rte,
 int NB, int NT, int& nseed) const
{
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0=rb[0], r;
  if (!data.endlist) {
    r0 = data.rMin_ITK;
    data.endlist = true;
  }

  float ipt2K = data.ipt2K;
  float ipt2C = data.ipt2C;
  float COFK  = data.COFK;
  float imaxs = m_divermax;

  data.CmSp_ITK.clear();

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {
    data.nOneSeeds = 0;
    data.mapOneSeeds_ITK.clear();

    float R = (*r0)->radius();

    const Trk::Surface* sur0 = (*r0)->sur();
    const Trk::Surface* surn = (*r0)->sun();
    float               X    = (*r0)->x();
    float               Y    = (*r0)->y();
    float               Z    = (*r0)->z();
    int                 Nb   = 0;

    // Bottom links production
    //
    for (int i=0; i<NB; ++i) {
      for (r=rb[i]; r!=rbe[i]; ++r) {
	float Rb =(*r)->radius();
	float dR = R-Rb;
	if (dR > m_drmax) {
          rb[i]=r;
          continue;
        }
	if (dR < m_drmin) break;
	if ((*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;
	float Tz = (Z-(*r)->z())/dR;
        float aTz =fabs(Tz);
	if (aTz < data.dzdrmin || aTz > data.dzdrmax) continue;

	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(data, Zo, Rb, Tz)) continue;
	data.SP_ITK[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i<NT; ++i) {
      for (r=rt[i]; r!=rte[i]; ++r) {
	float Rt =(*r)->radius();
	float dR = Rt-R;
	
	if (dR<m_drmin) {
          rt[i]=r; 
          continue;
        }
	if (dR>m_drmax) break;

	if ((*r)->sur()==sur0 || (surn && surn==(*r)->sun())) continue;
	float Tz = ((*r)->z()-Z)/dR;
        float aTz = fabs(Tz);
	if (aTz < data.dzdrmin || aTz > data.dzdrmax) continue;

	// Comparison with vertices Z coordinates
	//
	float Zo = Z-R*Tz;
        if (!isZCompatible(data, Zo, R, Tz)) continue;
  	data.SP_ITK[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;
    float covr0 = (*r0)->covr();
    float covz0 = (*r0)->covz();
    float ax    = X/R;
    float ay    = Y/R;

    for (int i=0; i<Nt; ++i) {
      InDet::SiSpacePointForSeedITK* sp = data.SP_ITK[i];

      float dx  = sp->x()-X;
      float dy  = sp->y()-Y;
      float dz  = sp->z()-Z;
      float x   = dx*ax+dy*ay;
      float y   = dy*ax-dx*ay;
      float r2  = 1./(x*x+y*y);
      float dr  = sqrt(r2);
      float tz  = dz*dr;
      if (i < Nb) tz = -tz;

      data.X [i]   = x;
      data.Y [i]   = y;
      data.Tz[i]   = tz;
      data.Zo[i]   = Z-R*tz;
      data.R [i]   = dr;
      data.U [i]   = x*r2;
      data.V [i]   = y*r2;
      data.Er[i]   = ((covz0+sp->covz())+(tz*tz)*(covr0+sp->covr()))*r2;
    }
    covr0 *= .5;
    covz0 *= 2.;
   
    // Three space points comparison
    //
    for (int b=0; b<Nb; ++b) {
      float  Zob  = data.Zo[b];
      float  Tzb  = data.Tz[b];
      float  Rb2r = data.R [b]*covr0;
      float  Rb2z = data.R [b]*covz0;
      float  Erb  = data.Er[b];
      float  Vb   = data.V [b];
      float  Ub   = data.U [b];
      float  Tzb2 = (1.+Tzb*Tzb);
      float sTzb2 = sqrt(Tzb2);
      float  CSA  = Tzb2*COFK;
      float ICSA  = Tzb2*ipt2C;
      float imax  = imaxs;
      
      float Se    = 1./sqrt(1.+Tzb*Tzb);
      float Ce    = Se*Tzb;
      float Sx    = Se*ax;
      float Sy    = Se*ay;

      for (int t=Nb; t<Nt; ++t) {
	// Trigger point
	//	
	float dU0   =  data.U[t]-Ub;
        if (dU0 == 0.) continue; 
	float A0    = (data.V[t]-Vb)/dU0;
	float C0    = 1./sqrt(1.+A0*A0); 
	float S0    = A0*C0;
	float d0[3] = {Sx*C0-Sy*S0, Sx*S0+Sy*C0, Ce};
	float rn[3];
        if (!(*r0)->coordinates(d0,rn)) continue;

	// Bottom  point
	//
	float B0    = 2.*(Vb-A0*Ub);
	float Cb    = (1.-B0*data.Y[b])*C0;
	float Sb    = (A0+B0*data.X[b])*C0;
	float db[3] = {Sx*Cb-Sy*Sb,Sx*Sb+Sy*Cb,Ce};
	float rbDup[3]; //a new and different rb
	if (!data.SP_ITK[b]->coordinates(db,rbDup)) continue;

	// Top     point
	//
	float Ct    = (1.-B0*data.Y[t])*C0;
	float St    = (A0+B0*data.X[t])*C0;
	float dt[3] = {Sx*Ct-Sy*St,Sx*St+Sy*Ct,Ce};
	float rtDup[3]; //doesnt hide previous declaration of rt
	if (!data.SP_ITK[t]->coordinates(dt,rtDup)) continue;

	float xb    = rbDup[0]-rn[0];
	float yb    = rbDup[1]-rn[1];
	float xt    = rtDup[0]-rn[0];
	float yt    = rtDup[1]-rn[1];

	float rb2   = 1./(xb*xb+yb*yb);
	float rt2   = 1./(xt*xt+yt*yt);
	
	float tb    =  (rn[2]-rbDup[2])*sqrt(rb2);
	float tz    =  (rtDup[2]-rn[2])*sqrt(rt2);

	float dT  = ((tb-tz)*(tb-tz)-data.R[t]*Rb2z-(Erb+data.Er[t]))-(data.R[t]*Rb2r)*((tb+tz)*(tb+tz));
	if ( dT > ICSA) continue;

	float Rn    = sqrt(rn[0]*rn[0]+rn[1]*rn[1]);
	float Ax    = rn[0]/Rn;
	float Ay    = rn[1]/Rn;

	float ub    = (xb*Ax+yb*Ay)*rb2;
	float vb    = (yb*Ax-xb*Ay)*rb2;
	float ut    = (xt*Ax+yt*Ay)*rt2;
	float vt    = (yt*Ax-xt*Ay)*rt2;
	
	float dU  = ut-ub;
	if (dU == 0.) continue;
	float A   = (vt-vb)/dU;
	float S2  = 1.+A*A;
	float B   = vb-A*ub;
	float B2  = B*B;
	if (B2 > ipt2K*S2 || dT*S2 > B2*CSA) continue;

	float Im  = fabs((A-B*Rn)*Rn);

	if (Im <= imax) {
	  float dr;
	  data.R[t] < data.R[b] ? dr = data.R[t] : dr = data.R[b];
	  Im+=fabs((Tzb-data.Tz[t])/(dr*sTzb2));
	  data.CmSp_ITK.emplace_back(std::make_pair(B/sqrt(S2), data.SP_ITK[t]));
	  data.SP_ITK[t]->setParam(Im);
	}
	
      }
      if (!data.CmSp_ITK.empty()) {
        newOneSeedWithCurvaturesComparison(data, data.SP_ITK[b], (*r0), Zob);
      }
    }
    fillSeeds(data);
    nseed += data.fillOneSeeds;
    if (nseed>=m_maxsize) {
      data.endlist=false;
      ++r0;
      data.rMin_ITK = r0;
      return;
    }
  }
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds in ROI
///////////////////////////////////////////////////////////////////

 
void InDet::SiSpacePointsSeedMaker_ITK::production3SpTrigger
(EventData& data,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rb ,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rbe,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rt ,
 std::list<InDet::SiSpacePointForSeedITK*>::iterator* rte,
 int NB, int NT, int& nseed) const
{
  std::list<InDet::SiSpacePointForSeedITK*>::iterator r0=rb[0], r;
  if (!data.endlist) {
    r0 = data.rMin_ITK;
    data.endlist = true;
  }

  constexpr float pi2 = 2.*M_PI;

  float ipt2K = data.ipt2K;
  float ipt2C = data.ipt2C;
  float COFK  = data.COFK;
  float imaxp = m_diver;
  float imaxs = m_diversss;

  data.CmSp_ITK.clear();

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {
    data.nOneSeeds = 0;
    data.mapOneSeeds_ITK.clear();
	
    float R = (*r0)->radius();

    const Trk::Surface* sur0 = (*r0)->sur();
    float               X    = (*r0)->x();
    float               Y    = (*r0)->y();
    float               Z    = (*r0)->z();
    int                 Nb   = 0;

    // Bottom links production
    //
    for (int i=0; i<NB; ++i) {
      for (r=rb[i]; r!=rbe[i]; ++r) {
	float Rb =(*r)->radius();

	float dR = R-Rb;
	if (dR < m_drmin || (data.iteration && (*r)->spacepoint->clusterList().second)) break;
	if (dR > m_drmax || (*r)->sur()==sur0) continue;

	// Comparison with  bottom and top Z 
	//
	float Tz = (Z-(*r)->z())/dR;
	float Zo = Z-R*Tz;
        if (Zo < data.zminB || Zo > data.zmaxB) continue;
	float Zu = Z+(550.-R)*Tz;
        if (Zu < data.zminU || Zu > data.zmaxU) continue;
	data.SP_ITK[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i<NT; ++i) {
      for (r=rt[i]; r!=rte[i]; ++r) {
	float Rt =(*r)->radius();
	float dR = Rt-R;
	
	if (dR<m_drmin) {
          rt[i]=r;
          continue;
        }
	if (dR>m_drmax) break;

	if ((*r)->sur()==sur0) continue;

	// Comparison with  bottom and top Z 
	//
	float Tz = ((*r)->z()-Z)/dR;
	float Zo = Z-R*Tz;
        if (Zo < data.zminB || Zo > data.zmaxB) continue;
	float Zu = Z+(550.-R)*Tz;
        if (Zu < data.zminU || Zu > data.zmaxU) continue;
  	data.SP_ITK[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;
    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();

    float ax = X/R;
    float ay = Y/R;
    
    for (int i=0; i<Nt; ++i) {
      InDet::SiSpacePointForSeedITK* sp = data.SP_ITK[i];

      float dx  = sp->x()-X;
      float dy  = sp->y()-Y;
      float dz  = sp->z()-Z;
      float x   = dx*ax+dy*ay;
      float y   = dy*ax-dx*ay;
      float r2  = 1./(x*x+y*y);
      float dr  = sqrt(r2);
      float tz  = dz*dr;
      if (i < Nb) tz = -tz;

      data.X [i]   = x;
      data.Y [i]   = y;
      data.Tz[i]   = tz;
      data.Zo[i]   = Z-R*tz;
      data.R [i]   = dr;
      data.U [i]   = x*r2;
      data.V [i]   = y*r2;
      data.Er[i]   = ((covz0+sp->covz())+(tz*tz)*(covr0+sp->covr()))*r2;
    }
    covr0 *= .5;
    covz0 *= 2.;
   
    // Three space points comparison
    //
    for (int b=0; b<Nb; ++b) {
      float  Zob  = data.Zo[b];
      float  Tzb  = data.Tz[b];
      float  Rb2r = data.R [b]*covr0;
      float  Rb2z = data.R [b]*covz0;
      float  Erb  = data.Er[b];
      float  Vb   = data.V [b];
      float  Ub   = data.U [b];
      float  Tzb2 = (1.+Tzb*Tzb);
      float  CSA  = Tzb2*COFK;
      float ICSA  = Tzb2*ipt2C;
      float imax  = imaxp;
      if (data.SP_ITK[b]->spacepoint->clusterList().second) imax = imaxs;
      
      for (int t=Nb;  t!=Nt; ++t) {
	float dT  = ((Tzb-data.Tz[t])*(Tzb-data.Tz[t])-data.R[t]*Rb2z-(Erb+data.Er[t]))-(data.R[t]*Rb2r)*((Tzb+data.Tz[t])*(Tzb+data.Tz[t]));
	if ( dT > ICSA) continue;
	float dU  = data.U[t]-Ub;
        if (dU == 0.) continue;
	float A   = (data.V[t]-Vb)/dU;
	float S2  = 1.+A*A;
	float B   = Vb-A*Ub;
	float B2  = B*B;
	if (B2  > ipt2K*S2 || dT*S2 > B2*CSA) continue;

	float Im  = fabs((A-B*R)*R);
	if (Im > imax) continue;

	// Azimuthal angle test
	//
	float y  = 1.;
	float x  = 2.*B*R-A;
	float df = fabs(atan2(ay*y-ax*x,ax*y+ay*x)-data.ftrig);
	if (df > M_PI) df=pi2-df;
	if (df > data.ftrigW) continue;
	data.CmSp_ITK.emplace_back(std::make_pair(B/sqrt(S2), data.SP_ITK[t]));
        data.SP_ITK[t]->setParam(Im);
      }
      if (!data.CmSp_ITK.empty()) {
        newOneSeedWithCurvaturesComparison(data, data.SP_ITK[b], (*r0), Zob);
      }
    }
    fillSeeds(data);
    nseed += data.fillOneSeeds;
    if (nseed>=m_maxsize) {
      data.endlist=false;
      ++r0;
      data.rMin_ITK = r0;
      return;
    } 
  }
}

///////////////////////////////////////////////////////////////////
// New 3 space points pro seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newOneSeed
(EventData& data,
 InDet::SiSpacePointForSeedITK*& p1, InDet::SiSpacePointForSeedITK*& p2,
 InDet::SiSpacePointForSeedITK*& p3, float z, float q) const
{
  if (data.nOneSeeds < m_maxOneSize) {
    data.OneSeeds_ITK[data.nOneSeeds].set(p1,p2,p3,z);
    data.mapOneSeeds_ITK.insert(std::make_pair(q, &(data.OneSeeds_ITK[data.nOneSeeds])));
    ++data.nOneSeeds;
  } else {
    std::multimap<float,InDet::SiSpacePointsProSeedITK*>::reverse_iterator 
      l = data.mapOneSeeds_ITK.rbegin();

    if ((*l).first <= q) return;
    
    InDet::SiSpacePointsProSeedITK* s = (*l).second;
    s->set(p1,p2,p3,z);

    std::multimap<float,InDet::SiSpacePointsProSeedITK*>::iterator 
      i = data.mapOneSeeds_ITK.insert(std::make_pair(q,s));
	
    for (++i; i!=data.mapOneSeeds_ITK.end(); ++i) {
      if ((*i).second==s) {
        data.mapOneSeeds_ITK.erase(i);
        return;
      }
    }
  }
}

///////////////////////////////////////////////////////////////////
// New 3 space points pro seeds production
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newOneSeedWithCurvaturesComparison
(EventData& data, SiSpacePointForSeedITK*& SPb, SiSpacePointForSeedITK*& SP0, float Zob) const
{
  const float dC = .00003;

  bool  pixb = !SPb->spacepoint->clusterList().second;

  std::sort(data.CmSp_ITK.begin(), data.CmSp_ITK.end(), comCurvatureITK());
  std::vector<std::pair<float,InDet::SiSpacePointForSeedITK*>>::iterator j,jn,i = data.CmSp_ITK.begin(),ie = data.CmSp_ITK.end();
  jn=i;
      
  for (; i!=ie; ++i) {
    float u    = (*i).second->param();
    bool                pixt = !(*i).second->spacepoint->clusterList().second;
    if (pixt && fabs(SPb->z() -(*i).second->z()) > m_dzmaxPPP) continue;

    const Trk::Surface* Sui  = (*i).second->sur   ();
    float               Ri   = (*i).second->radius();
    float               Ci1  =(*i).first-dC;
    float               Ci2  =(*i).first+dC;
    float               Rmi  = 0.;
    float               Rma  = 0.;
    bool                in   = false;
    
    if      (!pixb) u-=400.;
    else if ( pixt) u-=200.;

    for (j=jn; j!=ie; ++j) {  
      if (       j == i           ) continue;
      if ( (*j).first < Ci1       ) {jn=j; ++jn; continue;}
      if ( (*j).first > Ci2       ) break;
      if ( (*j).second->sur()==Sui) continue;
      
      float Rj = (*j).second->radius();
      if (fabs(Rj-Ri) < m_drmin) continue;

      if (in) {
	if      (Rj > Rma) Rma = Rj;
	else if (Rj < Rmi) Rmi = Rj;
	else continue;
	if ( (Rma-Rmi) > 20.) {
          u-=200.;
          break;
        }
      } else {
	in=true;
        Rma=Rmi=Rj;
        u-=200.;
      }
    }
    if (u > m_umax) continue;

    newOneSeed(data, SPb, SP0, (*i).second, Zob, u);
  }
  data.CmSp_ITK.clear();
}

///////////////////////////////////////////////////////////////////
// Fill seeds
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::fillSeeds(EventData& data) const
{
  data.fillOneSeeds = 0;

  std::multimap<float,InDet::SiSpacePointsProSeedITK*>::iterator 
    lf = data.mapOneSeeds_ITK.begin(),
    l  = data.mapOneSeeds_ITK.begin(),
    le = data.mapOneSeeds_ITK.end  ();
  
  if (l==le) return;

  SiSpacePointsProSeedITK* s = nullptr;

  for (; l!=le; ++l) {
    float w = (*l).first;
    s       = (*l).second;
    if (l!=lf && s->spacepoint0()->radius() < 43. && w > -200.) continue;
    if (!s->setQuality(w)) continue;
    
    if (data.i_seede_ITK!=data.l_seeds_ITK.end()) {
      s  = &(*data.i_seede_ITK++);
      *s = *(*l).second;
    } else {
      data.l_seeds_ITK.emplace_back(SiSpacePointsProSeedITK(*(*l).second));
      s = &(data.l_seeds_ITK.back());
      data.i_seede_ITK = data.l_seeds_ITK.end();
    }
    
    if      (s->spacepoint0()->spacepoint->clusterList().second) w-=3000.;
    else if (s->spacepoint1()->spacepoint->clusterList().second) w-=2000.;
    else if (s->spacepoint2()->spacepoint->clusterList().second) w-=1000.;

    data.seeds_ITK.insert(std::make_pair(w,s));
    ++data.fillOneSeeds;
  }
}

const InDet::SiSpacePointsSeed* InDet::SiSpacePointsSeedMaker_ITK::next(EventData& data) const
{
  if (not data.initialized) initializeEventData(data);

  if (data.nspoint==3) {
    do {
      if (data.i_seed_ITK==data.i_seede_ITK) {
        findNext(data);
        if (data.i_seed_ITK==data.i_seede_ITK) return nullptr;
      }
      ++data.i_seed_ITK;
    } while (!(*data.seed_ITK++).second->set3(data.seedOutput));
    return &data.seedOutput;
  } else {
    if (data.i_seed_ITK==data.i_seede_ITK) {
      findNext(data);
      if (data.i_seed_ITK==data.i_seede_ITK) return nullptr;
    } 
    (*data.i_seed_ITK++).set2(data.seedOutput);
    return &data.seedOutput;
  }
  return nullptr;
}
  

bool InDet::SiSpacePointsSeedMaker_ITK::isZCompatible  
(EventData& data, float& Zv, float& R, float& T) const
{
  if (Zv < data.zminU || Zv > data.zmaxU) return false;
  if (!data.isvertex) return true;

  float dZmin = std::numeric_limits<float>::max();
  for (const float& v: data.l_vertex) {
    float dZ = fabs(v-Zv);
    if (dZ >= dZmin) break;
    dZmin=dZ;
  }
  return dZmin < (m_dzver+m_dzdrver*R)*sqrt(1.+T*T);
}

///////////////////////////////////////////////////////////////////
// New space point for seeds 
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointForSeedITK* InDet::SiSpacePointsSeedMaker_ITK::newSpacePoint
(EventData& data, const Trk::SpacePoint*const& sp) const
{
  InDet::SiSpacePointForSeedITK* sps = nullptr;

  float r[15];
  convertToBeamFrameWork(data, sp, r);

  if (data.checketa) {
    float z = (fabs(r[2])+m_zmax);
    float x = r[0]*data.dzdrmin;
    float y = r[1]*data.dzdrmin;
    if ((z*z )<(x*x+y*y)) return sps;
  }

  if (data.i_spforseed_ITK!=data.l_spforseed_ITK.end()) {
    sps = &(*data.i_spforseed_ITK++);
    sps->set(sp, r);
  } else {
    data.l_spforseed_ITK.emplace_back(InDet::SiSpacePointForSeedITK(sp, r));
    sps = &(data.l_spforseed_ITK.back());
    data.i_spforseed_ITK = data.l_spforseed_ITK.end();
  }
      
  return sps;
}

///////////////////////////////////////////////////////////////////
// New 2 space points seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_ITK::newSeed
(EventData& data,
 InDet::SiSpacePointForSeedITK*& p1, InDet::SiSpacePointForSeedITK*& p2, float z) const
{
  InDet::SiSpacePointForSeedITK* p3 = nullptr;

  if (data.i_seede_ITK!=data.l_seeds_ITK.end()) {
    InDet::SiSpacePointsProSeedITK* s = &(*data.i_seede_ITK++);
    s->set(p1, p2, p3, z);
  } else {
    data.l_seeds_ITK.emplace_back(InDet::SiSpacePointsProSeedITK(p1, p2, p3, z));
    data.i_seede_ITK = data.l_seeds_ITK.end();
  }
}
 
void InDet::SiSpacePointsSeedMaker_ITK::initializeEventData(EventData& data) const {
  data.initialize(EventData::ITK,
                  m_maxsizeSP,
                  m_maxOneSize,
                  0, // maxsize not used
                  m_r_size,
                  0, // sizeRF not used
                  SizeRFZ,
                  SizeRFZV,
                  m_checketa);
}
