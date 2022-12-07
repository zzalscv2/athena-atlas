/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//
// Generators/EvtGen_i/EvtDecay.h
//
// This module takes HepMC events from Storegate and decays 
// all B weak decays including the secondaries unstable particles
// in EvtGen.
// It is a TopAlg can be used in conjunction Pythia PythiaB 
//   and SingleParticleGun 
//
// November 2003: Maria Smizanska 
// August 2002: Malte Muller,   maltemlr@yahoo.co.uk
// Original LHCb code by Witold Pokorski
//
// February 2004: James R Catmore
// Additional TEMPORARY amendments to allow use to 
// set random seed from JobOptions. Does not follow ATLAS coding rules.
//
// January 2007: Pavel Reznicek
// Proper random seeds handling, code cleanup
//
//*****************************************************************************

#ifndef GENERATORMODULESEVTDECAY_H
#define GENERATORMODULESEVTDECAY_H

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"

#include "EvtGenBase/EvtParticle.hh"
#include "EvtGen/EvtGen.hh"
#include "EvtGenBase/EvtRandomEngine.hh"

#include "AthenaBaseComps/AthAlgorithm.h"

#include "GeneratorObjects/McEventCollection.h"
#include "AthenaKernel/IAthRNGSvc.h"
#include "CLHEP/Random/RandomEngine.h"

class EvtCLHepRandom : public EvtRandomEngine {

public:
  EvtCLHepRandom(CLHEP::HepRandomEngine* engine);
  double random();
  virtual ~EvtCLHepRandom();
  CLHEP::HepRandomEngine* getEngine() { return m_engine; }
private:
  CLHEP::HepRandomEngine* m_engine{};
};


class EvtDecay : public AthAlgorithm {

public:
  EvtDecay(const std::string& name, ISvcLocator* pSvcLocator);
  ~EvtDecay();

  StatusCode initialize();
  StatusCode execute();
  StatusCode finalize();

  // To be called from PythiaBModule
  // StatusCode initializeUser( string userDecFileName, int targetID );
  // StatusCode executeSingle( McEvent* mcEvt, int& reject );

private:
  StatusCode callEvtGen( HepMC::GenEvent* hepMCevt );
  StatusCode makeHepMC( EvtParticle*, HepMC::GenEvent*, HepMC::GenParticle* );


class EvtDecay : public Algorithm {
	
	public:
		EvtDecay(const std::string& name, ISvcLocator* pSvcLocator);
		~EvtDecay();
		
		StatusCode initialize();
		StatusCode execute();
		StatusCode finalize();
		
		// To be called from PythiaBModule
//		StatusCode initializeUser( string userDecFileName, int targetID );
//		StatusCode executeSingle( McEvent* mcEvt, int& reject );
		
	private:
		StatusCode callEvtGen( HepMC::GenEvent* hepMCevt );
		StatusCode makeHepMC( EvtParticle*, HepMC::GenEvent*, HepMC::GenParticle* );
		
//		bool isBMeson(const int pID) const;
//		bool isBBaryon(const int pID) const;
//		bool isExcited(const int pID) const;
//		string longToStr(const long n) const;
		bool isGoodB(const int pID, const int test) const;
		void MeVToGeV(HepMC::GenEvent* hepMCevt);
                void GeVToMeV(HepMC::GenEvent* hepMCevt);
		void setLambdabSpinDensityMatrix(EvtParticle* part,double Polarization);
     
		// Data members
		int m_targetID;

  // Random number service
  ServiceHandle<IAthRNGSvc> m_rndmSvc{this, "RndmSvc", "AthRNGSvc"};

  //Gen_tf run args.
  IntegerProperty m_dsid{this, "Dsid", 999999};

  /// Seed for random number engine
  IntegerProperty m_randomSeed{this, "RandomSeed", 1234567, "Random seed for the built-in random engine"}; // FIXME make this into an unsigned long int?

//		const DataHandle<McEventCollection> m_McEvtColl;
		McEventCollection* m_McEvtColl;
  
		// For EvtGen interface
		EvtGen* m_myGen;
		EvtCLHepRandom* m_evtAtRndmGen;
	
		// jobOption parameters
		std::string m_DecayDec, m_PdtTable, m_userDecFileName;
		double m_LambdabPol;
		bool   m_PolarizedLambdab;
		std::string m_randomStreamName, m_inputKeyName;

};

#endif

