/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class TrackHandle_TruthTrack            //
//                                                            //
//  Author: Thomas H. Kittelmann (Thomas.Kittelmann@cern.ch)  //
//  Initial version: March 2008                               //
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1TrackSystems/TrackHandle_TruthTrack.h"
#include "VP1TrackSystems/AscObj_TruthPoint.h"
#include "VP1Base/VP1Msg.h"

#include "AtlasHepMC/GenVertex.h"
#include "CLHEP/Vector/LorentzVector.h"
#include "TrkTrack/Track.h"
#include "AthContainers/DataVector.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkSurfaces/PlaneSurface.h"
#include "AtlasHepMC/GenParticle.h"

//____________________________________________________________________
class TrackHandle_TruthTrack::Imp {
public:
  Imp(TrackHandle_TruthTrack * tc,
      const SimBarCode& sbc,const SimHitList& shl,HepMC::ConstGenParticlePtr p)
    : theclass(tc),
      simBarCode(sbc),
      simHitList(shl),
      genParticle(p),
      ascObjVis(false),
      ascObjs(nullptr),
      trkTrack(nullptr) {}
  TrackHandle_TruthTrack * theclass;
  SimBarCode simBarCode;
  SimHitList simHitList;
  HepMC::ConstGenParticlePtr genParticle;

  bool ascObjVis;
  std::vector<AscObj_TruthPoint*> * ascObjs;
  void ensureInitAscObjs();
  const Trk::Track * trkTrack;
  void ensureInitTrkTracks();

  static Trk::Perigee * createTrkPerigeeFromProdVertex(HepMC::ConstGenParticlePtr p, const double& charge )
  {
    if (!p)
      return nullptr;//Fixme: message!
    HepMC::ConstGenVertexPtr v = p->production_vertex();
    if (!v)
      return nullptr;//Fixme: message!
    Amg::Vector3D mom(p->momentum().px(),p->momentum().py(),p->momentum().pz());
    double absmom(mom.mag());
    if (absmom<=0)
      return nullptr;//Fixme: message!
    Amg::Vector3D pos(v->position().x(),v->position().y(),v->position().z());
    return new Trk::Perigee(0.,0.,mom.phi(), mom.theta(), charge/absmom, pos);
   }

  static Trk::TrackParameters * createTrkParamFromDecayVertex(HepMC::ConstGenParticlePtr p, const double& charge )
  {
    if (!p)
      return nullptr;//Fixme: message!
    HepMC::ConstGenVertexPtr v = p->end_vertex();
    if (!v)
      return nullptr;//Fixme: message!
    Amg::Vector3D mom(p->momentum().px(),p->momentum().py(),p->momentum().pz());
//     double absmom(mom.mag());
//     if (absmom<=0)
//       return 0;//Fixme: message!
    Amg::Vector3D pos(v->position().x(),v->position().y(),v->position().z());

    Amg::Translation3D amgtranslation(pos.x(),pos.y(),pos.z());
    Amg::Transform3D amgTransf(amgtranslation * Amg::RotationMatrix3D::Identity());

    return new Trk::AtaPlane(pos,mom,charge, *(new Trk::PlaneSurface(amgTransf)));
  }

  static Trk::TrackStateOnSurface * createTSOS(Trk::TrackParameters * pars)
  {
    return pars ? new Trk::TrackStateOnSurface(
                    nullptr,
                    std::unique_ptr<Trk::TrackParameters>(pars),
                    nullptr)
                : nullptr;
  }
  static void addPars(Trk::TrackStates* dv, Trk::TrackParameters * pars)
  {
    if (!pars)
      return;
    Trk::TrackStateOnSurface * tsos = createTSOS(pars);
    if (tsos)
      dv->push_back(tsos);
  }

  void createTrack(Trk::TrackStates* trackStateOnSurfaces)
  {
    if (!trackStateOnSurfaces) {
      VP1Msg::messageDebug("TrackHandle_TruthTrack WARNING: Could not create track due to null TSOS vector");
      return;
    }
    if (trackStateOnSurfaces->empty()) {
      VP1Msg::messageDebug("TrackHandle_TruthTrack WARNING: Could not create track due to empty TSOS vector");
      delete trackStateOnSurfaces;
      return;
    }
    if (trkTrack) {
      VP1Msg::messageDebug("TrackHandle_TruthTrack ERROR: Already create trkTrack previously!");
      delete trackStateOnSurfaces;
      return;
    }

    Trk::TrackInfo ti(Trk::TrackInfo::Unknown,theclass->extrapolationParticleHypothesis());
    std::unique_ptr<Trk::TrackStates> sink(trackStateOnSurfaces);
    trkTrack = new Trk::Track(ti,
                              std::move(sink),
                              nullptr /*fitquality*/);

  }

};

