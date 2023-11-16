/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/StorableConversions.h"
#include "AthenaKernel/DataBucketBase.h"
#include "CxxUtils/FPControl.h"
#include "CxxUtils/SealDebug.h"
#include "CxxUtils/SealSignal.h"
#include "CxxUtils/hexdump.h"
#include "RootUtils/WithRootErrorHandler.h"
#include "SGTools/DataProxy.h"
#include "TrigSerializeResult/StringSerializer.h"
#include "BareDataBucket.h"
#include "TBufferFile.h"
#include "TVirtualCollectionProxy.h"
#include "TClass.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainersInterfaces/AuxTypes.h"
#include "xAODCore/AuxContainerBase.h"
#include "AthContainersRoot/getDynamicAuxID.h"
#include "RootUtils/Type.h"

#include "TriggerEDMDeserialiserAlg.h"
#include "TriggerEDMCLIDs.h"

#include "TFile.h"
#include "TStreamerInfo.h"
#include "PathResolver/PathResolver.h"

#include <sys/resource.h>
#include <cstring>
#include <regex>

namespace {
const SG::BaseInfoBase* getBaseInfo(CLID clid) {
  const SG::BaseInfoBase* bi = SG::BaseInfoBase::find(clid);
  if (bi){
    return bi;
  }
  // Try to force a dictionary load to get it defined.
  ServiceHandle<IClassIDSvc> clidsvc("ClassIDSvc", "ProxyProviderSvc");
  if (!clidsvc.retrieve()){
    return nullptr;
  }
  std::string name;
  if (!clidsvc->getTypeNameOfID(clid, name).isSuccess()) {
    return nullptr;
  }
  (void)TClass::GetClass(name.c_str());
  return SG::BaseInfoBase::find(clid);
}
}  // anonymous namespace

class TriggerEDMDeserialiserAlg::WritableAuxStore : public SG::AuxStoreInternal {
public:
  WritableAuxStore() = default;
  using SG::AuxStoreInternal::addVector;
};


namespace  {
  /**
   * @brief Find the type of an element of a vector.
   * @param[in] tname The name of the type to analyze.
   * @param[out] elementTypeName The name of the type of an element of the vector.
   *
   * Returns the @c type_info for an element of the vector.
   */
  const std::type_info* getElementType ( const std::string& tname,
					 std::string& elementTypeName ) {
    TClass* cls = TClass::GetClass( tname.c_str() );
    if ( cls == nullptr ) return nullptr;
    TVirtualCollectionProxy* prox = cls->GetCollectionProxy();
    if ( prox == nullptr ) return nullptr;
    if ( prox->GetValueClass() != nullptr ) {
      elementTypeName = prox->GetValueClass()->GetName();
      return prox->GetValueClass()->GetTypeInfo();
    }
    RootUtils::Type type ( prox->GetType() );
    elementTypeName = type.getTypeName();
    return type.getTypeInfo();
  }

  /**
   * Strip "std::vector" from type name
   */
  std::string stripStdVec (const std::string& s_in) {
    std::string s = s_in;
    std::string::size_type pos{0};
    while ((pos = s.find ("std::vector<")) != std::string::npos) {
      s.erase (pos, 5);
    }
    return s;
  }

  /**
   * Detect if there is a version change between two types (e.g. v1, v2).
   *
   * Due to type-aliasing we cannot just compare the entire type name as e.g.
   * 'DataVector<xAOD::MyType_v1>' and 'xAOD::MyTypeContainer_v1' should be
   * considered the same version.
   */
  bool versionChange(const std::string& type1, const std::string& type2) {
    static const std::regex re(".+(_v[0-9]+).*");  // find last _v in string
    std::smatch m1, m2;
    return ( std::regex_match(type1, m1, re) and
             std::regex_match(type2, m2, re) and
             m1.str(1) != m2.str(1) );  // number 0 is full match
  }

  /**
   * Temporary ROOT error handler for deserialization (ATR-25049)
   */
  struct handleError
  {
    using Payload = std::vector<uint32_t>;

