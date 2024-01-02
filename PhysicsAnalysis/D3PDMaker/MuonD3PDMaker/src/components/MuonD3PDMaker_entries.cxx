#include "../MuonTrkHitFillerTool.h" //Serhan
#include "../MuonNumberOfSegmentsFillerTool.h"
#include "../MuonTrackParticleAssociationTool.h"
#include "../MuonGenParticleAssociationTool.h"
#include "../TruthMuonsToSG.h" // Srivas
#include "../MuonTruthClassificationFillerTool.h" // Max
//#include "../MuonIDIsolTool.h" // Lashkar

#include "../MDTSimHitFillerTool.h"
#include "../TrackRecordFillerTool.h"
#include "../MuonTruthHitsFillerTool.h"
#include "../TrackRecordCollectionGetterTool.h"


DECLARE_COMPONENT( D3PD::MuonTrkHitFillerTool ) // Serhan
DECLARE_COMPONENT( D3PD::MuonNumberOfSegmentsFillerTool )
DECLARE_COMPONENT( D3PD::MuonTrackParticleAssociationTool )
DECLARE_COMPONENT( D3PD::MuonGenParticleAssociationTool )
DECLARE_COMPONENT( D3PD::MuonTruthClassificationFillerTool ) // Max
DECLARE_COMPONENT( D3PD::TruthMuonsToSG ) // Srivas
// DECLARE_COMPONENT( D3PD::MuonIDIsolTool ) // Lashkar

DECLARE_COMPONENT( D3PD::MDTSimHitFillerTool )
DECLARE_COMPONENT( D3PD::TrackRecordFillerTool )
DECLARE_COMPONENT( D3PD::MuonTruthHitsFillerTool )
DECLARE_COMPONENT( D3PD::TrackRecordCollectionGetterTool )

