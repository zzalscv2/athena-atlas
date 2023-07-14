#!/bin/bash

# set -E

asetup () {
	source $AtlasSetup/scripts/asetup.sh $@
	return $?
}

build="$1"
version="$2"
tag="$3"

asetup "$build,$version"

echo

# Header
# Release,e-tag,HepMC,Herwig7,MadGraph,Pythia8,Sherpa,EvtGen,Lhapdf,Starlight,Superchic,Rivet,PowhegATLAS
# 21.6.16,e7947,2.6.9,7.1.5,2.6.7.atlas4,243,2.2.8,1.7.0,6.2.3,r313,3.0.5,2.7.2b-y177,00-04-02

if [[ -z ${HepMCVERS+x} ]]; then
  if [[ "${HEPMCVER}" = "3" ]]; then
    # Can't reproduce the full version number from the release, need to resort to a hack
    HepMCVERS=$(echo "$LD_LIBRARY_PATH" | grep -o -E "hepmc3/[0-9]+.[0-9]+.[0-9]+")
    HepMCVERS=${HepMCVERS#hepmc3/}
  else
    HepMCVERS=$(echo "$LD_LIBRARY_PATH" | grep -o -E "HepMC/[0-9]+.[0-9]+.[0-9]+")
    HepMCVERS=${HepMCVERS#HepMC/}
    HepMCVERS=${HepMCVERS//\.0/.}
  fi
  echo "HepMCVERS=$HepMCVERS"
else
  HepMCVERS=${HepMCVERS//\.0/.}
  echo "HepMCVERS=$HepMCVERS (from release)"
fi

if [[ -z ${HERWIG7VER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]] && cmt show versions External/Herwig7 > /dev/null 2>&1; then
    HERWIG7VER=$(cmt show versions External/Herwig7 | head -n 1 | grep -E "Herwig7-[0-9][0-9]-[0-9][0-9]-[0-9][0-9]" -o)
    HERWIG7VER=${HERWIG7VER#Herwig7-}
    HERWIG7VER=${HERWIG7VER//-/.}
    HERWIG7VER=${HERWIG7VER//\.0/.}
    HERWIG7VER=${HERWIG7VER:1}
  else
    HERWIG7VER="None"
  fi
  echo "HERWIG7VER=$HERWIG7VER"
else
  echo "HERWIG7VER=$HERWIG7VER (from release)"
fi

if [[ -z ${MADGRAPHVER+x} ]]; then
  MADGRAPHVER=$(echo "$MADPATH" | grep -E "MG5_aMC_v.*" -o)
  MADGRAPHVER=${MADGRAPHVER#MG5_aMC_v}
  MADGRAPHVER=${MADGRAPHVER//_/.}
  echo "MADGRAPHVER=$MADGRAPHVER"
else
  echo "MADGRAPHVER=$MADGRAPHVER (from release)"
fi

if [[ -z ${PYTHIA8VER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]]; then
    PYTHIA8VER=$(cmt show versions External/Pythia8 | head -n 1 | grep -E "Pythia8-[0-9][0-9]-[0-9][0-9]" -o)
    PYTHIA8VER=${PYTHIA8VER#Pythia8-}
    PYTHIA8VER=${PYTHIA8VER//-/}
    PYTHIA8VER=${PYTHIA8VER:1}
  else
    PYTHIA8VER="None"
  fi
  echo "PYTHIA8VER=$PYTHIA8VER"
else
  echo "PYTHIA8VER=$PYTHIA8VER (from release)"
fi

if [[ -z ${SHERPAVER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]]; then
    SHERPAVER=$(cmt show versions External/Sherpa | head -n 1 | grep -E "Sherpa-[0-9][0-9]-[0-9][0-9]-[0-9][0-9]" -o)
    SHERPAVER=${SHERPAVER#Sherpa-}
    SHERPAVER=${SHERPAVER//-/.}
    SHERPAVER=${SHERPAVER//\.0/.}
    SHERPAVER=${SHERPAVER:1}
  else
    SHERPAVER="None"
  fi
  echo "SHERPAVER=$SHERPAVER"
else
  echo "SHERPAVER=$SHERPAVER (from release)"
fi

if [[ -z ${EVTGENVER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]]; then
    EVTGENVER=$(cmt show versions External/EvtGen | head -n 1 | grep -E "EvtGen-[0-9][0-9]-[0-9][0-9]-[0-9][0-9]" -o)
    EVTGENVER=${EVTGENVER#EvtGen-}
    EVTGENVER=${EVTGENVER//-/.}
    EVTGENVER=${EVTGENVER//\.0/.}
    EVTGENVER=${EVTGENVER:1}
  else
    EVTGENVER="None"
  fi
  echo "EVTGENVER=$EVTGENVER"
else
  echo "EVTGENVER=$EVTGENVER (from release)"
fi

if [[ -z ${LHAPDFVER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]]; then
    LHAPDFVER=$(cmt show versions External/Lhapdf | head -n 1 | grep -E "Lhapdf-[0-9][0-9]-[0-9][0-9]-[0-9][0-9]" -o)
    LHAPDFVER=${LHAPDFVER#Lhapdf-}
    LHAPDFVER=${LHAPDFVER//-/.}
    LHAPDFVER=${LHAPDFVER//\.0/.}
    LHAPDFVER=${LHAPDFVER:1}
  else
    LHAPDFVER="None"
  fi
  echo "LHAPDFVER=$LHAPDFVER"
else
  echo "LHAPDFVER=$LHAPDFVER (from release)"
fi

if [[ -z ${STARLIGHTVER+x} ]]; then
  STARLIGHTVER=None
fi

if [[ -z ${SUPERCHICVER+x} ]]; then
  SUPERCHICVER=None
fi

if [[ -z ${RIVETVER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]]; then
    RIVETVER=$(cmt show versions External/Rivet | head -n 1 | grep -E "Rivet-[0-9][0-9]-[0-9][0-9]-[0-9][0-9]" -o)
    RIVETVER=${RIVETVER#Rivet-}
    RIVETVER=${RIVETVER//-/.}
    RIVETVER=${RIVETVER//\.0/.}
    RIVETVER=${RIVETVER:1}
  else
    RIVETVER="None"
  fi
  echo "RIVETVER=$RIVETVER"
else
  echo "RIVETVER=$RIVETVER (from release)"
fi

if [[ -z ${POWHEGVER+x} ]]; then
  if [[ -x "$(command -v cmt)" ]]; then
    POWHEGVER=$(cmt show versions External/Powheg | head -n 1 | grep -E "Powheg-[0-9][0-9]-[0-9][0-9]-[0-9][0-9]" -o)
    POWHEGVER=${POWHEGVER#Powheg-}
  else
    POWHEGVER=$(echo "$POWHEGPATH" | grep -E "ATLASOTF-.*" -o)
    POWHEGVER=${POWHEGVER#ATLASOTF-}
  fi
  echo "POWHEGVER=$POWHEGVER"
else
  echo "POWHEGVER=$POWHEGVER (from release)"
fi

echo

echo "$version,$tag,$HepMCVERS,$HERWIG7VER,$MADGRAPHVER,$PYTHIA8VER,$SHERPAVER,$EVTGENVER,$LHAPDFVER,$STARLIGHTVER,$SUPERCHICVER,$RIVETVER,$POWHEGVER"
