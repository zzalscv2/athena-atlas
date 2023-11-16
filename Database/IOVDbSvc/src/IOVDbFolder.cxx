/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// IOVDbFolder.cxx - helper class for IOVDbSvc to manage folder & data cache
// Richard Hawkings, started 24/11/08




#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IAddressCreator.h"
#include "GaudiKernel/ISvcLocator.h"

#include "StoreGate/StoreGateSvc.h"
#include "CoolKernel/IObject.h"
#include "CoolKernel/IObjectIterator.h"
#include "CoolKernel/IRecord.h"
#include "CoolKernel/IRecordIterator.h"
#include "CoralBase/AttributeList.h"
#include "CoralBase/AttributeListSpecification.h"
#include "CoralBase/Attribute.h"
#include "CoralBase/AttributeSpecification.h"
#include "CoralBase/Blob.h"
#include "TStopwatch.h"

#include "CoraCool/CoraCoolDatabase.h"
#include "CoraCool/CoraCoolFolder.h"
#include "CoraCool/CoraCoolObject.h"
#include "CoraCool/CoraCoolObjectIter.h"

#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "AthenaPoolUtilities/AthenaAttrListAddress.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaPoolUtilities/CondAttrListCollAddress.h"
#include "AthenaPoolUtilities/CondAttrListVec.h"
#include "AthenaPoolUtilities/CondAttrListVecAddress.h"

#include "GeoModelInterfaces/IGeoModelSvc.h"

#include "IOVDbMetaDataTools/IIOVDbMetaDataTool.h"

#include "IOVDbConn.h"

#include "ReadFromFileMetaData.h"
#include "IOVDbFolder.h"
#include "IOVDbStringFunctions.h"
#include "IOVDbCoolFunctions.h"
#include "TagFunctions.h"

#include "Cool2Json.h"
#include "Json2Cool.h"
#include "BasicFolder.h"

#include "CrestFunctions.h"
#include <sstream>
#include <stdexcept>
#include <fstream>

#include "CrestApi/CrestApi.h"

using namespace IOVDbNamespace;

namespace{
  const std::string fileSuffix{".json"};
  const std::string delimiter{"."};
 
}

IOVDbFolder::IOVDbFolder(IOVDbConn* conn,
                         const IOVDbParser& folderprop, MsgStream& msg,
                         IClassIDSvc* clidsvc, IIOVDbMetaDataTool* metadatatool,
                         const bool checklock, const bool outputToFile,
                         const std::string & source, const bool crestToFile,
                         const std::string & crestServer):
  AthMessaging("IOVDbFolder"),
  p_clidSvc(clidsvc),
  p_metaDataTool(metadatatool),
  m_conn(conn),
  m_checklock(checklock),
  m_foldertype(AttrList),
  m_chansel(cool::ChannelSelection::all()),
  m_outputToFile{outputToFile},
  m_crestToFile{crestToFile},
  m_source{source},
  m_crestServer{crestServer}
{
  // set message same message level as our parent (IOVDbSvc)
  setLevel(msg.level());
  // extract settings from the properties
  // foldername from the 'unnamed' property
  m_foldername=folderprop.folderName();
  // SG key from 'key' property, otherwise same as foldername
  // m_jokey is true if the 'key' property was set - need to remember this
  // to avoid using folder description <key> if present later
  m_key=folderprop.key();
  m_jokey=folderprop.hasKey();
  // tag from 'tag' property
  m_jotag=folderprop.tag();
  // event store from 'eventStoreName' property, default 'StoreGateSvc'
  m_eventstore=folderprop.eventStoreName();
  // cachelength (seconds or LB)
  m_cachepar = folderprop.cache();
  // check for <noover> - disables using tag override read from input file
  m_notagoverride=folderprop.noTagOverride();
  if (m_notagoverride) ATH_MSG_INFO( "Inputfile tag override disabled for " << m_foldername );

  // channel selection from 'channelSelection' property
  // syntax is A:B,C:D,E:F
  // :B implies zero lower limit, A: implies zero upper limit
  std::string chanspec;
  if (folderprop.getKey("channelSelection","",chanspec) && !chanspec.empty()) {
    m_chanrange=IOVDbNamespace::parseChannelSpec<cool::ChannelId>(chanspec);
    // explicit setting of channel selection
    // push to the channel selection
    try{
      bool first(true);
      for(const auto & i:m_chanrange){
        if (first){
          first = false;
          m_chansel = cool::ChannelSelection(i.first,i.second);
        } else {
          m_chansel.addRange(i.first,i.second);
        }
      }
    } catch (cool::Exception& e) {
        ATH_MSG_ERROR("defining channel range (must be given in ascending order)");
        throw;
    }
  }
  if (folderprop.overridesIov(msg)){
    m_iovoverridden=true;
    m_iovoverride=folderprop.iovOverrideValue(msg);
    if (m_timestamp){
      ATH_MSG_INFO( "Override timestamp to " << m_iovoverride << " for folder " << m_foldername );
    } else {
      const auto[run,lumi]=IOVDbNamespace::runLumiFromIovTime(m_iovoverride);
      ATH_MSG_INFO( "Override run/LB number to [" << run << ":" << lumi << "] for folder " << m_foldername );
    }
  }
 
  m_fromMetaDataOnly=folderprop.onlyReadMetadata();
  if (m_fromMetaDataOnly) {
    ATH_MSG_INFO( "Read from meta data only for folder " << m_foldername );
  }
 
  m_extensible=folderprop.extensible();
  if (m_extensible) {
    ATH_MSG_INFO( "Extensible folder " << m_foldername );
  }


  
}

IOVDbFolder::~IOVDbFolder() {
  if (m_cachespec!=nullptr) m_cachespec->release();
}

void IOVDbFolder::useFileMetaData() {
  // enable folder from FLMD at given connection
  m_useFileMetaData = true;
  // if previously connected to a real DB connection, remove association
  if (m_conn!=nullptr) {
    m_conn->decUsage();
    m_conn=nullptr;
  }
}

void 
IOVDbFolder::setTagOverride(const std::string& tag,const bool setFlag) {
  if (m_tagoverride) {
    ATH_MSG_WARNING( "Request to override tag for folder " <<
      m_foldername << " to " << tag << " supercedes earlier override to "  << m_jotag );
  } else {
    if (setFlag) m_tagoverride=true;
  }
  m_jotag=IOVDbNamespace::spaceStrip(tag);
}

void IOVDbFolder::setWriteMeta() {
  m_writemeta=true;
}

void 
IOVDbFolder::setIOVOverride(const unsigned int run,
                                 const unsigned int lumiblock,
                                 const unsigned int time) {
  // set IOV override for this folder if run!=0 or time!=0
  // folder-specific override takes precedence if set
  if (m_iovoverridden) return;
  if (m_timestamp) {
    if (time!=0) {
      m_iovoverride=IOVDbNamespace::iovTimeFromSeconds(time);
      ATH_MSG_INFO( "Override timestamp to " << m_iovoverride << " for folder "<< m_foldername );
      m_iovoverridden=true;
    }
  } else {
    if (run!=0 || lumiblock!=0) {
      m_iovoverride=IOVDbNamespace::iovTimeFromRunLumi(run,lumiblock);
      ATH_MSG_INFO( "Override run/LB number to [" << run << ":" << lumiblock << 
        "] for folder " << m_foldername );
      m_iovoverridden=true;
    }
  }
}

