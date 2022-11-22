/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//
// InDetTrkInJetType.h - Description
//
/*
   Tool to classify track origins in a jet.
   Track types:
    0 -  Heavy Flavour         (Signal)
    1 -  Fragmentation tracks  (Fragment)
    2 -  Garbage    (Interactions+V0s+Pileup)
   The corresponding weights are returned as vector<float> with track type ordering.
   Multiclass TMVA is used for classification, then wgt[0]+wgt[1]+wgt[2]=1 always.
    
   The tool works (calibrated) for 35GeV<JetPt<3.5TeV.
   Jets above 3.5TeV and below 35GeV are considered as having 3.5TeV and 35GeV correspondingly.
   The tool is trained using ttbar+Z'(2.5,5TeV)+JZ4,6,8 + Gbb7000 samples
   The tool uses trkPt vs JetAxis (no any dR cone cut!) therefore the tool can be used for any jet with "reasonable" dR size.

   When setting the python configuration useFivePtJetBinVersion to True, a retrained TCT BDT model is used, where five BDTs were
   trained separately in the regions [0.02,0.2]; [0.2,0.5]; [0.5,1]; [1,2] and [2,7] TeV with an Zprime, ZprimeExtended and ttbar sample.

   Either the TrackParticle objects can be decorated with the TCT score and a ElementLink to the Jet object, which was used to
   calculate the TCT score (decorateTrack) or the Jet object can be decorated with a std::vector of TCT scores and a std::vector of ElementLinks
   to TrackParticle associated with the given jet.

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch
*/
#ifndef InDet_InDetTrkInJetType_H
#define InDet_InDetTrkInJetType_H

#include <vector>
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODJet/JetContainer.h" 
#include "xAODTracking/TrackParticleContainer.h"
#include "TrkVertexFitterInterfaces/IVertexFitter.h"

#include "StoreGate/WriteDecorHandle.h"

//

class TLorentzVector;
class IChronoStatSvc;
namespace MVAUtils { class BDT; }
namespace Trk {  class TrkVKalVrtFitter; }

namespace InDet {

//------------------------------------------------------------------------
  class IInDetTrkInJetType : virtual public IAlgTool {
    public:
      DeclareInterfaceID( IInDetTrkInJetType, 1, 0 );
//---------------------------------------------------------------------------
//Interface itself

      virtual std::vector<float> trkTypeWgts( const xAOD::TrackParticle *, const xAOD::Vertex &, const TLorentzVector &) const =0;
      virtual bool usesFivePtJetBinVersion() const=0;
      virtual void decorateTrack(const xAOD::TrackParticle* , const xAOD::Vertex & , const xAOD::JetContainer & , const xAOD::Jet* ) const =0;
      virtual void decorateJet(const std::vector<const xAOD::TrackParticle*> & , const xAOD::TrackParticleContainer& , const xAOD::Vertex & , const xAOD::Jet* ) const =0;


  };




  class InDetTrkInJetType : public extends<AthAlgTool, IInDetTrkInJetType>
  {

   public:
       /* Constructor */
      InDetTrkInJetType(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~InDetTrkInJetType();


      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;

      virtual std::vector<float> trkTypeWgts(const xAOD::TrackParticle *, const xAOD::Vertex &, const TLorentzVector &) const override;
      /* return the five ptjet bin mode: if true, the retrained FivePtJetBin TCT BDT is used; if false, the default TCT BDT is used */
      virtual bool usesFivePtJetBinVersion() const override {return m_useFivePtJetBinVersion; }
      /* decorates a TrackParticle associated to the given Jet (element from the passed JetContainer)*/
      virtual void decorateTrack(const xAOD::TrackParticle* , const xAOD::Vertex & , const xAOD::JetContainer & , const xAOD::Jet* ) const override;
      /* decorates a Jet with the TCT scores and TrackParticleLinks of the tracks passed through the std::vector*/
      virtual void decorateJet(const std::vector<const xAOD::TrackParticle*> & , const xAOD::TrackParticleContainer& , const xAOD::Vertex & , const xAOD::Jet* ) const override;


//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//

   private:

    std::vector<std::unique_ptr<MVAUtils::BDT>> m_vTrkClassBDT{};
    IChronoStatSvc* m_timingProfile{}; 
   
    int m_trkSctHitsCut{};
    int m_trkPixelHitsCut{};
    float m_trkChi2Cut{};
    float m_trkMinPtCut{};
    float m_jetMaxPtCut{};
    float m_jetMinPtCut{};
    float m_d0_limLow{};
    float m_d0_limUpp{};
    float m_Z0_limLow{};
    float m_Z0_limUpp{};
    std::string m_calibFileName;
    std::string m_calibFileNameFivePtJetBin;
    std::string m_jetCollection;
    bool m_useFivePtJetBinVersion;
    ToolHandle < Trk::IVertexFitter >  m_fitterSvc
       {this, "VertexFitterTool", "Trk::TrkVKalVrtFitter/VertexFitterTool",""};
    Trk::TrkVKalVrtFitter*   m_fitSvc{};

    int m_initialised{};

    //numbering of the jet pt slices 
    enum e_ptjetRange {e_ptjet0to0p2TeV=0, e_ptjet0p2to0p5TeV=1, e_ptjet0p5to1TeV=2, e_ptjet1to2TeV=3, e_ptjet2to7TeV=4}; 

    /** The write key for adding TCT score as decoration to TrackParticle objects */
    //from https://acode-browser1.usatlas.bnl.gov/lxr/source/athena/Event/xAOD/xAODTrackingCnv/src/TrackParticleCnvAlg.cxx
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackWriteDecorKeyTCTScore{this,"trackDecorKeyTCTScore",
    "","WriteDecorHandleKey for adding TCT score to TrackParticles"};
    SG::WriteDecorHandleKey<xAOD::TrackParticleContainer> m_trackWriteDecorKeyJetLink{this,"trackDecorKeyJetLink",
    "","WriteDecorHandleKey for adding JetLink to TrackParticles"};

    /** The write key for adding TCT score as decoration to Jet objects */
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetWriteDecorKeyTCTScore{this,"jetDecorKeyTCTScore",
    "","WriteDecorHandleKey for adding TCT score to Jets"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_jetWriteDecorKeyTrackLink{this,"jetDecorKeyJetLink",
    "","WriteDecorHandleKey for adding TrackParticleLink to Jets"};


 };




}  //end namespace
#endif
