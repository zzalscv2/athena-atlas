/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGACCEL_IDITk_Module_H
#define TRIGACCEL_IDITk_Module_H

#include "TrigAccelEvent/Module.h"
#include "TrigInDetModuleCuda.h"
#include "TrigITkModuleCuda.h"

class TrigAccelJointModule : public TrigAccel::Module{
public:
	TrigAccelJointModule() = default;
	const std::vector<int> getFactoryIds();
	TrigAccel::WorkFactory* getFactoryById(int id);
};
#endif