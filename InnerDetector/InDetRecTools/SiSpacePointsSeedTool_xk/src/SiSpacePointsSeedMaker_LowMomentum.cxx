/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//   Implementation file for class SiSpacePointsSeedMaker_LowMomentum
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// AlgTool used for TRT_DriftCircleOnTrack object production
///////////////////////////////////////////////////////////////////
// Version 1.0 21/04/2004 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#include "SiSpacePointsSeedTool_xk/SiSpacePointsSeedMaker_LowMomentum.h"

#include <iomanip>
#include <limits>
#include <ostream>

///////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointsSeedMaker_LowMomentum::SiSpacePointsSeedMaker_LowMomentum
(const std::string& t, const std::string& n, const IInterface* p)
  : base_class(t, n, p)
{
}

///////////////////////////////////////////////////////////////////
// Initialisation
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_LowMomentum::initialize()
{
  StatusCode sc = AlgTool::initialize();

  ATH_CHECK(m_spacepointsPixel.initialize(m_pixel));
  ATH_CHECK(m_spacepointsSCT.initialize(m_sct));
  ATH_CHECK(m_spacepointsOverlap.initialize(m_useOverlap));

  // Get beam geometry
  //
  ATH_CHECK(m_beamSpotKey.initialize());

  ATH_CHECK( m_fieldCondObjInputKey.initialize());

  // PRD-to-track association (optional)
  ATH_CHECK( m_prdToTrackMap.initialize( !m_prdToTrackMap.key().empty()));

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

  m_initialized = true;

  return sc;
}

///////////////////////////////////////////////////////////////////
// Finalize
///////////////////////////////////////////////////////////////////

StatusCode InDet::SiSpacePointsSeedMaker_LowMomentum::finalize()
{
  return AlgTool::finalize();
}

