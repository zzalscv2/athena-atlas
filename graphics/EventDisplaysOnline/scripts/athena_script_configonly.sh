#!/bin/sh
exec athena.py --config-only=/atlas/EventDisplayEvents/EventProcessorConfig/EventDisplaysOnline_EventProcessorConfig_$( date '+%F_%H_%M_%S' ).pkl $*
