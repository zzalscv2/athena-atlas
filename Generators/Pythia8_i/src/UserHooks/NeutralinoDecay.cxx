/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "Pythia8_i/UserHooksFactory.h"
#include "UserHooksUtils.h"
#include "UserSetting.h"
#include "Pythia8/PhaseSpace.h"

namespace Pythia8 {
  // helper function
  constexpr double sqr(double x) { return x*x; }
  
  /// Transform neutralino decays to leptons according to the relative mass sign assuming
  //  production through virtual Z*
  //
  // signed mass parameters are given separately for each neutralino state
  struct NeutralinoDecay : public UserHooks {

    NeutralinoDecay() : m_mass_neut1("NeutralinoDecay:massneut1", 0.0),
			m_mass_neut2("NeutralinoDecay:massneut2", 0.0),
			m_mass_neut3("NeutralinoDecay:massneut3", 0.0),
			m_mass_neut4("NeutralinoDecay:massneut4", 0.0) { }

    bool canVetoResonanceDecays() { return true; }

    int neutralinoIdx(const int id) {
      const int neutralino_ids[4]={1000022,1000023,1000025,100035};
      for(int ii=0;ii<4;ii++)
	if (neutralino_ids[ii]==id) return ii;
      return -1;
    }

    bool isNeutralino(const int id) {
      return neutralinoIdx(id)!=-1;
    }

    bool isLepton(const int id) {
      return (id==11)||(id==13)||(id==15);
    }

    double amplitudeSq(double x, double y, double z, double r1, double rZ, double masssign) {
      // as per PHYSICAL REVIEW D, VOLUME 60, 015006
      double A=1/(z-rZ*rZ);
      double A2=A*A;
      double M2=(2*(A2+A2)*(1-y)*(y-r1*r1)+
		 2*(A2+A2)*(1-x)*(x-r1*r1)+
		 masssign*4*(A2+A2)*r1*z);
      return M2;
    }

