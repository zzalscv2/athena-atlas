/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGINDETCUDA_GPUHELPERS_H
#define TRIGINDETCUDA_GPUHELPERS_H

class GPUHelpers{
public:
	static int getNumberOfGPUs();
	static int getNumberOfCores(int, int);
};
#endif