// return validitykey for folder, given input reftime
// take into account an overridden IOV, if present for folder
cool::ValidityKey 
IOVDbFolder::iovTime(const IOVTime& reftime) const {
  if (m_iovoverridden) {
    return m_iovoverride;
  } else {
    return (m_timestamp ? reftime.timestamp() : reftime.re_time());
  }
}

bool 
IOVDbFolder::loadCache(const cool::ValidityKey vkey,
                            const unsigned int cacheDiv,
                            const std::string& globalTag,
                            const bool ignoreMissChan) {
  // load the cache for the given IOVTime, making a range around this time
  // according to the caching policy
  // if cacheDiv > 0, specifies number of slices of cache for query alignment
  // if ignoreMissChan set, don't worry about missing channels outside the cache range
  // return false if any problem
  // timer to track amount of time in loadCache
  TStopwatch cachetimer;
  const auto & [cachestart, cachestop] = m_iovs.getCacheBounds();
  BasicFolder basicFolder;
  if (m_source == "CREST"){
    //const std::string  jsonFolderName=sanitiseCrestTag(m_foldername);

    CrestFunctions cfunctions(m_crestServer);

    if (m_globaltag != globalTag){
      m_globaltag = globalTag;
      m_cresttagmap.clear();
      m_cresttagmap = cfunctions.getGlobalTagMap(globalTag);
    }
    const std::string completeTag = m_cresttagmap[m_foldername];
    ATH_MSG_INFO("Download tag would be: "<<completeTag);

    // *** *** *** *** *** ***
    // Routine which converts openended CREST IOVs into non-overlapping IOVs
    
    // Get a vector of pairs retrieved from crest
    //  <IOV_SINCE(string),HASH(string)>
    auto crestIOVs = cfunctions.getIovsForTag(completeTag);
    typedef std::pair<cool::ValidityKey,size_t> IOV2Index; // <CREST_IOV(converted to ull),Index_in_crestIOVs>
    std::vector<IOV2Index> iov2IndexVect;                  // Temporary vector for sorting IOV_SINCE values
    iov2IndexVect.reserve(crestIOVs.size());
    size_t hashInd{0};
    for(const auto& crestIOV : crestIOVs) {
      iov2IndexVect.emplace_back(std::stoull(crestIOV.first),hashInd++);
    }

    std::sort(iov2IndexVect.begin(),iov2IndexVect.end(),
	      [](const IOV2Index& a, const IOV2Index& b)
	      {
		return a.first < b.first;
	      });


    typedef std::pair<IovStore::Iov_t,std::string> IOVHash; // <<IOV_SINCE(ull),IOV_UNTIL(ull)>,HASH(string)>
    std::vector<IOVHash> iovHashVect;                       // Vector of non-overlapping IOVs + corresponding Hashes
    size_t nIOVs = iov2IndexVect.size();
    iovHashVect.reserve(nIOVs);
    for(size_t ind=0; ind<nIOVs-1; ++ind) {
      iovHashVect.emplace_back(IovStore::Iov_t(iov2IndexVect[ind].first
						    , iov2IndexVect[ind+1].first)
				    , crestIOVs[iov2IndexVect[ind].second].second);
    }
    iovHashVect.emplace_back(IovStore::Iov_t(iov2IndexVect[nIOVs-1].first
						  , cool::ValidityKeyMax)
				  , crestIOVs[iov2IndexVect[nIOVs-1].second].second);
    
    // End of the CREST IOV conversion routine
    // *** *** *** *** *** ***

    int indIOV = 0;
    for(const auto& iovhash : iovHashVect) {
      if(vkey >= iovhash.first.second) {
	++indIOV;
	continue;
      }
      if(vkey < iovhash.first.first) {
	ATH_MSG_WARNING("Load cache failed for " << m_foldername
			<< ". VKey " << vkey << " is earlier than the start of the first IOV retrieved from the DB");
	indIOV = -1;
      }
      break;
    }

    if(indIOV>=0) {
      ATH_MSG_DEBUG("Found IOV for " << m_foldername << " and VKEY " << vkey << " "
		   << iovHashVect[indIOV].first);
    }

    const std::string & reply = indIOV==-1? std::string{} : cfunctions.getPayloadForHash(iovHashVect[indIOV].second);

    if (m_crestToFile and (indIOV>=0)){ //indIOV must be >=0, it's used as a vector index in this block
     
      unsigned long long sinceT =  iovHashVect[indIOV].first.first;

      std::string crest_work_dir=std::filesystem::current_path();
      crest_work_dir += "/crest_data";
      bool crest_rewrite = true;
      Crest::CrestClient crestFSClient = Crest::CrestClient(crest_rewrite, crest_work_dir);

      nlohmann::json js =
      {
        {"name", completeTag}
      };

      try{
        crestFSClient.createTag(js);
        ATH_MSG_INFO("Tag " << completeTag << " saved to disk.");
        ATH_MSG_INFO("CREST Dump dir = " << crest_work_dir);
      }
      catch (const std::exception& e) {
        ATH_MSG_WARNING("Data saving for tag " << completeTag << " failed: " << e.what());
      }

      try{
        crestFSClient.storePayloadDump(completeTag, sinceT, reply);
        ATH_MSG_INFO("Data (payload and IOV) saved for tag " << completeTag << ".");
      }
      catch (const std::exception& e) {
        ATH_MSG_WARNING("Data (payload and IOV) saving for tag " << completeTag<<" failed; " << e.what());
      }
    }

    //
    if (m_crest_tag != completeTag){
      m_crest_tag = completeTag;
      m_tag_info = cfunctions.getTagInfo(completeTag);
    }

    const std::string& nodeDescription = cfunctions.getTagInfoElement(m_tag_info,"node_description");
    if(nodeDescription.find("CondAttrListVec") != std::string::npos) {
      basicFolder.setVectorPayloadFlag(true);
    }

    const std::string& specString = cfunctions.getTagInfoElement(m_tag_info,"payload_spec");
    if (specString.empty()){
      ATH_MSG_FATAL("Reading payload spec from "<<m_foldername<<" failed.");
      return false;
    }

    //basic folder now contains the info
    if(!reply.empty()) { //this also takes care of the case if indIOV<0, since reply is empty in this case
      std::istringstream ss(reply);
      Json2Cool inputJson(ss, basicFolder, specString, &(iovHashVect[indIOV].first));
      if (basicFolder.empty()){
        ATH_MSG_FATAL("Reading channel data from "<<m_foldername<<" failed.");
        return false;
      }
    }
  }

  ATH_MSG_DEBUG( "Load cache for folder " << m_foldername << " validitykey " << vkey);
  // if not first time through, and limit not reached,and cache was not reset, 
  // and we are going forwards in time, double cachesize
  if (m_ndbread>0 && m_cacheinc<3 && (cachestop!=cachestart) && vkey>cachestart && m_autocache) {
    m_cachelength*=2;
    ++m_cacheinc;
    ATH_MSG_INFO( "Increase cache length (step " << m_cacheinc << ") for folder " << m_foldername << " to " << m_cachelength << " at validityKey " << vkey );
  }
  ++m_ndbread;
  auto [changedCacheLo, changedCacheHi] = m_iovs.getCacheBounds();
  //
  //
  //new
  if (cacheDiv>0) {
    // quantise queries on boundaries that are sub-multiples of cache length
    unsigned long long cacheq=m_cachelength/cacheDiv;
    if (cacheq>0) changedCacheLo=vkey - vkey % cacheq;
    changedCacheHi=changedCacheLo+m_cachelength;
  } else {
    // for run/LB indexed folders and cache of at least one run
    // align the query to the run start
    if (!m_timestamp && m_cachelength>=IOVDbNamespace::ALL_LUMI_BLOCKS) {
      changedCacheLo=vkey & (0x7FFFFFFFLL << 32);
    } else {
      changedCacheLo=vkey;
    }
    changedCacheHi=vkey+m_cachelength;
  }
  if (changedCacheHi>cool::ValidityKeyMax) changedCacheHi=cool::ValidityKeyMax;
  //
  //
  m_iovs.setCacheBounds(IovStore::Iov_t(changedCacheLo, changedCacheHi));
  //
  const auto & [since, until] = m_iovs.getCacheBounds();
  ATH_MSG_DEBUG( "IOVDbFolder:loadCache limits set to ["  << since << "," << until << "]" );
  bool vectorPayload{};
  if (m_source=="CREST"){
    vectorPayload = basicFolder.isVectorPayload();
  } else {
    vectorPayload = (m_foldertype ==CoraCool) or (m_foldertype == CoolVector);
  }
  if (m_cachespec==nullptr) {
    // on first init, guess size based on channel count
    unsigned int estsize=m_nchan;
    if (m_cachehint > 0) {
      estsize=estsize*m_cachehint;
    } else if (m_timestamp) {
      // for timestamp indexed folder (likely to be DCS), increase this
      estsize=estsize*3;
    }
    // note this is only reserved size of the cache vectors
    // actual datastorage is mainly allocated by pointer elsewhere
    m_cachechan.reserve(estsize);
    m_cacheattr.reserve(estsize);
    if (vectorPayload) {
      m_cacheccstart.reserve(estsize);
      m_cacheccend.reserve(estsize);
    }
  } else {
    // reset cache if it already contained data
    // TBIO - could keep the attributelists and only change the data on reload
    // avoiding some attributelist construction/destruction
    clearCache();
  }
  bool retrievedone=false;
  unsigned int nChannelsExpected = (m_chanrange.empty())? (m_nchan) : (IOVDbNamespace::countSelectedChannels(m_channums, m_chansel));

  if (m_source == "COOL_DATABASE"){
    // query to fill cache - request for database activates connection
    if (not m_conn->open()) {
      ATH_MSG_FATAL( "Conditions database connection " <<m_conn->name() << " cannot be opened - STOP" );
      return false;
    }
    // access COOL inside try/catch in case of using stale connection
    unsigned int attempts=0;

    ATH_MSG_DEBUG( "loadCache: Expecting to see " << nChannelsExpected << " channels" );
    //
    while (attempts<2 && !retrievedone) {
      ++attempts;
      try {
        unsigned int iadd=0;
        m_iovs.setIovSpan(IovStore::Iov_t(0,cool::ValidityKeyMax));
        // check pointer is still valid - can go stale in AthenaMT environment
        // according to CORAL server tests done by Andrea Valassi (23/6/09)
        if (not m_conn->valid()) throw std::runtime_error("COOL database pointer invalidated");
        // access COOL folder in case needed to resolve tag (even for CoraCool)
        cool::IFolderPtr folder=m_conn->getFolderPtr(m_foldername);

        // resolve the tag for MV folders if not already done so
        if (m_multiversion && m_tag.empty()) {
          if (!resolveTag(folder,globalTag)) return false;
        
        }
        if (m_foldertype==CoraCool) {
          // CoraCool retrieve
          CoraCoolDatabasePtr ccDbPtr=m_conn->getCoraCoolDb();
          CoraCoolFolderPtr ccfolder=ccDbPtr->getFolder(m_foldername);

          auto [since,until] = m_iovs.getCacheBounds();
          CoraCoolObjectIterPtr itr=ccfolder->browseObjects(since, until,m_chansel,m_tag);
          while (itr->hasNext()) {
            CoraCoolObjectPtr obj=itr->next();
            //should be skipping non-selected channels here?
            addIOVtoCache(obj->since(),obj->until());
            m_cachechan.push_back(obj->channelId());
            // store all the attributeLists in the buffer
            // save pointer to start
            const unsigned int istart=m_cacheattr.size();
            for (CoraCoolObject::const_iterator pitr=obj->begin();pitr!=obj->end(); ++pitr) {
              // setup shared specification on first store
              if (m_cachespec==nullptr) setSharedSpec(*pitr);
              // use the shared specification in storing the payload
              m_cacheattr.emplace_back(*m_cachespec,true);
              m_cacheattr.back().fastCopyData(*pitr);
              m_nbytesread+=IOVDbNamespace::attributeListSize(*pitr);
            }
            // save pointers to start and end
            m_cacheccstart.push_back(istart);
            m_cacheccend.push_back(m_cacheattr.size());
            ++iadd;
          }
          itr->close();
          retrievedone=true;
        } else {
          auto [since,until] = m_iovs.getCacheBounds();
          cool::IObjectIteratorPtr itr=folder->browseObjects(since,until,m_chansel,m_tag);
          if (m_outputToFile){
            Cool2Json json(folder, since, until, m_chansel, m_tag);
            std::ofstream myFile;
            const std::string sanitisedFolder=sanitiseFilename(m_foldername);
            const std::string fabricatedName=sanitisedFolder+delimiter+std::to_string(since)+fileSuffix;
            myFile.open(fabricatedName,std::ios::out);
            if (not myFile.is_open()){
              ATH_MSG_FATAL("File creation for "<<fabricatedName<<" failed.");
            } else{
              ATH_MSG_INFO("File "<<fabricatedName<<" created.");
            }
            myFile<<IOVDbNamespace::Cool2Json::open();
            myFile<<json.description()<<IOVDbNamespace::Cool2Json::delimiter()<<std::endl;
            myFile<<json.payloadSpec()<<IOVDbNamespace::Cool2Json::delimiter()<<std::endl;
            myFile<<json.iov()<<IOVDbNamespace::Cool2Json::delimiter()<<std::endl;
            myFile<<json.payload()<<std::endl;
            myFile<<IOVDbNamespace::Cool2Json::close();
          }
          while (itr->goToNext()) {
            const cool::IObject& ref=itr->currentRef();
            addIOVtoCache(ref.since(),ref.until());
            m_cachechan.push_back(ref.channelId());
            if (m_foldertype==CoolVector) {
              // store all the attributeLists in the buffer
              // save pointer to start
              const unsigned int istart=m_cacheattr.size();
              // get payload iterator and vector of payload records
              cool::IRecordIterator& pitr=ref.payloadIterator();
              const cool::IRecordVectorPtr& pvec=pitr.fetchAllAsVector();
              for (cool::IRecordVector::const_iterator vitr=pvec->begin();vitr!=pvec->end();++vitr) {
                const coral::AttributeList& atrlist=(*vitr)->attributeList();
                // setup shared specification on first store
                if (m_cachespec==nullptr) setSharedSpec(atrlist);
                // use the shared specification in storing the payload
                m_cacheattr.emplace_back(*m_cachespec,true);
                m_cacheattr.back().fastCopyData(atrlist);
                m_nbytesread+=IOVDbNamespace::attributeListSize(atrlist);
              }
              // save pointers to start and end
              m_cacheccstart.push_back(istart);
              m_cacheccend.push_back(m_cacheattr.size());
              ++iadd;
              pitr.close();
            } else {
              // standard COOL retrieve
              const coral::AttributeList& atrlist=ref.payload().attributeList();
              // setup shared specification on first store
              if (m_cachespec==nullptr) setSharedSpec(atrlist);
              // use the shared specification in storing the payload
              m_cacheattr.emplace_back(*m_cachespec,true);
              m_cacheattr[iadd].fastCopyData(atrlist);
              ++iadd;
              m_nbytesread+=IOVDbNamespace::attributeListSize(atrlist);
            }
          }
          itr->close();
          retrievedone=true;
        }
        ATH_MSG_DEBUG( "Retrieved " << iadd << " objects for "<< m_nchan << " channels into cache" );
        m_nobjread+=iadd;
      } catch (std::exception& e) {
        ATH_MSG_WARNING( "COOL retrieve attempt " << attempts << " failed: " << e.what() );
        // disconnect and reconnect
        if (not m_conn->dropAndReconnect()) ATH_MSG_ERROR("Tried to reconnect in 'loadCache' but failed");
      }
    }
  } /*end of 'if ... COOL_DATABASE'*/ else {
    //this is code using CREST objects now
    ATH_MSG_DEBUG( "loadCache: Expecting to see " << nChannelsExpected << " channels" );
    if (!resolveTag(nullptr,globalTag)) return false;
    const auto & channelNumbers=basicFolder.channelIds();
    ATH_MSG_DEBUG( "ChannelIds is " << channelNumbers.size() << " long" );
    unsigned int iadd{};
    for (const auto & chan: channelNumbers){
      m_cachechan.push_back(chan);
      addIOVtoCache(basicFolder.iov().first, basicFolder.iov().second);
      if (basicFolder.isVectorPayload()) {
        const auto & vPayload = basicFolder.getVectorPayload(chan);
        const unsigned int istart=m_cacheattr.size();
        for (const auto & attList:vPayload){
          if (m_cachespec==nullptr) setSharedSpec(attList);
          m_cacheattr.emplace_back(*m_cachespec,true);// maybe needs to be cleared before
          m_cacheattr.back().fastCopyData(attList);
          m_nbytesread+=IOVDbNamespace::attributeListSize(attList);
        }
        m_cacheccstart.push_back(istart);
        m_cacheccend.push_back(m_cacheattr.size());
        ++iadd;
      } else {
        auto const & attList = basicFolder.getPayload(chan);
        if (m_cachespec==nullptr) setSharedSpec(attList);
        const coral::AttributeList c(*m_cachespec,true);
        m_cacheattr.push_back(c);// maybe needs to be cleared before
        m_cacheattr.back().fastCopyData(attList);
        m_nbytesread+=IOVDbNamespace::attributeListSize(attList);
        ++iadd;
      }
    }
    retrievedone=true;
    ATH_MSG_DEBUG( "Retrieved " << iadd << " objects for "<< m_nchan << " channels into cache" );
    m_nobjread+=iadd;
  } //end of attempted retrieves using one of the methods
  if (!retrievedone) {
    const auto & [since,until] = m_iovs.getCacheBounds();
    ATH_MSG_ERROR( "Could not retrieve COOL data for folder " <<
      m_foldername << " tag " << m_tag << " validityKeys [" << since <<
      "," << until << "]" );
    return false;
  }
  // check if cache can be stretched according to extent of IOVs crossing
  // boundaries - this requires all channels to have been seen
  const auto & [nChannelsLo, nChannelsHi] = m_iovs.numberOfIovsOnBoundaries();
  const auto  missing=std::pair<unsigned int, unsigned int>(nChannelsExpected-nChannelsLo, nChannelsExpected-nChannelsHi);
  ATH_MSG_DEBUG( "Cache retrieve missing " << missing.first  << " lower and " << missing.second << " upper channels" );
  //
  const auto & span = m_iovs.getMinimumStraddlingSpan();
  const auto & [cacheStart, cacheStop] =m_iovs.getCacheBounds();
  //new code
  if ((missing.first==0 or ignoreMissChan) and m_iovs.extendCacheLo()){
    ATH_MSG_DEBUG( "Lower cache limit extended from " << cacheStart << " to " << span.first );
  }
  
  if ((missing.second==0 or ignoreMissChan) and m_iovs.extendCacheHi()){
    ATH_MSG_DEBUG( "Upper cache limit extended from " << cacheStop << " tp " << span.second );
  }
  //
  // keep track of time spent
  const float timeinc=cachetimer.RealTime();
  m_readtime+=timeinc;
  ATH_MSG_DEBUG( "Cache retrieve done for " << m_foldername << " with " << 
      m_iovs.size() << " objects stored in" << std::fixed <<
      std::setw(8) << std::setprecision(2) << timeinc << " s" );
  return true;
}

