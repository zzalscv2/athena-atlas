/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GEO2G4_ASSEMBLYTRIPLET_H
#define GEO2G4_ASSEMBLYTRIPLET_H 

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

class G4LogicalVolume;
class Geo2G4AssemblyVolume;

class Geo2G4AssemblyTriplet
{
 public:  // with description

    Geo2G4AssemblyTriplet();
      // Default constructor

    Geo2G4AssemblyTriplet( G4LogicalVolume* pVolume,
                       G4ThreeVector& translation,
                       G4RotationMatrix* pRotation,
                       G4bool isReflection = false);
      // An explicit constructor for a logical volume
    
    Geo2G4AssemblyTriplet( Geo2G4AssemblyVolume* pAssembly,
                       G4ThreeVector& translation,
                       G4RotationMatrix* pRotation,
                       G4bool isReflection = false);
      // An explicit constructor for an assembly volume
    
    Geo2G4AssemblyTriplet( const Geo2G4AssemblyTriplet& second ) = default;
      // Copy constructor

    ~Geo2G4AssemblyTriplet();    
      // Destructor

    Geo2G4AssemblyTriplet& operator=( const Geo2G4AssemblyTriplet& second ) = default;
      // Assignment operator

    const G4LogicalVolume* GetVolume() const { return m_volume; }
    G4LogicalVolume* GetVolume() { return m_volume; }
      // Retrieve the logical volume reference

    void SetVolume( G4LogicalVolume* pVolume );
      // Update the logical volume reference

    const Geo2G4AssemblyVolume* GetAssembly() const { return m_assembly; }
    Geo2G4AssemblyVolume* GetAssembly() { return m_assembly; }
      // Retrieve the assembly volume reference

    void SetAssembly( Geo2G4AssemblyVolume* pAssembly );
      // Update the assembly volume reference

    const G4ThreeVector GetTranslation() const { return m_translation; }
    G4ThreeVector GetTranslation() { return m_translation; }
      // Retrieve the logical volume translation

    void SetTranslation( G4ThreeVector& pVolume );
      // Update the logical volume translation

    const G4RotationMatrix* GetRotation() const { return m_rotation; }
    G4RotationMatrix* GetRotation() { return m_rotation; }
      // Retrieve the logical volume rotation

    void SetRotation( G4RotationMatrix* pVolume );
      // Update the logical volume rotation

    G4bool IsReflection() const;
      // Return true if the logical or assembly volume has reflection

 private:

    G4LogicalVolume*  m_volume;
      // A logical volume

    G4ThreeVector     m_translation;
      // A logical volume translation

    G4RotationMatrix* m_rotation;
      // A logical volume rotation

 private:

    // Member data for handling assemblies of assemblies and reflections

    Geo2G4AssemblyVolume* m_assembly;
      // An assembly volume

    G4bool            m_isReflection;
      // True if the logical or assembly volume has reflection  
};

inline
Geo2G4AssemblyTriplet::Geo2G4AssemblyTriplet()
  : m_volume( 0 ), m_rotation( 0 ), m_assembly(0), m_isReflection(false)
{
  G4ThreeVector v(0.,0.,0.);
  m_translation = v;
}

inline
Geo2G4AssemblyTriplet::Geo2G4AssemblyTriplet( G4LogicalVolume* pVolume,
                                      G4ThreeVector& translation,
                                      G4RotationMatrix* pRotation,
                                      G4bool isReflection )
  : m_volume( pVolume ), m_translation( translation ), m_rotation( pRotation ),
    m_assembly( 0 ), m_isReflection(isReflection)
{
}

inline
Geo2G4AssemblyTriplet::Geo2G4AssemblyTriplet( Geo2G4AssemblyVolume* pAssembly,
                                      G4ThreeVector& translation,
                                      G4RotationMatrix* pRotation,
                                      G4bool isReflection )
  : m_volume( 0 ), m_translation( translation ), m_rotation( pRotation ),
    m_assembly( pAssembly ), m_isReflection(isReflection) 
{
}

inline
Geo2G4AssemblyTriplet::~Geo2G4AssemblyTriplet()
{
}

inline
void Geo2G4AssemblyTriplet::SetVolume( G4LogicalVolume* pVolume )
{
  if ( m_assembly ) 
  {
    G4Exception("Geo2G4AssemblyTriplet::SetVolume()",
                "IllegalCall", JustWarning,
                "There is an assembly already set, it will be ignored.");
  }
  m_volume = pVolume;
  m_assembly = 0;
}

inline
void Geo2G4AssemblyTriplet::SetAssembly( Geo2G4AssemblyVolume* pAssembly )
{
  if ( m_volume ) 
  {
    G4Exception("Geo2G4AssemblyTriplet::SetAssembly()",
                "IllegalCall", JustWarning,
                "There is a volume already set, it will be ignored.");
  }
  m_assembly = pAssembly;
  m_volume = 0;
}

inline
void Geo2G4AssemblyTriplet::SetTranslation( G4ThreeVector& translation )
{
  m_translation = translation;
}

inline
void Geo2G4AssemblyTriplet::SetRotation( G4RotationMatrix* pRotation )
{
  m_rotation = pRotation;
}

inline
G4bool Geo2G4AssemblyTriplet::IsReflection() const
{
  return m_isReflection;
}  

#endif