    bool operator()(int level, bool /*abort*/, const char* location, const char* /*msg*/) {
      if ( level >= kError && location && strstr(location, "TBufferFile::ReadClass")) {
        // Raise soft core dump size limit to hard limit
        struct rlimit core_limit;
        getrlimit(RLIMIT_CORE, &core_limit);
        core_limit.rlim_cur = core_limit.rlim_max;
        setrlimit(RLIMIT_CORE, &core_limit);
        
        std::cout << "TriggerEDMDeserialiserAlg: Raising core dump soft size limit to " << core_limit.rlim_cur
                  << " and trying to dump core file..." << std::endl;
        Athena::DebugAids::coredump(SIGSEGV);   // this is non-fatal, job continues
      }

      if ( level >= kError && location && strstr(location, "TClass::Load")) {
        std::cout << "TriggerEDMDeserialiserAlg: buff dump; start " << m_start << "\n";
        CxxUtils::hexdump (std::cout, m_buf, m_bufsize);
        std::cout << "TriggerEDMDeserialiserAlg: payload dump\n";
        CxxUtils::hexdump (std::cout, m_payload->data(), m_payload->size() * sizeof(Payload::value_type));
      }

      return true;  // call default handlers
    }

    handleError (const char* buf, size_t bufsize, const Payload* payload,
                 const void* start)
      : m_buf (buf), m_bufsize (bufsize), m_payload (payload), m_start(start)
    {
    }

    const char* m_buf;
    size_t m_bufsize;
    const Payload* m_payload;
    const void* m_start;
  };

}

/**
 * Collection of helper functions for raw pointer operations on the bytestream payload
 *
 * Most functions can be constexpr if the compiler implements ConstexprIterator (P0858R0)
 * Tested it works in clang9 (and 10 and 11?) regardless of --std flag and in gcc10+ only with --std=c++20
 * But clang12 requires --std=c++20.
 * TODO: Remove the C++ version checks when the release is built with --std=c++20 or newer
 */
namespace PayloadHelpers {
  using TDA = TriggerEDMDeserialiserAlg;

  /// CLID of the collection stored in the next fragment
  #if __cpp_lib_array_constexpr >= 201811L
  constexpr
  #endif
  CLID collectionCLID(TDA::PayloadIterator start) {
    return *( start + TDA::CLIDOffset );
  }

  /// Length of the serialised name payload
  #if __cpp_lib_array_constexpr >= 201811L
  constexpr
  #endif
  size_t nameLength(TDA::PayloadIterator start) {
    return *( start + TDA::NameLengthOffset );
  }

  /// Size in bytes of the buffer that is needed to decode next fragment data content
  #if __cpp_lib_array_constexpr >= 201811L
  constexpr
  #endif
  size_t dataSize(TDA::PayloadIterator start) {
    return *( start + TDA::NameOffset + nameLength(start) );
  }

  /**
   * Returns starting point of the next fragment, can be == end()
   *
   * Intended to be used like this: start = advance(start); if ( start != data.end() )... decode else ... done
   **/
  #if __cpp_lib_array_constexpr >= 201811L
  constexpr
  #endif
  TDA::PayloadIterator toNextFragment(TDA::PayloadIterator start) {
    return start + (*start); // point ahead by the number of words pointed to by start iterator
  }

  /// String description of the collection stored in the next fragment, returns persistent type name and the SG key
  std::vector<std::string> collectionDescription(TDA::PayloadIterator start) {
    StringSerializer ss;
    std::vector<std::string> labels;
    ss.deserialize( start + TDA::NameOffset, start + TDA::NameOffset + nameLength(start), labels );
    return labels;
  }

  /// Copies fragment to the buffer, no size checking, use @c dataSize to do so
  void toBuffer(TDA::PayloadIterator start, char* buffer) {
    // move to the beginning of the buffer memory
    TDA::PayloadIterator dataStart =  start + TDA::NameOffset + nameLength(start) + 1 /*skip size*/;
    // we rely on continuous memory layout of std::vector ...
    std::memcpy( buffer, &(*dataStart), dataSize(start) );
  }
}