bool IOVDbFolder::loadCacheIfDbChanged(const cool::ValidityKey vkey,
                                       const std::string& globalTag, 
                                       const cool::IDatabasePtr& /*dbPtr*/,
                                       const ServiceHandle<IIOVSvc>& iovSvc) {
  ATH_MSG_DEBUG( "IOVDbFolder::recheck with DB for folder " << m_foldername<< " validitykey: " << vkey );
  if (m_iovs.empty()) {
    ATH_MSG_DEBUG( "Cache empty ! returning ..." );
    return true;
  }
  ++m_ndbread;
  // access COOL inside try/catch in case of using stale connection
  unsigned int attempts     = 0;
  bool         retrievedone = false;
  //
  unsigned int nChannelsExpected = (m_chanrange.empty())? (m_nchan) : (IOVDbNamespace::countSelectedChannels(m_channums, m_chansel));
  ATH_MSG_DEBUG( "loadCacheIfDbChanged: Expecting to see " << nChannelsExpected << " channels" );
  //
  while (attempts<2 && !retrievedone) {
    ++attempts;
    try {
      m_iovs.setIovSpan(IovStore::Iov_t(0,cool::ValidityKeyMax));
      // access COOL folder in case needed to resolve tag (even for CoraCool)
      cool::IFolderPtr folder=m_conn->getFolderPtr(m_foldername);
      // resolve the tag for MV folders if not already done so
      if (m_multiversion && m_tag.empty()) { // NEEDED OR NOT?
        if (!resolveTag(folder,globalTag)) return false;
      }   
      int counter=0;
      const auto & [since,until] = m_iovs.getCacheBounds();
      ATH_MSG_DEBUG(IOVDbNamespace::folderTypeName(m_foldertype)<<" type. cachestart:\t"<<since<<" \t cachestop:"<< until);
      ATH_MSG_DEBUG("checking range:  "<<vkey+1<<" - "<<vkey+2);
      if (m_foldertype==CoraCool) {
        // CoraCool retrieve initialise CoraCool connection
        CoraCoolFolderPtr   ccfolder  = m_conn->getFolderPtr<CoraCoolFolderPtr>(m_foldername);
        // this returns all the objects whose IOVRanges crosses this range .
        CoraCoolObjectIterPtr itr = ccfolder->browseObjects(vkey+1, vkey+2,m_chansel,m_tag);
        while (objectIteratorIsValid(itr)) {
          CoraCoolObjectPtr obj = itr->next();
          //code delegated to templated member, allowing for difference between CoraCoolObjectPtr and IObject
          counter+=cacheUpdateImplementation(*obj,iovSvc);
        }
        itr->close();
      } else {
        // this returns all the objects whose IOVRanges crosses this range . 
        cool::IObjectIteratorPtr itr=folder->browseObjects(vkey+1, vkey+2, m_chansel,m_tag);
        while (objectIteratorIsValid(itr)) {
          const cool::IObject& ref=itr->currentRef();
          //code delegated to templated member, allowing for difference between CoraCoolObjectPtr and IObject
          counter+=cacheUpdateImplementation(ref,iovSvc);
        }
        itr->close();
      }
      retrievedone=true;
      ATH_MSG_DEBUG( "Need a special update for " << counter << " objects " );      
      m_nobjread+=counter;
    }catch (std::exception& e) {
      ATH_MSG_WARNING( "COOL retrieve attempt " << attempts <<  " failed: " << e.what() );
      if (not m_conn->dropAndReconnect()) ATH_MSG_ERROR("Tried reconnecting in loadCacheIfDbChanged but failed");
    }
  }
  return true;
}

