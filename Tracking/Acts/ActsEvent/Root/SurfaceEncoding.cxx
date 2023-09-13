#include "ActsEvent/SurfaceEncoding.h"
namespace ActsTrk {
void encodeSurface(xAOD::SurfaceBackend*, const Acts::Surface* surface,
                   const ActsGeometryContext&) {
  return;
  //TODO
  // common part of Surface encoding
  
  // surface type specifics
  if (surface->type() == Acts::Surface::Plane) {
    //    ...
  }
  if (surface->type() == Acts::Surface::Perigee) {
    //    ...
  }
  if (surface->type() == Acts::Surface::Cylinder) {
    //    ...
  }
  if (surface->type() == Acts::Surface::Straw) {
    //    ...
  } else {
    throw std::out_of_range(
        "encodeSurface this type " +
        std::to_string(static_cast<int>(surface->type())) +
        " of Acts Surface can not be saved in xAOD::SurfaceBackend");
  }
}


std::shared_ptr<const Acts::Surface> decodeSurface(
    const xAOD::SurfaceBackend*, const ActsGeometryContext& ) {
  return std::shared_ptr<const Acts::Surface>(
      nullptr);  // TODO fill with decoding code (use Acts::Surface::makeShared)
}
}