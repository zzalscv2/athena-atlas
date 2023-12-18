/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include "Acts/Utilities/HashedString.hpp"
#include "ActsEvent/Decoration.h"


namespace ActsTrk::detail {
std::vector<Decoration> restoreDecorations(
    const SG::IConstAuxStore* container,
    const std::set<std::string>& staticVariables) {
  std::vector<Decoration> decorations;
  for (auto id : container->getAuxIDs()) {
    const std::string name = SG::AuxTypeRegistry::instance().getName(id);
    const std::type_info* typeInfo =
        SG::AuxTypeRegistry::instance().getType(id);
    if (staticVariables.count(name) == 1) {
      continue;
    }

    // try making decoration accessor of matching type
    // there is a fixed set of supported types (as there is a fixed set
    // available in MutableMTJ) setters are not needed so replaced by a
    // "nullptr"
    if (*typeInfo == typeid(float)) {
      decorations.emplace_back(
          decoration<float>(name, ActsTrk::detail::constDecorationGetter<float>,
                            ActsTrk::detail::decorationCopier<float>));
    } else if (*typeInfo == typeid(double)) {
      decorations.emplace_back(decoration<double>(
          name, ActsTrk::detail::constDecorationGetter<double>,
          ActsTrk::detail::decorationCopier<double>));
    } else if (*typeInfo == typeid(short)) {
      decorations.emplace_back(
          decoration<short>(name, ActsTrk::detail::constDecorationGetter<short>,
                            ActsTrk::detail::decorationCopier<short>));
    } else if (*typeInfo == typeid(uint32_t)) {
      decorations.emplace_back(decoration<uint32_t>(
          name, ActsTrk::detail::constDecorationGetter<uint32_t>,
          ActsTrk::detail::decorationCopier<uint32_t>));
    } else {
      throw std::runtime_error("Can't restore decoration of  " + name +
                               " because it is of an unsupported type");
    }
  }
  return decorations;
}
}  // namespace ActsTrk::detail