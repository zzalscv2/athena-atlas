/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include <AsgDataHandles/ReadHandle.h>
#include <AsgTools/AsgToolConfig.h>
#include <FourMomUtils/xAODP4Helpers.h>
#include <IsolationSelection/IsolationCloseByCorrectionTool.h>
#include <IsolationSelection/IsolationSelectionTool.h>
#include <PATInterfaces/CorrectionCode.h>
#include <xAODBase/IParticleHelpers.h>
#include <xAODBase/ObjectType.h>
#include <xAODPrimitives/IsolationHelpers.h>
#include <xAODPrimitives/tools/getIsolationAccessor.h>
#include <xAODPrimitives/tools/getIsolationCorrectionAccessor.h>

#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

namespace CP {
    using namespace xAOD::Iso;
    using caloDecorNames = IsolationCloseByCorrectionTool::caloDecorNames;
    caloDecorNames IsolationCloseByCorrectionTool::caloDecors() {
        return {"IsoCloseByCorr_assocClustEta", "IsoCloseByCorr_assocClustPhi", "IsoCloseByCorr_assocClustEnergy",
                "IsoCloseByCorr_assocClustDecor"};
    }
    caloDecorNames IsolationCloseByCorrectionTool::pflowDecors() {
        return {"IsoCloseByCorr_assocPflowEta", "IsoCloseByCorr_assocPflowPhi", "IsoCloseByCorr_assocPflowEnergy",
                "IsoCloseByCorr_assocPflowDecor"};
    }
    constexpr float MinClusterEnergy = 100.;
    constexpr float MeVtoGeV = 1.e-3;

    IsolationCloseByCorrectionTool::IsolationCloseByCorrectionTool(const std::string& toolName) : asg::AsgTool(toolName) {}

    StatusCode IsolationCloseByCorrectionTool::initialize() {
        /// Retrieve the isolation tool and load the isolation cones
        ATH_CHECK(m_selectorTool.retrieve());
        isoTypesFromWP(m_selectorTool->getElectronWPs(), m_electron_isoTypes);
        isoTypesFromWP(m_selectorTool->getMuonWPs(), m_muon_isoTypes);
        isoTypesFromWP(m_selectorTool->getPhotonWPs(), m_photon_isoTypes);

        printIsolationCones(m_electron_isoTypes, xAOD::Type::ObjectType::Electron);
        printIsolationCones(m_muon_isoTypes, xAOD::Type::ObjectType::Muon);
        printIsolationCones(m_photon_isoTypes, xAOD::Type::ObjectType::Photon);
       
        ATH_CHECK(m_VtxKey.initialize());
        ATH_CHECK(m_CaloClusterKey.initialize(m_caloModel == TopoConeCorrectionModel::SubtractObjectsDirectly));
        ATH_CHECK(m_PflowKey.initialize(m_caloModel == TopoConeCorrectionModel::SubtractObjectsDirectly));

        // set default properties of track selection tool, if the user hasn't configured it
        if (m_trkselTool.empty()) {
            asg::AsgToolConfig config{"InDet::InDetTrackSelectionTool/TrackParticleSelectionTool"};
            ATH_MSG_INFO("No TrackSelectionTool provided, so I will create and configure my own, called: " << config.name());
            // The z0 cut is checked in any case either by the
            // track to vertex association tool or by the tracking tool
            ATH_CHECK(config.setProperty("maxZ0SinTheta", 3.));
            // The minimum Pt requirement is lowered to 500 MeV because
            // the Loose ttva cone variables accept very low-pt tracks
            // https://gitlab.cern.ch/atlas/athena/blob/21.2/Reconstruction/RecoAlgs/IsolationAlgs/python/IsoUpdatedTrackCones.py#L21
            ATH_CHECK(config.setProperty("minPt", 500.));
            ATH_CHECK(config.setProperty("CutLevel", "Loose"));
            ATH_CHECK(config.makePrivateTool(m_trkselTool));
        }
        /// Same holds for the TTVA selection tool
        if (m_ttvaTool.empty()) {
            asg::AsgToolConfig config{"CP::TrackVertexAssociationTool/ttva_selection_tool"};
            ATH_CHECK(config.setProperty("WorkingPoint", "Nonprompt_All_MaxWeight"));
            ATH_CHECK(config.makePrivateTool(m_ttvaTool));
        }
        ATH_CHECK(m_trkselTool.retrieve());
        ATH_CHECK(m_ttvaTool.retrieve());

        if (!m_quality_name.empty()) m_acc_quality = std::make_unique<CharAccessor>(m_quality_name);
        if (!m_passOR_name.empty()) m_acc_passOR = std::make_unique<CharAccessor>(m_passOR_name);
        if (!m_isoSelection_name.empty()) m_dec_isoselection = std::make_unique<CharDecorator>(m_isoSelection_name);
        m_isInitialised = true;
        return StatusCode::SUCCESS;
    }
    void IsolationCloseByCorrectionTool::isoTypesFromWP(const std::vector<std::unique_ptr<IsolationWP>>& WPs, IsoVector& types) {
        types.clear();
        for (const std::unique_ptr<IsolationWP>& W : WPs) {
            for (const std::unique_ptr<IsolationCondition>& C : W->conditions()) {
                for (unsigned int t = 0; t < C->num_types(); ++t) {
                    const IsoType iso_type = C->type(t);
                    if (std::find(types.begin(), types.end(), iso_type) == types.end()) types.emplace_back(iso_type);
                    if (m_isohelpers.find(iso_type) == m_isohelpers.end()) {
                        m_isohelpers.insert(std::make_pair(iso_type, std::make_unique<IsoVariableHelper>(iso_type, m_backup_prefix)));
                    }
                }
            }
        }
        m_has_nonTTVA |= std::find_if(types.begin(), types.end(),
                                      [](const IsolationType& t) { return isTrackIso(t) && !isTrackIsoTTVA(t); }) != types.end();
        m_hasPflowIso |= std::find_if(types.begin(), types.end(),
                                      [](const IsolationType& t) { return isPFlowIso(t); }) != types.end();
        m_hasEtConeIso |= std::find_if(types.begin(), types.end(),
                                      [](const IsolationType& t) { return isTopoEtIso(t); }) != types.end();
    }
   void IsolationCloseByCorrectionTool::loadPrimaryParticles(const xAOD::IParticleContainer* container, ObjectCache& cache) const {
        if (!container) return;
        for (const xAOD::IParticle* particle : *container) {
            if (m_dec_isoselection) (*m_dec_isoselection)(*particle) = true && m_selectorTool->accept(*particle);
            const IsoVector& iso_types = getIsolationTypes(particle);
            if (iso_types.empty()) { ATH_MSG_DEBUG("No isolation types have been defined for particle type " << particleName(particle)); }
            for (const IsoType type : iso_types) {
                IsoHelperMap::const_iterator Itr = m_isohelpers.find(type);
                if (Itr == m_isohelpers.end() || Itr->second->backupIsolation(particle) == CorrectionCode::Error) {
                    ATH_MSG_WARNING("Failed to properly access the vanilla isolation variable "
                                    << toString(type) << "for particle " << particleName(particle) << " with pT: "
                                    << particle->pt() * MeVtoGeV << " GeV, eta: " << particle->eta() << ", phi: " << particle->phi());
                }
            }
            if (!passSelectionQuality(particle)) continue;
            cache.prim_parts.insert(particle);
        }
    }
    void IsolationCloseByCorrectionTool::loadAssociatedObjects(const EventContext& ctx, ObjectCache& cache) const {
        cache.prim_vtx = retrieveIDBestPrimaryVertex(ctx);
        for (const xAOD::IParticle* prim : cache.prim_parts) {
            const TrackSet tracks = getAssociatedTracks(prim, cache.prim_vtx);
            cache.tracks.insert(tracks.begin(), tracks.end());
            const ClusterSet clusters = getAssociatedClusters(ctx, prim);
            cache.clusters.insert(clusters.begin(), clusters.end());
        }
        getAssocFlowElements(ctx, cache);
    }
    PflowSet IsolationCloseByCorrectionTool::getAssocFlowElements(const EventContext& ctx, const xAOD::IParticle* particle) const {
        ObjectCache cache{};
        cache.prim_parts = {particle};
        loadAssociatedObjects(ctx, cache);
        return cache.flows;
    }

