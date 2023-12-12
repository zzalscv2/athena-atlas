#include "AlgDataFetcherFactory.h"
#include "AlgDataFetcher.h"

namespace GlobalSim {
  std::pair<std::optional<std::shared_ptr<IAlgDataFetcher>>,
    std::vector<std::string>>
  makeAlgDataFetcher(bool do_eConn,
		     bool do_oConn,
		     const TrigConf::L1Menu* menu){
    auto adf =  std::shared_ptr<IAlgDataFetcher>(nullptr);
    adf.reset(new AlgDataFetcher(do_eConn, do_oConn, menu));

    return adf->isValid() ?
      std::pair(std::make_optional(adf), std::vector<std::string>()) :
      std::pair(std::optional<std::shared_ptr<IAlgDataFetcher>> (),
		adf->errMsgs());
    
  }
}
