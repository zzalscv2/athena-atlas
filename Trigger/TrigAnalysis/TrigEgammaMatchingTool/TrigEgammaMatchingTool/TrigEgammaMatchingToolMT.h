/*
 * (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TrigEgammaMatchingToolMT_H
#define TrigEgammaMatchingToolMT_H

#include "Gaudi/Property.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "xAODTrigger/EmTauRoIContainer.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigCalo/TrigEMClusterContainer.h"
#include "xAODTrigEgamma/TrigPhotonContainer.h"
#include "xAODTrigEgamma/TrigElectronContainer.h"
#include "xAODTrigRinger/TrigRingerRingsContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"





class TrigEgammaMatchingToolMT : public asg::AsgTool

{
    ASG_TOOL_INTERFACE( TrigEgammaMatchingToolMT )
    ASG_TOOL_CLASS0( TrigEgammaMatchingToolMT )

    public:

        TrigEgammaMatchingToolMT( const std::string& name );


        ~TrigEgammaMatchingToolMT()=default;
        
        StatusCode initialize();

        // check if the offline object passed by HLT step for one specific trigger
        bool isPassed( const xAOD::Egamma *,const std::string&, unsigned int condition=TrigDefs::Physics ) const;

        bool match( const xAOD::Egamma *,const std::string&, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        
        
        template<class T> bool ancestorPassed( const TrigCompositeUtils::Decision*, const std::string& trigger , const std::string& key,
                                               unsigned int condition=TrigDefs::Physics ) const;
        


        template<class T> TrigCompositeUtils::LinkInfo<T> getFeature( const TrigCompositeUtils::Decision *, const std::string& trigger,
                                                                      unsigned int condition=TrigDefs::Physics ) const;
        template<class T> std::vector<TrigCompositeUtils::LinkInfo<T>> getFeatures( const TrigCompositeUtils::Decision *, const std::string& trigger , 
                                                                                    unsigned int condition=TrigDefs::Physics ) const;
        template<class T> std::vector<TrigCompositeUtils::LinkInfo<T>> getFeatures( const TrigCompositeUtils::Decision *, const std::string& trigger, const std::string& key ,
                                                                                    unsigned int condition=TrigDefs::Physics ) const;
        
        // access the feature from the container
        const xAOD::EmTauRoI*  getL1Feature( const TrigCompositeUtils::Decision * ) const;
        const xAOD::eFexEMRoI* getL1eEMFeature( const TrigCompositeUtils::Decision * ) const;

        // access the feature from the container
        const xAOD::TrigRingerRings* getRingsFeature( const TrigCompositeUtils::Decision * ) const;
        // get the container key
        std::string key( std::string ) const;

    private:
        
        bool matchHLTElectron(const xAOD::Electron *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        bool matchHLTPhoton(  const xAOD::Photon   *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        bool matchHLTCalo(    const xAOD::Egamma   *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        bool matchL2Electron( const xAOD::Electron *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        bool matchL2Photon(   const xAOD::Photon   *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        bool matchL2Calo(     const xAOD::Egamma   *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
        bool matchL1(         const xAOD::Egamma   *,const std::string &, const TrigCompositeUtils::Decision *&, unsigned int condition=TrigDefs::Physics ) const;
      
        template<class T> bool closestObject( const xAOD::Egamma *, const TrigCompositeUtils::Decision *&, const std::string &trigger, 
                                              const std::string &key, float dR_value, unsigned int condition=TrigDefs::Physics ) const;
        

        inline double dR(const double eta1, const double phi1, const double eta2, const double phi2) const
        {
          double deta = std::abs(eta1 - eta2);
          double dphi = std::abs(phi1 - phi2) < TMath::Pi() ? std::abs(phi1 - phi2) : 2*TMath::Pi() - std::abs(phi1 - phi2);
          return sqrt(deta*deta + dphi*dphi);
        };


        std::map<std::string,std::string> m_keys; 
        
        ToolHandle<Trig::TrigDecisionTool>  m_trigDecTool{this, "TriggerTool", ""};
        const ToolHandle<Trig::TrigDecisionTool>& tdt() const { return m_trigDecTool; };
        
        Gaudi::Property<float> m_dR{this, "DeltaR", 0.07};
        Gaudi::Property<float> m_dRL1{this, "L1DeltaR", 0.15};
        SG::ReadHandleKey<xAOD::EmTauRoIContainer>         m_emTauRoIKey{this, "EmTauRoIKey" , "LVL1EmTauRoIs", ""};
        SG::ReadHandleKey<xAOD::TrigRingerRingsContainer>  m_ringerKey{this, "RingerKey" , "HLT_FastCaloRinger", ""};

};




#include "TrigEgammaMatchingToolMT.icc"

#endif
