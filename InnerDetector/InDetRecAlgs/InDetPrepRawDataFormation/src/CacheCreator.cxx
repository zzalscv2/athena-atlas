/*
Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/


#include "CacheCreator.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/TRT_ID.h"
#include "InDetIdentifier/SCT_ID.h"

#include "Identifier/IdentifierHash.h"
#include "AthViews/View.h"
namespace InDet{

    CacheCreator::CacheCreator(const std::string &name, ISvcLocator *pSvcLocator) :
    AthReentrantAlgorithm(name,pSvcLocator),
    m_pTRTHelper(nullptr),
    m_pix_idHelper(nullptr),
    m_sct_idHelper(nullptr),
    m_rioContainerCacheKey(""),
    m_SCTclusterContainerCacheKey(""),
    m_PIXclusterContainerCacheKey(""),
    m_PIXSpacePointCacheKey(""),
    m_SCTSpacePointCacheKey(""),

    m_SCTRDOCacheKey(""), m_PixRDOCacheKey(""),
    m_disableTRT(false), m_disableWarning(false)
    {
        declareProperty("TRT_DriftCircleKey", m_rioContainerCacheKey);
        declareProperty("SCT_ClusterKey"    , m_SCTclusterContainerCacheKey);
        declareProperty("Pixel_ClusterKey"  , m_PIXclusterContainerCacheKey);
        declareProperty("SpacePointCachePix"  , m_PIXSpacePointCacheKey);
        declareProperty("SpacePointCacheSCT"  , m_SCTSpacePointCacheKey);
        declareProperty("SCTRDOCacheKey", m_SCTRDOCacheKey);
        declareProperty("disableTRT"  , m_disableTRT);
        declareProperty("PixRDOCacheKey", m_PixRDOCacheKey);
        declareProperty("DisableViewWarning", m_disableWarning);
    }


    StatusCode CacheCreator::initialize(){
        ATH_CHECK( m_rioContainerCacheKey.initialize(!m_rioContainerCacheKey.key().empty()) );
        ATH_CHECK( m_SCTclusterContainerCacheKey.initialize(!m_SCTclusterContainerCacheKey.key().empty()) );
        ATH_CHECK( m_PIXclusterContainerCacheKey.initialize(!m_PIXclusterContainerCacheKey.key().empty()) );
        ATH_CHECK( m_PIXSpacePointCacheKey.initialize(!m_PIXSpacePointCacheKey.key().empty()) );
        ATH_CHECK( m_SCTSpacePointCacheKey.initialize(!m_SCTSpacePointCacheKey.key().empty()) );
        ATH_CHECK( m_SCTRDOCacheKey.initialize(!m_SCTRDOCacheKey.key().empty()) );
        ATH_CHECK( m_PixRDOCacheKey.initialize(!m_PixRDOCacheKey.key().empty()) );
        ATH_CHECK( m_condKey5.initialize() );
        if(!m_disableTRT) ATH_CHECK(detStore()->retrieve(m_pTRTHelper  , "TRT_ID"));
        ATH_CHECK(detStore()->retrieve(m_sct_idHelper, "SCT_ID"));
        ATH_CHECK(detStore()->retrieve(m_pix_idHelper, "PixelID"));
        return StatusCode::SUCCESS;
    }

    CacheCreator::~CacheCreator() {}

    bool CacheCreator::IsInsideView(const EventContext& context) const
    {
        const IProxyDict* proxy = context.getExtension<Atlas::ExtendedEventContext>().proxy();
        const SG::View* view = dynamic_cast<const SG::View*>(proxy);
        return view != nullptr;
    }

    StatusCode CacheCreator::execute (const EventContext& ctx) const
    {

        if(!m_disableWarning && IsInsideView(ctx)){
           ATH_MSG_WARNING("CacheCreator is running inside a view, this is probably a misconfiguration");
        }

        if(!m_disableTRT) ATH_CHECK(CreateContainer(m_rioContainerCacheKey, m_pTRTHelper->straw_layer_hash_max(), ctx));
        
        ATH_CHECK(CreateContainer(m_SCTclusterContainerCacheKey, m_sct_idHelper->wafer_hash_max(), ctx));
        
        ATH_CHECK(CreateContainer(m_PIXclusterContainerCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));

        ATH_CHECK(CreateContainer(m_PIXSpacePointCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));

        ATH_CHECK(CreateContainer(m_SCTSpacePointCacheKey, m_sct_idHelper->wafer_hash_max(), ctx));

        ATH_CHECK(CreateContainer(m_SCTRDOCacheKey, m_sct_idHelper->wafer_hash_max(), ctx));

        ATH_CHECK(CreateContainer(m_PixRDOCacheKey, m_pix_idHelper->wafer_hash_max(), ctx));

        return StatusCode::SUCCESS;
    }
}

