#ifndef SCellEncoder_H
#define SCellEncoder_H
#include "xAODTrigger/eFexTauRoI.h"
#include <vector>

namespace LVL1 {

typedef std::vector<unsigned int> supercells_t;

class SCellEncoder {
public:
  SCellEncoder();
  void setSuperCells(const unsigned int layer0[][3],
                     const unsigned int layer1[][3],
                     const unsigned int layer2[][3],
                     const unsigned int layer3[][3],
                     const unsigned int layer4[][3]);

  void decorateEFexTauRoI(xAOD::eFexTauRoI *myTauEDM);

private:
  supercells_t m_scell_values;
};
} // namespace LVL1
#endif // ScellEncoder_H
