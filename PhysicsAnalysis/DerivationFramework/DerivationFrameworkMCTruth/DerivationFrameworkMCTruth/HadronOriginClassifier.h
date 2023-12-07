/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

/*
 
 * @author Mirko Casolino
 * @date June 2015
 * @brief tool to compute oring of hadron to flag ttbar+HF
 
 */

#ifndef  DerivationFrameworkMCTruth_HadronOriginClassifier_H
#define  DerivationFrameworkMCTruth_HadronOriginClassifier_H

#include <vector>
#include <map>
#include <set>
#include <string>
#include <utility>

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODEventInfo/EventInfo.h"

namespace DerivationFramework{


  static const InterfaceID IID_HadronOriginClassifier("HadronOriginClassifier", 1, 0);

  class HadronOriginClassifier: public AthAlgTool {


  public:
    HadronOriginClassifier(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~HadronOriginClassifier();
    
    virtual StatusCode initialize() override;

    static const InterfaceID& interfaceID() { return IID_HadronOriginClassifier; }

    
    typedef enum {extrajet=0,
		  c_MPI     =-1, b_MPI      =1,
		  c_FSR     =-2, b_FSR      =2,
		  c_from_W  =-3, b_from_W   =3,
		  c_from_top=-4, b_from_top =4,
		  c_from_H  =-5, b_from_H   =5} HF_id;
    
    typedef enum { Pythia6=0, Pythia8=1, HerwigPP=2, Sherpa=3 } GEN_id;
        
    std::map<const xAOD::TruthParticle*, HF_id> GetOriginMap() const;
    
  private:

    void fillHadronMap(std::set<const xAOD::TruthParticle*>& usedHadron, std::map<const xAOD::TruthParticle*,int>& mainHadronMap, const xAOD::TruthParticle* mainhad, const xAOD::TruthParticle* ihad, bool decayed=false) const;

    void buildPartonsHadronsMaps(std::map<const xAOD::TruthParticle*,int>& mainHadronMap,
                                 std::map<const xAOD::TruthParticle*,HF_id>& partonsOrigin) const;

    bool passHadronSelection(const xAOD::TruthParticle* part) const;


    bool isQuarkFromHadron(const xAOD::TruthParticle* part) const;

    bool isCHadronFromB(const xAOD::TruthParticle* part) const;


    /// init_part needed to detect looping graphs (sherpa) and to switch on using barcode to resolve it without affecting pythia8
    /// up to know only seen at parton level
    bool isLooping(const xAOD::TruthParticle* part, std::set<const xAOD::TruthParticle*> init_part = std::set<const xAOD::TruthParticle*>()) const;
    
    const xAOD::TruthParticle* findInitial(const xAOD::TruthParticle* part, bool looping) const;
    
    bool isFromTop(const xAOD::TruthParticle* part, bool looping) const;
    static bool isDirectlyFromTop(const xAOD::TruthParticle* part, bool looping) ;
    bool isDirectlyFromWTop(const xAOD::TruthParticle* part, bool looping) const;

    static bool isDirectlyFromGluonQuark(const xAOD::TruthParticle* part, bool looping) ;
    bool isFromGluonQuark(const xAOD::TruthParticle* part, bool looping) const;
    bool isDirectlyFSRPythia6(const xAOD::TruthParticle* part, bool looping) const;

    bool isDirectlyFromQuarkTop(const xAOD::TruthParticle* part, bool looping) const;
    bool isFromQuarkTop(const xAOD::TruthParticle* part, bool looping) const;
    bool isDirectlyFSR(const xAOD::TruthParticle* part, bool looping) const;
    bool isFromWTop(const xAOD::TruthParticle* part, bool looping) const;

    static bool isDirectlyMPIPythia6(const xAOD::TruthParticle* part, bool looping) ;

    bool isDirectlyMPIPythia8(const xAOD::TruthParticle* part, bool looping) const;
    bool isDirectlyFromQuarkTopPythia8(const xAOD::TruthParticle* part, bool looping) const;
    bool isFromQuarkTopPythia8(const xAOD::TruthParticle* part, bool looping) const;
    bool isDirectlyFSRPythia8(const xAOD::TruthParticle* part, bool looping) const;

    static bool isDirectlyMPISherpa(const xAOD::TruthParticle* part) ;

     
    inline bool IsHerwigPP() const {return m_GenUsed==HerwigPP;};
    inline bool IsPythia8() const {return m_GenUsed==Pythia8;};
    inline bool IsPythia6() const {return m_GenUsed==Pythia6;};
    inline bool IsSherpa() const {return m_GenUsed==Sherpa;};
    inline bool IsTtBb() const {return m_ttbb;}

    std::string m_mcName;
    double m_HadronPtMinCut;
    double m_HadronEtaMaxCut;
    int m_DSID;
    GEN_id m_GenUsed;
    bool m_ttbb = false;
    
  };

} //namespace


#endif //DerivationFrameworkMCTruth_HadronOriginClassifier_H

