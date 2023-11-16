/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "JointModule.h"
#include <vector>


extern "C" TrigAccel::Module* getModule() {
  return new TrigAccelJointModule();
}

const std::vector<int> TrigAccelJointModule::getFactoryIds(){
	return std::vector<int>{TrigAccel::TrigInDetModuleID_CUDA, TrigAccel::TrigITkModuleID_CUDA};
}

TrigAccel::WorkFactory* TrigAccelJointModule::getFactoryById(int id){
	if(id==TrigAccel::TrigInDetModuleID_CUDA){
		return new TrigInDetModuleCuda();
	}

	if(id==TrigAccel::TrigITkModuleID_CUDA){
		return new TrigITkModuleCuda();
	}

	return nullptr;
}