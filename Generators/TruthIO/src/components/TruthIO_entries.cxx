#include "TruthIO/DumpMC.h"
#include "TruthIO/HepMCReadFromFile.h"
#include "TruthIO/WriteHepMC.h"
#include "TruthIO/PrintMC.h"
#include "TruthIO/PrintHijingPars.h"
#ifndef HEPMC3
#include "TruthIO/ReadHepEvtFromAscii.h"
#endif

DECLARE_COMPONENT( DumpMC )
DECLARE_COMPONENT( HepMCReadFromFile )
DECLARE_COMPONENT( PrintMC )
DECLARE_COMPONENT( WriteHepMC )
DECLARE_COMPONENT( PrintHijingPars )
#ifndef HEPMC3
DECLARE_COMPONENT( ReadHepEvtFromAscii )
#endif