    bool doVetoResonanceDecays(Event& process) {
      const double neut_masses[4] = {m_mass_neut1(settingsPtr),
				     m_mass_neut2(settingsPtr),
				     m_mass_neut3(settingsPtr),
				     m_mass_neut4(settingsPtr)};

      for (int i = 1; i < process.size(); ++i) {
        // Select neutralino -> neutralino l+ l-
        Particle& v = process[i];
        if (!isNeutralino(v.idAbs())) continue; // has to be neutralino
	if ((v.daughter2()-v.daughter1())!=2) continue; // has to be three body decay
        // Find neutralino daughter particles
	int d1idx=0; 
	int d2idx=1; 
	int d3idx=2;
        Particle d1 = process[v.daughter1()];
        Particle d2 = process[v.daughter1()+1];
        Particle d3 = process[v.daughter2()];
	if (isNeutralino(d2.idAbs())) {
	    d1 = process[v.daughter1()+1];
	    d2 = process[v.daughter1()];
	    d1idx=1;
	    d2idx=0;
	} else if (isNeutralino(d3.idAbs())) {
	    d1 = process[v.daughter2()];
	    d3 = process[v.daughter1()];
	    d1idx=2;
	    d3idx=0;
	}
	if (!isNeutralino(d1.idAbs())) continue;
	if (!isLepton(d2.idAbs())) continue;
	if (!isLepton(d3.idAbs())) continue;

	int neut2idx=neutralinoIdx(v.idAbs());
	int neut1idx=neutralinoIdx(d1.idAbs());
	double masssign=neut_masses[neut2idx]*neut_masses[neut1idx];
	if (masssign==0) continue; // ignore neutralinos with no specified signed mass
	masssign=masssign/fabs(masssign);

	const Vec4 neut2 = v.p();
	Vec4 neut1 = d1.p();
	Vec4 lep1 = d2.p();
	Vec4 lep2 = d3.p();

	// boost to decay rest frame
	neut1.bstback(neut2);
	lep1.bstback(neut2);
	lep2.bstback(neut2);

	// we will keep angular distribution of the decay plane
        double phi=neut1.phi();
	double theta=neut1.theta();
	double phil=lep1.phi();

	// resample decay in Dalitz plane according to distribution from https://arxiv.org/pdf/0704.2515.pdf
	double m2=v.m();
	double m1=d1.m();
	double mL=d2.m(); // will assume both leptons have the same mass
	double r1=m1/m2;
	double rL=mL/m2;
	double rZ=91.2/m2;
	double r1Sq=r1*r1;
	double rLSq=rL*rL;

	double xmax,ymax,zmax; //point of largest amplitude
	if (masssign>0) 
	  zmax=sqr(m2-m1)/sqr(m2);
	else
	  zmax=0;
	xmax=(1+r1Sq-zmax)/2.;
	ymax=xmax;
	double maxVal=amplitudeSq(xmax,ymax,zmax,r1,rZ,masssign);
	double x,y,z;
	while(true) {
	  // sample normalized m(N1+l1)^2 and m(N1+l2)^2
	  x=randintv(sqr(rL+r1),sqr(1-rL));
	  y=randintv(sqr(rL+r1),sqr(1-rL));
	  // calculate Dalitz limits - based on PDG formula
	  double E2=(x-rLSq+r1Sq)/(2*sqrt(x));
	  double E3=(1-x-rLSq)/(2*sqrt(x));
	  double ymax=sqr(E2+E3)-sqr(sqrt(E2*E2-r1Sq)-sqrt(E3*E3-rLSq));
	  double ymin=sqr(E2+E3)-sqr(sqrt(E2*E2-r1Sq)+sqrt(E3*E3-rLSq));
	  if (y<ymin) continue;
	  if (y>ymax) continue;
	  // di-lepton mass squared (normalized by parent mass squared)
	  z=1+r1Sq+2*rLSq-x-y;
	  double weight=amplitudeSq(x,y,z,r1,rZ,masssign)/maxVal;
	  if (rand01()>weight) continue;
	  break;
	}
	// convert mass pair values into 4-vector in Dalitz decay plane
	// neutralino will be pointing in z-direction and leptons are in x-z plane
	double E1=(m1*m1+(1-z)*m2*m2)/(2*m2);
	double E2=(mL*mL+(1-y)*m2*m2)/(2*m2);
	double E3=(mL*mL+(1-x)*m2*m2)/(2*m2);
	double pz1=sqrt(E1*E1-m1*m1);
	double pz2=(E3*E3-E2*E2-pz1*pz1)/(2*pz1);
	double pz3=(E2*E2-E3*E3-pz1*pz1)/(2*pz1);
	double px=sqrt(E3*E3-mL*mL-pz3*pz3);
	Vec4 new_neut1(0,0,pz1,E1);
	Vec4 new_lep1(px,0,pz2,E2);
	Vec4 new_lep2(-px,0,pz3,E3);
	new_lep1.rot(0,phil);
	new_lep2.rot(0,phil);
	new_neut1.rot(theta,phi);
	new_lep1.rot(theta,phi);
	new_lep2.rot(theta,phi);
	
	new_neut1.bst(neut2);
	new_lep1.bst(neut2);
	new_lep2.bst(neut2);
	
	process[v.daughter1()+d1idx].p(new_neut1);
	process[v.daughter1()+d2idx].p(new_lep1);
	process[v.daughter1()+d3idx].p(new_lep2);
      }
      return false;
    }

    double rand01() { return rndmPtr->flat(); }
    double randintv(double min,double max) { return min+(rand01()*(max-min)); }

    Pythia8_UserHooks::UserSetting<double> m_mass_neut1;
    Pythia8_UserHooks::UserSetting<double> m_mass_neut2;
    Pythia8_UserHooks::UserSetting<double> m_mass_neut3;
    Pythia8_UserHooks::UserSetting<double> m_mass_neut4;

  };


  Pythia8_UserHooks::UserHooksFactory::Creator<Pythia8::NeutralinoDecay> NEUTDECAY("NeutralinoDecay");


}