    void IsolationCloseByCorrectionTool::getAssocFlowElements(const EventContext& ctx, ObjectCache& cache) const {
        if (m_PflowKey.empty()) return;
        SG::ReadHandle<xAOD::FlowElementContainer> readHandle{m_PflowKey, ctx};
        if (!readHandle.isValid()) return;
        std::set<const xAOD::IParticle*> tombola{};
        for (const xAOD::IParticle* p : cache.prim_parts) tombola.insert(p);
        for (const TrackPtr& p : cache.tracks) tombola.insert(p);
        for (const CaloClusterPtr& p : cache.clusters) tombola.insert(p);

        for (const xAOD::FlowElement* flow : *readHandle) {
            if (!flow) continue;
            for (size_t ch = 0; ch < flow->nChargedObjects(); ++ch) {
                const xAOD::IParticle* obj = flow->chargedObject(ch);
                if (tombola.count(obj)) {
                    const std::vector<float>& weights = flow->chargedObjectWeights();
                    cache.flows.emplace(flow, ch < weights.size() ? weights[ch] : 1.f);
                }
            }
            for (size_t ne = 0; ne < flow->nOtherObjects(); ++ne) {
                const xAOD::IParticle* obj = flow->otherObject(ne);
                if (tombola.count(obj)) {
                    const std::vector<float>& weights = flow->otherObjectWeights();
                    cache.flows.emplace(flow, ne < weights.size() ? weights[ne] : 1.f);
                }
            }
        }
    }

    /// not thread-safe because of const_cast
    CorrectionCode IsolationCloseByCorrectionTool::getCloseByIsoCorrection ATLAS_NOT_THREAD_SAFE (xAOD::ElectronContainer* Electrons, xAOD::MuonContainer* Muons,
                                                                           xAOD::PhotonContainer* Photons) const {
        if (!m_isInitialised) {
            ATH_MSG_ERROR("The IsolationCloseByCorrectionTool was not initialised!!!");
            return CorrectionCode::Error;
        }
        const EventContext& ctx = Gaudi::Hive::currentContext();

        ObjectCache cache{};
        /// Pick up first all objects that a considerable for the close-by correction
        loadPrimaryParticles(Electrons, cache);
        loadPrimaryParticles(Muons, cache);
        loadPrimaryParticles(Photons, cache);

        if (cache.prim_parts.empty()) {
            ATH_MSG_DEBUG("No considerably good objects found.");
            return CorrectionCode::Ok;
        }
        loadAssociatedObjects(ctx, cache);
        if (!cache.prim_vtx) { return CorrectionCode::OutOfValidityRange; }

        return performCloseByCorrection(ctx, cache);
    }
    CorrectionCode IsolationCloseByCorrectionTool::performCloseByCorrection ATLAS_NOT_THREAD_SAFE (const EventContext& ctx, ObjectCache& cache) const {
        for (const xAOD::IParticle* Particle : cache.prim_parts) {
            auto Particle_nc = const_cast<xAOD::IParticle*>(Particle); // this needs to be fixed
            if (subtractCloseByContribution(ctx, Particle_nc, cache) == CorrectionCode::Error) {
                ATH_MSG_ERROR("Failed to correct the isolation of particle with pt: " << Particle->pt() * MeVtoGeV << " GeV"
                                                                                      << " eta: " << Particle->eta()
                                                                                      << " phi: " << Particle->phi());
                return CorrectionCode::Error;
            }
            if (m_dec_isoselection) (*m_dec_isoselection)(*Particle) = bool(m_selectorTool->accept(*Particle));
        }
        return CorrectionCode::Ok;
    }
    const IsoVector& IsolationCloseByCorrectionTool::getIsolationTypes(const xAOD::IParticle* particle) const {
        static const IsoVector dummy{};
        if (!particle) return dummy;
        if (particle->type() == xAOD::Type::ObjectType::Electron)
            return m_electron_isoTypes;
        else if (particle->type() == xAOD::Type::ObjectType::Muon)
            return m_muon_isoTypes;
        else if (particle->type() == xAOD::Type::ObjectType::Photon)
            return m_photon_isoTypes;
        return dummy;
    }
    CorrectionCode IsolationCloseByCorrectionTool::subtractCloseByContribution(xAOD::IParticle& x,
                                                                               const xAOD::IParticleContainer& closebyPar) const {
        const EventContext& ctx = Gaudi::Hive::currentContext();
        ObjectCache cache{};
        loadPrimaryParticles(&closebyPar, cache);
        loadAssociatedObjects(ctx, cache);
        return subtractCloseByContribution(ctx, &x, cache);
    }

