/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_NSWT0DATA_H
#define MUONCONDDATA_NSWT0DATA_H

// STL includes
#include <vector>

// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 

// Forward declarations
class Identifier;
class MmIdHelper;
class sTgcIdHelper;


class NswT0Data {


public:
    NswT0Data(const MmIdHelper&,  const sTgcIdHelper& stgcIdHelper);
     ~NswT0Data() = default;

	// setting functions
	void setData(const Identifier&, const float);

	// retrieval functions
	bool getT0 (const Identifier&   , float&) const;
 
private:
  using ChannelArray = std::vector<std::vector<float>>;
    
	// containers
  ChannelArray m_data_mmg{};
  ChannelArray m_data_stg{};

	// ID helpers
	const MmIdHelper&   m_mmIdHelper;
	const sTgcIdHelper&   m_stgcIdHelper;

  unsigned int identToModuleIdx(const Identifier& chan_id) const;

};

CLASS_DEF( NswT0Data ,148122593  , 1 )
CLASS_DEF( CondCont<NswT0Data> , 69140433 , 1 )

#endif
