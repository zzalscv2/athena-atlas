#include "../FPGATrackSimRawHitsWrapperAlg.h"
#include "../FPGATrackSimRawNtupleWrapperAlg.h"
#include "../FPGATrackSimReadLogicalHitsAlg.h"
#include "../FPGATrackSimLogicalHitsWrapperAlg.h"
#include "../FPGATrackSimReadRawRandomHitsTool.h"
#include "../FPGATrackSimInputHeaderTool.h"
#include "FPGATrackSimInput/FPGATrackSimRawToLogicalHitsTool.h"
#include "../FPGATrackSimDetectorTool.h"
#include "../FPGATrackSimDumpDetStatusAlgo.h"
#include "../FPGATrackSimOutputHeaderTool.h"
#include "../FPGATrackSimDumpOutputStatAlg.h"


DECLARE_COMPONENT( FPGATrackSimDetectorTool )
DECLARE_COMPONENT( FPGATrackSimReadRawRandomHitsTool )
DECLARE_COMPONENT( FPGATrackSimInputHeaderTool )
DECLARE_COMPONENT( FPGATrackSimRawToLogicalHitsTool )
DECLARE_COMPONENT( FPGATrackSimOutputHeaderTool )

DECLARE_COMPONENT( FPGATrackSimDumpDetStatusAlgo )
DECLARE_COMPONENT( FPGATrackSimRawHitsWrapperAlg )
DECLARE_COMPONENT( FPGATrackSimRawNtupleWrapperAlg )
DECLARE_COMPONENT( FPGATrackSimReadLogicalHitsAlg)
DECLARE_COMPONENT( FPGATrackSimLogicalHitsWrapperAlg)
DECLARE_COMPONENT( FPGATrackSimDumpOutputStatAlg )