    CorrectionCode IsolationCloseByCorrectionTool::subtractCloseByContribution(const EventContext& ctx, xAOD::IParticle* par,
                                                                               const ObjectCache& cache) const {
        const IsoVector& types = getIsolationTypes(par);
        if (types.empty()) {
            ATH_MSG_WARNING("No isolation types are defiend for " << particleName(par));
            return CorrectionCode::OutOfValidityRange;
        }
        for (const IsolationType iso_type : types) {
            float iso_variable{0.f};
            if (isTrackIso(iso_type)) {
                if (getCloseByCorrectionTrackIso(par, iso_type, cache, iso_variable) == CorrectionCode::Error) {
                    ATH_MSG_ERROR("Failed to apply track correction");
                    return CorrectionCode::Error;
                }
            } else if (isTopoEtIso(iso_type)) {
                if (getCloseByCorrectionTopoIso(ctx, par, iso_type, cache, iso_variable) == CorrectionCode::Error) {
                    ATH_MSG_ERROR("Failed to apply topo cluster correction");
                    return CorrectionCode::Error;
                }
            } else if (isPFlowIso(iso_type)) {
                if (getCloseByCorrectionPflowIso(ctx, par, iso_type, cache, iso_variable) == CorrectionCode::Error) {
                    ATH_MSG_ERROR("Failed to apply pflow correction");
                    return CorrectionCode::Error;
                }
            }
            if (m_isohelpers.at(iso_type)->setIsolation(par, iso_variable) == CorrectionCode::Error) { return CorrectionCode::Error; }
        }
        return CorrectionCode::Ok;
    }
    CorrectionCode IsolationCloseByCorrectionTool::getCloseByCorrection(std::vector<float>& corrections, const xAOD::IParticle& par,
                                                                        const std::vector<IsolationType>& types,
                                                                        const xAOD::IParticleContainer& closePar) const {
        if (!m_isInitialised) {
            ATH_MSG_ERROR("The IsolationCloseByCorrectionTool was not initialised!!!");
            return CorrectionCode::Error;
        }
        /// Check first if all isolation types are known to the tool
        {
            std::lock_guard<std::mutex> guard{m_isoHelpersMutex};
            for (const IsolationType& t : types) {
                IsoHelperMap::const_iterator Itr = m_isohelpers.find(t);
                if (Itr != m_isohelpers.end()) { continue; }
                Itr = m_isohelpers.insert(std::make_pair(t, std::make_unique<IsoVariableHelper>(t, m_backup_prefix))).first;
            }
        }
        corrections.assign(types.size(), 0);
        ObjectCache cache{};
        loadPrimaryParticles(&closePar, cache);
        const EventContext& ctx = Gaudi::Hive::currentContext();
        loadAssociatedObjects(ctx, cache);
        std::vector<float>::iterator Cone = corrections.begin();
        for (const IsolationType& iso_type : types) {
            IsoHelperMap::const_iterator Itr = m_isohelpers.find(iso_type);
            if (Itr->second->backupIsolation(&par) == CP::CorrectionCode::Error) {
                ATH_MSG_ERROR("Failed to backup isolation");
                return CorrectionCode::Error;
            }
            if (isTrackIso(iso_type)) {
                if (getCloseByCorrectionTrackIso(&par, iso_type, cache, (*Cone)) == CorrectionCode::Error) {
                    ATH_MSG_ERROR("Failed to apply track correction");
                    return CorrectionCode::Error;

                }
            }
            else if (isTopoEtIso(iso_type)) {
              if (getCloseByCorrectionTopoIso(ctx, &par, iso_type, cache, (*Cone)) == CorrectionCode::Error) {
                ATH_MSG_ERROR("Failed to apply topo cluster correction");
                return CorrectionCode::Error;
              }
            }
            else if (isPFlowIso(iso_type)) {
              if (getCloseByCorrectionPflowIso(ctx, &par, iso_type, cache, (*Cone)) == CorrectionCode::Error) {
                ATH_MSG_ERROR("Failed to apply pflow correction");
                return CorrectionCode::Error;
              }
            }
            ++Cone;
        }
        return CorrectionCode::Ok;
    }
    bool IsolationCloseByCorrectionTool::passFirstStage(const xAOD::TrackParticle* trk, const xAOD::Vertex* vtx) const {
        return trk && m_trkselTool->accept(*trk, vtx) && (!m_has_nonTTVA || m_ttvaTool->isCompatible(*trk, *vtx));
    }
    TrackSet IsolationCloseByCorrectionTool::getAssociatedTracks(const xAOD::IParticle* P) const {
        TrackSet to_return{};
        if (P->type() == xAOD::Type::Muon) {
            const xAOD::Muon* mu = static_cast<const xAOD::Muon*>(P);
            if (mu->muonType() != xAOD::Muon::SiliconAssociatedForwardMuon)
                to_return.emplace(mu->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle));
        } else if (P->type() == xAOD::Type::TrackParticle) {
            const xAOD::TrackParticle* trk = static_cast<const xAOD::TrackParticle*>(P);
            to_return.emplace(trk);
        } else if (isEgamma(P)) {
            const xAOD::Egamma* EG = static_cast<const xAOD::Egamma*>(P);
            std::set<const xAOD::TrackParticle*> trk_vec = xAOD::EgammaHelpers::getTrackParticles(EG, true, true);
            for (const xAOD::TrackParticle* trk : trk_vec) to_return.emplace(trk);
        }
        return to_return;
    }
    TrackSet IsolationCloseByCorrectionTool::getAssociatedTracks(const xAOD::IParticle* P, const xAOD::Vertex* vtx) const {
        const TrackSet assoc_tracks = getAssociatedTracks(P);
        TrackSet to_ret{};
        for (const TrackPtr trk : assoc_tracks) {
            if (passFirstStage(trk, vtx)) to_ret.insert(trk);
        }
        return to_ret;
    }
    TrackSet IsolationCloseByCorrectionTool::getTrackCandidates(const EventContext& ctx, const xAOD::IParticle* particle) const {
        return getAssociatedTracks(particle, retrieveIDBestPrimaryVertex(ctx));
    }
    bool IsolationCloseByCorrectionTool::isEgamma(const xAOD::IParticle* P) {
        return P && (P->type() == xAOD::Type::ObjectType::Electron || P->type() == xAOD::Type::ObjectType::Photon);
    }

    ClusterSet IsolationCloseByCorrectionTool::getAssociatedClusters(const EventContext& ctx, const xAOD::IParticle* P) const {
        ClusterSet clusters;
        if (isEgamma(P)) {
            const xAOD::Egamma* egamm = static_cast<const xAOD::Egamma*>(P);
            for (size_t calo = 0; calo < egamm->nCaloClusters(); ++calo) {
                const xAOD::CaloCluster* clust = egamm->caloCluster(calo);
                if (!clust) continue;
                std::vector<const xAOD::CaloCluster*> constituents = xAOD::EgammaHelpers::getAssociatedTopoClusters(clust);
                for (const xAOD::CaloCluster* cluster : constituents) {
                    if (cluster && std::abs(cluster->eta()) < 7. && cluster->e() > MinClusterEnergy) { clusters.emplace(cluster); }
                }
            }
            if (clusters.size()) return clusters;
        }
        if (m_CaloClusterKey.empty()) return clusters;
        SG::ReadHandle<xAOD::CaloClusterContainer> topoClusters{m_CaloClusterKey, ctx};
        if (!topoClusters.isValid()) return clusters;
        const xAOD::IParticle* Ref = isoRefParticle(P);
        for (const xAOD::CaloCluster* cluster : *topoClusters) {
            if (cluster && std::abs(cluster->eta()) < 7. && cluster->e() > MinClusterEnergy &&
                overlap(cluster, Ref, P->type() == xAOD::Type::ObjectType::Muon ? m_coreConeMu : m_coreConeEl)) {
                clusters.emplace(cluster);
            }
        }
        return clusters;
    }
    bool IsolationCloseByCorrectionTool::passSelectionQuality(const xAOD::IParticle* P) const {
        if (!P) return false;
        if (m_acc_quality && (!m_acc_quality->isAvailable(*P) || !(*m_acc_quality)(*P))) return false;
        if (m_acc_passOR && (!m_acc_passOR->isAvailable(*P) || !(*m_acc_passOR)(*P))) return false;
        return true;
    }
    CorrectionCode IsolationCloseByCorrectionTool::getCloseByCorrectionTrackIso(const xAOD::IParticle* par, const IsoType type,
                                                                                const ObjectCache& cache, float& isoValue) const {
        if (!isTrackIso(type)) {
            ATH_MSG_ERROR("Invalid isolation type " << toString(type));
            return CorrectionCode::Error;
        }
        IsoHelperMap::const_iterator Itr = m_isohelpers.find(type);
        if (Itr == m_isohelpers.end() || Itr->second->getOrignalIsolation(par, isoValue) == CorrectionCode::Error) {
            ATH_MSG_WARNING("Could not retrieve the isolation variable " << toString(type));
            return CorrectionCode::Error;
        } else if (cache.tracks.empty())
            return CorrectionCode::Ok;

        float MaxDR = coneSize(par, type);
        const TrackSet ToExclude = getAssociatedTracks(par);

        const xAOD::IParticle* Ref = isoRefParticle(par);
        ATH_MSG_VERBOSE(toString(type) << " of " << particleName(par) << " with pt: " << par->pt() * MeVtoGeV << " GeV, eta: " << par->eta()
                                     << ", phi: " << par->phi() << " before correction: " << isoValue * MeVtoGeV << " GeV. "
                                     << ToExclude.size() << " tracks will be excluded.");

        for (const TrackPtr& poluting_trk : cache.tracks) {
            // Checks for the Pile-up robust isolation WP's
            if (poluting_trk->pt() < trackPtCut(type)) continue;
            /// Only check the TTVA working point again if the tool has non-TTVA working points
            if (isTrackIsoTTVA(type) && m_has_nonTTVA && !m_ttvaTool->isCompatible(*poluting_trk, *cache.prim_vtx)) continue;

            if (overlap(Ref, poluting_trk, MaxDR) && !ToExclude.count(poluting_trk)) {
                ATH_MSG_VERBOSE("Subtract track with "
                                << poluting_trk->pt() * MeVtoGeV << " GeV, eta: " << poluting_trk->eta() << ", phi: " << poluting_trk->phi()
                                << " with dR: " << std::sqrt(deltaR2(Ref, poluting_trk)) << " from the isolation cone " << toString(type)
                                << " " << (isoValue * MeVtoGeV) << " GeV.");
                isoValue -= poluting_trk->pt();
            }
        }
        isoValue = std::max(0.f, isoValue);
        ATH_MSG_VERBOSE(toString(type) << " of " << particleName(par) << " with pt: " << par->pt() * MeVtoGeV << " GeV, eta: " << par->eta()
                                     << ", phi: " << par->phi() << " after correction: " << isoValue * MeVtoGeV << " GeV");
        return CorrectionCode::Ok;
    }
    CorrectionCode IsolationCloseByCorrectionTool::getCloseByCorrectionPflowIso(const EventContext& ctx, const xAOD::IParticle* primary,
                                                                                const IsoType type, const ObjectCache& cache,
                                                                                float& isoValue) const {
        if (!isPFlowIso(type)) {
            ATH_MSG_ERROR("getCloseByCorrectionPflowIso() -- The isolation type is not a Pflow variable " << toString(type));
            return CorrectionCode::Error;
        }
        if (m_isohelpers.at(type)->getOrignalIsolation(primary, isoValue) == CorrectionCode::Error) {
            ATH_MSG_ERROR("getCloseByCorrectionPflowIso()  -- Could not retrieve the isolation variable.");
            return CorrectionCode::Error;
        }
        /// Disable the correction of already isolated objects
        if (isoValue <= 0.) {
            ATH_MSG_DEBUG("Pflow varible is already sufficiently isolated ");
            return CorrectionCode::Ok;
        }
        const float coneDR = coneSize(primary, type);
        float ref_eta{0.f}, ref_phi{0.f};
        getExtrapEtaPhi(primary, ref_eta, ref_phi);          
        if (m_caloModel == TopoConeCorrectionModel::SubtractObjectsDirectly) {
            /// Find the pflow elements associated with this primary
        
            PflowSet assoc_coll = getAssocFlowElements(ctx, primary);
            for (const FlowElementPtr& flow : cache.flows) {  
                const float dR = xAOD::P4Helpers::deltaR(ref_eta,ref_phi, flow->eta(),flow->phi());
                if(dR < (primary->type() == xAOD::Type::ObjectType::Muon ? m_coreConeMu : m_coreConeEl)) {
                    ATH_MSG_VERBOSE("Flow element is in core cone");
                    continue;
                }
                if (assoc_coll.count(flow)) {
                    ATH_MSG_VERBOSE("Flow element is directly associated with the object");
                    continue;
                }
                if (dR < coneDR) {
                    ATH_MSG_ALWAYS("Found overlapping pflow element: " << flow->pt() << " GeV, eta: " << flow->eta()
                                                                       << " phi: " << flow->phi());
                    isoValue -= flow->pt() * flow.weight;
                }
            }
        } else if (m_caloModel == TopoConeCorrectionModel::UseAveragedDecorators) {
            static const FloatAccessor acc_eta{pflowDecors()[0]};
            static const FloatAccessor acc_phi{pflowDecors()[1]};
            static const FloatAccessor acc_ene{pflowDecors()[2]};
            static const CharAccessor acc_isDecor{pflowDecors()[3]};
            for (const xAOD::IParticle* others : cache.prim_parts) {
                if (others == primary) continue;
                if (!acc_isDecor.isAvailable(*others) || !acc_isDecor(*others)) {
                    ATH_MSG_ERROR("The variable energy averaged pflow decorations are not available for "<<particleName(others)<<". Please check");
                    return CorrectionCode::Error;
                }
                const float other_eta = acc_eta(*others);
                const float other_phi = acc_phi(*others);
                const float dR = xAOD::P4Helpers::deltaR(ref_eta,ref_phi,other_eta,other_phi);
                if (dR > coneDR) continue;
                if (dR< (primary->type() == xAOD::Type::ObjectType::Muon ? m_coreConeMu : m_coreConeEl)) continue;
                isoValue -= acc_ene(*others);
            }
        } else {
            ATH_MSG_ERROR("Unknown calo correction model "<<m_caloModel);
            return CorrectionCode::Error;
        }
        isoValue = std::max(0.f, isoValue);
        return CorrectionCode::Ok;
    }

    CorrectionCode IsolationCloseByCorrectionTool::getCloseByCorrectionTopoIso(const EventContext& ctx, const xAOD::IParticle* primary,
                                                                               const IsoType type, const ObjectCache& cache,
                                                                               float& isoValue) const {
        // check if the isolation can be loaded
        if (!isTopoEtIso(type)) {
            ATH_MSG_ERROR("getCloseByCorrectionTopoIso() -- The isolation type is not an et cone variable " << toString(type));
            return CorrectionCode::Error;
        }
        if (m_isohelpers.at(type)->getOrignalIsolation(primary, isoValue) == CorrectionCode::Error) {
            ATH_MSG_WARNING("Could not retrieve the isolation variable.");
            return CorrectionCode::Error;
        }
        /// Disable the correction of already isolated objects
        if (isoValue <= 0.) {
            ATH_MSG_DEBUG("Topo et cone variable is already sufficiently isolated");
            return CorrectionCode::Ok;
        }
        float ref_eta{0.f}, ref_phi{0.f};
        getExtrapEtaPhi(primary, ref_eta, ref_phi);
        
        float MaxDR = coneSize(primary, type) * (primary->type() != xAOD::Type::ObjectType::Muon ? 1. : m_ConeSizeVariation.value());
        if (m_caloModel == TopoConeCorrectionModel::SubtractObjectsDirectly) {
            ClusterSet assoc = getAssociatedClusters(ctx, primary);
            for (const CaloClusterPtr& calo : cache.clusters) {
                const float dR = xAOD::P4Helpers::deltaR(ref_eta, ref_phi, calo->eta(), calo->phi());
                if (dR > MaxDR) continue;
                if (dR< (primary->type() == xAOD::Type::ObjectType::Muon ? m_coreConeMu : m_coreConeEl)) continue;
                float Polution = clusterEtMinusTile(calo) / (isoValue != 0 ? isoValue : 1.);
                if (Polution < 0. || Polution > m_maxTopoPolution) continue;
                if (assoc.count(calo)) continue;
                isoValue -= clusterEtMinusTile(calo);
            }
        } else if (m_caloModel == TopoConeCorrectionModel::UseAveragedDecorators) {
            static const FloatAccessor acc_eta{caloDecors()[0]};
            static const FloatAccessor acc_phi{caloDecors()[1]};
            static const FloatAccessor acc_ene{caloDecors()[2]};
            static const CharAccessor acc_isDecor{caloDecors()[3]};
             for (const xAOD::IParticle* others : cache.prim_parts) {
                if (others == primary) continue;
                if (!acc_isDecor.isAvailable(*others) || !acc_isDecor(*others)) {
                    ATH_MSG_ERROR("The averaged calo cluster decorations are not available for "<<particleName(others)<<". Please check");
                    return CorrectionCode::Error;
                }
                const float other_eta = acc_eta(*others);
                const float other_phi = acc_phi(*others);
                const float dR = xAOD::P4Helpers::deltaR(ref_eta,ref_phi,other_eta,other_phi);
                if (dR > MaxDR) continue;
                if (dR< (primary->type() == xAOD::Type::ObjectType::Muon ? m_coreConeMu : m_coreConeEl)) continue;
                isoValue -= acc_ene(*others);
            }        
        }
        isoValue = std::max(0.f,  isoValue);
        return CorrectionCode::Ok;
    }
    void IsolationCloseByCorrectionTool::getExtrapEtaPhi(const xAOD::IParticle* par, float& eta, float& phi) const {
        static const FloatAccessor acc_assocEta{IsolationCloseByCorrectionTool::caloDecors()[0]};
        static const FloatAccessor acc_assocPhi{IsolationCloseByCorrectionTool::caloDecors()[1]};
        static const CharAccessor acc_isDecor{caloDecors()[3]};
        if (par->type() != xAOD::Type::ObjectType::Muon) {
            eta = par->eta();
            phi = par->phi();
        } else if (acc_isDecor.isAvailable(*par) && acc_isDecor(*par)) {
            eta = acc_assocEta(*par);
            phi = acc_assocPhi(*par);
        } else {
            float assoc_ene{0.f};
            static const FloatDecorator dec_assocEta{IsolationCloseByCorrectionTool::caloDecors()[0]};
            static const FloatDecorator dec_assocPhi{IsolationCloseByCorrectionTool::caloDecors()[1]};
            static const  CharDecorator dec_isDecor{caloDecors()[3]};
            associateCluster(par,eta, phi, assoc_ene);
            dec_assocEta(*par) = eta;
            dec_assocPhi(*par) = phi;
            dec_isDecor(*par) = true;
        }
    }
    void IsolationCloseByCorrectionTool::associateFlowElement(const EventContext& ctx, const xAOD::IParticle* particle, float& eta,
                                                              float& phi, float& energy) const {
        phi = eta = energy = 0.;
        PflowSet flowCollection = getAssocFlowElements(ctx, particle);
        if (flowCollection.empty()) {
            phi = particle->phi();
            eta = particle->eta();
            return;
        }
        for (const FlowElementPtr& ele : flowCollection) {
            const float flow_energy = ele->e() * ele.weight;
            if (flow_energy < MinClusterEnergy) continue;
            phi += ele->phi() * flow_energy;
            eta += ele->eta() * flow_energy;
            energy += flow_energy;
        }
        if (energy < MinClusterEnergy) {
            phi = particle->phi();
            eta = particle->eta();
            return;
        }
        phi = xAOD::P4Helpers::deltaPhi(phi / energy, 0.);
        eta /= energy;
    }
    void IsolationCloseByCorrectionTool::associateCluster(const xAOD::IParticle* particle, float& eta, float& phi, float& energy) const {
        phi = particle->phi();
        eta = particle->eta();
        energy = -1.;
        if (particle->type() == xAOD::Type::ObjectType::Muon) {
            const xAOD::Muon* mu = static_cast<const xAOD::Muon*>(particle);
            const xAOD::CaloCluster* cluster = mu->cluster();
            if (!cluster) return;
            energy = cluster->e();
            // At the moment no cluster associated with muons is in the derivations
            int nSample{0};
            float etaT{0.f}, phiT{0.f}, dphiT{0.f};

            for (unsigned int i = 0; i < CaloSampling::Unknown; ++i) {
                CaloSampling::CaloSample s = static_cast<CaloSampling::CaloSample>(i);
                if (cluster->hasSampling(s)) {
                    ATH_MSG_VERBOSE("Sampling: " << i << "eta-phi (" << cluster->etaSample(s) << ", " << cluster->phiSample(s) << ")");
                    etaT += cluster->etaSample(s);
                    if (!nSample)
                        phiT = cluster->phiSample(s);
                    else
                        dphiT += xAOD::P4Helpers::deltaPhi(cluster->phiSample(s), phiT);
                    ++nSample;
                }
            }
            if (!nSample) return;
            ATH_MSG_DEBUG("Eta, phi before sampling: " << eta << ", " << phi << " and after sampling: " << etaT / nSample << ", "
                                                       << phiT / nSample);
            phi = xAOD::P4Helpers::deltaPhi(phiT + dphiT / nSample, 0);
            eta = etaT / nSample;
        }
        if (!isEgamma(particle)) return;
        const xAOD::Egamma* egamm = static_cast<const xAOD::Egamma*>(particle);
        eta = phi = energy = 0.;
        for (unsigned int cl = 0; cl < egamm->nCaloClusters(); ++cl) {
            const xAOD::CaloCluster* prim_cluster = egamm->caloCluster(cl);
            if (!prim_cluster) {
                ATH_MSG_DEBUG("Cluster " << cl << " is not defined " << egamm);
                continue;
            }
            std::vector<const xAOD::CaloCluster*> constituents = xAOD::EgammaHelpers::getAssociatedTopoClusters(prim_cluster);
            for (const xAOD::CaloCluster* cluster : constituents) {
                if (!cluster) continue;

                const float clus_e = clusterEtMinusTile(cluster);
                /// Remove super low energy clusters
                if (clus_e < MinClusterEnergy) continue;
                eta += cluster->eta() * clus_e;
                phi += cluster->phi() * clus_e;
                energy += clus_e;
            }
        }
        if (energy >= MinClusterEnergy) {
            phi = xAOD::P4Helpers::deltaPhi(phi / energy, 0.);
            eta /= energy;
        } else {
            ATH_MSG_DEBUG("Average energy from the clusters is too low " << energy << " copy particle properties");
            eta = egamm->eta();
            phi = egamm->phi();
            energy = egamm->e();
        }
    }
    asg::AcceptData IsolationCloseByCorrectionTool::acceptCorrected(const xAOD::IParticle& x,
                                                                    const xAOD::IParticleContainer& closePar) const {
        if (!m_isInitialised) { ATH_MSG_WARNING("The IsolationCloseByCorrectionTool was not initialised!!!"); }
        assert(!m_selectorTool.empty());
        const IsoVector& iso_types = getIsolationTypes(&x);
        if (iso_types.empty()) {
            // TODO: figure out if this is actually a valid situation
            // or if we should just fail at this point.
            ATH_MSG_WARNING("Could not cast particle for acceptCorrected. Will return false.");
            static const asg::AcceptInfo dummyAcceptInfo = []() {
              asg::AcceptInfo info;
              info.addCut("castCut", "whether we managed to cast to a known type");
              return info;
            }();
            if (m_dec_isoselection) (*m_dec_isoselection)(x) = false;
            return asg::AcceptData(&dummyAcceptInfo);
        }

        if (closePar.empty()) return m_selectorTool->accept(x);
        strObj strPar;
        strPar.isolationValues.resize(numIsolationTypes);
        strPar.pt = x.pt();
        strPar.eta = x.eta();
        strPar.type = x.type();
        std::vector<float> corrections;
        if (getCloseByCorrection(corrections, x, iso_types, closePar) == CorrectionCode::Error) {
            ATH_MSG_WARNING("Could not calculate the corrections. acceptCorrected(x) is done without the corrections.");
            if (m_dec_isoselection) (*m_dec_isoselection)(x) = bool(m_selectorTool->accept(x));
            return m_selectorTool->accept(x);
        }
        for (unsigned int i = 0; i < iso_types.size(); ++i) {
            strPar.isolationValues[iso_types[i]] = corrections[i];
            const SG::AuxElement::Accessor<float>* acc = xAOD::getIsolationAccessor(iso_types.at(i));
            float old = (*acc)(x);
            ATH_MSG_DEBUG("Correcting " << toString(iso_types.at(i)) << " from " << old << " to " << corrections[i]);
        }
        auto accept = m_selectorTool->accept(strPar);
        if (m_dec_isoselection) (*m_dec_isoselection)(x) = bool(accept);
        return accept;
    }
    const xAOD::Vertex* IsolationCloseByCorrectionTool::retrieveIDBestPrimaryVertex(const EventContext& ctx) const {
        SG::ReadHandle<xAOD::VertexContainer> Verticies{m_VtxKey, ctx};
        if (!Verticies.isValid() || !Verticies->size()) {
            ATH_MSG_WARNING("Failed to load vertex collection " << m_VtxKey.key());
            return nullptr;
        }
        for (const xAOD::Vertex* V : *Verticies) {
            if (V->vertexType() == xAOD::VxType::VertexType::PriVtx) return V;
        }
        return nullptr;
    }

    float IsolationCloseByCorrectionTool::coneSize(const xAOD::IParticle* P, IsoType Cone) const {
        using xAOD::Iso::coneSize;
        float ConeDR = coneSize(Cone);
        if (isVarTrackIso(Cone) || isVarTrackIsoTTVA(Cone)) {
            const xAOD::IParticle* Reference = isoRefParticle(P);
            float MiniIso = m_ptvarconeRadius / unCalibPt(Reference);
            if (MiniIso < ConeDR) return MiniIso;
        }
        return ConeDR;
    }
    float IsolationCloseByCorrectionTool::unCalibPt(const xAOD::IParticle* P) const {
        if (!P) {
            ATH_MSG_WARNING("No partcile given. Return stupidly big number. ");
            return 1.e25;
        }
        const xAOD::IParticle* OR = xAOD::getOriginalObject(*P);
        if (!OR) {
            ATH_MSG_VERBOSE("No reference from the shallow copy container of " << particleName(P) << " could be found");
            return P->pt();
        }
        return OR->pt();
    }

    const xAOD::IParticle* IsolationCloseByCorrectionTool::isoRefParticle(const xAOD::IParticle* P) const {
        if (!P) {
            ATH_MSG_ERROR("Nullptr given");
            return nullptr;
        }
        // Use for muons the associated ID track. Else the particle itself
        if (P->type() == xAOD::Type::ObjectType::Muon) {
            const xAOD::Muon* muon = static_cast<const xAOD::Muon*>(P);
            const xAOD::TrackParticle* idTrk = muon->trackParticle(xAOD::Muon::TrackParticleType::InnerDetectorTrackParticle);
            return idTrk ? idTrk : muon->primaryTrackParticle();
        }
        return P;
    }
    bool IsolationCloseByCorrectionTool::isSame(const xAOD::IParticle* P, const xAOD::IParticle* P1) const {
        if (!P || !P1) {
            ATH_MSG_WARNING("Nullptr were given");
            return true;
        }
        return (P == P1);
    }

    float IsolationCloseByCorrectionTool::deltaR2(const xAOD::IParticle* P, const xAOD::IParticle* P1, bool AvgCalo) const {
        if (isSame(P, P1)) return 0.;
        // Check if one of the objects is a CaloCluster or the Averaging over the clusters is requested.
        if (AvgCalo || (P->type() != P1->type() &&
                        (P->type() == xAOD::Type::ObjectType::CaloCluster || P1->type() == xAOD::Type::ObjectType::CaloCluster))) {
            float phi1{0.f}, eta1{0.f}, eta2{0.f}, phi2{0.f};
            getExtrapEtaPhi(P, eta1, phi1);
            getExtrapEtaPhi(P1, eta2, phi2);
            return xAOD::P4Helpers::deltaR2(eta1, phi1, eta2, phi2);
        }
        float dPhi = xAOD::P4Helpers::deltaPhi(P, P1);
        float dEta = P->eta() - P1->eta();
        return dEta * dEta + dPhi * dPhi;
    }
    bool IsolationCloseByCorrectionTool::overlap(const xAOD::IParticle* P, const xAOD::IParticle* P1, float dR) const {
        return (!isSame(P, P1) && deltaR2(P, P1) < (dR * dR));
    }
    float IsolationCloseByCorrectionTool::getOriginalIsolation(const xAOD::IParticle* particle, IsoType isoVariable) const {
        IsoHelperMap::const_iterator itr = m_isohelpers.find(isoVariable);
        float isovalue = 0;
        if (itr == m_isohelpers.end() || itr->second->getOrignalIsolation(particle, isovalue) == CorrectionCode::Error) {
            ATH_MSG_ERROR("Failed to retrive the original isolation cone ");
            isovalue = FLT_MAX;
        }
        return isovalue;
    }
    float IsolationCloseByCorrectionTool::getOriginalIsolation(const xAOD::IParticle& particle, IsoType type) const {
        return getOriginalIsolation(&particle, type);
    }
    float IsolationCloseByCorrectionTool::clusterEtMinusTile(const xAOD::CaloCluster* cluster) {
        float Et{0.f};
        if (cluster) {
            try {
                Et = cluster->p4(xAOD::CaloCluster::State::UNCALIBRATED).Et();
                Et = Et - cluster->eSample(xAOD::CaloCluster::CaloSample::TileGap3) /
                              std::cosh(cluster->p4(xAOD::CaloCluster::State::UNCALIBRATED).Eta());
            } catch (...) { Et = cluster->p4().Et(); }
        }
        return std::max(Et, 0.f);
    }
    std::string IsolationCloseByCorrectionTool::particleName(const xAOD::IParticle* C) { return particleName(C->type()); }
    std::string IsolationCloseByCorrectionTool::particleName(xAOD::Type::ObjectType T) {
        if (T == xAOD::Type::ObjectType::Electron) return "Electron";
        if (T == xAOD::Type::ObjectType::Photon) return "Photon";
        if (T == xAOD::Type::ObjectType::Muon) return "Muon";
        if (T == xAOD::Type::ObjectType::TrackParticle) return "Track";
        if (T == xAOD::Type::ObjectType::CaloCluster) return "Cluster";
        return "Unknown";
    }
    bool IsolationCloseByCorrectionTool::isFixedTrackIso(IsolationType type) { return IsolationFlavour::ptcone == isolationFlavour(type); }
    bool IsolationCloseByCorrectionTool::isVarTrackIso(IsolationType type) { return IsolationFlavour::ptvarcone == isolationFlavour(type); }
    bool IsolationCloseByCorrectionTool::isTrackIso(IsolationType type) {
        return isVarTrackIso(type) || isFixedTrackIso(type) || isTrackIsoTTVA(type);
    }
    bool IsolationCloseByCorrectionTool::isTopoEtIso(IsolationType type) { return IsolationFlavour::topoetcone == isolationFlavour(type); }
    bool IsolationCloseByCorrectionTool::isFixedTrackIsoTTVA(IsolationType type) {
        IsolationFlavour flavour = isolationFlavour(type);
        static const std::set<IsolationFlavour> ttvaFlavours{IsolationFlavour::ptcone_Nonprompt_All_MaxWeightTTVA_pt500,
                                                             IsolationFlavour::ptcone_Nonprompt_All_MaxWeightTTVA_pt1000,
                                                             IsolationFlavour::ptcone_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                             IsolationFlavour::ptcone_Nonprompt_All_MaxWeightTTVALooseCone_pt1000};
        return ttvaFlavours.count(flavour);
    }
    bool IsolationCloseByCorrectionTool::isVarTrackIsoTTVA(IsolationType type) {
        IsolationFlavour flavour = isolationFlavour(type);
        static const std::set<IsolationFlavour> ttvaFlavours{IsolationFlavour::ptvarcone_Nonprompt_All_MaxWeightTTVA_pt500,
                                                             IsolationFlavour::ptvarcone_Nonprompt_All_MaxWeightTTVA_pt1000,
                                                             IsolationFlavour::ptvarcone_Nonprompt_All_MaxWeightTTVALooseCone_pt500,
                                                             IsolationFlavour::ptvarcone_Nonprompt_All_MaxWeightTTVALooseCone_pt1000};
        return ttvaFlavours.count(flavour);
    }
    bool IsolationCloseByCorrectionTool::isTrackIsoTTVA(IsolationType type) { return isFixedTrackIsoTTVA(type) || isVarTrackIsoTTVA(type); }
    void IsolationCloseByCorrectionTool::printIsolationCones(const IsoVector& types, xAOD::Type::ObjectType T) const {
        ATH_MSG_INFO("The following isolation cones are considered for " << particleName(T));
        for (const IsoType& cone : types) { ATH_MSG_INFO("     --- " << toString(cone)); }
    }
    float IsolationCloseByCorrectionTool::trackPtCut(IsolationType type) {
        if (!isTrackIso(type)) return -1;
        IsolationFlavour flavour = isolationFlavour(type);
        static const std::set<IsolationFlavour> Pt1000_Flavours{IsolationFlavour::ptcone_Nonprompt_All_MaxWeightTTVA_pt1000,
                                                                IsolationFlavour::ptcone_Nonprompt_All_MaxWeightTTVALooseCone_pt1000,
                                                                IsolationFlavour::ptvarcone_Nonprompt_All_MaxWeightTTVA_pt1000,
                                                                IsolationFlavour::ptvarcone_Nonprompt_All_MaxWeightTTVALooseCone_pt1000};
        if (Pt1000_Flavours.count(flavour)) return 1000;
        return 500;
    }
    bool IsolationCloseByCorrectionTool::isPFlowIso(IsolationType type) { return isolationFlavour(type) == IsolationFlavour::neflowisol; }
}  // namespace CP
