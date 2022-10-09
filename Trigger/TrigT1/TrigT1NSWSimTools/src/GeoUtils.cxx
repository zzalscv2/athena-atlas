/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/GeoUtils.h"

namespace NSWL1{

float eta(float x,float y,float z){
    ROOT::Math::XYZVector V(x,y,z);
    return V.Eta();
}

float phi(float x, float y, float z){
    ROOT::Math::XYZVector V(x,y,z);
    return V.Phi();
}

Polygon Project(const Polygon& p,float Zinit,float Zfin){
    //loop over vertices
    Vertices vt;
    for(const auto& v : boost::geometry::exterior_ring(p)){
        float x=coordinate<0>(v);
        float y=coordinate<1>(v);
        ROOT::Math::XYZVector vec(x, y, Zinit);
        float phi=vec.Phi();
        float theta=vec.Theta();
        vec.SetZ(Zfin);
        ROOT::Math::Polar3DVector projectedVec(vec.R(), theta, phi);
        vt.push_back(Vertex(projectedVec.X(), projectedVec.Y()));
    }
    return buildPolygon(vt);
}

int nVertices(const Polygon& p){
    return boost::geometry::num_points(p);
}

Vertex getVertex(const Polygon& p,unsigned int i){
    return boost::geometry::exterior_ring(p)[i];
}


Polygon buildPolygon(const Vertices& pts){//hm
    Polygon shape;
    Vertices vts=pts;
    //to complete the loop...
    vts.push_back(vts[0]);
    
    boost::geometry::assign_points(shape, pts);
    boost::geometry::correct(shape);
    return shape;
}

float area(const Polygon& p){
    return boost::geometry::area(p);
}

Vertex centroid(const Polygon& p){
    Vertex v;
    boost::geometry::centroid(p, v);
    return v;
}

 std::vector<Polygon> allIntersections(const Polygon& p1, const Polygon& p2){
    std::vector<Polygon> out;
    boost::geometry::intersection(p1, p2, out);
    return out;
    
}


Polygon firstIntersection(const Polygon& p1,const Polygon& p2){
    std::vector<Polygon> pols=allIntersections(p1,p2);
     //we have to return something
    if(pols.size()==0) return NilPolygon();
    return allIntersections(p1,p2).at(0);
}

Polygon largestIntersection(const Polygon& p1,const Polygon& p2){
    std::vector<Polygon> pols=allIntersections(p1,p2);
     if(pols.size()==0) return NilPolygon();
     
     std::sort(pols.begin(),pols.end(),[](const Polygon& a,const Polygon& b){return area(a)>area(b);});
     return pols[0];
    
}



Polygon intersectionRegion(const std::vector<Polygon>& polygons){
    Polygon ovl=polygons[0];
    for(unsigned int i=1;i<polygons.size();i++){
        Polygon curr=polygons[i];
        Polygon overlap=largestIntersection(curr,ovl);
        ovl=overlap;
    }
    return ovl;
}



Polygon globalToLocal(const Polygon& pol,float z,const Trk::PlaneSurface & surf ){
    
    Vertices local_vertices;
    for(const auto& v : boost::geometry::exterior_ring(pol)){
                float x=coordinate<0>(v);
                float y=coordinate<1>(v);
                Amg::Vector2D local_corner;
                surf.globalToLocal(Amg::Vector3D(x,y,z),Amg::Vector3D(),local_corner);
                local_vertices.emplace_back(Vertex(local_corner.x(),local_corner.y()));
    }
    return buildPolygon(local_vertices);
}



Polygon NilPolygon(){
    Vertices nil={
        Vertex(0,0),
        Vertex(0,0),
        Vertex(0,0),
        Vertex(0,0),
        Vertex(0,0)
    };
    return buildPolygon(nil);
}

}