void 
IOVDbFolder::specialCacheUpdate(CoraCoolObject & obj, const ServiceHandle<IIOVSvc>& iovSvc) {

  // reset IOVRange in IOVSvc to trigger reset of object. Set to a
  // time earlier than since.
  IOVRange range = IOVDbNamespace::makeRange(obj.since()-2, obj.since()-1, m_timestamp);
  if (StatusCode::SUCCESS != iovSvc->setRange(clid(), key(), range, eventStore())) {
    ATH_MSG_ERROR( "IOVDbFolder::specialCacheUpdate - setRange failed for folder " 
           << folderName() );
    return;
  }
  addIOVtoCache(obj.since(),obj.until());
  m_cachechan.push_back(obj.channelId());
  // store all the attributeLists in the buffer save pointer to start
  const unsigned int istart=m_cacheattr.size();
  for (CoraCoolObject::const_iterator pitr=obj.begin(); pitr!=obj.end();++pitr) {
    // use the shared specification in storing the payload
    m_cacheattr.emplace_back(*m_cachespec,true);
    m_cacheattr.back().fastCopyData(*pitr);
    m_nbytesread+=IOVDbNamespace::attributeListSize(*pitr);
  }
  // save pointers to start and end
  m_cacheccstart.push_back(istart);
  m_cacheccend.push_back(m_cacheattr.size());
}

