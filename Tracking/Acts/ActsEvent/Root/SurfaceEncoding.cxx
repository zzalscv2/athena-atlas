/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


#include "ActsEvent/SurfaceEncoding.h"
namespace ActsTrk {
void encodeSurface(xAOD::SurfaceBackend* surfBackend, const Acts::Surface* surface,
                   const ActsGeometryContext& gctx) {
  // return if surf is a nullptr
  if (surface == nullptr) {
    std::cout<<"input surface in encodeSurface is a nullptr."<<std::endl;
    return;
  }  

  // surface type specifics
  if (surface->type() == Acts::Surface::Cone) {
    surfBackend->setSurfaceType(xAOD::SurfaceType::Cone);
  }
  else if (surface->type() == Acts::Surface::Cylinder) {
    surfBackend->setSurfaceType(xAOD::SurfaceType::Cylinder);
  }
  else if (surface->type() == Acts::Surface::Disc) {
    surfBackend->setSurfaceType(xAOD::SurfaceType::Disc);
  } 
  else if (surface->type() == Acts::Surface::Perigee) {
    surfBackend->setSurfaceType(xAOD::SurfaceType::Perigee);
  }
  else if (surface->type() == Acts::Surface::Plane) {
    surfBackend->setSurfaceType(xAOD::SurfaceType::Plane);
  }
  else if (surface->type() == Acts::Surface::Straw) {
    surfBackend->setSurfaceType(xAOD::SurfaceType::Straw);
  }
  else {
    throw std::out_of_range(
        "encodeSurface this type " +
        std::to_string(static_cast<int>(surface->type())) +
        " of Acts Surface can not be saved in xAOD::SurfaceBackend");
    return;
  }




  Acts::RotationMatrix3 lRotation = surface->transform(gctx.context()).rotation();
  Acts::Vector3 eulerAngles = lRotation.eulerAngles(2, 1, 0);
  Acts::Vector3 lTranslation = surface->center(gctx.context());

   
  std::vector<float> euler, translation; 
  for (int i=0;i<3;i++) {
    euler.push_back(eulerAngles[i]);
    translation.push_back(lTranslation[i]);
  }

  surfBackend->setTranslation(translation);
  surfBackend->setRotation(euler);

  // Transforming double->float
  const std::vector<double> values = surface->bounds().values();
  std::vector<float> fvalues;
  for (unsigned int i=0;i<size(values); i++) 
    fvalues.push_back(values[i]);
  surfBackend->setBoundValues(fvalues);

}


std::shared_ptr<const Acts::Surface> decodeSurface(const xAOD::SurfaceBackend* surfBackend, const ActsGeometryContext& ) {

      // Translation and rotation
      auto translation = surfBackend->translation();
      auto rotation    = surfBackend->rotation();

      // create the transformation matrix
      auto transform = Acts::Transform3(Acts::Translation3(translation[0],translation[1],translation[2]));
      transform *= Acts::AngleAxis3(rotation[0], Acts::Vector3(0., 0., 1.));  //rotZ
      transform *= Acts::AngleAxis3(rotation[1], Acts::Vector3(0., 1., 0.));  //rotY
      transform *= Acts::AngleAxis3(rotation[2], Acts::Vector3(1., 0., 0.));  //rotX

      auto boundValues  = surfBackend->boundValues();

      //cone
      if (surfBackend->SurfaceType() == xAOD::Cone) {
        auto surface = Acts::Surface::makeShared<Acts::ConeSurface>(
            transform, boundValues[0], boundValues[1], boundValues[2], boundValues[3]);     
        return surface;
      }
      //Cylinder
      else if (surfBackend->SurfaceType() == xAOD::Cylinder) {
        // phi/2 must be slightly < Pi to avoid crashing
        if (boundValues[2] > M_PI-0.001)
            boundValues[2] = M_PI-0.001;
        auto surface = Acts::Surface::makeShared<Acts::CylinderSurface>(
            transform, boundValues[0], boundValues[1], boundValues[2], boundValues[3], boundValues[4]);     
        return surface;                
      }
      //Disc
      else if (surfBackend->SurfaceType() == xAOD::Disc) {
        auto surface = Acts::Surface::makeShared<Acts::DiscSurface>(
            transform, boundValues[0], boundValues[1], boundValues[2]);     
        return surface;
      } 
      //Perigee
      else if (surfBackend->SurfaceType() == xAOD::Perigee) {
        auto surface = Acts::Surface::makeShared<Acts::PerigeeSurface>(
            transform);     
        return surface;
      }
      //Plane
      else if (surfBackend->SurfaceType() == xAOD::Plane) {
        Acts::Vector2 min(boundValues[0], boundValues[1]), max(boundValues[2], boundValues[3]);
        auto rBounds = std::make_shared<const Acts::RectangleBounds>(min, max);
        auto surface = Acts::Surface::makeShared<Acts::PlaneSurface>(
            transform, rBounds);     
        return surface;
      }
      //Straw
      else if (surfBackend->SurfaceType() == xAOD::Straw) {
        auto surface = Acts::Surface::makeShared<Acts::StrawSurface>(
            transform, boundValues[0], boundValues[1]);     
        return surface;
      }
      else {
        throw std::out_of_range(
            "encodeSurface this type " +
            std::to_string(static_cast<int>(surfBackend->SurfaceType())) +
            " of Acts Surface can not be saved in xAOD::SurfaceBackend");
        return nullptr;
      }
      return nullptr; 
}
}
