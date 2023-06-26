#include "../AthenaEventLoopMgr.h"
#include "../PyAthenaEventLoopMgr.h"
#include "../AthenaOutputStream.h"
#include "../AthenaOutputStreamTool.h"
#include "../AthenaConditionStream.h"
#include "../MultipleEventLoopMgr.h"
#include "../MixingEventSelector.h"
#include "../ThinningCacheTool.h"
//#include "../EventDumperSvc.h"
#include "../FPEControlSvc.h"
#include "../JobIDSvc.h"
#include "../CoreDumpSvc.h"
#include "../AthDictLoaderSvc.h"
#include "../PageAccessControlSvc.h"
#include "../DecisionSvc.h"
#include "../ItemListSvc.h"
#include "../AthenaSummarySvc.h"
#include "../LoggedMessageSvc.h"
#include "../RCUSvc.h"
#include "../AthTPCnvSvc.h"
#include "../EvtIdModifierSvc.h"
#include "../TestRandomSeqAlg.h"
#include "../MetaDataSvc.h"
#include "../OutputStreamSequencerSvc.h"
#include "../AthenaHiveEventLoopMgr.h"
#include "../AthenaMtesEventLoopMgr.h"
#include "../AthIncFirerAlg.h"
#include "../ToyNextPassFilterAlg.h"
#include "../ToyNextPassFilterTool.h"
#include "../ConditionsCleanerSvc.h"
#include "../DelayedConditionsCleanerSvc.h"
#include "../DecisionAlg.h"
#include "../AthReadAlg.h"
#include "../../test/MetaDataToolStub.h"

DECLARE_COMPONENT( AthenaOutputStream )
DECLARE_COMPONENT( AthenaConditionStream )
DECLARE_COMPONENT( TestRandomSeqAlg )
DECLARE_COMPONENT( MultipleEventLoopMgr )
DECLARE_COMPONENT( AthenaEventLoopMgr )
DECLARE_COMPONENT( AthenaHiveEventLoopMgr )
DECLARE_COMPONENT( AthenaMtesEventLoopMgr )
DECLARE_COMPONENT( PyAthenaEventLoopMgr )
DECLARE_COMPONENT( MixingEventSelector )
DECLARE_COMPONENT( FPEControlSvc )
DECLARE_COMPONENT( JobIDSvc )
DECLARE_COMPONENT( CoreDumpSvc )
DECLARE_COMPONENT( PageAccessControlSvc )
DECLARE_COMPONENT( AthDictLoaderSvc )
DECLARE_COMPONENT( DecisionSvc )
DECLARE_COMPONENT( ItemListSvc )
DECLARE_COMPONENT( AthenaSummarySvc )
DECLARE_COMPONENT( LoggedMessageSvc )
DECLARE_COMPONENT( Athena::RCUSvc )
DECLARE_COMPONENT( AthTPCnvSvc )
DECLARE_COMPONENT( EvtIdModifierSvc )
DECLARE_COMPONENT( MetaDataSvc )
DECLARE_COMPONENT( OutputStreamSequencerSvc )
DECLARE_COMPONENT( AthenaOutputStreamTool )
DECLARE_COMPONENT( Athena::ThinningCacheTool )
DECLARE_COMPONENT( AthIncFirerAlg )
DECLARE_COMPONENT( ToyNextPassFilterAlg )
DECLARE_COMPONENT( ToyNextPassFilterTool )
DECLARE_COMPONENT( Athena::ConditionsCleanerSvc )
DECLARE_COMPONENT( Athena::DelayedConditionsCleanerSvc )
DECLARE_COMPONENT( DecisionAlg )
DECLARE_COMPONENT( AthReadAlg )
DECLARE_COMPONENT( MetaDataToolStub )
