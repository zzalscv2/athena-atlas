/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
@page AtlasCLHEPDict_page AtlasCLHEPDict Package

This package contains Reflex dictionary definitions for some CLHEP and
HepGeom classes used within ATLAS.

@author R.D.Schaffer@cern.ch, Marcin.Nowak@cern.ch

@section AtlasCLHEPDict_IntroductionAtlasCLHEPDict Introduction

Dictionaries for CLHEP classes used in ATLAS are gathered in this package.
These dictionaries are needed when CLHEP classes were being written out
directly. This has mostly changed with the transient/persistent
separation but in some places there are still used.
The dictionaries are also useful for python interaction with CLHEP classes.

ROOT streamers for old versions or CLHEP classes that were used in Athena r12
and earlier were rmoved in relese 23.
*/
