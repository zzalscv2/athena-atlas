/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CalibT0DbOperations_h
#define CalibT0DbOperations_h

#include <AthenaBaseComps/AthMessaging.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <GaudiKernel/ServiceHandle.h>
#include <string>
#include <vector>

namespace coral {
    class IQuery;
    class AttributeList;
    class ITableDataEditor;
}  // namespace coral

namespace MuonCalib {

    class CalibDbConnection;
    class MdtStationT0Container;
    class NtupleStationId;
    class MdtTubeFitContainer;
    class MuonFixedId;
    class IConditionsStorage;

    class CalibT0DbOperations: public AthMessaging {
    public:
        //=====================constructor - destructor=================================
        CalibT0DbOperations(CalibDbConnection &db_conn);
        virtual ~CalibT0DbOperations() = default;
        //=====================publlic member functions=================================
        // load t0 for chamber
        MdtStationT0Container *LoadT0Calibration(const NtupleStationId &id, int head_id, std::string &site_name);
        // load data for validation
        MdtTubeFitContainer *LoadT0Validation(const NtupleStationId &id, int head_id, std::string &site_name);
        // write t0 for chamber/set validation flag
        // validation_flag[tb_nr]!=3 write complete tube entry
        // if validation_flag.size()==0: all tubes are new
        bool WriteT0Chamber(const NtupleStationId &id, const MdtTubeFitContainer *t0, std::vector<int> &validation_flag, int head_id,
                            const std::string &site_name);
        // read for storage in a conditinos database
        bool ReadForConditions(const std::string& site_name, int head_id, IConditionsStorage &storage);

    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", "CalibT0DbOperations"};
        //=====================private data members====================================
        // db connection
        CalibDbConnection &m_db_conn;
        coral::IQuery *m_query{nullptr};
        //=====================private functions=======================================
        inline void initRowBuffer(std::vector<coral::AttributeList> &rowBuffer, const NtupleStationId &id, const int head_id,
                                  const std::string &site_name, const MdtTubeFitContainer *t0);
        inline void fillRowBuffer(std::vector<coral::AttributeList> &rowBuffer, const MdtTubeFitContainer *t0, const int ml1,
                                  const int ly, const int ml2, const MuonFixedId &fixId);
        bool setValidFlag(const std::string &site_name, const int head_id, const int tube_id, const int new_validflag,
                          coral::ITableDataEditor &editor);
        // insert all tubes of chamber
        bool insertTubes(const std::string &site_name, int head_id, const NtupleStationId &id, const MdtTubeFitContainer *t0,
                         const std::vector<int> &validation_flag, coral::ITableDataEditor *editor[]);
        bool setValidationFlag(const std::string &site_name, int head_id, const NtupleStationId &id, int from, int to,
                               coral::ITableDataEditor &editor);
        // check if data already existsd in db
        inline bool checkTubesPresent(const int head_id, const std::string &site_name, const NtupleStationId &id,
                                      const std::vector<int> &validflag);
    };  // class

}  // namespace MuonCalib

#endif
