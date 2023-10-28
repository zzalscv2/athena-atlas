/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef ACTSGEOMETRY_SIMPLECYLINDERDETBUILDERTOOL_H
#define ACTSGEOMETRY_SIMPLECYLINDERDETBUILDERTOOL_H

#include <ActsGeometryInterfaces/IDetectorVolumeBuilderTool.h> 
#include <AthenaBaseComps/AthAlgTool.h>

namespace ActsTrk {
  class SimpleCylinderDetBuilderTool: public AthAlgTool, virtual public IDetectorVolumeBuilderTool {
    
    public:
        /** @brief Standard tool constructor **/
        SimpleCylinderDetBuilderTool( const std::string& type, const std::string& name, const IInterface* parent );

        virtual ~SimpleCylinderDetBuilderTool() = default;

 
        Acts::Experimental::DetectorComponent construct(const Acts::GeometryContext& context) const override final;        
    private:
       Gaudi::Property<double> m_radiusMin{this, "MinRadius", 0.};
       Gaudi::Property<double> m_radiusMed{this, "MediumRadius", 2000};
       Gaudi::Property<double> m_radiusMax{this, "MaxRadius", 4000};
       Gaudi::Property<double> m_outerZ{this, "OuterZ", 3200.};
       Gaudi::Property<double> m_innerZ{this, "InnerZ", 1000.};

  };
}
#endif