///////////////////////////////////////////////////////////////////
// Initialize tool for new event 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newEvent(const EventContext& ctx, EventData& data, int) const
{
  if (not data.initialized) initializeEventData(data);

  data.trigger = false;
  if (!m_pixel && !m_sct) return;
  erase(data);
  data.i_spforseed = data.l_spforseed.begin();
  buildBeamFrameWork(data);

  float irstep = 1./m_r_rstep;
  int   irmax  = m_r_size-1;

  SG::ReadHandle<Trk::PRDtoTrackMap>  prd_to_track_map;
  const Trk::PRDtoTrackMap *prd_to_track_map_cptr = nullptr;
  if (!m_prdToTrackMap.key().empty()) {
    prd_to_track_map=SG::ReadHandle<Trk::PRDtoTrackMap>(m_prdToTrackMap, ctx);
    if (!prd_to_track_map.isValid()) {
      ATH_MSG_ERROR("Failed to read PRD to track association map: " << m_prdToTrackMap.key());
    }
    prd_to_track_map_cptr = prd_to_track_map.cptr();
  }

  // Get pixels space points containers from store gate 
  //
  if (m_pixel) {

    SG::ReadHandle<SpacePointContainer> spacepointsPixel{m_spacepointsPixel, ctx};
    if (spacepointsPixel.isValid()) {

      for (const SpacePointCollection* spc: *spacepointsPixel) {
        for (const Trk::SpacePoint* sp: *spc) {

          float r = sp->r();
          if (r<0. || r>=m_r_rmax) continue;
          if (prd_to_track_map_cptr && isUsed(sp,*prd_to_track_map_cptr)) continue;

          InDet::SiSpacePointForSeed* sps = newSpacePoint(data, sp);

          int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
          data.r_Sorted[ir].push_back(sps);
          ++data.r_map[ir];
          if (data.r_map[ir]==1) data.r_index[data.nr++] = ir;
          ++data.ns;
        }
      }
    }
  }

  // Get sct space points containers from store gate 
  //
  if (m_sct) {

    SG::ReadHandle<SpacePointContainer> spacepointsSCT{m_spacepointsSCT, ctx};
    if (spacepointsSCT.isValid()) {

      for (const SpacePointCollection* spc: *spacepointsSCT) {
        for (const Trk::SpacePoint* sp: *spc) {

          float r = sp->r();
          if (r<0. || r>=m_r_rmax) continue;
          if (prd_to_track_map_cptr && isUsed(sp,*prd_to_track_map_cptr)) continue;

          InDet::SiSpacePointForSeed* sps = newSpacePoint(data, sp);

          int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
          data.r_Sorted[ir].push_back(sps);
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

void InDet::SiSpacePointsSeedMaker_LowMomentum::newRegion
(const EventContext& ctx, EventData& data,
 const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT) const{
  if (not data.initialized) initializeEventData(data);

  data.trigger = false;
  if (!m_pixel && !m_sct) return;
  erase(data);
  data.i_spforseed = data.l_spforseed.begin();
  buildBeamFrameWork(data);

  int   irmax  = m_r_size-1;
  float irstep = 1./m_r_rstep;

  SG::ReadHandle<Trk::PRDtoTrackMap>  prd_to_track_map;
  const Trk::PRDtoTrackMap  *prd_to_track_map_cptr = nullptr;
  if (!m_prdToTrackMap.key().empty()) {
    prd_to_track_map=SG::ReadHandle<Trk::PRDtoTrackMap>(m_prdToTrackMap, ctx);
    if (!prd_to_track_map.isValid()) {
      ATH_MSG_ERROR("Failed to read PRD to track association map: " << m_prdToTrackMap.key());
    }
    prd_to_track_map_cptr = prd_to_track_map.cptr();
  }

  // Get pixels space points containers from store gate 
  //
  if (m_pixel && vPixel.size()) {

    SG::ReadHandle<SpacePointContainer> spacepointsPixel{m_spacepointsPixel, ctx};
    if (spacepointsPixel.isValid()) {
      // Loop through all trigger collections
      //
      for (const IdentifierHash& l: vPixel) {
        auto w = spacepointsPixel->indexFindPtr(l);
        if (w==nullptr) continue;
        for (const Trk::SpacePoint* sp: *w) {
          float r = sp->r();
          if (r<0. || r>=m_r_rmax) continue;
          if (prd_to_track_map_cptr && isUsed(sp,*prd_to_track_map_cptr)) continue;

          InDet::SiSpacePointForSeed* sps = newSpacePoint(data, sp);

          int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
          data.r_Sorted[ir].push_back(sps);
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

    SG::ReadHandle<SpacePointContainer> spacepointsSCT{m_spacepointsSCT, ctx};
    if (spacepointsSCT.isValid()) {
      // Loop through all trigger collections
      //
      for (const IdentifierHash& l: vSCT) {
        auto w = spacepointsSCT->indexFindPtr(l);
        if (w==nullptr) continue;
        for (const Trk::SpacePoint* sp: *w) {
          float r = sp->r();
          if (r<0. || r>=m_r_rmax) continue;
          if (prd_to_track_map_cptr && isUsed(sp,*prd_to_track_map_cptr)) continue;
          InDet::SiSpacePointForSeed* sps = newSpacePoint(data, sp);
          int ir = static_cast<int>(sps->radius()*irstep);
          if (ir>irmax) ir = irmax;
          data.r_Sorted[ir].push_back(sps);
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

void InDet::SiSpacePointsSeedMaker_LowMomentum::newRegion
(const EventContext& ctx, EventData& data,
 const std::vector<IdentifierHash>& vPixel, const std::vector<IdentifierHash>& vSCT,
 const IRoiDescriptor&) const
{
  newRegion(ctx, data, vPixel, vSCT);
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with two space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::find2Sp(EventData& data, const std::list<Trk::Vertex>& lv) const
{
  if (not data.initialized) initializeEventData(data);

  int mode = 0;
  if (lv.begin()!=lv.end()) mode = 1;
  bool newv = newVertices(data, lv);
  
  if (newv || !data.state || data.nspoint!=2 || data.mode!=mode || data.nlist) {
    data.i_seede = data.l_seeds.begin();
    data.state   = 1;
    data.nspoint = 2;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fNmin   = 0;
    data.zMin    = 0;
    production2Sp(data);
  }
  data.i_seed = data.l_seeds.begin();
  
  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with three space points with or without vertex constraint
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::find3Sp(const EventContext& ctx, EventData& data, const std::list<Trk::Vertex>& lv) const
{
  if (not data.initialized) initializeEventData(data);

  int mode = 2;
  if (lv.begin()!=lv.end()) mode = 3;
  bool newv = newVertices(data, lv);

  if (newv || !data.state || data.nspoint!=3 || data.mode!=mode || data.nlist) {
    data.i_seede = data.l_seeds.begin();
    data.state   = 1;
    data.nspoint = 3;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fNmin   = 0;
    data.zMin    = 0;
    production3Sp(ctx, data);
  }
  data.i_seed = data.l_seeds.begin();

  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

void InDet::SiSpacePointsSeedMaker_LowMomentum::find3Sp(const EventContext& ctx, EventData& data, const std::list<Trk::Vertex>& lv, const double*) const
{
  find3Sp(ctx, data, lv);
}

///////////////////////////////////////////////////////////////////
// Methods to initilize different strategies of seeds production
// with variable number space points with or without vertex constraint
// Variable means (2,3,4,....) any number space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::findVSp(const EventContext& ctx, EventData& data, const std::list<Trk::Vertex>& lv) const
{
  if (not data.initialized) initializeEventData(data);

  int mode = 5;
  if (lv.begin()!=lv.end()) mode = 6;
  bool newv = newVertices(data, lv);
  
  if (newv || !data.state || data.nspoint!=4 || data.mode!=mode || data.nlist) {
    data.i_seede = data.l_seeds.begin();
    data.state   = 1;
    data.nspoint = 4;
    data.nlist   = 0;
    data.mode    = mode;
    data.endlist = true;
    data.fNmin   = 0;
    data.zMin    = 0;
    production3Sp(ctx, data);
  }
  data.i_seed = data.l_seeds.begin();

  if (m_outputlevel<=0) {
    data.nprint=1;
    dump(data, msg(MSG::DEBUG));
  }
}

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_LowMomentum::dump(EventData& data, MsgStream& out) const
{
  if (not data.initialized) initializeEventData(data);

  if (data.nprint) return dumpEvent(data, out);
  return dumpConditions(data, out);
}

///////////////////////////////////////////////////////////////////
// Dumps conditions information into the MsgStream
///////////////////////////////////////////////////////////////////

MsgStream& InDet::SiSpacePointsSeedMaker_LowMomentum::dumpConditions(EventData& data, MsgStream& out) const
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
  out<<"| Use PRD-to-track assoc.?| "
     <<std::setw(12)<< (!m_prdToTrackMap.key().empty() ? "yes" : "no ")
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
  out<<"| pTmax  (mev)            | "
     <<std::setw(12)<<std::setprecision(5)<<m_ptmax
     <<"                              |"<<endmsg;
  out<<"| |rapidity|          <=  | " 
     <<std::setw(12)<<std::setprecision(5)<<m_rapcut
     <<"                              |"<<endmsg;
  out<<"| max radius SP           | "
     <<std::setw(12)<<std::setprecision(5)<<m_r_rmax 
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

MsgStream& InDet::SiSpacePointsSeedMaker_LowMomentum::dumpEvent(EventData& data, MsgStream& out) const
{
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  out<<"| data.ns                    | "
     <<std::setw(12)<<data.ns
     <<"                              |"<<endmsg;
  out<<"| data.nsaz                  | "
     <<std::setw(12)<<data.nsaz
     <<"                              |"<<endmsg;
  out<<"| seeds                   | "
     <<std::setw(12)<<data.l_seeds.size()
     <<"                              |"<<endmsg;
  out<<"|---------------------------------------------------------------------|"
     <<endmsg;
  return out;
}

///////////////////////////////////////////////////////////////////
// Find next set space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::findNext(const EventContext& ctx, EventData& data) const
{
  if (data.endlist) return;
  
  data.i_seede = data.l_seeds.begin();
  if (data.mode==0 || data.mode==1) {
    production2Sp(data);
  } else if (data.mode==2 || data.mode==3) {
    production3Sp(ctx, data);
  } else if (data.mode==5 || data.mode==6) {
    production3Sp(ctx, data);
  }

  data.i_seed = data.l_seeds.begin();
  ++data.nlist;
}                       

///////////////////////////////////////////////////////////////////
// New and old list vertices comparison
///////////////////////////////////////////////////////////////////

bool InDet::SiSpacePointsSeedMaker_LowMomentum::newVertices(EventData& data, const std::list<Trk::Vertex>& lV) const
{
  unsigned int s1 = data.l_vertex.size();
  unsigned int s2 = lV.size();

  if (s1==0 && s2==0) return false;

  data.l_vertex.clear();
  
  for (const Trk::Vertex& v : lV) {
    data.l_vertex.insert(static_cast<float>(v.position().z()));
  }
  return false;
}

///////////////////////////////////////////////////////////////////
// Initiate frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::buildFrameWork() 
{
  m_ptmin     = fabs(m_ptmin);
  if (m_ptmin < 50.) m_ptmin = 50.;
  m_iptmax  = 1./fabs(m_ptmax);
  m_iptmin  = 1./fabs(m_ptmin);
  m_rapcut    = fabs(m_rapcut);
  m_dzdrmax   = 1./tan(2.*atan(exp(-m_rapcut)));
  m_dzdrmin   =-m_dzdrmax;
  m_r3max     = m_r_rmax;

  // Build radius sorted containers
  //
  m_r_size = static_cast<int>((m_r_rmax+.1)/m_r_rstep);

  // Build radius-azimuthal sorted containers
  //
  constexpr float pi2 = 2.*M_PI;
  const int   NFmax = SizeRF;
  const float sFmax   = static_cast<float>(NFmax )/pi2;
  const float sFmin   = 100./60.;
  m_sF = m_ptmin /60.;
  if (m_sF > sFmax ) m_sF = sFmax;
  else if (m_sF < sFmin) m_sF = sFmin;
  m_fNmax = static_cast<int>(pi2*m_sF);
  if (m_fNmax >=NFmax) m_fNmax = NFmax-1;

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
}

///////////////////////////////////////////////////////////////////
// Initiate beam frame work for seed generator
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::buildBeamFrameWork(EventData& data) const
{ 
  SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle { m_beamSpotKey };

  const Amg::Vector3D &cb =     beamSpotHandle->beamPos();
  double     tx = tan(beamSpotHandle->beamTilt(0));
  double     ty = tan(beamSpotHandle->beamTilt(1));

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

void  InDet::SiSpacePointsSeedMaker_LowMomentum::convertToBeamFrameWork
(EventData& data, const Trk::SpacePoint*const& sp, float* r) const
{
  r[0] = static_cast<float>(sp->globalPosition().x())-data.xbeam[0];
  r[1] = static_cast<float>(sp->globalPosition().y())-data.ybeam[0];
  r[2] = static_cast<float>(sp->globalPosition().z())-data.zbeam[0];
}
   
///////////////////////////////////////////////////////////////////
// Initiate space points seed maker
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::fillLists(EventData& data) const
{
  constexpr float pi2 = 2.*M_PI;
  
  for (int i=0; i<m_r_size;  ++i) {

    if (!data.r_map[i]) continue;

    for (InDet::SiSpacePointForSeed* r  : data.r_Sorted[i]) {
      
      // Azimuthal angle sort
      //
      float F = r->phi();
      if (F<0.) F+=pi2;

      int   f = static_cast<int>(F*m_sF);
      if (f < 0) f = m_fNmax;
      else if (f > m_fNmax) f = 0;

      int z;
      float Z = r->z();

      // Azimuthal angle and Z-coordinate sort
      //
      if (Z>0.) {
        Z< 250.?z=5:Z< 450.?z=6:Z< 925.?z=7:Z< 1400.?z=8:Z< 2500.?z=9:z=10;
      } else {
        Z>-250.?z=5:Z>-450.?z=4:Z>-925.?z=3:Z>-1400.?z=2:Z>-2500.?z=1:z= 0;
      }
      int n = f*SizeZ+z;
      ++data.nsaz;
      data.rfz_Sorted[n].push_back(r);
      if (!data.rfz_map[n]++) data.rfz_index[data.nrfz++] = n;
    }
    data.r_Sorted[i].clear();
    data.r_map[i] = 0;
  }
  data.nr    = 0;
  data.state = 0;
}
 
///////////////////////////////////////////////////////////////////
// Erase space point information
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::erase(EventData& data) const
{
  for (int i=0; i!=data.nr; ++i) {
    int n = data.r_index[i];
    data.r_map[n] = 0;
    data.r_Sorted[n].clear();
  }

  for (int i=0; i!=data.nrfz; ++i) {
    int n = data.rfz_index[i];
    data.rfz_map[n] = 0;
    data.rfz_Sorted[n].clear();
  }

  data.state = 0;
  data.ns    = 0;
  data.nsaz  = 0;
  data.nr    = 0;
  data.nrfz  = 0;
}

///////////////////////////////////////////////////////////////////
// 2 space points seeds production
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::production2Sp(EventData& data) const
{
  data.endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds (new version)
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::production3Sp(const EventContext& ctx, EventData& data) const
{
  if (data.nsaz<3) return;

  float K = 0.;
  double f[3], gP[3] ={10.,10.,0.};

  MagField::AtlasFieldCache    fieldCache;

  // Get field cache object
  SG::ReadCondHandle<AtlasFieldCacheCondObj> readHandle{m_fieldCondObjInputKey, ctx};
  const AtlasFieldCacheCondObj* fieldCondObj{*readHandle};
  if (fieldCondObj == nullptr) {
    ATH_MSG_ERROR("SiSpacePointsSeedMaker_LowMomentum: Failed to retrieve AtlasFieldCacheCondObj with key " << m_fieldCondObjInputKey.key());
    return;
  }
  fieldCondObj->getInitializedCache (fieldCache);

  if (fieldCache.solenoidOn()) {
    fieldCache.getFieldZR(gP, f);

    K = 2./(300.*f[2]);
  } else {
    K = 2./(300.* 5. );
  }

  const int   ZI[SizeZ]= {5,6,7,8,9,10,4,3,2,1,0};
  std::vector<InDet::SiSpacePointForSeed*>::iterator rt[9],rte[9],rb[9],rbe[9];
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
      for (int i=0; i!=m_rfz_b[a]; ++i) {
        int an =  m_rfz_ib[a][i];
        if (!data.rfz_map[an]) continue;
        rb [NB] = data.rfz_Sorted[an].begin();
        rbe[NB++] = data.rfz_Sorted[an].end();
      } 
      for (int i=0; i!=m_rfz_t[a]; ++i) {
        int an =  m_rfz_it[a][i];
        if (!data.rfz_map[an]) continue;
        rt [NT] = data.rfz_Sorted[an].begin();
        rte[NT++] = data.rfz_Sorted[an].end();
      } 
      production3Sp(data, rb, rbe, rt, rte, NB, NT, nseed, K);
      if (!data.endlist) {
        data.fNmin=f;
        data.zMin = z;
        return;
      }
    }
  }
  data.endlist = true;
}

///////////////////////////////////////////////////////////////////
// Production 3 space points seeds (new version)
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::production3Sp
(EventData& data,
 std::vector<InDet::SiSpacePointForSeed*>::iterator* rb ,
 std::vector<InDet::SiSpacePointForSeed*>::iterator* rbe,
 std::vector<InDet::SiSpacePointForSeed*>::iterator* rt ,
 std::vector<InDet::SiSpacePointForSeed*>::iterator* rte,
 int NB, int NT, int& nseed, float K) const
{

  const float COF  = 134*.07*9.;
  const float COFP = 134*.2*9.;

  std::vector<InDet::SiSpacePointForSeed*>::iterator r0=rb[0],r;
  if (!data.endlist) {
    r0 = data.rMin;
    data.endlist = true;
  }

  // Loop through all trigger space points
  //
  for (; r0!=rbe[0]; ++r0) {
    data.nOneSeeds = 0;
    data.mapOneSeeds.erase(data.mapOneSeeds.begin(), data.mapOneSeeds.end());
 
    float R = (*r0)->radius();
    if (R<m_r2min) continue;
    if (R>m_r2max) break;

    bool pix = true;
    if ((*r0)->spacepoint->clusterList().second) pix = false;
    const Trk::Surface* sur0 = (*r0)->sur();
    float X = (*r0)->x();
    float Y = (*r0)->y();
    float Z = (*r0)->z();
    int Nb = 0;
    
    // Bottom links production
    //
    for (int i=0; i!=NB; ++i) {
      for (r=rb[i]; r!=rbe[i]; ++r) {
        float Rb =(*r)->radius();
        if (Rb<m_r1min) {
          rb[i]=r;
          continue;
        }
        if (Rb>m_r1max) break;
 
        float dR = R-Rb;
        if (dR<m_drmin) break;

        if (dR>m_drmax || (*r)->sur()==sur0) continue;

        if ( !pix && !(*r)->spacepoint->clusterList().second) continue;

        float dx = X-(*r)->x();
        float dy = Y-(*r)->y();
        float dZ = Z-(*r)->z();
        data.Tz[Nb] = dZ/sqrt(dx*dx+dy*dy);
        if (data.Tz[Nb]<m_dzdrmin || data.Tz[Nb]>m_dzdrmax) continue;
        data.Zo[Nb] = Z-R*data.Tz[Nb];

        // Comparison with vertices Z coordinates
        //
        if (!isZCompatible(data, data.Zo[Nb], Rb, data.Tz[Nb])) continue;
        data.SP[Nb] = (*r);
        if (++Nb==m_maxsizeSP) goto breakb;
      }
    }
  breakb:
    if (!Nb || Nb==m_maxsizeSP) continue;
    int Nt = Nb;
    
    // Top   links production
    //
    for (int i=0; i!=NT; ++i) {
      for (r=rt[i]; r!=rte[i]; ++r) {
        float Rt =(*r)->radius();
        float dR = Rt-R;
        if (dR<m_drmin || Rt<m_r3min) {
          rt[i]=r;
          continue;
        }
        if (Rt>m_r3max || dR>m_drmax) break;

        if ((*r)->sur()==sur0) continue;

        float dx = X-(*r)->x();
        float dy = Y-(*r)->y();
        float dZ = (*r)->z()-Z;
        data.Tz[Nt]   = dZ/sqrt(dx*dx+dy*dy);
        if (data.Tz[Nt]<m_dzdrmin || data.Tz[Nt]>m_dzdrmax) continue;
        data.Zo[Nt]   = Z-R*data.Tz[Nt];

        // Comparison with vertices Z coordinates
        //
        if (!isZCompatible(data, data.Zo[Nt], Rt, data.Tz[Nt])) continue;
        data.SP[Nt] = (*r);
        if (++Nt==m_maxsizeSP) goto breakt;
      }
    }
    
  breakt:
    if (!(Nt-Nb)) continue;

    float covr0 = (*r0)->covr ();
    float covz0 = (*r0)->covz ();
    float ax   = X/R;
    float ay   = Y/R;
    
    for (int i=0; i!=Nt; ++i) {
      float dx = data.SP[i]->x()-X;
      float dy = data.SP[i]->y()-Y;
      float dz = data.SP[i]->z()-Z;
      float x  = dx*ax+dy*ay;
      float y  =-dx*ay+dy*ax;
      float r2 = 1./(x*x+y*y);
      float dr  = sqrt(r2);

      i < Nb ?  data.Tz[i] = -dz*dr :  data.Tz[i] = dz*dr;

      data.U [i] = x*r2;
      data.V [i] = y*r2;
      data.Er[i] = (covz0+data.SP[i]->covz()+data.Tz[i]*data.Tz[i]*(covr0+data.SP[i]->covr()))*r2;
      data.R [i] = dr;
    }

    // Three space points comparison
    //
    for (int b=Nb-1; b>=0; --b) {
      float SA  = 1.+data.Tz[b]*data.Tz[b];
      for (int t=Nb;  t!=Nt; ++t) {
        float cof = COF;
        if (!data.SP[t]->spacepoint->clusterList().second) cof = COFP;

        float Ts = .5*(data.Tz[b]+data.Tz[t]);
        float dT =  data.Tz[b]-data.Tz[t];
        dT        = dT*dT-data.Er[b]-data.Er[t]-2.*data.R[b]*data.R[t]*(Ts*Ts*covr0+covz0);

        if (dT > 0. && dT > (m_iptmin*m_iptmin)*cof*SA) continue;
        float dU = data.U[t]-data.U[b];
        if (dU == 0.) continue;
        float A  = (data.V[t]-data.V[b])/dU;
        float B  =  data.V[t]-A*data.U[t];
        float S2 = 1.+A*A;
        float S  = sqrt(S2);
        float BK = fabs(B*K);
        if (BK > m_iptmin*S || BK < m_iptmax*S) continue; // Momentum    cut
        if (dT > 0. && dT  > (BK*BK/S2)*cof*SA) continue; // Polar angle cut

        float Im = fabs((A-B*R)*R);
        if (Im > m_diver) continue;

        newOneSeed(data, data.SP[b]->spacepoint,(*r0)->spacepoint,data.SP[t]->spacepoint,data.Zo[b],Im);
      }
    }
    nseed += data.mapOneSeeds.size();
    fillSeeds(data);
    if (nseed>=m_maxsize) {
      data.endlist=false;
      ++r0;
      data.rMin = r0;
      return;
    } 
  }
}

///////////////////////////////////////////////////////////////////
// New 3 space points seeds from one space points
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newOneSeed
(EventData& data,
 const Trk::SpacePoint*& p1,const Trk::SpacePoint*& p2, 
 const Trk::SpacePoint*& p3,const float& z,const float& q) const
{
  if (data.nOneSeeds < m_maxOneSize) {
    data.OneSeeds[data.nOneSeeds].erase();
    data.OneSeeds[data.nOneSeeds].add(p1);
    data.OneSeeds[data.nOneSeeds].add(p2);
    data.OneSeeds[data.nOneSeeds].add(p3);
    data.OneSeeds[data.nOneSeeds].setZVertex(static_cast<double>(z));
    data.mapOneSeeds.insert(std::make_pair(q, &(data.OneSeeds[data.nOneSeeds])));
    ++data.nOneSeeds;
  } else {
    std::multimap<float,InDet::SiSpacePointsSeed*>::reverse_iterator 
      l = data.mapOneSeeds.rbegin();
    if ((*l).first <= q) return;
    
    InDet::SiSpacePointsSeed* s = (*l).second;
    s->erase     (  );
    s->add       (p1);
    s->add       (p2);
    s->add       (p3);
    s->setZVertex(static_cast<double>(z));
    std::multimap<float,InDet::SiSpacePointsSeed*>::iterator 
      i = data.mapOneSeeds.insert(std::make_pair(q,s));
    for (++i; i!=data.mapOneSeeds.end(); ++i) {
      if ((*i).second==s) {
        data.mapOneSeeds.erase(i);
        return;
      }
    }
  }
}

const InDet::SiSpacePointsSeed* InDet::SiSpacePointsSeedMaker_LowMomentum::next(const EventContext& ctx, EventData& data) const
{
  if (not data.initialized) initializeEventData(data);

  if (data.i_seed==data.i_seede) {
    findNext(ctx, data);
    if (data.i_seed==data.i_seede) return nullptr;
  } 
  return &(*data.i_seed++);
}

bool InDet::SiSpacePointsSeedMaker_LowMomentum::isZCompatible  
(EventData& data, float& Zv, float& R, float& T) const
{
  if (Zv < m_zmin || Zv > m_zmax) return false;

  if (data.l_vertex.size()==0) return true;

  float dZmin = std::numeric_limits<float>::max();
  for (const float& v : data.l_vertex) {
    float dZ = fabs(v-Zv);
    if (dZ<dZmin) dZmin=dZ;
  }
  return dZmin < (m_dzver+m_dzdrver*R)*sqrt(1.+T*T);
}
  
///////////////////////////////////////////////////////////////////
// New space point for seeds 
///////////////////////////////////////////////////////////////////

InDet::SiSpacePointForSeed* InDet::SiSpacePointsSeedMaker_LowMomentum::newSpacePoint
(EventData& data, const Trk::SpacePoint*const& sp) const
{
  InDet::SiSpacePointForSeed* sps = nullptr;

  float r[3];
  convertToBeamFrameWork(data, sp, r);

  if (data.i_spforseed!=data.l_spforseed.end()) {
    sps = &(*data.i_spforseed++);
    sps->set(sp, r);
  } else {
    data.l_spforseed.emplace_back(InDet::SiSpacePointForSeed(sp, r));
    sps = &(data.l_spforseed.back());
    data.i_spforseed = data.l_spforseed.end();
  }
      
  return sps;
}

///////////////////////////////////////////////////////////////////
// New 2 space points seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newSeed
(EventData& data,
 const Trk::SpacePoint*& p1,const Trk::SpacePoint*& p2, 
 const float& z) const
{
  if (data.i_seede!=data.l_seeds.end()) {
    InDet::SiSpacePointsSeed* s = &(*data.i_seede++);
    s->erase();
    s->add(p1);
    s->add(p2);
    s->setZVertex(static_cast<double>(z));
  } else {
    data.l_seeds.emplace_back(InDet::SiSpacePointsSeed(p1, p2, z));
    data.i_seede = data.l_seeds.end();
  }
}

///////////////////////////////////////////////////////////////////
// New 3 space points seeds 
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::newSeed
(EventData& data,
 const Trk::SpacePoint*& p1,const Trk::SpacePoint*& p2, 
 const Trk::SpacePoint*& p3,const float& z) const
{
  if (data.i_seede!=data.l_seeds.end()) {
    InDet::SiSpacePointsSeed* s = &(*data.i_seede++);
    s->erase();
    s->add(p1);
    s->add(p2);
    s->add(p3);
    s->setZVertex(static_cast<double>(z));
  } else {
    data.l_seeds.emplace_back(InDet::SiSpacePointsSeed(p1, p2, p3, z));
    data.i_seede = data.l_seeds.end();
  }
}

///////////////////////////////////////////////////////////////////
// Fill seeds
///////////////////////////////////////////////////////////////////

void InDet::SiSpacePointsSeedMaker_LowMomentum::fillSeeds(EventData& data) const
{
  std::multimap<float,InDet::SiSpacePointsSeed*>::iterator 
    l  = data.mapOneSeeds.begin(),
    le = data.mapOneSeeds.end  ();

  for (; l!=le; ++l) {
    if (data.i_seede!=data.l_seeds.end()) {
      InDet::SiSpacePointsSeed* s = &(*data.i_seede++);
      *s = *(*l).second;
    } else {
      data.l_seeds.emplace_back(InDet::SiSpacePointsSeed(*(*l).second));
      data.i_seede = data.l_seeds.end();
    }
  }
}

void InDet::SiSpacePointsSeedMaker_LowMomentum::initializeEventData(EventData& data) const {
  data.initialize(EventData::LowMomentum,
                  m_maxsizeSP,
                  m_maxOneSize,
                  0, // maxsize not used
                  m_r_size,
                  0, // sizeRF not used
                  SizeRFZ,
                  0, // sizeRFZV not used
                  false); // checkEta not used
}