void 
IOVDbFolder::specialCacheUpdate(const cool::IObject& ref,const ServiceHandle<IIOVSvc>& iovSvc) {

  // reset IOVRange in IOVSvc to trigger reset of object. Set to a
  // time earlier than since.
  IOVRange range = IOVDbNamespace::makeRange(ref.since()-2, ref.since()-1, m_timestamp);
  if (StatusCode::SUCCESS != iovSvc->setRange(clid(), key(), range, eventStore())) {
    ATH_MSG_ERROR( "IOVDbFolder::specialCacheUpdate - setRange failed for folder " 
           << folderName() );
    return;
  }
  // add new object.
  addIOVtoCache(ref.since(),ref.until());
  m_cachechan.push_back(ref.channelId());
  const coral::AttributeList& atrlist = ref.payload().attributeList();
  // use the shared specification in storing the payload
  const unsigned int istart=m_cacheattr.size();
  m_cacheattr.emplace_back(*m_cachespec,true);// maybe needs to be cleared before
  m_cacheattr.back().fastCopyData(atrlist);
  m_nbytesread+=IOVDbNamespace::attributeListSize(atrlist);
  if (m_foldertype==CoolVector) {
    // save pointers to start and end
    m_cacheccstart.push_back(istart);
    m_cacheccend.push_back(m_cacheattr.size());
  }
}

void 
IOVDbFolder::resetCache() {
  // reset the cache to unfilled state, used if no more data will be required
  // from this folder
  m_iovs.setCacheBounds(IovStore::Iov_t(0,0));
  clearCache();
}

