// -*- c++ -*-

/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEVENT_TRIGMUONEFTRACK_H
#define TRIGMUONEVENT_TRIGMUONEFTRACK_H

// STL include(s):
#include <string>
#include <sstream>
#include <map>

// Gaudi/Athena include(s):
#include "FourMom/P4IPtCotThPhiM.h"
#include "CLIDSvc/CLASS_DEF.h"
#include "Navigation/NavigableTerminalNode.h"
#include "EventKernel/INavigable4Momentum.h"

// Forward declaration(s):
class MsgStream;

class TrigMuonEFTrack : public P4IPtCotThPhiM,
public NavigableTerminalNode,
virtual public INavigable4Momentum
{
public:

	/** Default constructor */
	TrigMuonEFTrack( );

	/** Default constructor */
	TrigMuonEFTrack( double theIPt, double theCotTh, double thePhi, double theM );

	/** Copy constructor */
	TrigMuonEFTrack( const TrigMuonEFTrack& rhs );

	/** Assignement operator */
	TrigMuonEFTrack& operator=( const TrigMuonEFTrack& rhs );

	/** Destructor */
	~TrigMuonEFTrack();

	/* Constants */
	static const int CHARGE_NA = -999;

	/* set & get methods */
	void setPerigee(double iPt, double cotTh, double phi, double mass);

	double Charge() const { return m_charge; }
	double charge() const { return this->Charge(); }

	void setCharge( double theCharge )
	{
		this->m_charge = theCharge;
	}

	double d0() const
	{
		return m_d0;
	}

	void setD0(double m_d0)
	{
		this->m_d0 = m_d0;
	}

	double z0() const
	{
		return m_z0;
	}

	void setZ0(double m_z0)
	{
		this->m_z0 = m_z0;
	}

	double chi2() const
	{
		return m_chi2;
	}

	void setChi2(double m_chi2)
	{
		this->m_chi2 = m_chi2;
	}

	double chi2prob() const
	{
		return m_chi2prob;
	}

	void setChi2prob(double m_chi2prob)
	{
		this->m_chi2prob = m_chi2prob;
	}
	double posX() const
	{
		return m_posx;
	}

	void setPosX(double m_posx)
	{
		this->m_posx = m_posx;
	}

	double posY() const
	{
		return m_posy;
	}

	void setPosY(double m_posy)
	{
		this->m_posy = m_posy;
	}

	double posZ() const
	{
		return m_posz;
	}

	void setPosZ(double m_posz)
	{
		this->m_posz = m_posz;
	}


	unsigned short int NRpcHitsPhi() const
	{
		return m_nRpcHitsPhi;
	}

	void setNRpcHitsPhi(unsigned short int m_nRpcHitsPhi)
	{
		this->m_nRpcHitsPhi = m_nRpcHitsPhi;
	}

	unsigned short int NTgcHitsPhi() const
	{
		return m_nTgcHitsPhi;
	}

	void setNTgcHitsPhi(unsigned short int m_nTgcHitsPhi)
	{
		this->m_nTgcHitsPhi = m_nTgcHitsPhi;
	}

	unsigned short int NCscHitsPhi() const
	{
		return m_nCscHitsPhi;
	}

	void setNCscHitsPhi(unsigned short int m_nCscHitsPhi)
	{
		this->m_nCscHitsPhi = m_nCscHitsPhi;
	}

	unsigned short int NRpcHitsEta() const
	{
		return m_nRpcHitsEta;
	}

	void setNRpcHitsEta(unsigned short int m_nRpcHitsEta)
	{
		this->m_nRpcHitsEta = m_nRpcHitsEta;
	}

	unsigned short int NTgcHitsEta() const
	{
		return m_nTgcHitsEta;
	}

	void setNTgcHitsEta(unsigned short int m_nTgcHitsEta)
	{
		this->m_nTgcHitsEta = m_nTgcHitsEta;
	}

	unsigned short int NCscHitsEta() const
	{
		return m_nCscHitsEta;
	}

	void setNCscHitsEta(unsigned short int m_nCscHitsEta)
	{
		this->m_nCscHitsEta = m_nCscHitsEta;
	}


	unsigned short int NMdtHits() const
	{
		return m_nMdtHits;
	}

	void setNMdtHits(unsigned short int m_nMdtHits)
	{
		this->m_nMdtHits = m_nMdtHits;
	}

private:

	double m_charge;
	double m_d0;
	double m_z0;
	double m_chi2;
	double m_chi2prob;
	double m_posx;
	double m_posy;
	double m_posz;
	unsigned short int m_nMdtHitsPhi;  // legacy
	unsigned short int m_nRpcHitsPhi;
	unsigned short int m_nTgcHitsPhi;
	unsigned short int m_nCscHitsPhi;
	unsigned short int m_nMdtHitsEta;  // legacy
	unsigned short int m_nRpcHitsEta;
	unsigned short int m_nTgcHitsEta;
	unsigned short int m_nCscHitsEta;
	unsigned short int m_nMdtHits;
};

/// Helper function for printing the object
std::string str( const TrigMuonEFTrack& d );
/// Helper operator for printing the object
MsgStream& operator<< ( MsgStream& m, const TrigMuonEFTrack& d );

/// Operator comparing two TrigMuonEFTrack objects for equality
bool operator== ( const TrigMuonEFTrack& a, const TrigMuonEFTrack& b );
/// Operator comparing two TrigMuonEFTrack objects for inequality
inline bool operator!= ( const TrigMuonEFTrack& a, const TrigMuonEFTrack& b ) {
	return !( a == b );
}

/// Comparison with feedback
void diff( const TrigMuonEFTrack& a, const TrigMuonEFTrack& b,
		std::map< std::string, double >& variableChange );

CLASS_DEF( TrigMuonEFTrack , 148483816 , 1 )

#endif // TRIGMUONEVENT_TRIGMUONEFTRACK_H
