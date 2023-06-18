// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

#ifndef ITRIGFPGATrackSimBANKSVC_H
#define ITRIGFPGATrackSimBANKSVC_H

/**
 * This file defines an interface for a bank service. The service is in charge
 * of loading all constants, including the fit coefficients, etc
 * Other classes can use this service to retrieve the banks.
 */


#include "GaudiKernel/IService.h"
#include "GaudiKernel/IInterface.h"

class FPGATrackSimFitConstantBank;
class FPGATrackSimSectorSlice;
class FPGATrackSimSectorBank;

class IFPGATrackSimBankSvc : virtual public IService
{
    public:
	DeclareInterfaceID(IFPGATrackSimBankSvc, 1, 0);
        virtual ~IFPGATrackSimBankSvc() = default;

        virtual const FPGATrackSimFitConstantBank* FitConstantBank_1st(int missedPlane = -1) = 0;
        virtual const FPGATrackSimFitConstantBank* FitConstantBank_2nd(int missedPlane = -1) = 0;
        virtual const FPGATrackSimSectorSlice* SectorSlice() = 0;
        virtual const FPGATrackSimSectorBank* SectorBank_1st() = 0;
        virtual const FPGATrackSimSectorBank* SectorBank_2nd() = 0;
};

#endif   // ITRIGFPGATrackSimBANKSVC_H
