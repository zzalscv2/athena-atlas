/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <iostream>

// Local include(s):
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include "GeoPrimitives/GeoPrimitives.h"

template< typename T >
std::ostream& operator<< ( std::ostream& out,
			   const std::vector<T>& vec ) {
  out << "[";
  for( size_t i = 0; i < vec.size(); ++i ) {
    out << vec[ i ];
    if( i < vec.size() - 1 ) {
      out << ", ";
    }
  }
  out << "]";
  return out;
}

template< typename T, std::size_t N >
std::ostream& operator<< ( std::ostream& out,
                           const Eigen::Matrix<T,N,1>& arr) {
  std::vector<T> vec;
  vec.reserve(N);

  for( size_t i = 0; i < N; ++i ) {
    vec.push_back(arr(i,0));
  }

  out << vec;
  return out;
}

void fill(xAOD::SpacePoint& pixel, xAOD::SpacePoint& strip) {
  
  IdentifierHash::value_type idHashVal(123485);
  IdentifierHash idHash(idHashVal);

  Eigen::Matrix<float,3,1> globalPosition(1, 0, 32);
  Eigen::Matrix<float,2,1> globalVariance(0.21, 0.34);
  std::vector<std::size_t> pixel_meas_indexes = {1};

  pixel.setSpacePoint( idHash,
		       globalPosition,
		       globalVariance(0,0),
		       globalVariance(1,0),
		       pixel_meas_indexes );

  std::vector<std::size_t> strip_meas_indexes = {1, 4};
  float topHalfStripLength = 1.20;
  float bottomHalfStripLength = 0.32;
  Eigen::Matrix<float,3,1> topStripDirection(2.3, 55.1, 0.3355);
  Eigen::Matrix<float,3,1> bottomStripDirection(1., 2., 5.);
  Eigen::Matrix<float,3,1> stripCenterDistance(0.012, 3.5, 2.11);
  Eigen::Matrix<float,3,1> topStripCenter(0., 0., 1.02);

  IdentifierHash::value_type idHashVals2(123486);
  IdentifierHash idHashs2(idHashVals2);

  strip.setSpacePoint( {idHash, idHashs2},
		       globalPosition,
                       globalVariance(0,0),
                       globalVariance(1,0),
		       strip_meas_indexes,
		       topHalfStripLength,
		       bottomHalfStripLength,
		       topStripDirection,
		       bottomStripDirection,
		       stripCenterDistance,
		       topStripCenter );
}

void print(xAOD::SpacePoint& pixel, xAOD::SpacePoint& strip) {
  std::cout << " --------- PIXEL SPACE POINT  ------------ " << std::endl;
  std::cout << "Identifier Hash = " << pixel.elementIdList()[0] << std::endl;
  std::cout << "Global Position = " << pixel.globalPosition() << std::endl;
  std::cout << "Global x = " << pixel.x() << std::endl; 
  std::cout << "Global y = " << pixel.y() << std::endl; 
  std::cout << "Global z = " << pixel.z() << std::endl; 
  std::cout << "Global Radius = " << pixel.radius() << std::endl;
  std::cout << "varianceR = " << pixel.varianceR() << std::endl;
  std::cout << "varianceZ = " << pixel.varianceZ() << std::endl;
  std::cout << "measurementIndexes = " << pixel.measurementIndexes() << std::endl;

  std::cout << " --------- STRIP SPACE POINT  ------------ " << std::endl;
  std::cout << "Identifier Hash = " << strip.elementIdList()[0] << " " << strip.elementIdList()[1] << std::endl;
  std::cout << "Global Position = " << strip.globalPosition() << std::endl;
  std::cout << "Global x = " << strip.x() << std::endl;
  std::cout << "Global y = " << strip.y() << std::endl;
  std::cout << "Global z = " << strip.z() << std::endl;
  std::cout << "Global Radius = " << strip.radius() << std::endl;
  std::cout << "varianceR = " << strip.varianceR() << std::endl;
  std::cout << "varianceZ = " << strip.varianceZ() << std::endl;
  std::cout << "measurementIndexes = " << strip.measurementIndexes() <<std::endl;
  std::cout << "topHalfStripLength = " << strip.topHalfStripLength() << std::endl;
  std::cout << "bottomHalfStripLength = " << strip.bottomHalfStripLength() << std::endl;
  std::cout << "topStripDirection = " << strip.topStripDirection() << std::endl;
  std::cout << "bottomStripDirection = " << strip.bottomStripDirection() << std::endl;
  std::cout << "stripCenterDistance = " << strip.stripCenterDistance() << std::endl;
  std::cout << "topStripCenter = " << strip.topStripCenter() << std::endl;
}

int main() {
  // Create the main containers to test:
  // Pixel Space Point
  xAOD::SpacePointContainer pixel_tpc;
  xAOD::SpacePointAuxContainer pixel_aux;
  pixel_tpc.setStore(&pixel_aux);

  // Strip Space Point
  xAOD::SpacePointContainer strip_tpc;
  xAOD::SpacePointAuxContainer strip_aux;
  strip_tpc.setStore(&strip_aux);

  // Add one Space Point to the containers
  // Pixel
  xAOD::SpacePoint *pixel = new xAOD::SpacePoint();
  pixel_tpc.push_back(pixel);

  // Strip
  xAOD::SpacePoint *strip = new xAOD::SpacePoint();
  strip_tpc.push_back(strip);

  // Fill information
  fill(*pixel, *strip);

  // Print information
  print(*pixel, *strip);
}
