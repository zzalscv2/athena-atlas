#include "TruthGuidedProtoTrackCreator.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"
#include "Acts/Surfaces/PerigeeSurface.hpp"
#include "TruthUtils/AtlasPID.h"


ActsTrk::TruthGuidedProtoTrackCreator::TruthGuidedProtoTrackCreator(const std::string& type, 
		const std::string& name,
		const IInterface* parent): base_class(type,name,parent){

}

StatusCode  ActsTrk::TruthGuidedProtoTrackCreator::initialize()
{
    ATH_CHECK( m_prdMultiTruthCollectionNames.initialize() );
    return StatusCode::SUCCESS;
}
StatusCode ActsTrk::TruthGuidedProtoTrackCreator::findProtoTracks(const EventContext& ctx,
                  const xAOD::PixelClusterContainer & pixelContainer,
                  const xAOD::StripClusterContainer & stripContainer,
                  std::vector<ActsTrk::ProtoTrack> & foundProtoTracks ) const {

    // Read the PRD information
    std::vector<const PRD_MultiTruthCollection*> prdMultiTruthCollections;        
    prdMultiTruthCollections.reserve(m_prdMultiTruthCollectionNames.size());
    // load the PRD collections from SG
    for(const auto& pmtCollNameIter:m_prdMultiTruthCollectionNames)
    {
     // try to retrieve the PRD multi truth collection
     SG::ReadHandle<PRD_MultiTruthCollection> curColl (pmtCollNameIter, ctx);
     if (!curColl.isValid())
     {
       ATH_MSG_WARNING("Could not retrieve " << pmtCollNameIter << ". Ignoring ... ");
     }
     else
     {
       ATH_MSG_INFO("Added " << pmtCollNameIter << " to collection list for truth track creation.");
       prdMultiTruthCollections.push_back(curColl.cptr());
     }
   }

    // create the map for the ine
    std::map<Identifier, HepMC::ConstGenParticlePtr> identToHepMCMap;
    for (auto & PRD_truthCollec: prdMultiTruthCollections )
    {
        // loop over the map and get the identifier, GenParticle relation
        PRD_MultiTruthCollection::const_iterator prdMtCIter  = PRD_truthCollec->begin();
        PRD_MultiTruthCollection::const_iterator prdMtCIterE = PRD_truthCollec->end();
        for ( ; prdMtCIter != prdMtCIterE; ++ prdMtCIter ){

            // check if entry exists and if   
#ifdef HEPMC3
            HepMC::ConstGenParticlePtr curGenP       = (*prdMtCIter).second.scptr();
#else
//AV Looks like an implicit conversion
            HepMC::ConstGenParticlePtr curGenP       = (*prdMtCIter).second;
#endif
            Identifier                curIdentifier = (*prdMtCIter).first;

            // Min pT cut
            if ( curGenP->momentum().perp() < 500. ) continue;



            identToHepMCMap[curIdentifier] = curGenP;
        }
    }

    // Now loop over the pixel and strip container and make collectiong
    std::map<HepMC::ConstGenParticlePtr, std::vector<ActsTrk::ATLASUncalibSourceLink>> trackCollections;

    for(const auto& cluster: pixelContainer)
    {
        // Get the idetifier list for the RDOs
        auto identifierList = cluster->rdoList();

        // Loop and push back the cluster in the corresponding trith particle
        for(auto& id: identifierList)
        {
            // Found a match, so push it into the track collection
            if(identToHepMCMap.find(id) != identToHepMCMap.end())
            {
                auto truthParticle = identToHepMCMap.at(id);
                trackCollections[truthParticle].emplace_back(ATLASUncalibSourceLink(cluster, pixelContainer, ctx));
            }
        }
    }

    for(const auto& cluster: stripContainer)
    {
        // Get the idetifier list for the RDOs
        auto identifierList = cluster->rdoList();

        // Loop and push back the cluster in the corresponding trith particle
        for(auto& id: identifierList)
        {
            // Found a match, so push it into the track collection
            if(identToHepMCMap.find(id) != identToHepMCMap.end())
            {
                auto truthParticle = identToHepMCMap.at(id);
                trackCollections[truthParticle].emplace_back(ATLASUncalibSourceLink(cluster, stripContainer, ctx));
            }
        }
    }

    for(const auto& var: trackCollections)
    {
        // Skip if we find less than 3 clusters per truth track
        if(var.second.size() < 3) continue;

        // Make the intput perigee
        auto inputPerigee = makeDummyParams(var.first);
        ATH_MSG_INFO("Found " << var.second.size() << " clusters for truth partcle "<<var.first);
        foundProtoTracks.push_back({var.second,std::move(inputPerigee)});
    }

    // and add to the list (will only make one prototrack per event for now)

    return StatusCode::SUCCESS;
}


std::unique_ptr<Acts::BoundTrackParameters> ActsTrk::TruthGuidedProtoTrackCreator::makeDummyParams (const HepMC::ConstGenParticlePtr& truthParticle) const{

  using namespace Acts::UnitLiterals;
  std::shared_ptr<const Acts::Surface> actsSurface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
        Acts::Vector3(0., 0., 0.));
  Acts::BoundVector params;


  // No, this is not a physically correct parameter estimate! 
  // We just want a placeholder to point in roughly the expected direction... 
  // A real track finder would do something more reasonable here. 
  params << 0., 0.,
        truthParticle->momentum().phi(), truthParticle->momentum().theta(),
        static_cast<float>(::charge(truthParticle)) / (truthParticle->momentum().e()), 0.;
 

  // Covariance - let's be honest and say we have no clue ;-) 
  Acts::BoundSquareMatrix cov = Acts::BoundSquareMatrix::Identity();
  cov *= 100000; 

  // some ACTS paperwork 
  Trk::ParticleHypothesis hypothesis = Trk::pion;
  float mass = Trk::ParticleMasses::mass[hypothesis] * Acts::UnitConstants::MeV;
  Acts::PdgParticle absPdg = Acts::makeAbsolutePdgParticle(Acts::ePionPlus);
  Acts::ParticleHypothesis actsHypothesis{
    absPdg, mass, Acts::AnyCharge{static_cast<float>(::charge(truthParticle))}};

  return std::make_unique<Acts::BoundTrackParameters>(actsSurface, params,
                                    cov, actsHypothesis);

}