std::unique_ptr<TList> TriggerEDMDeserialiserAlg::s_streamerInfoList{};
std::mutex TriggerEDMDeserialiserAlg::s_mutex;

TriggerEDMDeserialiserAlg::TriggerEDMDeserialiserAlg(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode TriggerEDMDeserialiserAlg::initialize() {
  ATH_CHECK( m_resultKey.initialize() );
  ATH_CHECK( m_clidSvc.retrieve() );
  ATH_CHECK( m_serializerSvc.retrieve() );
  ATH_CHECK( m_tpTool.retrieve() );
  add_bs_streamerinfos();
  return StatusCode::SUCCESS;
}


StatusCode TriggerEDMDeserialiserAlg::finalize() {
  s_streamerInfoList.reset();
  return StatusCode::SUCCESS;
}


StatusCode TriggerEDMDeserialiserAlg::execute(const EventContext& context) const {

  auto resultHandle = SG::makeHandle( m_resultKey, context );
  if ( not resultHandle.isValid() ) {
    ATH_MSG_ERROR("Failed to obtain HLTResultMT with key " << m_resultKey.key());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Obtained HLTResultMT with key " << m_resultKey.key());
  
  const Payload* dataptr = nullptr;
  if ( resultHandle->getSerialisedData( m_moduleID, dataptr ).isFailure() ) {
    if ( m_permitMissingModule ) {
      ATH_MSG_DEBUG("No payload available with moduleId " << m_moduleID << " in this event, ignored");
      return StatusCode::SUCCESS;
    } else {
      ATH_MSG_ERROR("No payload available with moduleId " << m_moduleID << " in this event");
      return StatusCode::FAILURE;
    }
  }
  ATH_CHECK( deserialise( dataptr ) );
  return StatusCode::SUCCESS;
}

StatusCode TriggerEDMDeserialiserAlg::deserialise( const Payload* dataptr ) const {

  size_t buffSize = m_initialSerialisationBufferSize;
  std::unique_ptr<char[]> buff = std::make_unique<char[]>(buffSize);

  // returns a char* buffer that is at minimum as large as specified in the argument
  auto resize = [&buffSize, &buff]( const size_t neededSize ) -> void {
		  if ( neededSize > buffSize ) {
		    buffSize = neededSize;
		    buff = std::make_unique<char[]>(buffSize);
		  }
		};  

  // the pointers defined below need to be used in decoding consecutive fragments of xAOD containers:
  // 1) xAOD interface, 2) Aux store, 3) decorations
  // invalid conditions are: invalid interface pointer when decoding Aux store
  //                         invalid aux store and interface when decoding the decoration
  // these pointer should be invalidated when: decoding TP containers, aux store when decoding the xAOD interface 
  WritableAuxStore* currentAuxStore = nullptr;         // set when decoding Aux
  SG::AuxVectorBase* xAODInterfaceContainer = nullptr; // set when decoding xAOD interface
  
  size_t fragmentCount = 0;
  PayloadIterator start = dataptr->begin();
  std::string previousKey;
  while ( start != dataptr->end() )  {
    fragmentCount++;
    const CLID clid{ PayloadHelpers::collectionCLID( start ) };
    std::string transientTypeName, transientTypeInfoName;
    ATH_CHECK( m_clidSvc->getTypeNameOfID( clid, transientTypeName ) );
    ATH_CHECK( m_clidSvc->getTypeInfoNameOfID( clid, transientTypeInfoName ) ); // version

    const std::vector<std::string> descr( PayloadHelpers::collectionDescription( start ) );
    ATH_CHECK( descr.size() == 2 );
    std::string persistentTypeName{ descr[0] };
    const std::string key{ descr[1] };
    const size_t bsize{ PayloadHelpers::dataSize( start ) };

    if( m_skipDuplicates && evtStore()->contains(clid,m_prefix+key) ) {
      ATH_MSG_DEBUG("Skipping duplicate record " << m_prefix+key);
      // Advance
      start = PayloadHelpers::toNextFragment( start );
      continue;
    }

    ATH_MSG_DEBUG( "fragment #" << fragmentCount <<
                   " type: "<< transientTypeName << " (" << transientTypeInfoName << ")" <<
                   " persistent type: " << persistentTypeName << " key: " << key << " size: " << bsize );
    resize( bsize );
    PayloadHelpers::toBuffer( start, buff.get() );

    // point the start to the next chunk, irrespectively of what happens in deserialisation below
    start = PayloadHelpers::toNextFragment( start );
        
    RootType classDesc = RootType::ByNameNoQuiet( persistentTypeName );
    ATH_CHECK( classDesc.IsComplete() );

    // Many variables in this class were changed from double to float.
    // However, we wrote data in the past which contained values
    // that were valid doubles but which were out of range for floats.
    // So we can get FPEs when we read them.
    // Disable FPEs when we're reading an instance of this class.
    CxxUtils::FPControl fpcontrol;
    if (persistentTypeName == "xAOD::BTaggingTrigAuxContainer_v1") {
      fpcontrol.holdExceptions();
    }

    size_t usedBytes{ bsize };
    void* obj{ nullptr };
    {
      // Temporary error handler to debug ATR-25049
      RootUtils::WithRootErrorHandler hand( handleError(buff.get(), usedBytes, dataptr, &*start) );
      obj = m_serializerSvc->deserialize( buff.get(), usedBytes, classDesc );
    }

    ATH_MSG_DEBUG( "Deserialised object of ptr: " << obj << " which used: " << usedBytes <<
                   " bytes from available: " << bsize );
    if ( obj == nullptr ) {
      ATH_MSG_ERROR( "Deserialisation of object of CLID " << clid << " and transientTypeName " <<
                     transientTypeName << " # " << key << " failed" );
      return StatusCode::FAILURE;
    }
    const bool isxAODInterfaceContainer = (transientTypeName.rfind("xAOD", 0) != std::string::npos and
                                           transientTypeName.find("Aux") == std::string::npos and
                                           transientTypeName.find("ElementLink") == std::string::npos);
    const bool isxAODAuxContainer       = (transientTypeName.rfind("xAOD", 0) != std::string::npos and
                                           transientTypeName.find("Aux") != std::string::npos);
    const bool isxAODDecoration	        = transientTypeName.find("vector") != std::string::npos;
    const bool isTPContainer	        = persistentTypeName.find("_p")	!= std::string::npos;
    const bool isVersionChange          = versionChange(persistentTypeName, transientTypeInfoName);

    ATH_CHECK( checkSanity( transientTypeName, isxAODInterfaceContainer,
                            isxAODAuxContainer, isxAODDecoration, isTPContainer ) );
    
    if ( isTPContainer or isVersionChange ) {
      if ( isVersionChange ) ATH_MSG_DEBUG( "Version change detected from " << persistentTypeName << " to "
                                            << transientTypeInfoName << ". Will invoke PT converter." );

      std::string decodedTransientName;
      void * converted = m_tpTool->convertPT( persistentTypeName, obj, decodedTransientName );
      ATH_CHECK( converted != nullptr );
      classDesc.Destruct( obj );

      // from now on in case of T/P class we deal with a new class, the transient one
      classDesc = RootType::ByNameNoQuiet( transientTypeName );
      ATH_CHECK( classDesc.IsComplete() );
      obj = converted;
    }

    if ( isxAODInterfaceContainer or isxAODAuxContainer or isTPContainer ) {
      BareDataBucket* dataBucket = new BareDataBucket( obj, clid, classDesc );
      const std::string outputName = m_prefix + key;
      auto proxyPtr = evtStore()->recordObject( SG::DataObjectSharedPtr<BareDataBucket>( dataBucket ),
                                                outputName, false, false );
      if ( proxyPtr == nullptr )  {
        ATH_MSG_WARNING( "Recording of object of CLID " << clid << " and name " << outputName << " failed" );
      }

      if ( isxAODInterfaceContainer ) {
        // If the container of the previous iteration was supposed to have an Aux store (trackIndices)
        // but we didn't find one, then create at least a DataLink with the correct key name.
        // The EDMCreatorAlg will take care of creating an empty Aux store with the correct type.
        if (xAODInterfaceContainer!=nullptr &&
            xAODInterfaceContainer->trackIndices() && currentAuxStore==nullptr) {
          ATH_MSG_DEBUG("Container with key " << previousKey << " is missing its Aux store");
          xAODInterfaceContainer->setStore( DataLink<SG::IConstAuxStore>(previousKey+"Aux.") );
        }
        currentAuxStore = nullptr; // the store will be following, setting it to nullptr assure we catch issue with of missing Aux
        const SG::BaseInfoBase* bib = getBaseInfo(clid);
        if(!bib){
          ATH_MSG_WARNING("No BaseInfoBase for CLID "<< clid << " and name " << outputName); 
        }
        xAODInterfaceContainer =
            bib ? reinterpret_cast<SG::AuxVectorBase*>(
                      bib->cast(dataBucket->object(),
                                ClassID_traits<SG::AuxVectorBase>::ID()))
                : nullptr;
      } else if (isxAODAuxContainer) {
        // key contains exactly one '.' at the end
        ATH_CHECK( key.find('.') == key.size()-1 );
        ATH_CHECK( currentAuxStore == nullptr and xAODInterfaceContainer != nullptr );
        const SG::BaseInfoBase* bib = getBaseInfo(clid);
        xAOD::AuxContainerBase* auxHolder =
            reinterpret_cast<xAOD::AuxContainerBase*>(
                bib->cast(dataBucket->object(), ClassID_traits<SG::IAuxStore>::ID()));
        ATH_CHECK(auxHolder != nullptr);
        xAODInterfaceContainer->setStore(auxHolder);
        currentAuxStore = new WritableAuxStore();
        auxHolder->setStore( currentAuxStore );
      } else {
        currentAuxStore = nullptr;
        xAODInterfaceContainer = nullptr; // invalidate xAOD related pointers
      }

    } else if ( isxAODDecoration ) {
      if(m_skipDuplicates and (currentAuxStore == nullptr || xAODInterfaceContainer == nullptr)) {
        ATH_MSG_DEBUG("Decoration " << key << " encountered with no active container. Assume this was already handled.");
      } else {
        ATH_CHECK( currentAuxStore != nullptr and xAODInterfaceContainer != nullptr );
        ATH_CHECK( deserialiseDynAux( transientTypeName, persistentTypeName, key, obj,
                                      currentAuxStore, xAODInterfaceContainer ) );
      }
    }
    previousKey = key;
  }
  return StatusCode::SUCCESS;
}



StatusCode TriggerEDMDeserialiserAlg::deserialiseDynAux( const std::string& transientTypeName, const std::string& persistentTypeName, const std::string& decorationName,
							 void* obj,   WritableAuxStore* currentAuxStore, SG::AuxVectorBase* interfaceContainer ) const {
  const bool isPacked = persistentTypeName.find("SG::PackedContainer") != std::string::npos;      

  SG::AuxTypeRegistry& registry = SG::AuxTypeRegistry::instance();     
  SG::auxid_t id = registry.findAuxID ( decorationName );
  if (id != SG::null_auxid ) {
    std::string regTypeName = stripStdVec( registry.getVecTypeName(id) );
    if ( regTypeName != stripStdVec(transientTypeName) and transientTypeName.find("ElementLink") == std::string::npos )
    {
      // Before giving up, also translate any typedefs in the transient name.
      RootUtils::Type tname (transientTypeName);
      if ( regTypeName != stripStdVec(tname.getTypeName()) ) {
        ATH_MSG_INFO( "Schema evolution required for decoration \"" << decorationName << "\" from " << transientTypeName << " to "  <<  registry.getVecTypeName( id ) << " not handled yet");
        return StatusCode::SUCCESS;
      }
    }
  } else {
      std::string elementTypeName;
      const std::type_info* elt_tinfo = getElementType( transientTypeName, elementTypeName );
      ATH_CHECK( elt_tinfo != nullptr );
      ATH_MSG_DEBUG( "Dynamic decoration: \"" << decorationName << "\" of type " << transientTypeName << " will create a dynamic ID, stored type" << elementTypeName );    
      id = SG::getDynamicAuxID ( *elt_tinfo, decorationName, elementTypeName, transientTypeName, false );
  }        
  ATH_MSG_DEBUG( "Unstreaming decoration \"" << decorationName << "\" of type " << transientTypeName  << " aux ID " << id << " class " << persistentTypeName << " packed " << isPacked  );  
  std::unique_ptr<SG::IAuxTypeVector> vec( registry.makeVectorFromData (id, obj, isPacked, true) ); 
  ATH_CHECK( vec.get() != nullptr );
  ATH_MSG_DEBUG("Size for \"" << decorationName << "\" " << vec->size() << " interface " << interfaceContainer->size_v() );
  ATH_CHECK( vec->size() == interfaceContainer->size_v() );
  if ( vec->size() != 0 ) {
    ATH_CHECK( currentAuxStore != nullptr );
    currentAuxStore->addVector(std::move(vec), false);    
    // trigger loading of the dynamic variables
    SG::AuxElement::TypelessConstAccessor accessor( decorationName );
    accessor.getDataArray( *interfaceContainer );
  }
  return StatusCode::SUCCESS;  
}

StatusCode TriggerEDMDeserialiserAlg::checkSanity( const std::string& transientTypeName, bool isxAODInterfaceContainer, bool isxAODAuxContainer, bool isDecoration, bool isTPContainer ) const {
  ATH_MSG_DEBUG( "Recognised type " << transientTypeName <<" as: "
		 << (isxAODInterfaceContainer ? "xAOD Interface Container":"" )
		 << (isxAODAuxContainer ? "xAOD Aux Container ":"" )
		 << ( isDecoration ? "xAOD Decoration" : "")
		 << ( isTPContainer ? "T/P Container " : "") );
  
  const std::vector<bool> typeOfContainer( { isxAODInterfaceContainer, isxAODAuxContainer, isDecoration, isTPContainer } );			     
  const size_t count = std::count( typeOfContainer.begin(), typeOfContainer.end(), true );
  if ( count == 0 ) {
    ATH_MSG_ERROR( "Could not recognise the kind of container " << transientTypeName );
    return StatusCode::FAILURE;
  }
  if (count > 1 ) {
    ATH_MSG_ERROR( "Ambiguous container kind deduced from the transient type name " << transientTypeName );
    ATH_MSG_ERROR( "Recognised type as: " 
		   << (isxAODInterfaceContainer ? "xAOD Interface Context":"" )
		   << (isxAODAuxContainer ? " xAOD Aux Container ":"" )
		   << ( isDecoration ? "xAOD Decoration" : "")
		   << ( isTPContainer ? "T/P Container " : "") );
    return StatusCode::FAILURE;    
  }
  return StatusCode::SUCCESS;
}


void TriggerEDMDeserialiserAlg::add_bs_streamerinfos(){
  std::lock_guard<std::mutex> lock(s_mutex);

  if (s_streamerInfoList) {
    return;
  }

  std::string extStreamerInfos = "bs-streamerinfos.root";
  std::string extFilePath = PathResolver::find_file(extStreamerInfos, "DATAPATH");
  ATH_MSG_DEBUG( "Using " << extFilePath );
  TFile extFile(extFilePath.c_str());

  s_streamerInfoList = std::unique_ptr<TList>(extFile.GetStreamerInfoList());
  for(const auto&& infObj: *s_streamerInfoList) {
    TString t_name=infObj->GetName();
    if (t_name.BeginsWith("listOfRules")){
      ATH_MSG_WARNING( "Could not re-load  class " << t_name );
      continue;
    }

    TStreamerInfo* inf = dynamic_cast<TStreamerInfo*>(infObj);
    inf->BuildCheck();
    TClass *cl = inf->GetClass();
    if (cl != nullptr) {
      ATH_MSG_DEBUG( "external TStreamerInfo for " << cl->GetName() <<
                     " checksum: " << std::hex << inf->GetCheckSum() << std::dec );
    }
  }
}