bool 
IOVDbFolder::getAddress(const cool::ValidityKey reftime,
                             IAddressCreator* persSvc,
                             const unsigned int poolSvcContext,
                             std::unique_ptr<IOpaqueAddress>& address,
                             IOVRange& range, bool& poolPayloadReq) {
  ++m_ncacheread;
  // will produce strAddress and one pointer type depending on folder data
  std::string strAddress;
  AthenaAttributeList* attrList=nullptr;
  CondAttrListCollection* attrListColl=nullptr;
  CondAttrListVec* attrListVec=nullptr;
  cool::ValidityKey naystart=0;
  cool::ValidityKey naystop=cool::ValidityKeyMax;
  if( m_useFileMetaData ) {    
    IOVDbNamespace::SafeReadFromFileMetaData
       readFromMetaData(m_foldername, p_metaDataTool, reftime, m_timestamp);
    if (not readFromMetaData.isValid()){
      ATH_MSG_ERROR( "read:Could not find IOVPayloadContainer for folder "<< m_foldername );
      return false;
    }
    // read from file metadata
    m_foldertype=readFromMetaData.folderType();
    m_nobjread+=readFromMetaData.numberOfObjects();
    poolPayloadReq=readFromMetaData.poolPayloadRequested();
    strAddress = readFromMetaData.stringAddress();
    range = readFromMetaData.range();
    attrList = readFromMetaData.attributeList();
    attrListColl = readFromMetaData.attrListCollection();
    ATH_MSG_DEBUG( "Read file metadata for folder " << m_foldername << " foldertype is " << m_foldertype );
  } else {
    // COOL/CoraCool data to be read from cache
    // for AttrListColl or PoolRefColl, need a CondAttrListCollection ready
    // to receive the data
    if (m_foldertype==AttrListColl || m_foldertype==PoolRefColl) {
      attrListColl=new CondAttrListCollection(!m_timestamp);
    } else if (m_foldertype==CoraCool || m_foldertype==CoolVector) {
      // for CoraCool/CoolVector, assume we will get everything in the cache
      attrListVec=new CondAttrListVec(!m_timestamp, m_cacheattr.size());
    }
    // loop over cached data
    unsigned int nobj=0;
    // keep track of closest neighbouring IOVs
    std::tie(naystart, naystop) = m_iovs.getCacheBounds();
    for (unsigned int ic=0; ic!=m_iovs.size();++ic) {
      const auto & thisIov  = m_iovs.at(ic);
      if (thisIov.first<=reftime && reftime<thisIov.second) {
        ++nobj;
        if (m_foldertype==AttrList || m_foldertype==PoolRef) {
          // retrieve of AthenaAttributeList or single PoolRef
          if (m_foldertype==AttrList) {
            attrList=new AthenaAttributeList(m_cacheattr[ic]);
            strAddress="POOLContainer_AthenaAttributeList][CLID=x";
          } else {
            strAddress=(m_cacheattr[ic])["PoolRef"].data<std::string>();
          }
          range=IOVDbNamespace::makeRange(thisIov.first,thisIov.second, m_timestamp);
          // write meta-data if required
          if (m_writemeta) 
            if (!addMetaAttrList(m_cacheattr[ic],range)) return false;
        } else if (m_foldertype==AttrListColl || m_foldertype==PoolRefColl) {
          // retrieve of CondAttrListCollection
          attrListColl->addShared(m_cachechan[ic],m_cacheattr[ic]);
          attrListColl->add(m_cachechan[ic],IOVDbNamespace::makeRange(thisIov.first,thisIov.second, m_timestamp));
        } else if (m_foldertype==CoraCool || m_foldertype==CoolVector) {
          // retrieval of CoraCool data
          attrListVec->addSlice(IOVDbNamespace::makeRange(thisIov.first,thisIov.second, m_timestamp),
                                m_cachechan[ic],m_cacheattr,
                                m_cacheccstart[ic],m_cacheccend[ic]);
          if (m_writemeta) {
            ATH_MSG_ERROR( "Writing of CoraCool folders to file metadata not implemented");
            return false;
          }
        } else {
          ATH_MSG_ERROR( "Unhandled folder type " << m_foldertype);
          return false;
        }
      } else if (thisIov.second<=reftime && thisIov.second>naystart) {
        naystart=thisIov.second;
      } else if (thisIov.first>reftime && thisIov.first<naystop) {
        naystop=thisIov.first;
      }
    }
    // post-loop actions
    if (m_foldertype==AttrListColl || m_foldertype==PoolRefColl) {
      // set up channel names if required
      if (m_named) {
        std::vector<std::string>::const_iterator nitr=m_channames.begin();
        for (std::vector<cool::ChannelId>::const_iterator chitr=m_channums.begin();
             chitr!=m_channums.end(); ++chitr,++nitr) {
          attrListColl->add(*chitr,*nitr);
        }
      }
      // set range
      range=attrListColl->minRange();
      strAddress="POOLContainer_CondAttrListCollection][CLID=x";
    } else if (m_foldertype==CoraCool || m_foldertype==CoolVector) {
      range=attrListVec->minRange();
      strAddress="POOLContainer_CondAttrListVec][CLID=x";
    } else if (m_foldertype==AttrList || m_foldertype==PoolRef) {
      // single object retrieve - should have exactly one object
      if (nobj==0) {
        ATH_MSG_ERROR("COOL object not found in single-channel retrieve, folder " 
               << m_foldername << " currentTime " << reftime );
        return false;
      } else if (nobj>1) {
        ATH_MSG_ERROR( nobj << 
          " valid objects found for single-channel retrieve, folder " << 
          m_foldername << " currentTime " << reftime );
        return false;
      }
    }
    ATH_MSG_DEBUG( "Retrieved object: folder " << m_foldername 
		   <<  " at IOV " << reftime << " channels " << nobj << " has range "
		   << range );
    // shrink range so it does not extend into 'gap' channels or outside cache
    IOVTime tnaystart=makeEpochOrRunLumi(naystart, m_timestamp);
    IOVTime tnaystop=makeEpochOrRunLumi(naystop, m_timestamp);
    IOVTime rstart=range.start();
    IOVTime rstop=range.stop();
    if (tnaystart > rstart || rstop > tnaystop) {
      ATH_MSG_DEBUG( "Shrink IOV range for " << m_foldername
		     << " from [" << rstart << ":" << rstop << "] to ["
		     << tnaystart << ":" << tnaystop << "]" );
      if (tnaystart > rstart) rstart=tnaystart;
      if (tnaystop  < rstop)  rstop=tnaystop;
      range=IOVRange(rstart,rstop);
      if (m_foldertype==AttrListColl || m_foldertype==PoolRefColl) {
        attrListColl->addNewStart(rstart);
        attrListColl->addNewStop(rstop);
      }
    }
  }
  // save the range for possible later lookup in IOVDbSvc::getKeyInfo
  m_currange=range;
  m_retrieved=true;
  // write metadata for attrListColl if required (after range shrinking)
  if (m_writemeta && 
      (m_foldertype==AttrListColl || m_foldertype==PoolRefColl)) {
    if (!addMetaAttrListColl(attrListColl)) return false;
  }

  // turn the data into an IOpaqueAddress
  strAddress=m_addrheader+strAddress;
  IOpaqueAddress* addrp = nullptr;
  if (StatusCode::SUCCESS!=persSvc->createAddress(0,0,strAddress,addrp)) {
    ATH_MSG_ERROR( "Could not get IOpaqueAddress from string address "<< strAddress );
    return false;
  }
  address = std::unique_ptr<IOpaqueAddress>(addrp);
  GenericAddress* gAddr=dynamic_cast<GenericAddress*>(address.get());
  if (!gAddr) {
    ATH_MSG_ERROR( "Could not cast IOpaqueAddress to GenericAddress");
    return false;
  }
  // create a new GenericAddress to set pool context
  if (m_foldertype==AttrListColl) {
    auto addr = std::make_unique<CondAttrListCollAddress>(*gAddr);
    addr->setAttrListColl(attrListColl);
    address = std::move(addr);
  } else if (m_foldertype==PoolRefColl || m_foldertype==PoolRef) {
    auto addr = std::make_unique<CondAttrListCollAddress>(gAddr->svcType(),
                                                          gAddr->clID(),gAddr->par()[0],gAddr->par()[1],
                                                          poolSvcContext,gAddr->ipar()[1]);
    if (m_foldertype==PoolRefColl) {
      addr->setAttrListColl(attrListColl);
    }
    address = std::move(addr);
    poolPayloadReq=true;
  } else if (m_foldertype==AttrList) {
    auto addr = std::make_unique<AthenaAttrListAddress>(*gAddr);
    addr->setAttrList(attrList);
    address = std::move(addr);
  } else if (m_foldertype==CoraCool || m_foldertype==CoolVector) {
    auto addr = std::make_unique<CondAttrListVecAddress>(*gAddr);
    addr->setAttrListVec(attrListVec);
    address = std::move(addr);
  }
  return true;
}


void IOVDbFolder::summary() {
  const std::string & folderTypeName=IOVDbNamespace::folderTypeName(m_foldertype);
  // summarise the read statistics for this folder
  ATH_MSG_INFO(  "Folder " << m_foldername << " ("<<folderTypeName
	    << ") db-read " << m_ndbread << "/" <<
	    m_ncacheread << " objs/chan/bytes " << m_nobjread << "/" <<
	    m_nchan << "/" << m_nbytesread << " (( " << std::fixed << std::setw(8)
	    << std::setprecision(2) << m_readtime << " ))s" );
  // print WARNING if data for this folder was never read from Storegate
  if (m_ncacheread==0 && m_ndbread>0) {
    ATH_MSG_WARNING( "Folder " << m_foldername << " is requested but no data retrieved" );
  }
}

bool 
IOVDbFolder::overrideOptionsFromParsedDescription(const IOVDbParser & parsedDescription){
  bool success{true};
  // check for timeStamp indicating folder is timestamp indexed
  m_timestamp=parsedDescription.timebaseIs_nsOfEpoch();
  // check for key, giving a different key to the foldername
  if (auto newkey=parsedDescription.key(); not newkey.empty() and not m_jokey) {
    ATH_MSG_DEBUG( "Key for folder " << m_foldername << " set to "<< newkey << " from description string" );
    m_key=newkey;
  }
  // check for 'cache' but only if not already found in joboptions
  if (m_cachepar.empty()) m_cachepar=parsedDescription.cache();
  // check for cachehint
  if (int newCachehint=parsedDescription.cachehint();newCachehint!=0) m_cachehint=newCachehint;
  // check for <named/>   
  m_named=parsedDescription.named();
   // get addressHeader
  if (auto newAddrHeader = parsedDescription.addressHeader();not newAddrHeader.empty()){
    IOVDbNamespace::replaceServiceType71(newAddrHeader);
    m_addrheader=newAddrHeader;
  }
  //get clid, if it exists (set to zero otherwise)
  m_clid=parsedDescription.classId(msg());
  // decode the typeName
  if (!parsedDescription.getKey("typeName","",m_typename)) {
    ATH_MSG_ERROR( "Primary type name is empty" );
    return false;
  }
  bool gotCLID=(m_clid!=0);
  
  ATH_MSG_DEBUG( "Got folder typename " << m_typename );
  if (!gotCLID)
    if (StatusCode::SUCCESS==p_clidSvc->getIDOfTypeName(m_typename,m_clid)) 
      gotCLID=true;
  if (!gotCLID) {
    ATH_MSG_ERROR("Could not get clid for typeName: " << m_typename);
    return false;
  }
  ATH_MSG_DEBUG( "Got folder typename " << m_typename <<  " with CLID " << m_clid );
  return success;
}