//____________________________________________________________________
TrackHandle_TruthTrack::TrackHandle_TruthTrack( TrackCollHandleBase* ch,
						const SimBarCode& simBarCode,
						const SimHitList& simHitList,
						HepMC::ConstGenParticlePtr genPart )
  : TrackHandleBase(ch), m_d(new Imp(this,simBarCode,simHitList,genPart))
{
  if (VP1Msg::verbose()) {
    //Check genparticle barcode is same as in simBarCode. (and event index in parent_event())
    //Check that genparticle has production vertex.
    //Check if genparticle has end vertex, that there are no sim hits.
    //all pdg codes of simhits and genparticle should be identical.
    //all simhits should have same barcode.
    //hitTime should be rising in all simhits. NB: Remember to correct hit times with bunch crossing!!
    //fixme!
  }
}

//____________________________________________________________________
TrackHandle_TruthTrack::~TrackHandle_TruthTrack()
{
  //Fixme: delete simhitlist here?
  setAscObjsVisible(false);
  delete m_d->ascObjs;
  delete m_d->trkTrack;
  delete m_d;
}

//____________________________________________________________________
void TrackHandle_TruthTrack::Imp::ensureInitTrkTracks()
{
  if (trkTrack)
    return;

  //The GenParticle part is used if it is available with a production
  //vertex. The sim. hits are used if present and the genparticle does
  //not have an end vertex:
  bool useGenParticle = genParticle && genParticle->production_vertex();
  bool decayedGenParticle = useGenParticle && genParticle->end_vertex();
  bool useSimHits = !decayedGenParticle && !simHitList.empty();

  if (!useGenParticle&&!useSimHits) {
    VP1Msg::message("TrackHandle_TruthTrack ERROR: Track has neither a genparticle or sim. hits!!");
    return;
  }

  if (!theclass->hasCharge()) {
    VP1Msg::message("TrackHandle_TruthTrack ERROR: Could not determine particle charge (pdg="
		    +QString::number(theclass->pdgCode())+").");//Fixme: I guess we could show non-extrapolated version?
    return;
  }
  const double charge = theclass->charge();

  Trk::TrackStates* trackStateOnSurfaces = new Trk::TrackStates;

  if (useGenParticle) {
    addPars(trackStateOnSurfaces,createTrkPerigeeFromProdVertex(genParticle,charge));

    if (decayedGenParticle) {
      addPars(trackStateOnSurfaces,createTrkParamFromDecayVertex(genParticle,charge));
      createTrack(trackStateOnSurfaces);
      return;
    }
  }

  if (useSimHits) {
    //Add parameters from simhits (yes, if !useGenParticle, we get no perigee).
    SimHitList::const_iterator it, itE(simHitList.end());
    for ( it = simHitList.begin(); it != itE; ++it ) {
      //Fixme: momentum() < 0 (i.e. not present);
      //Fixme: Possibly add points for both posStart() and posEnd() (and use energy loss information to get different momenta?)
      addPars(trackStateOnSurfaces,it->second->createTrackParameters());
    }

  }

  createTrack(trackStateOnSurfaces);
}

//____________________________________________________________________
QStringList TrackHandle_TruthTrack::clicked() const
{

  QStringList l;
  l << "Truth track";
  l << TrackHandleBase::baseInfo();
  l << "Evt index = "+QString::number(m_d->simBarCode.evtIndex());
  l << "BarCode = "+QString::number(m_d->simBarCode.barCode());
  //fixme - more info
  //   l << "Truth track clicked [evt index = "+QString::number(m_d->simBarCode.second)
  //     +", barcode = "+(m_d->simBarCode.barCode<0?QString("Unknown (G4 secondary)"):QString::number(m_d->simBarCode.first))+"]";//fixme - more info
  return l;
}

//____________________________________________________________________
void TrackHandle_TruthTrack::ensureTouchedMuonChambersInitialised() const
{
}


//____________________________________________________________________
const Trk::Track * TrackHandle_TruthTrack::provide_pathInfoTrkTrack() const
{
  m_d->ensureInitTrkTracks();
  return m_d->trkTrack;
}


