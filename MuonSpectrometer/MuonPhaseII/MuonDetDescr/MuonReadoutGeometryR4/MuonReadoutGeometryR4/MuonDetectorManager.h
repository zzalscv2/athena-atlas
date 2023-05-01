/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONREADOUTGEOMETRY_MUONDETECTORMANAGER_H
#define MUONREADOUTGEOMETRY_MUONDETECTORMANAGER_H

#include <map>
#include <memory>

#include "AthenaKernel/CLASS_DEF.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometryR4/MuonDetectorDefs.h"

/// The muon detector manager is the central class administrating the readout
/// elements of All muon subdetectors defined in the Geometry. The detector
/// elements are stored in a std::vector and their IdentifierHashes are used as
/// their corresponding position index. For each element type, e.g. CakeElement,
/// it provides one setter method and four getter methods.
///
///
///  Add the detector element to the manager. Fails if an element with the same
///  hash has already been added
///       StatusCode addCakeElement(std::unique_ptr<CakeElement> ele_ptr);
///
///  Return the (const) pointer to the detector element. The input Identifier is
///  the full ATLAS Identifier of the measurement
///       (const) CakeElement* getCakeElement(const Identifier& id) const;
///
///  Return the  (const) pointer to the detector element. The IdentifierHash has
///  to correspond to the hash of the readout element
///       (const) CakeElement* getCakeElement(const IdentifierHash& id) const;

/// Helper macros to declare the interface
#define DECLARE_GETTERSETTER(ELE_TYPE, GETTER, SETTER)        \
    ELE_TYPE* GETTER(const IdentifierHash& hash);             \
    ELE_TYPE* GETTER(const Identifier& hash);                 \
                                                              \
    const ELE_TYPE* GETTER(const IdentifierHash& hash) const; \
    const ELE_TYPE* GETTER(const Identifier& hash) const;     \
                                                              \
    StatusCode SETTER(ElementPtr<ELE_TYPE> element);

#define DECLARE_ELEMENT(ELE_TYPE) \
    DECLARE_GETTERSETTER(ELE_TYPE, get##ELE_TYPE, add##ELE_TYPE)

namespace MuonGMR4 {
class MdtReadoutElement;

class MuonDetectorManager : public GeoVDetectorManager, public AthMessaging {

   public:
    MuonDetectorManager();
    ~MuonDetectorManager() = default;

    template <class MuonDetectorType>
    using ElementPtr = std::unique_ptr<MuonDetectorType>;
    template <class MuonDetectorType>
    using ElementStorage = std::vector<ElementPtr<MuonDetectorType>>;

    DECLARE_ELEMENT(MdtReadoutElement)

    /// No idea what these things are doing
    unsigned int getNumTreeTops() const override final;
    PVConstLink getTreeTop(unsigned int i) const override final;

    void addTreeTop(PVConstLink pv);

   private:
    /// Returns the detector Identifier Hash
    IdentifierHash buildHash(const Identifier& id,
                             const MuonIdHelper& idHelper) const;
    IdentifierHash buildHash(const Identifier& id) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonDetectorManager"};

    ElementStorage<MdtReadoutElement> m_mdtEles{};

    std::vector<PVConstLink> m_treeTopVector{};
};

}  // namespace MuonGMR4

CLASS_DEF(MuonGMR4::MuonDetectorManager, 248531088, 1)
/// Delete the macro again
#undef DECLARE_GETTERSETTER
#undef DECLARE_ELEMENT
#include <MuonReadoutGeometryR4/MuonDetectorManager.icc>
#endif
