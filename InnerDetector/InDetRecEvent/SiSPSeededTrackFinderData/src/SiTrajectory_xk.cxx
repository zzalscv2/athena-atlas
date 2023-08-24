/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "SiSPSeededTrackFinderData/SiTrajectory_xk.h"

#include <iostream>
#include <iomanip>
#include <boost/io/ios_state.hpp>

///////////////////////////////////////////////////////////////////
// Set work information to trajectory
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectory_xk::setTools(const InDet::SiTools_xk* t)
{
  m_tools = t;
  for (int i=0; i!=300; ++i) m_elements[i].setTools(t);
} 

void InDet::SiTrajectory_xk::setParameters()
{
  for (int i=0; i!=300; ++i) m_elements[i].setParameters();
} 

///////////////////////////////////////////////////////////////////
// Erase trajector element
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectory_xk::erase(int n) 
{
  if (n>=0 && n<m_nElements) {
    for (int i=n; i!=m_nElements-1; ++i) m_elementsMap[i] = m_elementsMap[i+1];
    --m_nElements;
  }
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to TrackStateOnSurface  
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToTrackStateOnSurface(int cosmic)
{
  if (!cosmic ||  m_elements[m_elementsMap[m_firstElement]].parametersUB().parameters()[2] < 0.) {
    return convertToTrackStateOnSurface();
  }
  return convertToTrackStateOnSurfaceWithNewDirection();
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to TrackStateOnSurface  with old direction
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToTrackStateOnSurface()
{

  auto dtsos = Trk::TrackStates();

  bool multi = m_tools->multiTrack();
  if (m_nclusters <= m_tools->clustersmin() ||
      pTfirst() < m_tools->pTmin()) multi = false;
 
  int i = m_firstElement;
  
  const Trk::TrackStateOnSurface* 
    tsos = m_elements[m_elementsMap[i]].trackStateOnSurface(false,true,multi,1);

  if (tsos) dtsos.push_back(tsos);

  for (++i; i!=m_lastElement; ++i) {

    int m = m_elementsMap[i];
    if (m_elements[m].cluster() || m_elements[m].clusterNoAdd() ) {
      tsos = m_elements[m].trackStateOnSurface(false,false,multi,0);
      if (tsos) dtsos.push_back(tsos);
    }
  }

  i = m_lastElement;
  tsos = m_elements[m_elementsMap[i]].trackStateOnSurface(false,false,multi,2);
  if (tsos) dtsos.push_back(tsos);

  if (multi) {
    m_ntos = 0;
    for (int i=m_firstElement; i<=m_lastElement; ++i) {
      
      int m = m_elementsMap[i];
      if (!m_elements[m].ntsos()) continue;
      m_atos[m_ntos  ] = m;
      m_itos[m_ntos++] = 0;
    }
  }
  return dtsos;
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to TrackStateOnSurface  with new direction
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToTrackStateOnSurfaceWithNewDirection()
{

  auto dtsos = Trk::TrackStates();

  bool multi = m_tools->multiTrack();
  if (pTfirst() < m_tools->pTmin()) multi = false;

  int i = m_lastElement;

  const Trk::TrackStateOnSurface* 
    tsos = m_elements[m_elementsMap[i]].trackStateOnSurface(true,true,multi,2);

  if (tsos) dtsos.push_back(tsos);

  for (--i; i!=m_firstElement; --i) {

    int m = m_elementsMap[i];
    if (m_elements[m].cluster() || m_elements[m].clusterNoAdd() ) {
      tsos = m_elements[m].trackStateOnSurface(true,false,multi,0);
      if (tsos) dtsos.push_back(tsos);
    }
  }

  i = m_firstElement;
  tsos = m_elements[m_elementsMap[i]].trackStateOnSurface(true,false,multi,1);
  if (tsos) dtsos.push_back(tsos);

  return dtsos;
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to simple TrackStateOnSurface   
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToSimpleTrackStateOnSurface(int cosmic, const EventContext& ctx)
{
  if (!cosmic ||  m_elements[m_elementsMap[m_firstElement]].parametersUB().parameters()[2] < 0.) {
    return convertToSimpleTrackStateOnSurface(ctx);
  }
  return convertToSimpleTrackStateOnSurfaceWithNewDirection();
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to simple TrackStateOnSurface  with old direction
///////////////////////////////////////////////////////////////////

Trk::TrackStates 
InDet::SiTrajectory_xk::convertToSimpleTrackStateOnSurface(const EventContext& ctx)
{
  auto dtsos = Trk::TrackStates();

  int i = m_firstElement;
  
  const Trk::TrackStateOnSurface* 
    tsos = m_elements[m_elementsMap[i]].trackPerigeeStateOnSurface(ctx);

  if (tsos) dtsos.push_back(tsos);
  
  tsos = m_elements[m_elementsMap[i]].trackSimpleStateOnSurface(false,false,m_tools->useFastTracking());

  if (tsos) dtsos.push_back(tsos);
  
  int lastClusterElement = 0;
  for (int j=m_lastElement; j>=i; j--) {
     int m = m_elementsMap[j];
     if (m_elements[m].cluster()) { 
	lastClusterElement = j;
	break;
     }
  }
  if( lastClusterElement==0 || lastClusterElement==i ) return dtsos;

  for (++i; i<std::min(lastClusterElement,m_lastElement); ++i) {
    
    int m = m_elementsMap[i];
    if (m_elements[m].cluster()) {
      tsos = m_elements[m].trackSimpleStateOnSurface(false,false,0);
      if (tsos) dtsos.push_back(tsos);
    }
  }

  i = std::min(lastClusterElement,m_lastElement);
  tsos = m_elements[m_elementsMap[i]].trackSimpleStateOnSurface(false,true,2);
  if (tsos) dtsos.push_back(tsos);

  return dtsos;
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to simple TrackStateOnSurface with new direction
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToSimpleTrackStateOnSurfaceWithNewDirection()
{
  auto dtsos = Trk::TrackStates();

  int i = m_lastElement;

  const Trk::TrackStateOnSurface* 
    tsos = m_elements[m_elementsMap[i]].trackSimpleStateOnSurface(true,true,2);

  if (tsos) dtsos.push_back(tsos);

  for (--i; i!=m_firstElement; --i) {

    int m = m_elementsMap[i];
    if (m_elements[m].cluster() || m_elements[m].clusterNoAdd() ) {
      tsos = m_elements[m].trackSimpleStateOnSurface(true,false,0);
      if (tsos) dtsos.push_back(tsos);
    }
  }

  i = m_firstElement;
  tsos = m_elements[m_elementsMap[i]].trackSimpleStateOnSurface(true,false,1);
  if (tsos) dtsos.push_back(tsos);

  return dtsos;
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to simple TrackStateOnSurface   
// Only for Disappearing Track Trigger that uses also failed tracks
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToSimpleTrackStateOnSurfaceForDisTrackTrigger(int cosmic, const EventContext& ctx)
{
  if (!cosmic ||  m_elements[m_elementsMap[m_firstElement]].parametersUB().parameters()[2] < 0.) {
    return convertToSimpleTrackStateOnSurfaceForDisTrackTrigger(ctx);
  }
  return convertToSimpleTrackStateOnSurfaceWithNewDirection();
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to simple TrackStateOnSurface  with old direction
// Only for Disappearing Track Trigger that uses also failed tracks
///////////////////////////////////////////////////////////////////

Trk::TrackStates 
InDet::SiTrajectory_xk::convertToSimpleTrackStateOnSurfaceForDisTrackTrigger(const EventContext& ctx)
{
  auto dtsos = Trk::TrackStates();

  int i = m_firstElement;
  
  const Trk::TrackStateOnSurface* 
    tsos = m_elements[m_elementsMap[i]].trackPerigeeStateOnSurface(ctx);

  if (tsos) dtsos.push_back(tsos);
  
  tsos = m_elements[m_elementsMap[i]].trackSimpleStateOnSurface(false,false,0);

  if (tsos) dtsos.push_back(tsos);
  
  int lastClusterElement = 0;
  for (int j=m_lastElement; j>=i; j--) {
     int m = m_elementsMap[j];
     if (m_elements[m].cluster()) { 
	lastClusterElement = j;
	break;
     }
  }
  if( lastClusterElement==0 || lastClusterElement==i ) return dtsos;

  for (++i; i<std::min(lastClusterElement,m_lastElement); ++i) {
    
    int m = m_elementsMap[i];
    if (m_elements[m].cluster()) {
      tsos = m_elements[m].trackSimpleStateOnSurface(false,false,0);
      if (tsos) dtsos.push_back(tsos);
    }
  }

  i = std::min(lastClusterElement,m_lastElement);
  tsos = m_elements[m_elementsMap[i]].trackSimpleStateOnSurface(false,true,2);
  if (tsos) dtsos.push_back(tsos);

  return dtsos;
}

///////////////////////////////////////////////////////////////////
// FitQuality production
///////////////////////////////////////////////////////////////////

std::unique_ptr<Trk::FitQuality> InDet::SiTrajectory_xk::convertToFitQuality() const{
  double xi2 = m_elements[m_elementsMap[m_firstElement]].xi2totalB();
  return std::make_unique<Trk::FitQuality>(xi2, (m_ndf - 5));
}

///////////////////////////////////////////////////////////////////
// Test is it new track
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::isNewTrack
(std::multimap<const Trk::PrepRawData*,const Trk::Track*>& map) const
{
  const Trk::PrepRawData* prd   [100];
  std::multimap<const Trk::PrepRawData*,const Trk::Track*>::const_iterator 
    ti,t[100],te = map.end();
  int n = 0 ;
  if (m_firstElement==-100) return false;//i.e. the int array never had elements inserted
  for (int i=m_firstElement; i<=m_lastElement; ++i) {
    int m = m_elementsMap[i];
    
    if (m_elements[m].cluster()) {
      prd[n] = m_elements[m].cluster();
      t[n] = map.find(prd[n]);
      if (t[n]==te) return true;
      ++n;
    } else if (m_elements[m].clusterNoAdd()) {
      prd[n] = m_elements[m].clusterNoAdd();
      t[n] = map.find(prd[n]);
      if (t[n]==te) return true;
      ++n;
    }
  }

  int nclt = m_nclusters + m_nclustersNoAdd;
  
  for (int i=0; i!=n; ++i) {
    int nclmax = 0;
    for (ti=t[i]; ti!=te; ++ti) {
      if ( (*ti).first != prd[i] ) break;
      int ncl = (*ti).second->measurementsOnTrack()->size();
      if (ncl > nclmax) nclmax = ncl;
    }   
    if (nclt > nclmax) return true;
  }
  return false;
}

///////////////////////////////////////////////////////////////////
// Overload of << operator std::ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::operator << 
(std::ostream& sl,const InDet::SiTrajectory_xk& se)
{ 
  return se.dump(sl);
}   

///////////////////////////////////////////////////////////////////
// Dumps relevant information into the ostream
///////////////////////////////////////////////////////////////////

std::ostream& InDet::SiTrajectory_xk::dump( std::ostream& out ) const
{
  boost::io::ios_all_saver ias(out);
  
  if (m_nElements <=0 ) {
    out<<"Trajectory does not exist"<<std::endl; ias.restore();
    return out;
  }
  if (m_firstElement >= m_lastElement ) {
    out<<"Trajectory is wrong"<<std::endl; ias.restore();
    return out;
  }

  out<<"|--------------------------------------------------------------------------------------------------------|"
     <<std::endl;
  out<<"|                       TRAJECTORY "
     <<"                                                                      |"
     <<std::endl;

  out<<"| Has"<<std::setw(3)<<m_nElements
     <<" ("
     <<std::setw(3)<<m_nActiveElements
     <<")"
     <<" elements and "
     <<std::setw(2)<<m_nclusters+m_nclustersNoAdd<<" ("
     <<std::setw(2)<<m_nclustersNoAdd<<") clusters and "
     <<std::setw(2)<<m_ndf<<" weighted clusters and quality = "<<std::setw(12)<<std::setprecision(5)<<quality()
     <<"         |"
     <<std::endl;
  out<<"| Has number of holes before, inside, after and gap= "
     <<std::setw(2)<<m_nHolesBefore
     <<std::setw(2)<<m_nholes
     <<std::setw(2)<<m_nHolesAfter
     <<std::setw(3)<<m_dholes<<"                                           |"
     <<std::endl;
  out<<"|                                                                                        F   B           |"
     <<std::endl;

  out<<"|---|--|---|-----|-|-|---------|---------|---------|---------|----------|---------|-|--|--|--|-|-|-|-|-|-|---------|"
     <<std::endl;
  out<<"| # |SS|  D| Ncl |C|O|   Xi2F  |   Xi2B  | Az.Angle| Radius  |  pT(GeV) |  dZ/dR  |N|In|Lf|Lb|S|D|H|G|H|G|   Step  |"
     <<std::endl;
  out<<"|---|--|---|-----|-|-|---------|---------|---------|---------|----------|---------|-|--|--|--|-|-|-|-|-|-|---------|"
     <<std::endl;

  for (int i=0; i!=m_nElements; ++i) {
    
    int m = m_elementsMap[i];

    std::string DET = "D ";
    const InDetDD::SiDetectorElement* D = m_elements[m].detElement();
    std::string DE = " ";
    if (m_elements[m].detstatus() < 0) DE = "-";
    if (D) {
      if (D->isPixel()) {
        if (D->isBarrel()) DET = "Pb"; else DET = "Pe";
      }
      else if (D->isSCT()) {
        if (D->isBarrel()) DET = "Sb"; else DET = "Se";
      }
    }
    int c = 0;
    if (m_elements[m].detstatus() > 0) c = m_elements[m].numberClusters();

    out<<"|"<<std::setw(3)<<unsigned(i);

    std::string S0="  ";
    if (m_firstElement == i) S0="=>";
    if (m_lastElement  == i) S0="=>";
 
    std::string S1=" ";
    std::string S2=" ";
    if (m_elements[m].cluster     ()) S1="+";
    if (m_elements[m].clusterNoAdd()) S2="+";

    out<<"|"
       <<S0<<"|"
       <<std::setw(1)<<DE
       <<std::setw(2)<<DET        <<"|"
       <<std::setw(5)<<c          <<"|"
       <<S1<<"|"
       <<S2<<"|";

    if (m_elements[m].status()) {

      out<<std::setw(9)<<std::setprecision(3)<<m_elements[m].xi2F()<<"|";
      out<<std::setw(9)<<std::setprecision(3)<<m_elements[m].xi2B()<<"|";

      double ra = 0.;
      double pt = 0.;
      double tz = 0.;
      double fa = 0.;

      if (m_elements[m].status()==1) {

        if (m_elements[m].cluster()) {

          Amg::Vector3D gp = m_elements[m].parametersUF().position();
          ra = sqrt(gp.x()*gp.x()+gp.y()*gp.y());
          fa = atan2(gp.y(),gp.x());
          pt = m_elements[m].parametersUF().pT      ();
          tz = m_elements[m].parametersUF().cotTheta();
        }
        else {

          Amg::Vector3D gp = m_elements[m].parametersPF().position();
          ra = sqrt(gp.x()*gp.x()+gp.y()*gp.y());
          fa = atan2(gp.y(),gp.x());
          pt = m_elements[m].parametersPF().pT      ();
          tz = m_elements[m].parametersPF().cotTheta();
        }
      }
      else if ((m_tools->useFastTracking() and m_elements[m].status()>2) or m_elements[m].status()==2) {

        if (m_elements[m].cluster()) {

          Amg::Vector3D gp = m_elements[m].parametersUB().position();
          ra = sqrt(gp.x()*gp.x()+gp.y()*gp.y());
          fa = atan2(gp.y(),gp.x());
          pt = m_elements[m].parametersUB().pT      ();
          tz = m_elements[m].parametersUB().cotTheta();
        }
        else {

          Amg::Vector3D gp = m_elements[m].parametersPB().position();
          ra = sqrt(gp.x()*gp.x()+gp.y()*gp.y());
          fa = atan2(gp.y(),gp.x());
          pt = m_elements[m].parametersPB().pT      ();
          tz = m_elements[m].parametersPB().cotTheta();
        }
      }
      else {

        Trk::PatternTrackParameters S1,SM,S2(m_elements[m].parametersPF());
 
        if (m_elements[m].cluster()) S1 = m_elements[m].parametersUB();
        else                        S1 = m_elements[m].parametersPB();

        bool QA = m_tools->updatorTool()->combineStates(S1,S2,SM);

        if (QA) {

          Amg::Vector3D gp = SM.position();
          ra = sqrt(gp.x()*gp.x()+gp.y()*gp.y());
          fa = atan2(gp.y(),gp.x());
          pt = SM.pT      ();
          tz = SM.cotTheta();
        }
      }
      out<<std::setw( 9)<<std::setprecision(4)<<fa     <<"|";
      out<<std::setw( 9)<<std::setprecision(4)<<ra     <<"|";
      out<<std::setw(10)<<std::setprecision(4)<<pt*.001<<"|";
      out<<std::setw( 9)<<std::setprecision(4)<<tz     <<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].noiseModel())<<"|";
      out<<std::setw(2)<<m_elements[m].inside()<<"|";
      out<<std::setw(2)<<unsigned(m_elements[m].nlinksF())<<"|";
      out<<std::setw(2)<<unsigned(m_elements[m].nlinksB())<<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].status())<<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].difference())<<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].nholesF())<<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].dholesF())<<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].nholesB())<<"|";
      out<<std::setw(1)<<unsigned(m_elements[m].dholesB())<<"|";
      out<<std::setw(9)<<std::setprecision(4)<<m_elements[m].step()<<"|";
    }
    else                       {
      out<<"         |";
      out<<"         |";
      out<<"         |";
      out<<"         |";
      out<<"          |";
      out<<"         |";
      out<<" |";
      out<<"  |";
      out<<"  |";
      out<<"  |";
      out<<" |";
      out<<" |";
      out<<" |";
      out<<" |";
      out<<" |";
      out<<" |";
      out<<std::setw(9)<<std::setprecision(4)<<m_elements[m].step();
      out<<"|";
    }
    out<<std::endl;
  }
  out<<"|---|--|---|-----|-|-|---------|---------|---------|---------|----------|---------|-|--|--|--|-|-|-|-|-|-|---------|"
     <<std::endl;
  ias.restore();
  return out;
}   