std::unique_ptr<SG::TransientAddress>
IOVDbFolder::createTransientAddress(const std::vector<std::string> & symlinks){
  auto tad = std::make_unique<SG::TransientAddress>(m_clid,m_key);
  //
  for (const auto & linkname:symlinks){
    if (not linkname.empty()) {
      CLID sclid;
      if (StatusCode::SUCCESS==p_clidSvc->getIDOfTypeName(linkname,sclid)) {
        tad->setTransientID(sclid);
        ATH_MSG_DEBUG( "Setup symlink " << linkname << " CLID " <<sclid << " for folder " << m_foldername );
      } else {
        ATH_MSG_ERROR( "Could not get clid for symlink: "<< linkname );
        return nullptr;
      }
    }
  }
  return tad;
}

std::unique_ptr<SG::TransientAddress>
IOVDbFolder::preLoadFolder(ITagInfoMgr *tagInfoMgr , const unsigned int cacheRun, const unsigned int cacheTime, const std::string& globalTag) {
  // preload Address from SG - does folder setup including COOL access
  // also set detector store location - cannot be done in constructor
  // as detector store does not exist yet in IOVDbSvc initialisation
  // and sets up cache length, taking into account optional overrides
  // returns null pointer in case of problem
  p_tagInfoMgr = tagInfoMgr;
  if( not m_useFileMetaData ) {
    if(m_source=="CREST"){
      CrestFunctions cfunctions(m_crestServer);
      if (m_globaltag != globalTag){
        m_globaltag = globalTag;
        m_cresttagmap.clear();
        m_cresttagmap = cfunctions.getGlobalTagMap(globalTag);
      }
      const std::string  & tagName=m_cresttagmap[m_foldername];

      if (m_crest_tag != tagName){
        m_crest_tag = tagName;
        m_tag_info = cfunctions.getTagInfo(tagName);
      }
      m_folderDescription = cfunctions.getTagInfoElement(m_tag_info,"node_description");
    } else {
      //folder desc from db
      std::tie(m_multiversion, m_folderDescription) = IOVDbNamespace::folderMetadata(m_conn, m_foldername);
    }
  } else {
    // folder description from meta-data set already earlier
  }
  ATH_MSG_DEBUG( "Folder description for " << m_foldername << ": " << m_folderDescription);
  // register folder with meta-data tool if writing metadata
  if (m_writemeta) {
    if (StatusCode::SUCCESS!=p_metaDataTool->registerFolder(m_foldername,m_folderDescription)) {
      ATH_MSG_ERROR( "Failed to register folder " << m_foldername<< " for meta-data write" );
      return nullptr;
    }
  }
  // parse the description string
  IOVDbParser folderpar(m_folderDescription, msg());
  //use the overrides in the folderdescription, return nullptr immediately if something went wrong
  if (not overrideOptionsFromParsedDescription(folderpar)) return nullptr;
  // setup channel list and folder type
  if( not m_useFileMetaData ) {
    if(m_source=="CREST"){
        CrestFunctions cfunctions(m_crestServer);
        if (m_globaltag != globalTag){
          m_globaltag = globalTag;
          m_cresttagmap.clear();
          m_cresttagmap = cfunctions.getGlobalTagMap(globalTag);
        }       
        const std::string  & crestTag=m_cresttagmap[m_foldername];

        if (m_crest_tag != crestTag){
          m_crest_tag = crestTag;
          m_tag_info = cfunctions.getTagInfo(crestTag);
        }
 
        const std::string & payloadSpec = cfunctions.getTagInfoElement(m_tag_info,"payload_spec");  
        std::string chanList = cfunctions.getTagInfoElement(m_tag_info,"channel_list");
        std::tie(m_channums, m_channames) = cfunctions.extractChannelListFromString(chanList);

        //determine foldertype from the description, the spec and the number of channels
        m_foldertype = IOVDbNamespace::determineFolderType(m_folderDescription, payloadSpec, m_channums);
 
        if (m_crestToFile){
          int n_size = m_channums.size();
	  nlohmann::json chan_list = nlohmann::json::array();
          for (int i = 0; i < n_size; i++) {

            nlohmann::json elem;
	    std::string key = std::to_string(m_channums[i]);
            elem[key] = m_channames[i];
            chan_list.push_back(elem);
          }

          char ch = ':';
          int colsize = std::count(payloadSpec.begin(), payloadSpec.end(), ch);

	  nlohmann::json tag_meta;
          tag_meta["tagName"] = crestTag;
          tag_meta["description"] = "";
          tag_meta["chansize"] = n_size;
          tag_meta["colsize"] = colsize;

	  nlohmann::json tagInfo;
          tagInfo["channel_list"] = chan_list;
          tagInfo["node_description"] = m_folderDescription;
          tagInfo["payload_spec"] = payloadSpec;

          tag_meta["tagInfo"] = tagInfo;

          std::string crest_work_dir=std::filesystem::current_path();
          crest_work_dir += "/crest_data";
          bool crest_rewrite = true;
          Crest::CrestClient crestFSClient = Crest::CrestClient(crest_rewrite, crest_work_dir);

          try{
            crestFSClient.createTagMetaInfo(tag_meta);;
            ATH_MSG_INFO("Tag meta info for " << crestTag << " saved to disk.");
            ATH_MSG_INFO("CREST Dump dir = " << crest_work_dir);
          }
          catch (const std::exception& e) {
            ATH_MSG_WARNING("Tag meta info saving for tag " << crestTag << " failed: " << e.what());
          }
	} // m_crestToFile

    } else {
      // data being read from COOL
      auto fldPtr=m_conn->getFolderPtr<cool::IFolderPtr>(m_foldername);
      // get the list of channels
      std::tie(m_channums, m_channames) = IOVDbNamespace::channelList(m_conn, m_foldername,m_named);
      // set folder type 
      m_foldertype = IOVDbNamespace::determineFolderType(fldPtr);
    }
  }
  m_nchan=m_channums.size();
  ATH_MSG_DEBUG( "Folder identified as type " << m_foldertype );
  // note that for folders read from metadata, folder type identification
  // is deferred until getAddress when first data is read
  // and channel number/name information is not read

  // change channel selection for single-object read
  if (m_foldertype==AttrList || m_foldertype==PoolRef) m_chansel=cool::ChannelSelection(0);
  const auto & linknameVector = folderpar.symLinks();
  // now create TAD
  auto tad{createTransientAddress(linknameVector)};
  if (not tad) {
    ATH_MSG_WARNING("Transient address is null in "<<__func__);
    return nullptr;
  }
  setCacheLength(m_timestamp, cacheRun, cacheTime);
  return tad;
}