//____________________________________________________________________
int TrackHandle_TruthTrack::pdgCode() const
{
  return m_d->simBarCode.pdgCode();
}

//____________________________________________________________________
bool TrackHandle_TruthTrack::hasBarCodeZero() const
{
  return m_d->simBarCode.isNonUniqueSecondary();
}

//____________________________________________________________________
Amg::Vector3D TrackHandle_TruthTrack::momentum() const
{
  if (m_d->genParticle) {
    return Amg::Vector3D(m_d->genParticle->momentum().px(),m_d->genParticle->momentum().py(),m_d->genParticle->momentum().pz());
  }
  SimHitList::const_iterator it, itE(m_d->simHitList.end());
  for ( it = m_d->simHitList.begin(); it != itE; ++it ) {
    if (it->second->momentum()>=0) {
      return (it->second->momentum()) * (it->second->momentumDirection());
    }
  }
  //Unknown:
  return TrackHandleBase::momentum();
}

//____________________________________________________________________
bool TrackHandle_TruthTrack::hasVertexAtIR(const double& rmaxsq, const double& zmax) const
{
  if (!m_d->genParticle)
    return false;
  HepMC::ConstGenVertexPtr v = m_d->genParticle->production_vertex();
  if (!v)
    return false;

  double x(v->position().x()), y(v->position().y());
  if (x*x+y*y>rmaxsq)
    return false;
  return fabs(v->position().z())<=zmax;
}

//____________________________________________________________________
void TrackHandle_TruthTrack::visibleStateChanged()
{
  if (visible()&&m_d->ascObjVis&&!m_d->ascObjs)
    m_d->ensureInitAscObjs();
}

//____________________________________________________________________
void TrackHandle_TruthTrack::setAscObjsVisible(bool b)
{
  if (m_d->ascObjVis==b)
    return;
  m_d->ascObjVis=b;
//   const bool visnow = visible()&&m_d->ascObjVis;
//   const bool visbefore = visible()&&!m_d->ascObjVis;
//   if (visnow==visbefore)
//     return;
//   VP1Msg::messageVerbose("TrackHandle_TruthTrack::AscObjs visible state -> "+VP1Msg::str(b));

  if (!m_d->ascObjs) {
    if (!b||!visible())
      return;
    m_d->ensureInitAscObjs();
  }

  std::vector<AscObj_TruthPoint*>::iterator it(m_d->ascObjs->begin()), itE(m_d->ascObjs->end());
  for (;it!=itE;++it)
    (*it)->setVisible(b);
}

//____________________________________________________________________
void TrackHandle_TruthTrack::Imp::ensureInitAscObjs()
{
  if (ascObjs)
    return;
  ascObjs = new std::vector<AscObj_TruthPoint*>;
  HepMC::ConstGenVertexPtr vprod{nullptr};
  HepMC::ConstGenVertexPtr vend{nullptr};
  if (genParticle) {
   vprod=genParticle->production_vertex();
   vend=genParticle->end_vertex();
  }
  ascObjs->reserve((vprod?1:0)+(vend?1:simHitList.size()));
  if (vprod)
    ascObjs->push_back(new AscObj_TruthPoint(theclass,vprod,genParticle));

  if (vend) {
    ascObjs->push_back(new AscObj_TruthPoint(theclass,vend,genParticle));
  } else {
    SimHitList::const_iterator it, itE(simHitList.end());
    for ( it = simHitList.begin(); it != itE; ++it )
      ascObjs->push_back(new AscObj_TruthPoint(theclass,it->second));
  }
  std::vector<AscObj_TruthPoint*>::iterator it, itE(ascObjs->end());
  for (it=ascObjs->begin();it!=itE;++it)
    theclass->registerAssocObject(*it);
  for (it=ascObjs->begin();it!=itE;++it)
    (*it)->setVisible(ascObjVis);
}

//____________________________________________________________________
double TrackHandle_TruthTrack::calculateCharge() const
{
  if (!m_d->simHitList.empty()) {
    if (m_d->simHitList.at(0).second->hasCharge())
      return m_d->simHitList.at(0).second->charge();
    else
      VP1Msg::messageVerbose("TrackHandle_TruthTrack::calculateCharge() WARNING: Simhit did not have charge!");
  }

  return TrackHandleBase::calculateCharge();
}