///////////////////////////////////////////////////////////////////
// pT seed estimation
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectory_xk::pTseed
 (const Trk::TrackParameters                            & Tp,
  std::vector<const InDet::SiCluster*>                    & Cl,
  std::vector<const InDet::SiDetElementBoundaryLink_xk*>& DE,
  const EventContext                                    & ctx)
{
  double Xi2cut     =  30.;

  InDet::SiClusterCollection::const_iterator  sib,sie;
  std::vector<const InDet::SiDetElementBoundaryLink_xk*>::iterator r=DE.begin(),re=DE.end();
  std::vector<const InDet::SiCluster*>                    ::iterator s=Cl.begin();

  int n = 0;
  if(!m_elements[n].set(1,(*r),sib,sie,(*s),ctx) ) return 0.;
  if(!m_elements[n].firstTrajectorElement(Tp,ctx)) return 0.;

  for(++r; r!=re; ++r) {
    ++n; ++s;
    if(!m_elements[n].set(1,(*r),sib,sie,(*s),ctx)                    ) return 0.;
    if(!m_elements[n].ForwardPropagationWithoutSearch(m_elements[n-1], ctx)) return 0.;
    if( m_elements[n].xi2F()      >      Xi2cut                       ) return 0.;
  }
  return m_elements[n].parametersUF().pT();
}


