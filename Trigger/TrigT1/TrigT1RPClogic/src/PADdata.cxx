/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1RPClogic/PADdata.h"
#include "TrigT1RPClogic/CMAdata.h"


PADdata::PADdata(CMAdata* cma_data,unsigned long int debug) : 
    BaseObject(Data,"PAD data"),m_debug(debug) 
{
    m_pad_patterns.clear();
    
    CMAdata::PatternsList cma_patterns = cma_data->give_patterns();
    CMAdata::PatternsList::const_iterator CMApatterns = cma_patterns.begin();

    while(CMApatterns != cma_patterns.end())
    {
        create_pad_patterns(*CMApatterns);
	++CMApatterns;
    }     
}

PADdata::PADdata(const PADdata& pad_data) : 
    BaseObject(Data,pad_data.name()),
    m_debug(pad_data.debug()),
    m_pad_patterns(pad_data.pad_patterns())
{
}

PADdata::~PADdata()
{
    m_pad_patterns.clear();
}

PADdata
PADdata::operator=(const PADdata& pad_data)
{
    static_cast<BaseObject&>(*this) = static_cast<const BaseObject&>(pad_data);
    m_pad_patterns.clear();
    m_pad_patterns = pad_data.pad_patterns();
    m_debug        = pad_data.debug();
    return *this;
}

void
PADdata::create_pad_patterns(CMApatterns* cma_patterns)
{
    const int pad_id = cma_patterns->cma_parameters().id().PAD_index();
    const int sector = cma_patterns->sector();
    PADpatterns* patterns = find(sector,pad_id);
    if(patterns) patterns->load_cma_patterns(cma_patterns);
    else {
        PADpatterns thePatterns(sector,pad_id,m_debug);
        thePatterns.load_cma_patterns(cma_patterns);
        m_pad_patterns.push_back(thePatterns);
    }
}

    
PADpatterns*
PADdata::find(const int sector,const int pad_id)
{
    PATTERNSlist::iterator it = m_pad_patterns.begin();
    while (it != m_pad_patterns.end())
    {
        if((*it).pad_id() == pad_id &&
           (*it).sector() == sector  ) return &(*it);
        ++it;
    }

    return 0;
}


PADdata::PatternsList
PADdata::give_patterns()
{
    PatternsList patterns;

    PATTERNSlist::iterator pad = m_pad_patterns.begin();

    while(pad != m_pad_patterns.end())
    {   
        patterns.push_back(&(*pad));
	++pad;
    }

    return patterns;
}


void PADdata::PrintElement(std::ostream& stream,std::string element,bool detail) 
    const
{
    bool all  = (element == name() || element == "")? true : false;
    bool nPad = m_pad_patterns.size();
    bool printed = false;

    if(nPad && (element == (*m_pad_patterns.begin()).name() || all))
    {
        stream << name() << " contains " << m_pad_patterns.size()
	       << " pad patterns:" << std::endl;  
        printed = true;
	PATTERNSlist::const_iterator it = m_pad_patterns.begin();
	while(it != m_pad_patterns.end())
        {
            it->Print(stream,detail);
	    ++it;
	}
    }

    if(!printed)
    {
        if (element == "") element = "PADs";
        stream << name() << " contains no " << element << "!" << std::endl;
    }
}


void PADdata::Print(std::ostream& stream,bool detail) const
{
    stream << name() << " contains " 
           << m_pad_patterns.size()
	   << " pad patterns " << std::endl;
    
    PATTERNSlist::const_iterator pad = pad_patterns().begin();

    while(pad != pad_patterns().end())
    {
        (*pad).Print(stream,detail);
	++pad;
    }
}