void IOVDbFolder::setCacheLength(const bool timeIs_nsOfEpoch, const unsigned int cacheRun, const unsigned int cacheTime){
  if (timeIs_nsOfEpoch){
    long long int clen=600; // default value of 10 minutes
    if (cacheTime!=0) {
      clen=cacheTime;
      m_autocache=false;
    } else {
      // for timestamp, cache parameter (if set) sets length in seconds
      if (not m_cachepar.empty()) clen=std::stoi(m_cachepar);
    }
    m_cachelength=IOVDbNamespace::iovTimeFromSeconds(clen);
    ATH_MSG_DEBUG( "Cache length set to " << clen << " seconds" );
  } else {
    // for run/event, cache parameter sets length in LB
    // default value is 1 whole run
    m_cachelength=IOVDbNamespace::ALL_LUMI_BLOCKS;
    if (cacheRun!=0) {
      m_cachelength=IOVDbNamespace::iovTimeFromRunLumi(cacheRun-1,IOVDbNamespace::ALL_LUMI_BLOCKS);
      m_autocache=false;
    } else {
      if (not m_cachepar.empty()) m_cachelength=std::stoi(m_cachepar);
    }
    const auto [run,lumi]=IOVDbNamespace::runLumiFromIovTime(m_cachelength);
    ATH_MSG_DEBUG( "Cache length set to " << run <<" runs " << lumi << " lumiblocks" );
  }
}

void 
IOVDbFolder::clearCache() {
  // clear all the cache vectors of information
  m_iovs.clear();
  m_cachechan.clear();
  m_cacheattr.clear();
  m_cacheccstart.clear();
  m_cacheccend.clear();
}

bool 
IOVDbFolder::resolveTag(const cool::IFolderPtr& fptr,const std::string& globalTag) {
  // resolve the tag 
  // if specified in job options or already-processed override use that,
  // else use global tag
  // return false for failure
  std::string tag=m_jotag;
  if (tag=="HEAD") return true;
  if (tag.empty()) tag=globalTag;
  if (tag.empty()) {
    ATH_MSG_ERROR( "No IOVDbSvc.GlobalTag specified on job options or input file" );
    return false;
  }
  if(m_source=="CREST"){
    if (m_globaltag != globalTag){
      m_globaltag = globalTag;
      m_cresttagmap.clear();
      CrestFunctions cfunctions(m_crestServer);
      m_cresttagmap = cfunctions.getGlobalTagMap(globalTag);
    }
    m_tag = m_cresttagmap[m_foldername];
    ATH_MSG_DEBUG( "resolveTag returns " << m_tag );
    return true;
  }
  // check for magic tags
  if (IOVDbNamespace::looksLikeMagicTag(tag) and not magicTag(tag)) return false;
  // check tag exists - if not, lookup hierarchically
  const std::vector<std::string>& taglist=fptr->listTags();
  if (find(taglist.begin(),taglist.end(),tag)!=taglist.end()) {
    // tag exists directly in folder
    ATH_MSG_DEBUG( "Using tag "<< tag << " for folder " << m_foldername );
  } else {
    // tag maybe an HVS tag 
    try {
      std::string restag=fptr->resolveTag(tag);
      ATH_MSG_INFO( "HVS tag " << tag << " resolved to "<< restag << " for folder " << m_foldername );
      // HVS tag may itself be magic
      if (IOVDbNamespace::looksLikeMagicTag(restag) and not magicTag(restag)) return false;
      tag=restag;
    }catch (cool::Exception& e) {
      ATH_MSG_ERROR( "Tag " << tag <<" cannot be resolved for folder " << m_foldername );
      return false;
    }
  }
  m_tag=tag;
  // optionally check if tag is locked
  if (m_checklock) {
    const auto tagLock=IOVDbNamespace::checkTagLock(fptr,tag);
    if (not tagLock.has_value()){
      ATH_MSG_ERROR( "Could not check tag lock status for " << tag );
      return false;
    }
    if (not tagLock.value()){
      ATH_MSG_ERROR("Tag " << tag <<" is not locked and IOVDbSvc.CheckLock is set" );
      return false;
    }
  }
  ATH_MSG_DEBUG( "resolveTag returns " << m_tag );
  return true;
}

bool 
IOVDbFolder::magicTag(std::string& tag) { //alters the argument
  tag = IOVDbNamespace::resolveUsingTagInfo(tag, p_tagInfoMgr);
  return (not tag.empty());
}



bool 
IOVDbFolder::addMetaAttrList(const coral::AttributeList& atrlist,
                                  const IOVRange& range) {
  // make a temporary CondAttrListCollection with channel 0xFFFF
  // This channel number is used to flag on readback that an 
  // AthenaAttributeList and not a CondAttrListCollection must be created
  CondAttrListCollection tmpColl(!m_timestamp);
  tmpColl.add(0xFFFF,atrlist);
  tmpColl.add(0xFFFF,range);
  return addMetaAttrListColl(&tmpColl);
}

bool 
IOVDbFolder::addMetaAttrListColl(const CondAttrListCollection* coll) {
  // send given payload to folder metadata
  // make a new CondAttrListCollection for the payload
  CondAttrListCollection* flmdColl=new CondAttrListCollection(*coll);
  if (StatusCode::SUCCESS!=p_metaDataTool->addPayload(m_foldername,flmdColl)) {
    ATH_MSG_ERROR( "addMetaAttrList: Failed to write metadata for folder " << m_foldername);
    return false;
  } else {
    ATH_MSG_DEBUG( "addMetaAttrList: write metadata for folder " << m_foldername );
    return true;
  }
}

void 
IOVDbFolder::setSharedSpec(const coral::AttributeList& atrlist) {
  m_cachespec=new coral::AttributeListSpecification;
  for (const auto & attribute:atrlist){
    const coral::AttributeSpecification& aspec=attribute.specification();
    m_cachespec->extend(aspec.name(),aspec.type());
    if (not typeSizeIsKnown(attribute)) {
      ATH_MSG_WARNING( "addType: unknown type " << aspec.typeName()<< 
      " in folder " << m_foldername <<  " will not be counted for bytes-read statistics" );
    }
  }
  ATH_MSG_DEBUG( "Setup shared AttributeListSpecification with " <<  m_cachespec->size() << " elements" );
}

void 
IOVDbFolder::addIOVtoCache(cool::ValidityKey since,cool::ValidityKey until) {
  // add IOV to the cache
  ATH_MSG_DEBUG("Adding IOV to cache, from "<<since<<" to "<<until);
  m_iovs.addIov(since, until);
}

void 
IOVDbFolder::printCache(){
    const auto & [since,until] = m_iovs.getCacheBounds(); 
    ATH_MSG_DEBUG("folder cache printout -------------------");
    ATH_MSG_DEBUG(m_foldername << " length: "<<m_cachelength<<"\tstart: "<<since<<"\tstop: "<<until);
    ATH_MSG_DEBUG("current range: "<<m_currange);
    const auto & iovs = m_iovs.vectorStore();
    std::vector<cool::ChannelId>::iterator      ci= m_cachechan.begin();
    for (const auto & iov:iovs){
      ATH_MSG_DEBUG("channelID:\t"<<(*ci++)<<"\t since: "<<iov.first<<"\t until: "<<iov.second);
    }
    ATH_MSG_DEBUG("folder cache printout -------------------");
  
}