///////////////////////////////////////////////////////////////////
// Initiate trajectory
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::initialize
(bool PIX,
 bool SCT,
 const InDet::PixelClusterContainer*                  PIXc       ,
 const InDet::SCT_ClusterContainer*                   SCTc       ,
 const Trk::TrackParameters                          & Tp        ,
 std::vector<const InDet::SiCluster*>                  & lSiCluster, 
 std::vector<const InDet::SiDetElementBoundaryLink_xk*>& DE      ,
 bool                                                & rquality  ,
 const EventContext                                  & ctx       )
{
  /// reset state
  m_nholes          =    0;
  m_nHolesBefore         =    0;
  m_nHolesAfter         =    0;
  m_dholes          =    0;
  m_nclusters       =    0;
  m_nclustersNoAdd  =    0;
  m_nElements       =    0;
  m_nActiveElements      =    0;
  m_firstElement    = -100;
  m_lastElement     =    0;
  m_ndfcut          =    0;
  rquality          = true;
  m_ntos            =    0;
  int    ndfwrong   =    0;
  double Xi2cut     = 2.*m_tools->xi2max();

  // radius of the dead cylinder
  double Rdead      = 142.5;
  // boolean to decide if initialisation is needed or not
  // initDeadMaterial is False (which means dead material needs be initialised)
  // for ITk fast tracking configuration
  bool initDeadMaterial = not(m_tools->isITkGeometry() and m_tools->useFastTracking());

  if(!initDeadMaterial and !m_surfacedead) m_surfacedead = std::make_unique<const Trk::CylinderSurface>(Rdead,5000.);

  std::vector<const InDet::SiCluster*>::iterator iter_cluster;
  if (lSiCluster.size() < 2) return false;

  std::vector<const InDet::SiDetElementBoundaryLink_xk*>::iterator iter_boundaryLink,endBoundaryLinks=DE.end();

  int up    = 0;
  int last  = 0;

  /// loop over all detector elements and assign the starting set of silicon clusters 
  /// at the right indices 
  for (iter_boundaryLink=DE.begin(); iter_boundaryLink!=endBoundaryLinks; ++iter_boundaryLink) {

    /// get the associated detector element from the boundary link 
    const InDetDD::SiDetectorElement* detectorElement = (*iter_boundaryLink)->detElement();
    IdentifierHash           id = detectorElement->identifyHash();

    /// book a placeholder for a potential cluster on this element
    const InDet::SiCluster* theCluster = nullptr;

    /// First case: Current element is a pixel module
    if (detectorElement->isPixel()) {
      if (PIX) {  /// check if we are configured to use pixels! 

	// Set dead material
	//
	// if already initialised, not doing it again
        if(not initDeadMaterial) {
          const Trk::PlaneSurface* pla = static_cast<const Trk::PlaneSurface*>(&detectorElement->surface());
          double R = pla->center().perp();
          if(R > Rdead) {
            initDeadMaterial = true;
            if(!m_elements[m_nElements].setDead(m_surfacedead.get())) return false;
            m_elementsMap[m_nElements] = m_nElements;
            if(m_nclusters && !lSiCluster.empty()) {
              if(!m_elements[m_nElements].ForwardPropagationWithoutSearch(m_elements[up], ctx)) return false;
              up = m_nElements;
            }
            if(++m_nElements==300) break;
          }
        }

        InDet::PixelClusterCollection::const_iterator iter_PixelClusterColl, iter_PixelClusterCollEnd;
        /// check for the pixel clusters on the given detector element, using the ID hash 
        const InDet::PixelClusterCollection *clustersOnElement = (*PIXc).indexFindPtr(id);
        /// if we have any hits: 
        if (clustersOnElement!=nullptr && clustersOnElement->begin()!=clustersOnElement->end()) {

          /// set iterators to the local cluster collection
          iter_PixelClusterColl = clustersOnElement->begin();
          iter_PixelClusterCollEnd = clustersOnElement->end();
          
          /// Loop over the passed list with Si clusters to initiate the track with - these 
          /// are for example the clusters on our seed in inside-out- or backtracking. 
          for (iter_cluster=lSiCluster.begin(); iter_cluster!=lSiCluster.end(); ++iter_cluster) {
            ///if this cluster is on the current element... 
            if ((*iter_cluster)->detectorElement()==detectorElement) {
              /// if it is the first cluster we see, set the first element to the current index
              if (m_nclusters==0){ 
                m_firstElement = m_nElements;
              }
              /// otherwise, set the last element to the current index (will eventually point to the final cluster)
              else{
                m_lastElement  = m_nElements;
              }
              /// increment cluster counter
              ++m_nclusters;
              /// add 2 to the number of degrees of freedom counter (Pix is 2D)
              m_ndfcut+=2;
              /// set our cluster pointer to point to this cluster 
              theCluster=(*iter_cluster);
              /// remove the cluster from the list 
              iter_cluster=lSiCluster.erase(iter_cluster);
              /// and exit the loop over Si clusters. We can do this 
              /// because no two clusters are allowed to be on the same
              /// detector element 
              break;
            }
          }
          /// done, now we know if one of the existing clusters is on this element
          /// set status = 1 (there are hits on this module), give it the boundary link and the space points on this element. Finally, also give it the cluster, if we found one. 
          bool valid_set = m_elements[m_nElements].set(1,(*iter_boundaryLink),iter_PixelClusterColl,iter_PixelClusterCollEnd,theCluster,ctx);
          if(m_tools->isITkGeometry() && !valid_set) return false;
          /// and increment the counter of active (nonzero hits) elements
          ++m_nActiveElements;
        } 
        /// this branch is the case of a pixel module with no hits on it, if we have previously had an active element 
        else if (m_nActiveElements) {
          /// here we set a status of 0. 
          bool valid_set = m_elements[m_nElements].set(0,(*iter_boundaryLink),iter_PixelClusterColl,iter_PixelClusterCollEnd,theCluster,ctx);
          if(m_tools->isITkGeometry() && !valid_set) return false;
        } 
        /// this branch is taken if we have not yet found an active element and there are no hits on this module. No need to already start the trajectory! 
        else {

          continue;
        }
        /// map the index to itself. Always useful. Don't worry, it will get 
        /// more interesting later... 
        m_elementsMap[m_nElements] = m_nElements;
        /// if we exceed the bounds of our array, bail out. 
        /// Also increment the detector element index
        if (++m_nElements==300) break;
      }   /// end of check on pixel flag
    }   /// end of pixel case
    /// case 2: Strip module 
    else if (SCT) {
      InDet::SCT_ClusterCollection::const_iterator iter_stripClusterColl, iter_StripClusterCollEnd;
      /// again, we fetch the clusters for this detector element
      const InDet::SCT_ClusterCollection *clustersOnElement = (*SCTc).indexFindPtr(id);

      /// do we have any? 
      if (clustersOnElement!=nullptr && clustersOnElement->begin()!=clustersOnElement->end()) {

        iter_stripClusterColl = clustersOnElement->begin();
        iter_StripClusterCollEnd = clustersOnElement->end();

        /// 
        for (iter_cluster=lSiCluster.begin(); iter_cluster!=lSiCluster.end(); ++iter_cluster) {
          if ((*iter_cluster)->detectorElement()==detectorElement) {
            /// is this the first cluster we found? 
            if (m_nclusters==0){
              /// then mark it as the first element 
              m_firstElement = m_nElements;
            }
            /// otherwise, mark it as the last element 
            else{
              m_lastElement  = m_nElements;
            }
            /// increment cluster counter 
            ++m_nclusters;
            /// for SCT, add one degree of freedom (1D measurement) 
            m_ndfcut+=1;
            /// and update the cluster pointer, before cleaning up 
            theCluster=(*iter_cluster);
            iter_cluster=lSiCluster.erase(iter_cluster);
            /// remember - only one cluster per detector element is possible due to 
            /// upstream filtering. So we can exit when we found one. 
            break;
          }
        }
        /// Now, set up the trajectory element (det status 1) as in the pixel case  
        bool valid_set = m_elements[m_nElements].set(1,(*iter_boundaryLink),iter_stripClusterColl,iter_StripClusterCollEnd,theCluster,ctx);
        if(m_tools->isITkGeometry() && !valid_set) return false;
        /// and increment the active element count 
        ++m_nActiveElements;
      } 
      /// branch if no clusters on module and previously seen active element 
      else if (m_nActiveElements) {
        /// set an corresponding element to detstatus = 0  
        bool valid_set = m_elements[m_nElements].set(0,(*iter_boundaryLink),iter_stripClusterColl,iter_StripClusterCollEnd,theCluster,ctx);
        if(m_tools->isITkGeometry() && !valid_set) return false;
      }
      /// branch for no clusters and no active elements seen so far  
      else {
        /// skip this one
        continue;
      }
      /// update elements map 
      m_elementsMap[m_nElements] = m_nElements;
      /// and array boundary checking (yuck!!)
      /// Also increment the detector element index
      if (++m_nElements==300) break;
    } /// end of SCT if-statement    

    /// if the element we are currently processing is the first one where we saw a cluster: 
    if (m_firstElement == m_nElements-1) {
      up = m_nElements-1;         /// set the upper populated point to the current index 
      if (!m_elements[up].firstTrajectorElement(Tp, ctx)) return false;  /// and update the current trajectory     
                                                                    /// element with the track parameters 
                                                                    /// we obtained upstream for 
                                                                    /// the starting surface
    }
    /// if the element we are currently processing saw a cluster but is not the first one  
    else if (theCluster) {

      /// run forward propagation from the last element with a cluster to this one 
      if (!m_elements[m_nElements-1].ForwardPropagationWithoutSearch(m_elements[up],ctx)) {
        return false;
      }
      /// update index of last element that had a cluster to point to this one 
      up = m_nElements-1;
      /// if the chi2 looks good for the forward extension step, update index of last good cluster
      if (m_elements[m_nElements-1].xi2F() <= Xi2cut) {
        last=up;
      } 
      /// if it does not look good
      else {
        if (m_tools->isITkGeometry()) return false;
        else{
          /// add the ndf to "ndfwrong"
          ndfwrong+=m_elements[m_nElements-1].ndf();
          /// if we have collected more than 3 badly fitting DoF (2 pix or 1 Pix + 1 SCT or 3 SCT), bail out
          if (ndfwrong > 3) return false;
          /// reduce ndf for cut by the bad degrees of freedom
          m_ndfcut-=m_elements[m_nElements-1].ndf();
          /// reduce cluster count, don't include this track
          --m_nclusters;
          /// and erase the cluster again
          m_elements[m_nElements-1].eraseClusterForwardPropagation();
        }
      }
    }
    /// Case if we already have clusters from the seed on track, 
    /// and some clusters left to put on it, but no seed cluster on this element 
    /// Corresponds to have a hole in the seed. 
    else if (m_nclusters && !lSiCluster.empty()) {
      /// propagate to the current DE from the last one where we had a cluster from the seed
      /// if the propagation fails
      if (!m_elements[m_nElements-1].ForwardPropagationWithoutSearch(m_elements[up], ctx)) {
        /// if we have a cluster here, something went wrong in the upstream logic 
        if(not m_tools->isITkGeometry() and m_elements[m_nElements-1].cluster()) return false;
        /// otherwise, remove this guy from consideration. 
        /// It will be overwritten by the next one we see
        --m_nElements;
        /// also adapt the number of active elements if needed
        if (m_elements[m_nElements-1].detstatus()) --m_nActiveElements;
      } 
      /// if the propagation succeeded 
      else {
        /// increment hole count if we expect a hit 
        if (m_elements[m_nElements-1].inside()<0) ++m_nholes;
        /// update the index of the last element
        up = m_nElements-1;
      }
    } /// end of case of no hit and expecting a hit
  } /// end of loop over boundary links

  /// did we manage to assign all our seed hits to elements on the road? 
  if (!lSiCluster.empty()) {
    /// no? Then our search road was badly chosen. Return this info for logging and bail out 
    rquality = false;
    return false;
  }
  /// if some seed hits are badly fitting and we have less than 6 remaining good DoF, 
  /// we give up 
  if (not m_tools->isITkGeometry() && ndfwrong && m_ndfcut < 6) return false;

  /// update degrees of freedom to the current count of good ones
  m_ndf = m_ndfcut;
  /// truncate the ndfcut variable to 6 
  if (m_tools->isITkGeometry() || m_ndfcut > 6) m_ndfcut = 6;

  /// Kill empty trajectory elements at the end of our trajectory
  int n = m_nElements-1;
  /// count downwards 
  for (; n>0; --n) {
    /// and if the element is active, stop looping
    if (m_elements[n].detstatus()>=0) break;
  }
  /// the index where we aborted is the last one where we 
  /// found an active element. 
  m_nElements = n+1;
  
  /// Find last detector element with clusters
  for (; n>0; --n) {
    if (m_elements[n].detstatus() == 1) {
      m_elements[n].lastActive();
      break;
    }
  }

  /// Kill uncrossed detector elements
  /// this repopulates the elementsMap we started to fill earlier
  int m         = m_firstElement+1;
  m_lastElement = last            ;
  /// loop from the element after the one with the first hit to the one 
  /// with the last hit from the seed
  for (n = m; n!=m_lastElement; ++n) {
    InDet::SiTrajectoryElement_xk& En = m_elements[m_elementsMap[n]];
    /// if we either have a cluster on track or at least cross the element, 
    /// plug it into the m_elementsMap at the next position
    if (En.cluster() || En.inside() <= 0) m_elementsMap[m++] = m_elementsMap[n];
  }
  /// now m_elementsMap contains the interesting elements

  /// if we kicked out some elements: 
  if (m!=n) {
    /// update the index of the last element
    m_lastElement = m;
    /// then add the remaining ones beyond the last cluster back to the map
    for (; n!=m_nElements; ++n){
       m_elementsMap[m++] = m_elementsMap[n];
    }
    m_nElements = m;
  }
  if (!m_tools->bremNoise()) return true;

  /// If we run with brem, update noise model for last trajectory elements
  for (n=m_lastElement; n!=m_nElements; ++n) {
    m_elements[m_elementsMap[n]].bremNoiseModel();
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Seacrh cluster compatible with track parameters
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::trackParametersToClusters
(const InDet::PixelClusterContainer*                       PIXc      ,
 const InDet::SCT_ClusterContainer*                        SCTc      ,
 const Trk::TrackParameters                              & Tp        ,
 std::vector<const InDet::SiDetElementBoundaryLink_xk*>    & DE      ,
 std::multimap<const Trk::PrepRawData*,const Trk::Track*>& PT        ,
 std::vector<const InDet::SiCluster*>                      & lSiCluster, 
 const EventContext& ctx)
{
  m_nElements = 0;
  m_ndf       = 0;

  std::multimap<double,const InDet::SiCluster*> xi2cluster;

  std::vector<const InDet::SiDetElementBoundaryLink_xk*>::iterator iter_boundaryLink,endBoundaryLinks=DE.end();
  std::multimap<const Trk::PrepRawData*,const Trk::Track*>::const_iterator t, te =PT.end();

  double xi2Cut = .5;
  int    ndfCut =  6;

  for (iter_boundaryLink=DE.begin(); iter_boundaryLink!=endBoundaryLinks; ++iter_boundaryLink) {

    const InDetDD::SiDetectorElement* detectorElement = (*iter_boundaryLink)->detElement();
    IdentifierHash id = detectorElement->identifyHash();

    bool sct = detectorElement->isSCT();

    if (!sct) {
      InDet::PixelClusterCollection::const_iterator sib, sie;
      const InDet::PixelClusterCollection *w = (*PIXc).indexFindPtr(id);

      if (w!=nullptr && w->begin()!=w->end()) {
        sib = w->begin();
        sie = w->end  ();
      } else {
        continue;
      }
      if (!m_elements[0].ForwardPropagationForClusterSeach(m_nElements,Tp,(*iter_boundaryLink),sib,sie,ctx)) return false;
    } else {
      InDet::SCT_ClusterCollection::const_iterator sib, sie;
      const InDet::SCT_ClusterCollection *w = (*SCTc).indexFindPtr(id);

      if (w!=nullptr && w->begin()!=w->end()) {
        sib = w->begin();
        sie = w->end  ();
      } else {
        continue;
      }
      if (!m_elements[0].ForwardPropagationForClusterSeach(m_nElements,Tp,(*iter_boundaryLink),sib,sie,ctx)) return false;
    }

    for (int i=0; i!=m_elements[0].nlinksF(); ++i) {
    
      double x = m_elements[0].linkF(i).xi2();

      if (sct) {
        t = PT.find(m_elements[0].linkF(i).cluster());
        if (t!=te && (*t).second->measurementsOnTrack()->size() >= 10) continue;
      } else {
        x*=.5;
      }
 
      if (x <= xi2Cut) xi2cluster.insert(std::make_pair(x,m_elements[0].linkF(i).cluster()));
      break;
    }
    ++m_nElements;
  }
  
  if (xi2cluster.size() < 3) return false;

  std::multimap<double,const InDet::SiCluster*>::iterator xc = xi2cluster.begin(), xce = xi2cluster.end();

  for (; xc!=xce; ++xc) {
    lSiCluster.push_back((*xc).second);
    (*xc).second->detectorElement()->isSCT() ? m_ndf+=1 : m_ndf+=2;
    if ( m_ndf >= ndfCut ) break;
  }

  return m_ndf >= 6;
}

///////////////////////////////////////////////////////////////////
// Seacrh cluster compatible with global positions
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::globalPositionsToClusters
(const InDet::PixelClusterContainer*                       PIXc      ,
 const InDet::SCT_ClusterContainer*                        SCTc      ,
 const std::vector<Amg::Vector3D>                          & Gp        ,
 std::vector<const InDet::SiDetElementBoundaryLink_xk*>    & DE        ,
 std::multimap<const Trk::PrepRawData*,const Trk::Track*>& PT        ,
 std::vector<const InDet::SiCluster*>                      & lSiCluster)
{
  std::vector<const InDet::SiDetElementBoundaryLink_xk*>::iterator iter_boundaryLink = DE.begin(), endBoundaryLinks = DE.end();
  std::vector<Amg::Vector3D>::const_iterator g,gb = Gp.begin(), ge = Gp.end();
  InDet::PixelClusterCollection::const_iterator pib, pie;
  InDet::SCT_ClusterCollection::const_iterator sib, sie;
  std::multimap<const Trk::PrepRawData*,const Trk::Track*>::const_iterator t, te =PT.end();

  Trk::PatternTrackParameters Tp;
  
  double pv[ 5]={0.,0.,0.,0.,0.};
  double cv[15]={ .1 ,
                  0. , .1,
                  0. , 0.,.001,
                  0. , 0.,  0.,.001,
                  0. , 0.,  0.,  0.,.00001};

  double xi2Cut = 10.;
  m_ndf         = 0  ;

  for (; iter_boundaryLink!=endBoundaryLinks; ++iter_boundaryLink) {

    const InDetDD::SiDetectorElement* d  = (*iter_boundaryLink)->detElement();
    IdentifierHash                    id = d->identifyHash ();
    const Trk::Surface*               su = &d->surface();
    const Trk::PlaneSurface* pla = static_cast<const Trk::PlaneSurface*>(su);
    if (!pla) continue;

    const Amg::Transform3D&  tr = pla->transform();
    double Ax[3] = {tr(0,0),tr(1,0),tr(2,0)};
    double Ay[3] = {tr(0,1),tr(1,1),tr(2,1)};
    double Az[3] = {tr(0,2),tr(1,2),tr(2,2)};
    double x0    = tr(0,3);
    double y0    = tr(1,3);
    double z0    = tr(2,3);
    double zcut  = .001  ;
    
    bool sct = d->isSCT();
    if (!sct) {
      const InDet::PixelClusterCollection *w = (*PIXc).indexFindPtr(id);
      if (w!=nullptr && w->begin()!=w->end()) {
        pib = w->begin();
        pie = w->end  ();
      } else {
        continue;
      }
    } else {
      zcut = 1.;
      const InDet::SCT_ClusterCollection *w = (*SCTc).indexFindPtr(id);
      if (w!=nullptr && w->begin()!=w->end()) {
        sib = w->begin();
        sie = w->end  ();
      } else {
        continue;
      }
    }

    for (g=gb; g!=ge; ++g) {
      
      double dx = (*g).x()-x0;
      double dy = (*g).y()-y0;
      double dz = (*g).z()-z0;
      double z  = dx*Az[0]+dy*Az[1]+dz*Az[2];
      if (std::abs(z) > zcut) continue;

      pv[0]     = dx*Ax[0]+dy*Ax[1]+dz*Ax[2];
      pv[1]     = dx*Ay[0]+dy*Ay[1]+dz*Ay[2];

      Tp.setParametersWithCovariance(su,pv,cv);

      if (!sct) m_elements[0].CloseClusterSeach(Tp, (*iter_boundaryLink), pib, pie);
      else      m_elements[0].CloseClusterSeach(Tp, (*iter_boundaryLink), sib, sie);
      const InDet::SiCluster* c =  m_elements[0].cluster();
      if (!c || m_elements[0].xi2F() > xi2Cut) continue;
      if (sct) {
        t = PT.find(c);
        if (t!=te && (*t).second->measurementsOnTrack()->size() >= 10) continue;
      }
      sct ? m_ndf+=1 : m_ndf+=2;
      lSiCluster.push_back(c);
    }
  }
  return m_ndf >= 6;
}

///////////////////////////////////////////////////////////////////
// Backward test initial trajectory
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::backwardSmoother(bool TWO, const EventContext& ctx)
{
  if (m_firstElement >= m_lastElement) return false;

  // Trajectory difference test
  //
  int m = m_lastElement;
  for (; m>=m_firstElement; --m) {
    if (m_elements[m_elementsMap[m]].difference()) break;
  }
  if (m < m_firstElement) return true;

  if (!m_elements[m_elementsMap[m_lastElement]].lastTrajectorElement()) return false;
  
  int firstElement = m_lastElement       ;
  int maxholes     = m_tools->maxholes ();
  int maxdholes    = m_tools->maxdholes();
  m_nclustersNoAdd = 0                   ;
  m_difference     = 0                   ;

  m     = m_lastElement-1;
  int n = m              ;

  for (; m>=m_firstElement; --m) {

    InDet::SiTrajectoryElement_xk& En = m_elements[m_elementsMap[m+1]];
    InDet::SiTrajectoryElement_xk& Em = m_elements[m_elementsMap[m  ]];

    if (!Em.BackwardPropagationSmoother(En,TWO,ctx)) {
      
      if (m == m_firstElement) break;

      for (int i=m+1; i!=m_nElements; ++i) m_elementsMap[i-1] = m_elementsMap[i];
      --m_lastElement;
      --m_nElements;
      --firstElement;
      continue;
    }

    if ((Em.cluster() && Em.clusterOld()) && (Em.cluster()!=Em.clusterOld())) ++m_difference;

    if     (Em.cluster()) {
      firstElement  = m;
    }
    else                  {
      n=m;
      if (Em.clusterNoAdd()) ++m_nclustersNoAdd;
      if (Em.nholesB() > maxholes || Em.dholesB() > maxdholes) {
        ++m_difference; break;
      }
    }
  } 

  m_firstElement = firstElement                       ;
  m              = m_elementsMap[firstElement]        ;
  n              = m_elementsMap[     n      ]        ;
  m_nclusters    = m_elements[m].nclustersB()         ;
  m_nholes       = m_elements[m].nholesB   ()         ;
  m_nHolesBefore      = m_elements[n].nholesB   ()-m_nholes;
  m_ndf          = m_elements[m].ndfB()               ;
  if (m_ndf < m_ndfcut) return false;

  // Erase trajectory elements with big distance from the track
  //
  m = firstElement+1;
  n = m;
  for (; m!=m_lastElement; ++m) {

    InDet::SiTrajectoryElement_xk& Em = m_elements[m_elementsMap[m]];
    if (Em.inside() > 0) {
      if (Em.detstatus() > 0) --m_nActiveElements;
    }
    else                 {
      m_elementsMap[n++] = m_elementsMap[m];
    }
  }
  m_lastElement = n;

  // Test number trajector elemenst with clusters
  //
  if (m_nActiveElements < m_tools->clustersmin() && m_nholes+m_nHolesAfter) return false;
  if (n!=m) {
    for (; m!=m_nElements; ++m) m_elementsMap[n++] = m_elementsMap[m];
    m_nElements = n;
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Backward trajectory extension
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::backwardExtension(int itmax, const EventContext& ctx)
{
  if (m_firstElement >= m_lastElement) return false;
  int L = m_firstElement;
  if (L==0) return true;

  int                     MPbest[300]         ;
  int                     TE    [100]         ;
  const InDet::SiCluster* CL    [100]         ;
  double                  XI2B  [100]         ;
  Trk::PatternTrackParameters PUB[100]        ;
  Trk::PatternTrackParameters  PA             ;
 
  int    maxholes       = m_tools->maxholes ();
  int    maxdholes      = m_tools->maxdholes();
  const int itm         = itmax-1             ;
  int    it             = 0                   ;
  int    itbest         = 0                   ;
  int    qbest          =-100                 ;
  int    nbest          = 0                   ;
  int    ndfbest        = 0                   ;
  int    lbest          = L                   ;
  int    hbest          = 0                   ;
  int    hbestb         = 0                   ;
  int    nclbest        = 0                   ;
  int    ndcut          = 3                   ;
  int    F              = L                   ;
  double Xi2best        = 0.                  ;

  m_elements[m_elementsMap[F]].setNdist(0);

  for (; it!=itmax; ++it) {

    int  l = F;
    int  lastElementWithExpHit = F;

    for (--F; F>=0; --F) {
      
      InDet::SiTrajectoryElement_xk& El = m_elements[m_elementsMap[F+1]];
      InDet::SiTrajectoryElement_xk& Ef = m_elements[m_elementsMap[F  ]];

      if (!Ef.BackwardPropagationFilter(El, ctx))  break;
      
      if (Ef.cluster()) {
        lastElementWithExpHit = F;
        l = F;
      }
      else if (Ef.inside() < 0) {
        lastElementWithExpHit = F;
        if (Ef.nholesB() > maxholes || Ef.dholesB() > maxdholes) break;
      }

      int nm = Ef.nclustersB()+F;
      if (Ef.ndist() >  ndcut ||
	  nm < nclbest ||
	  (nm == nclbest && Ef.xi2totalB() > Xi2best) ) break;
    } 

    int    fl = F;
    if (fl<0) fl = 0;

    int    m  = m_elementsMap[l];
    int    nc = m_elements[m].nclustersB();

    if (it==0 && nc==m_nclusters) return true;

    int    np = m_elements[m].npixelsB();
    int    nh = m_elements[m].nholesB();
    int    nd = m_elements[m_elementsMap[fl]].ndist();
    int    q  = nc-nh;
    double X  = m_elements[m].xi2totalB();

    if     ( (q > qbest) || (q==qbest && X < Xi2best ) ) {

      qbest   = q                                          ;
      nbest   = 0                                          ;
      ndfbest = 0                                          ;
      hbest   = nh                                         ;
      hbestb  = m_elements[m_elementsMap[lastElementWithExpHit]].nholesB()-nh  ;
      itbest  = it                                         ;
      Xi2best = X                                          ;
      PA      = m_elements[m_elementsMap[l]].parametersUB();
 
      if (fl==0 && nd < ndcut) ndcut = nd;

      if (fl!=0 || nd > 0 || np < 3) {

        lbest = l-1;
        for (int i=l; i!=L; ++i) {

          InDet::SiTrajectoryElement_xk& Ei = m_elements[m_elementsMap[i]];
   
          if (Ei.inside() <= 0 ) {
            MPbest[++lbest] = i;
            if (Ei.cluster()) {
	      CL[nbest]   = Ei.cluster();
	      XI2B[nbest] = Ei.xi2B();
	      PUB[nbest]  = Ei.parametersUB();
	      TE[nbest++] = lbest;
	      ndfbest += Ei.ndf();
	    }
          }
        }
      }
      else    {

        l     =   -1;
        lbest = fl-1;
        for (int i=fl; i!=L; ++i) {

          InDet::SiTrajectoryElement_xk& Ei = m_elements[m_elementsMap[i]];

          if (Ei.inside() <= 0 && ++lbest >=0 ) {
            MPbest[lbest] = lbest;
            if (Ei.cluster()) {
	      CL[nbest]   = Ei.cluster();
	      XI2B[nbest] = Ei.xi2B();
	      PUB[nbest]  = Ei.parametersUB();
	      TE[nbest++] = lbest;
	      ndfbest += Ei.ndf();
	      if (l<0) l=lbest;
            }
            m_elementsMap[lbest] = m_elementsMap[i];
          }
   
        }

        int dn = L-1-lbest;

        if (dn!=0) {

          for (int i=L; i!= m_nElements; ++i) {
            m_elementsMap[i-dn]=m_elementsMap[i];
          }

          L            -=dn;
          m_nElements  -=dn;
          m_lastElement-=dn;
        }
      }
      nclbest = m_nclusters+nbest;
    }
    
    F = -1;
    if (l<=0) l=1;
    bool cl = false;
    double Xn = 0.;

    for (; l < L; ++l) {
      InDet::SiTrajectoryElement_xk& Ei = m_elements[m_elementsMap[l]];

      if (Ei.cluster() && Ei.isNextClusterHoleB(cl,Xn))  {
        int nm = l+Ei.nclustersB();
        if (!cl) {
	  if (Ei.dist() < -2. && Ei.ndist() > ndcut-1 ) continue;
	  --nm;
	}
        if (nm < nclbest || (nm == nclbest && Xn > Xi2best)) continue;
        F=l; break;
      }
    }

    if (F < 0 ) break;
    if (it!=itm) if (!m_elements[m_elementsMap[F]].addNextClusterB()) break;
  }
  if (it == itmax) --it;
  if (!nbest) return true;

  m_nholes       = hbest ;
  m_nHolesBefore      = hbestb;
  m_nclusters   += nbest ;
  m_ndf         += ndfbest;
  m_firstElement = TE[0] ;
  m_elements[m_elementsMap[TE[0]]].setParametersB(PA);

  int dn = L-1-lbest;
 
  if (dn != 0) {

    m_nElements  -=dn;
    m_lastElement-=dn;

    int n = m_firstElement;
    for (; n <= lbest     ; ++n) m_elementsMap[n]=m_elementsMap[MPbest[n]];
    for (; n!= m_nElements; ++n) m_elementsMap[n]=m_elementsMap[n    +dn ];
  }

  if (itbest==it) return true;
  
  for (int n = L-1-dn; n>=0; --n) {

    InDet::SiTrajectoryElement_xk& En = m_elements[m_elementsMap[n]];
    int m = nbest-1;
    for (; m>=0; --m) if (TE[m]==n) break;

    if (m>=0) {
      if (m_tools->useFastTracking()) {
	En.setClusterB(CL[m],XI2B[m]);
	En.setParametersB(PUB[m]);
      }
      else En.setCluster(CL[m]);
      if (--nbest==0) break;
    }
    else     {
      if (m_tools->useFastTracking()) En.setClusterB(  nullptr  ,10000.);
      En.setCluster(  nullptr  );
    }
  } 
  return true;
}

///////////////////////////////////////////////////////////////////
// Forward trajectory extension
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::forwardExtension(bool smoother,int itmax, const EventContext& ctx)
{
  const double pi2 = 2.*M_PI;
  const double pi = M_PI;
  
  if (m_firstElement >= m_lastElement) return false;

  /// index at which we start the extension 
  int extensionStartIndex = m_lastElement;
  /// last element on the trajectory
  int lastElementOnTraj = m_nElements-1;

  /// smoothing step, if requested.
  /// Will re-evaluate the extension start and parameters there
  if (smoother) {

    extensionStartIndex = m_firstElement;

    /// This checks if we already ran a forward and backward propagation within the current 
    /// hits on the trajectory. Will enter this branch if we have not. 
    if (m_elements[m_elementsMap[extensionStartIndex]].difference()) {
      /// in this case, we will simply set up for forward propagation, projecting the predicted 
      /// state forward without searching for new hits yet 

      /// prepare first element for forward propagation
      if (!m_elements[m_elementsMap[extensionStartIndex]].firstTrajectorElement(false)) return false;
      /// for all following elements with until the last with a known hit: 
      for (++extensionStartIndex; extensionStartIndex<=m_lastElement; ++extensionStartIndex) {
        InDet::SiTrajectoryElement_xk& previousElement = m_elements[m_elementsMap[extensionStartIndex-1]];
        InDet::SiTrajectoryElement_xk& thisElement = m_elements[m_elementsMap[extensionStartIndex  ]];
        /// propagate forward the state from the previous element
        if (!thisElement.ForwardPropagationWithoutSearch(previousElement,ctx)) return false;
      }
    }
    /// this branch is entered if the first element is already in agreement between forward and back
    /// so we already ran a smoothing step for the current hits on the trajectory
    else{
      bool diff = false;
      /// for all elements starting from the one after our first hit, up to the last one with a hit: 
      for (++extensionStartIndex; extensionStartIndex<=m_lastElement; ++extensionStartIndex) {
 
        InDet::SiTrajectoryElement_xk& previousElement = m_elements[m_elementsMap[extensionStartIndex-1]];
        InDet::SiTrajectoryElement_xk& thisElement = m_elements[m_elementsMap[extensionStartIndex  ]];
        /// this branch is entered while the range of hits we are looking at was still covered 
        /// by the previously made backward smoothing 
        if (!diff) {
          /// update flag if we are still covered by the smoothing
          diff = thisElement.difference();
          /// if not (first element not covered):  
          if (diff) {
            /// re-add cluster - this will update the counters and chi2 based on the refined upstream elements
            if (!thisElement.addNextClusterF(previousElement,thisElement.cluster())) return false;
          }
        }
        /// case if we have entered a piece of the tracks where there was no back smoothing yet
        else      {
          /// propagate forward
          if (!thisElement.ForwardPropagationWithoutSearch(previousElement,ctx)) return false;
        }
      } /// end loop from second to last element with hit 
    } /// end case of existing smoothing
    --extensionStartIndex;  /// decrement extensionStartIndex to get back to the last element with a hit on it
  } /// end smoother 

  /// if we are already at the last element on the trajectory, we have no-where to extend further
  if ( extensionStartIndex== lastElementOnTraj) return true;

  // Search best forward trajectory prolongation
  /// These represent the indices used for the extension. Only contains 
  /// the elements beyond the start (so not the piece already existing). 
  int                     MP    [300]                              ;
  int                     MPbest[300]                              ;
  /// Trajectory elements of clusters on the best candidate
  int                     TE    [100]                              ;
  /// Clusters on the best candidate
  const InDet::SiCluster* CL    [100]                              ;

  /// hole and double-hole cuts
  int    maxholes       = m_tools->maxholes ()                     ;
  int    maxdholes      = m_tools->maxdholes()                     ;
  /// max iteration we expect to reach
  const int itm         = itmax-1                                  ;
  /// current iteration
  int    iteration      = 0                                        ;
  /// index of best iteration so far
  int    itbest         = 0                                        ;
  /// best quality seen so far
  int    qbest          =-100                                      ;
  /// number of hits on the best extension
  int    nbest          = 0                                        ;
  int    ndfbest        = 0                                        ;
  /// holes for best extension (only up to last hit)
  int    hbest          = 0                                        ;
  /// holes beyond last hit for best extension
  int    hbeste         = 0                                        ;
  /// number of missing expected hits on best extension (holes + deads)
  int    ndbest         = 0                                        ;
  /// max holes plus deads
  int    ndcut          = 3                                        ;
  /// best number of clusters
  int    nclbest        = 0                                        ;
  /// start index for best extension
  int    lbest          = extensionStartIndex                      ;
  /// index for looping over detector elements
  int    index_currentElement              = extensionStartIndex   ;
  /// current index in the MP array - runs along all elements on the candidate track
  int    M              = extensionStartIndex                      ;
  /// first point of current trajectory: start of extension
  MP    [M]             = extensionStartIndex                      ;
  double Xi2best        = 0.                                       ;
  /// maximum dPhi
  const double dfmax    = 2.2                                      ;

  /// this is the at the location of the first hit on track
  double f0             = m_elements[m_elementsMap[m_firstElement]].parametersUF().parameters()[2];


  m_elements[m_elementsMap[index_currentElement]].setNdist(0);

  /// start to iterate 
  for (; iteration!=itmax; ++iteration) {

    int  lastElementWithSeenHit = index_currentElement;
    int  lastElementWithExpHit  = index_currentElement;
    /// index of the previous detector element in the forward propagation 
    int  index_previousElement  = index_currentElement;
    /// Last index in the M array where we found a cluster 
    int  mLastCluster = M;
    /// missing clusters on the best candidate compared to the maximum possible 
    int  Cm = nclbest-lastElementOnTraj;
    /// hole flag
    bool haveHole = false;

    /// loop over detector elements - the first time this is called, we start at the first 
    /// one beyond the one that saw the last hit 
    for (++index_currentElement; index_currentElement!=m_nElements; ++index_currentElement) {
      
      /// 
      InDet::SiTrajectoryElement_xk& prevElement    = m_elements[m_elementsMap[index_previousElement]];
      InDet::SiTrajectoryElement_xk& currentElement = m_elements[m_elementsMap[index_currentElement ]];

      /// propagate forward to the current element, and search for matching clusters
      if (!currentElement.ForwardPropagationWithSearch(prevElement, ctx)) {
        /// In case of a forward propagation error, try to recover cases due to barrel-endcap transitions 

        /// we are already in the endcaps, or if the incompatible elements the failure are not subsequent: abooooort!  
        if (!currentElement.isBarrel() || index_previousElement!=index_currentElement-1) break;

        /// in the barrel or if the previous element is the one just before this, loop over the remaining elements
        /// and look for the first one not in the barrel. 
        /// This detects barrel-endcap transitions
        int index_auxElement = index_currentElement;
        for (; index_auxElement!=m_nElements; ++index_auxElement) {
          if (!m_elements[m_elementsMap[index_auxElement]].isBarrel() ) break;
        }
        /// we didn't find any endcap elements? Likely a bad attempt, so abort 
        if (index_auxElement==m_nElements) break;

        /// If we found a barrel-endcap transition, 
        /// we continue our loop at the first endcap element 
        /// along the trajectory. 
        index_currentElement = index_auxElement-1;
        continue;
      } /// end of propagation failure handling 

      /// NOTE: The 'else' here is mainly for clarity - due to the 'continue'/'break' statements
      /// in the 'if' above, we only reach here if the condition above (propagation failure) is not met. 
      else {    
        /// if the forward propagation was succesful: 
        /// update the 'previous element' to the current one 
        index_previousElement = index_currentElement;
      }  
      /// Reminder: we only reach this point in case of a succesful forward propagation
      
      /// add the current element to the MP list, increment M index
      MP[++M] = index_currentElement;

      /// if we found a matching cluster at this element: 
      if (currentElement.cluster()) {
        if (not m_tools->isITkGeometry()) {
          /// get the dPhi w.r.t the first hit
          double df = std::abs(currentElement.parametersUF().parameters()[2]-f0);
          /// wrap into 0...pi space
          if (df > pi) df = pi2-df;
          /// and bail out if the track is curving too extremely in phi
          if (df > dfmax) break;
        }
        /// update indices for last hit 
        lastElementWithExpHit  = index_currentElement;
        lastElementWithSeenHit = index_currentElement;
        mLastCluster = M;
        haveHole = false;
      }

      /// we did not find a compatible hit, but we would expect one --> hole! 
      else if (currentElement.inside() < 0  ) {
        lastElementWithExpHit=index_currentElement;
        /// check if we exceeded the maximum number of holes or double holes. If yes, abort! 
        if (currentElement.nholesF() > maxholes || currentElement.dholesF() > maxdholes) break;

        haveHole = true;
        if (not m_tools->isITkGeometry()) {
          /// and do the dPhi check again, but using the predicted parameters (as no hit)
          double df = std::abs(currentElement.parametersPF().parameters()[2]-f0);
          if (df > pi) df = pi2-df;
          if (df > dfmax ) break;
        }
      }

      /// we did not find a hit, but also do not cross the active surface (no hit expected)
      else if (not m_tools->isITkGeometry()) {
        /// apply only the dPhi check 
        double df = std::abs(currentElement.parametersPF().parameters()[2]-f0);
        if (df > pi) df = pi2-df;
        if (df > dfmax) break;
      }
      /// number of missing clusters so far (for whatever reason) 
      /// note: negative number (0 == all possible collected)
      int nm = currentElement.nclustersF()-index_currentElement;
      /// Now, check a number of reasons to exit early: 
      if (     currentElement.ndist() > ndcut                       /// if we have too many deads
           ||  nm < Cm                                              /// or we have  already missed more hits than in the best so far
           || (nm == Cm && currentElement.xi2totalF() > Xi2best)    /// or we have missed the same #hits but have a worse chi2
          ) break;
    } /// end of loop over elements

    /// now we have assembled a full candidate for the extended trajectory. 

    /// get the index of the element where we last saw a hit 
    int    m  = m_elementsMap[lastElementWithSeenHit];
    /// get the number of clusters we saw at that point 
    int    nc = m_elements[m].nclustersF();
    /// and the number of holes (this means we exclude holes after the last hit!) 
    int    nh = m_elements[m].nholesF();
    /// set the number of holes after the last hit - total holes minus holes until last hit
    m_nHolesAfter = m_elements[m_elementsMap[lastElementWithExpHit]].nholesF()-nh;

    /// if we are in the first iteration, and have not found any new clusters 
    /// beyond the ones already collected in initialize(), return. 
    if (iteration==0 && nc==m_nclusters) return true;

    /// get the last element we reached before exiting the loop
    int    lastElementProcessed = index_currentElement;
    /// if we looped over all, make it the last element there is 
    if (lastElementProcessed==m_nElements) --lastElementProcessed;

    /// number of missed expected hits seen 
    int    nd =  m_elements[m_elementsMap[lastElementProcessed]].ndist();
    /// clusters minus holes before last hit
    /// This is used as a quality estimator
    int    q  = nc-nh;
    /// total chi2 
    double X  = m_elements[m].xi2totalF();
    /// if the quality is better than the current best (chi2 as tie-breaker): 
    if     ( (q > qbest) || (q==qbest && X < Xi2best ) ) {
      /// update the quality flags
      qbest   = q;
      ///reset the running index of clusters on best track (will now be populated) 
      nbest   = 0;
      /// and the degree-of-freedom count
      ndfbest = 0;
      /// update best candidate hole counts
      hbest   = nh;
      /// as well as holes after last hit 
      hbeste  = m_elements[m_elementsMap[lastElementWithExpHit]].nholesF()-nh;
      /// store the iteration number...
      itbest  = iteration;
      /// extensionStartIndex-value used for the best candidate 
      /// (starting point)
      lbest   = extensionStartIndex ;
      /// the chi2... 
      Xi2best = X ;
      /// the number of missed expected hits
      ndbest  = nd;
      /// if our track went through the full trajectory and we collect a small number 
      /// of  missed expected hits: update the cut for the future (need to be better)
      if (lastElementProcessed==lastElementOnTraj && nd < ndcut) ndcut = nd;

      /// now, loop over the elements on this possible extension again, up to the last cluster
      for (int j=extensionStartIndex+1; j<=mLastCluster; ++j) {
        /// get the corresponding element 
        int    i  = MP[j];
        InDet::SiTrajectoryElement_xk& Ei = m_elements[m_elementsMap[i]];
        /// if we are inside or within tolerance
        if (Ei.inside() <= 0) {
          /// then store this index in MPbest (meaning we skip any element that isn't crossed)  
          MPbest[++lbest] = i;
          /// if we have a hit here, 
          if (Ei.cluster()) {
            /// store the cluster and l-index 
            CL[nbest]   = Ei.cluster();
            TE[nbest++] = lbest;
            /// and increment NDF count
            ndfbest += Ei.ndf();
          }
        }
      }
      /// best total cluster count: clusters we had so far + newly found ones 
      nclbest = m_nclusters+nbest;
      /// if we have an amazing track with at least 14(!) clusters and no holes, or if we passed along the full trajectory 
      /// without any missed expected hits, we stop iterating, as we can not improve
      if ( (nclbest >= 14 && !haveHole) || (lastElementProcessed==lastElementOnTraj && ndbest == 0)) break;
    } /// end of branch for quality better than current best

    /// set index_currentElement negative (use as iteration exit condition below)
    index_currentElement = -1;
    /// boolean to check if we have two clusters on a given element 
    bool cl = false;
    /// helper number - used below
    int nb = lastElementOnTraj-nclbest-1;
    /// chi square holder
    double Xn;

    /// Now, we prepare for the next iteration. 
    /// This is done by walking backward along the found trajectory and seeing if, by not using one cluster,
    /// we could theoretically come to a better solution if doing so would give us hits everywhere
    /// else along the remaining
    /// trajectory. The first such case is used to start the next iteration. 
    
    /// Start reverse loop  
    for (int j=mLastCluster; j!=extensionStartIndex; --j) {
      /// corresponding element 
      int i = MP[j];
      InDet::SiTrajectoryElement_xk& Ei = m_elements[m_elementsMap[i]];

      /// if we are not on the last element, have a hit, and removing the hit would not make us fail the cuts: 
      if (i!=lastElementOnTraj && Ei.cluster() && Ei.isNextClusterHoleF(cl,Xn)) {

        /// this is Ei.nclustersF() + [steps from lastElementOnTraj to i] - nclbest - 1
        /// ==> change in number of hits that would be possible w.r.t the best track 
        /// if ALL following elements had hits, if we removed this hit (to get a better extension) 
        int nm = nb-i+Ei.nclustersF();

        /// if removing this hit would result in us losing a hit on track (no backup on hand): 
        if (!cl) {
          /// if we would gain a missed expected hit by removing this one and fail the missing hit cut due to this,
          /// keep iterating and don't do anything here
          if (Ei.dist() < -2. && Ei.ndist() > ndcut-1) continue;
        }
        /// if we would keep a hit even if we remove this cluster, increment the counter to account for this
        else  ++nm;
        
        /// if we can not possibly improve (nm < 0) or if we can not improve and the chi2 can not 
        /// go below the best one, keep looping, look for another cluster to pull out
        if (nm < 0 || (nm == 0 &&   Xn > Xi2best)) continue;
        /// otherwise, start the next iteration here! 
        index_currentElement = i;
        M = j;
        break;
      }
    } /// end of reverse loop 

    /// if we did not find any candidate where we could improve by removing a hit, stop iteration 
    if (index_currentElement < 0 ) break;
    /// if we are not in the last iteration, final preparation for iterating by making our new start 
    /// point skip the cluster used there so far. 
    if (iteration!=itm && !m_elements[m_elementsMap[index_currentElement]].addNextClusterF()) break;
  }

  /// if reaching max iterations, reset iteration counter to the last iteration that was run 
  if (iteration == itmax) --iteration;

  /// exit if no good track found 
  if (!nbest) return true;

  /// set members to counters of best track 
  m_nholes      = hbest      ;
  m_nHolesAfter     = hbeste     ;
  m_nclusters  += nbest      ;
  m_ndf        += ndfbest    ;
  m_lastElement = TE[nbest-1];
  m_nElements   = m_lastElement+1;

  /// if the indices do not match, there is an element along the way which we do not traverse on the sensitive area
  if (m_lastElement != MPbest[m_lastElement]) {
    /// update the element map, removing unused elements 
    for (int n = extensionStartIndex+1; n<=m_lastElement; ++n){
       m_elementsMap[n]=m_elementsMap[MPbest[n]];
    }
  }
  /// if the last iteration was the best, all is good and we return 
  if (itbest==iteration) return true;

  /// 
  int mb =  0;
  /// reset index_currentElement again
  index_currentElement      = -1;

  /// loop over all detector elements beyond the start point
  for (int n = extensionStartIndex+1; n!=m_nElements; ++n) {
    /// get the corresponding element 
    InDet::SiTrajectoryElement_xk& En = m_elements[m_elementsMap[n]];

    int m = mb;
    /// find the index in the hits-on-track array corresponding to this DE 
    for (; m!=nbest; ++m) if (TE[m]==n) break;
    /// if we found this element in the array: 
    if (m!=nbest) {
      /// if we have since switched to a different cluster on this element compared to the one in the best track: 
      if (CL[m]!=En.cluster()) {
        /// if this is the time we need to update a cluster: 
        if (index_currentElement<0) {
          /// update the index
          index_currentElement=n;
          /// set the cluster to be used at this element to the one used for the best extension
          /// and propagate info from previous element 
          En.addNextClusterF(m_elements[m_elementsMap[n-1]],CL[m]);
        } 
        /// if we already updated one element: Can't use addNextClusterF directly, 
        /// need to clean up later. For now, just update the cluster info itself
        else    {     
          En.setCluster(CL[m]);
        }
      } /// end branch for different active cluster on DE
      /// exit if we have dealt with all of the clusters now
      if (++mb == nbest) break;
    }  /// end of case if we found this element in the cluster-on-track array
    /// case if a detector element is not on best trajectory 
    else {
      /// if this element has an active cluster: 
      if (En.cluster()) {
        /// if this is the first we need to update a cluster, can do the full work 
        if (index_currentElement<0) {
          index_currentElement = n;
          /// tell this DE to not use a cluster, and propagate info from prev. element
          En.addNextClusterF(m_elements[m_elementsMap[n-1]],nullptr);
        }
        /// if we already updated one element: Can't use addNextClusterF directly, 
        /// need to clean up later. For now, just update the cluster info itself
        else    {     
          /// tell this DE to not use a cluster 
          En.setCluster(nullptr);
        }
      }
    }
  }  /// end of loop over all DE beyond start point 

  /// if we did not have to update any cluster, or if we only had to 
  /// change the last hit: all good, can exit
  if (index_currentElement < 0 || m_lastElement == index_currentElement) {
    return true;
  }  

  /// otherwise, if we had to update a cluster along the way, need one more forward propagation run
  /// to make sure all the counters and chi2 etc reflect the best track's cluster configuration
  for (++index_currentElement; index_currentElement<=m_lastElement; ++index_currentElement) {
    InDet::SiTrajectoryElement_xk& prevElement = m_elements[m_elementsMap[index_currentElement-1]];
    InDet::SiTrajectoryElement_xk& currentElement = m_elements[m_elementsMap[index_currentElement  ]];
    if (!currentElement.ForwardPropagationWithoutSearch(prevElement, ctx)) return false;
  }
  /// now we can finally exit
  return true;
}

///////////////////////////////////////////////////////////////////
// Get clusters list from trajectory
///////////////////////////////////////////////////////////////////

void InDet::SiTrajectory_xk::getClusters
(std::vector<const InDet::SiCluster*>& Cl) const
{
  for (int i = m_firstElement; i<=m_lastElement; ++i) {
    int m = m_elementsMap[i];
    if (m_elements[m].cluster()) Cl.push_back(m_elements[m].cluster());
  }
}

///////////////////////////////////////////////////////////////////
// Forward filter without search
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::forwardFilter(const EventContext& ctx)
{
  int L = m_firstElement;
  
  if (!m_elements[m_elementsMap[L]].firstTrajectorElement(false)) return false;

  for (++L; L<=m_lastElement; ++L) {

    InDet::SiTrajectoryElement_xk& El = m_elements[m_elementsMap[L-1]];
    InDet::SiTrajectoryElement_xk& Ef = m_elements[m_elementsMap[L  ]];
    if (!Ef.ForwardPropagationWithoutSearch(El,ctx)) return false;
  }
  return true;
}


///////////////////////////////////////////////////////////////////
// Filter with precise clusters error
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::filterWithPreciseClustersError(const EventContext& ctx)
{
  int L       = m_firstElement;
  int I       = 0             ;

  if(!m_elements[m_elementsMap[L]].cluster()) return false;
  m_elementsMap[I] =  m_elementsMap[L];

  for(++L; L<=m_lastElement; ++L) {

    int K = m_elementsMap[L];
    if(m_elements[K].cluster() ||
       m_elements[K].clusterNoAdd() ||
       m_elements[K].inside() < 0 ) m_elementsMap[++I] = K;
  }
  m_firstElement = 0  ;
  m_lastElement  = I  ;
  m_nElements    = I+1;

  // Forward filter
  //
  L = 0;

  // firstTrajectorElement with correction = true
  if(!m_elements[m_elementsMap[L]].firstTrajectorElement(true)) return false;

  for(++L; L<=m_lastElement; ++L) {

    InDet::SiTrajectoryElement_xk& El = m_elements[m_elementsMap[L-1]];
    InDet::SiTrajectoryElement_xk& Ef = m_elements[m_elementsMap[L  ]];

    if(!Ef.ForwardPropagationWithoutSearchPreciseWithCorrection(El, ctx)) return false;
  }

  // Backward smoother
  //
  if(!m_elements[m_elementsMap[m_lastElement]].lastTrajectorElementPrecise()) return false;

  int m = m_lastElement-1;
  for(; m>=0; --m) {

    InDet::SiTrajectoryElement_xk& En = m_elements[m_elementsMap[m+1]];
    InDet::SiTrajectoryElement_xk& Em = m_elements[m_elementsMap[m  ]];

    if(!Em.BackwardPropagationPrecise(En, ctx)) return false;
  }

  return true;
}


///////////////////////////////////////////////////////////////////
// Test order of detector elements
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::goodOrder()
{
  int    L    = m_firstElement  ;
  int    n    = m_elementsMap[L];
  double step = 0.              ;
  bool  order = true            ;

  for (++L; L<=m_lastElement; ++L) {

    int m = m_elementsMap[L];
    if (!m_elements[m].cluster() &&
	!m_elements[m].clusterNoAdd() &&
	m_elements[m].inside()>=0) continue;

    double stp =  m_elements[m].step(m_elements[n]);
    if     ( step   ==   0.) step = stp  ;
    else if ((step*stp) < 0.) {order = false; break;} 
    n = m;
  }
  if (order) return true;

  L = m_firstElement ;
  n = m_elementsMap[L];
  Amg::Vector3D gp = m_elements[n].globalPosition();
  double rad = gp.x()*gp.x()+gp.y()*gp.y();
  for (++L; L<=m_lastElement; ++L) {
  
    int m = m_elementsMap[L];
    if (!m_elements[m].cluster() &&
	!m_elements[m].clusterNoAdd() &&
	m_elements[m].inside()>=0) continue;

    gp = m_elements[m].globalPosition();
    double R = gp.x()*gp.x()+gp.y()*gp.y();
    if (R < rad) return false;
    rad = R;
    n=m;
  }
  return true;
}

///////////////////////////////////////////////////////////////////
// Sort of detector elements in step order
///////////////////////////////////////////////////////////////////

void  InDet::SiTrajectory_xk::sortStep()
{
  int    L  = m_firstElement;
  int    LA = m_firstElement;

  for (++L; L<=m_lastElement; ++L) {
    
    int m = m_elementsMap[L];
    if (!m_elements[m].cluster() &&
	!m_elements[m].clusterNoAdd() &&
	m_elements[m].inside()>=0) continue;

    m_elementsMap[++LA] = m;
  }

  m_lastElement = LA;
  L = m_firstElement;
  m_nElements = LA+1;

  bool   nc = true;
  bool   so = true;
  double ds = m_elements[m_elementsMap[LA]].step()-m_elements[m_elementsMap[L]].step();
  
  if (ds > 0.) {                // Sort in increase order

    while(nc) {
      nc = false;
      int m = L, n = L+1;
      for (; n<=LA; ++n) {
 
        int Mn = m_elementsMap[n];
        int Mm = m_elementsMap[m];

        if (m_elements[Mn].step() < m_elements[Mm].step()) {
          if (m_elements[Mn].step(m_elements[Mm]) < 0.) {
            m_elementsMap[m] = Mn;
            m_elementsMap[n] = Mm;
            nc = true; so = false;
          }
        }
        ++m;
      }
    }
  }
  else {

    while(nc) {                  // Sort in decrease order
      nc = false;
      int m = L, n = L+1;
      for (; n<=LA; ++n) {
 
        int Mn = m_elementsMap[n];
        int Mm = m_elementsMap[m];
 
        if (m_elements[Mn].step() > m_elements[Mm].step()) {
          if (m_elements[Mn].step(m_elements[Mm]) > 0.) {
            m_elementsMap[m] = Mn;
            m_elementsMap[n] = Mm;
            nc = true; so = false;
          }
        }
        ++m;
      }
    }
  }
  if (so) return;

  // Search first detector elements with cluster
  //
  int n = L;
  for (; n<= LA; ++n) {
    
    int e = m_elementsMap[n];
    if     (m_elements[e].cluster()) break;
    if     (m_elements[e].clusterNoAdd()) --m_nclustersNoAdd;
    else if (m_elements[e].inside() < 0 &&
	     m_elements[e].detstatus()>=0) {--m_nholes; ++m_nHolesBefore;}
  }

  // Search last detector elements with cluster
  //
  int m = LA;
  for (; m>=n  ; --m) {

    int e = m_elementsMap[m];
    if     (m_elements[e].cluster()) break;
    if     (m_elements[e].clusterNoAdd()) --m_nclustersNoAdd;
    else if (m_elements[e].inside() < 0 &&
	     m_elements[e].detstatus()>=0) {--m_nholes; ++m_nHolesAfter;}
  }
  m_firstElement = n;
  m_lastElement  = m;

}

///////////////////////////////////////////////////////////////////
// Test possibility for trajectory to jump through perigee 
///////////////////////////////////////////////////////////////////

bool InDet::SiTrajectory_xk::jumpThroughPerigee()
{
  int i = m_firstElement;
  double St = m_elements[m_elementsMap[m_lastElement]].step()-m_elements[m_elementsMap[i]].step();

  for (; i<=m_lastElement; ++i) {
    int m = m_elementsMap[i];
    if (m_elements[m].cluster() &&
	(m_elements[m].ndf()!=2 || (m_elements[m].stepToPerigee()*St) <= 0.)) break;

    if (m_elements[m].cluster()){
      --m_nclusters;
      m_ndf -= m_elements[m].ndf();
    }
    else if (m_elements[m].clusterNoAdd()) --m_nclustersNoAdd;
    else                                   --m_nholes        ;
  }

  if (i == m_firstElement) return false;
  m_firstElement = i;
  return true;
}

///////////////////////////////////////////////////////////////////
// Trajectory quality without optimization
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectory_xk::quality() const
{
  int    holes   = 0 ;
  double quality = 0.;

  for (int i = m_firstElement; i<=m_lastElement; ++i) {
    quality+=m_elements[m_elementsMap[i]].quality(holes);
  }
  return quality;
}

///////////////////////////////////////////////////////////////////
// Trajectory quality with otimization
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectory_xk::qualityOptimization() 
{
  int    lE = m_firstElement;
  int    h  = 0             ;
  double q  = 0             ;
  double qM = 0.            ;

  for (int i = m_firstElement; i<=m_lastElement; ++i) {
    int  m = m_elementsMap[i];
    q+=m_elements[m].quality(h);
    if (m_elements[m].cluster() && q > qM) {
      qM = q;
      lE = i;
    }   
  }

  if (lE == m_firstElement) return -100;

  int fE             = lE;
  int nclustersNoAdd = 0 ;
  int nclusters      = 0 ;
  int nholes         = 0 ;
  int dholes         = 0 ;
  int ndf            = 0 ;
  h                  = 0 ;
  q                  = 0.;
  qM                 = 0.;
  
  for (int i = lE; i>=m_firstElement; --i) {

    int  m = m_elementsMap[i];
    q+=m_elements[m].quality(h);
    
    if (m_elements[m].cluster()) {

      ++nclusters;
      ndf+=m_elements[m].ndf();

      if (q > qM) {
        qM = q;
	fE = i;
        m_nclusters      = nclusters     ;
        m_nclustersNoAdd = nclustersNoAdd;
        m_nholes         = nholes        ;
        m_dholes         = dholes        ;
        m_ndf            = ndf           ;
      }

    }
    else if (m_elements[m].clusterNoAdd()) {
      ++nclustersNoAdd;
    }
    else if (m_elements[m].inside() < 0 && m_elements[m].detstatus() >=0) {
      ++nholes;
      if (h > dholes) dholes = h;
    }
  }

  if (fE==lE || m_nclusters+m_nclustersNoAdd < m_tools->clustersmin()) return -100.;
  m_firstElement = fE;
  m_lastElement  = lE;
  return qM;
}

///////////////////////////////////////////////////////////////////
// Trajectory conversion to TrackStateOnSurface for next tracks
///////////////////////////////////////////////////////////////////

Trk::TrackStates
InDet::SiTrajectory_xk::convertToNextTrackStateOnSurface()
{
  int i=0;
  for (; i!=m_ntos; ++i) {
    if (m_itos[i]+1 < m_elements[m_atos[i]].ntsos()) {++m_itos[i]; break;}
    m_itos[i] = 0;
  }
  if (i==m_ntos) {
    return Trk::TrackStates();
  }

   auto dtsos = Trk::TrackStates();

  for (i=0; i!=m_ntos; ++i) {

    Trk::TrackStateOnSurface* tsos = m_elements[m_atos[i]].tsos(m_itos[i]);
    if (tsos) dtsos.push_back(tsos);
  }
  return dtsos;
}

///////////////////////////////////////////////////////////////////
// pT of the first elementtrajectory
///////////////////////////////////////////////////////////////////

double InDet::SiTrajectory_xk::pTfirst() const
{
  int n = m_firstElement  ; if (n <0 || n>=300) return 0.;
  n     = m_elementsMap[n]; if (n <0 || n>=300) return 0.;
  int s = m_elements[n].status();
  if (s<=1) return 0.;
  return m_elements[n].parametersUB().pT();
}

/// Helper method to determine the hole search outcome for use in the later reco
void InDet::SiTrajectory_xk::updateHoleSearchResult(){

  /// instantiate an outcome object to populate
  m_patternHoleOutcome = InDet::PatternHoleSearchOutcome{0,0,0,0,0,true};

  /// counter to find subsequent SCT holes 
  bool prevIsSctHole = false;

  /// loop between the first and last hit on the trajectory
  
  for (int theEle  = m_firstElement+1; theEle<m_lastElement; ++theEle) {
    /// Now get the current element
    int m = m_elementsMap[theEle];
    InDet::SiTrajectoryElement_xk & theElement = m_elements[m];
    /// check if this is a pixel element
    bool isPix = theElement.ndf() == 2;
    bool isSCTHole=false;

    /// if we have a cluster on-track, this is neither a hole nor a dead
    /// Same goes for outliers
    if (theElement.cluster() || theElement.clusterNoAdd()) {
      prevIsSctHole = false;
      continue;
    } 
    
    /// otherwise, need to check more carefully: 
    else {
      std::unique_ptr<const Trk::TrackParameters> pars {theElement.trackParameters(true,0)};
      Trk::BoundaryCheckResult boundaryStatus = m_tools->boundaryCheckTool()->boundaryCheck(*pars);
      switch (boundaryStatus){
        /// if this is a candidate (we expect a hit and we are not on a dead sensor) 
        case Trk::BoundaryCheckResult::Candidate: 
          /// in this case, we have a hole. 
          if (isPix) {
            ++m_patternHoleOutcome.nPixelHoles;
          }
          else {
            ++m_patternHoleOutcome.nSCTHoles;
            isSCTHole=true;
          } /// end of SCT case
          break;
        /// in all other cases, apart from dead, nothing to be incremented
        case Trk::BoundaryCheckResult::Insensitive: /// fallthrough
        case Trk::BoundaryCheckResult::OnEdge:      /// fallthrough
        case Trk::BoundaryCheckResult::Outside:     /// fallthrough
        case Trk::BoundaryCheckResult::Error:       /// fallthrough
          /// in all of these cases, we have to reset the double holes
          break;
        /// if we encounter a dead element, increment the appropriate counters
        case Trk::BoundaryCheckResult::DeadElement:
          if (isPix){
            ++m_patternHoleOutcome.nPixelDeads;
          }
          else{
            ++m_patternHoleOutcome.nSCTDeads;
          }
          break;
      } /// end switch over boundary check result
    } /// end no-cluster case 

    /// if the previous element was also an SCT hole, 
    /// we now have a double hole
    if (isSCTHole && prevIsSctHole){
      prevIsSctHole = false;
      ++m_patternHoleOutcome.nSCTDoubleHoles;
    }
    else {
      prevIsSctHole = isSCTHole;
    }
  } /// end loop over elements
  if (m_patternHoleOutcome.nPixelHoles+m_patternHoleOutcome.nSCTHoles > m_tools->maxholes()
      || m_patternHoleOutcome.nSCTDoubleHoles > m_tools->maxdholes()){
     m_patternHoleOutcome.passPatternHoleCut = false;
  }
}
