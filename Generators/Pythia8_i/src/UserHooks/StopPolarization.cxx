/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "Pythia8_i/UserHooksFactory.h"
#include "UserHooksUtils.h"
#include "UserSetting.h"
#include "Pythia8/PhaseSpace.h"

namespace Pythia8 {

  /// Transform resonant stop quark decays to the angular distribution of a specified polarisation state
  //
  // Polarisation parameter set separately for each neutralino state
  struct StopPolarization : public UserHooks {

    StopPolarization() : m_polarization1("StopPolarization:poltopneut1", 0.0),
			 m_polarization2("StopPolarization:poltopneut2", 0.0),
			 m_polarization3("StopPolarization:poltopneut3", 0.0),
			 m_polarization4("StopPolarization:poltopneut4", 0.0) { }

    bool canVetoResonanceDecays() { return true; }

    bool doVetoResonanceDecays(Event& process) {
      const double top_neut_polarization[4] = {m_polarization1(settingsPtr),
					       m_polarization2(settingsPtr),
					       m_polarization3(settingsPtr),
					       m_polarization4(settingsPtr)};
      const int neutralino_ids[4]={1000022,1000023,1000025,100035};

      for (int i = 1; i < process.size(); ++i) {
        // Select stop -> top neutralino
        Particle& v = process[i];
        if (v.idAbs() != 1000006 ) continue; // has to be stop
	if ((v.daughter2()-v.daughter1())!=1) continue; // has to be two body decay
        // Find stop daughter particles
        Particle& d1 = process[v.daughter1()];
        Particle& d2 = process[v.daughter2()];
	if (d1.idAbs()==6 || d2.idAbs()==6) { // decay to top+X

	  if (d2.idAbs()==6) {
	    d1 = process[v.daughter2()];
	    d2 = process[v.daughter1()];
	  }
	  int neutralino_type;
	  for (neutralino_type = 0; neutralino_type < 4; neutralino_type++) {
	    if (d2.idAbs()==neutralino_ids[neutralino_type]) break;
	  }
	  if (neutralino_type==4) continue; // did not find neutralino
	  if ((d1.daughter2()-d1.daughter1())!=1) continue; // not a two-body top decay?
	  const Vec4 top = d1.p();
	  Vec4 neutralino = d2.p();

	  Particle& td1 = process[d1.daughter1()];
	  Particle& td2 = process[d1.daughter2()];
	  if (td1.idAbs() != 5) {
	    td1 = process[d1.daughter2()];
	    td2 = process[d1.daughter1()];
	  }
	  if (td1.idAbs() != 5 || td2.idAbs()!=24) continue; // not a top->Wb decay
	  Particle& wd1 = process[td2.daughter1()];
	  Particle& wd2 = process[td2.daughter2()];
	  int wd1_id=wd1.idAbs();
	  if (wd1_id==2 || wd1_id==4 || wd1_id==12 || wd1_id==14 || wd1_id==16 ) {
	    wd1 = process[td2.daughter2()];
	    wd2 = process[td2.daughter1()];
	  }

	  // Get daughter momenta in the top rest frame
	  Vec4 bottom = td1.p();
	  Vec4 Wboson = td2.p();
	  Vec4 Lepton = wd1.p();  //can also be quark
	  Vec4 Neutrino = wd2.p();
	  bottom.bstback(top);
	  Wboson.bstback(top);
	  Lepton.bstback(top);
	  Neutrino.bstback(top);
	  neutralino.bstback(top);

	  while(true) {
	    // rotate randomly b+W system
	    const double dtheta = acos(2*rand01() - 1);
	    const double dphi = 2*M_PI*rand01();
	    bottom.rot(dtheta, dphi);
	    Wboson.rot(dtheta, dphi);
	    Lepton.rot(dtheta, dphi);
	    Neutrino.rot(dtheta, dphi);

	    // rotate randomly W decay system
	    const double dtheta2 = acos(2*rand01() - 1);
	    const double dphi2 = 2*M_PI*rand01();
	    Lepton.bstback(Wboson);
	    Neutrino.bstback(Wboson);
	    Lepton.rot(dtheta2, dphi2);
	    Neutrino.rot(dtheta2, dphi2);
	    Lepton.bst(Wboson);
	    Neutrino.bst(Wboson);
	    // sample from polarization distribution
	    const double thlrot2 = theta(neutralino, Lepton);
	    const double polarization = top_neut_polarization[neutralino_type];
	    if (rand01() < (1 + polarization*(cos(thlrot2)))/(1+fabs(polarization))) break;
	  }

	  bottom.bst(top);
	  Wboson.bst(top);
	  Lepton.bst(top);
	  Neutrino.bst(top);
	  td1.p(bottom);
	  td2.p(Wboson);
	  wd1.p(Lepton);
	  wd2.p(Neutrino);
	}
      }
      return false;
    }

    double rand01() { return rndmPtr->flat(); }

    Pythia8_UserHooks::UserSetting<double> m_polarization1;
    Pythia8_UserHooks::UserSetting<double> m_polarization2;
    Pythia8_UserHooks::UserSetting<double> m_polarization3;
    Pythia8_UserHooks::UserSetting<double> m_polarization4;

  };


  Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::StopPolarization> STOPPOL("StopPolarization");

}
