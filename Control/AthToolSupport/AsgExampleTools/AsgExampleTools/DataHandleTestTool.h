/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef ASG_TOOLS__DATA_HANDLES_TEST_TOOL_H
#define ASG_TOOLS__DATA_HANDLES_TEST_TOOL_H

#include <AsgTools/AsgTool.h>
#include <AsgExampleTools/IDataHandleTestTool.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandleKeyArray.h>
#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/WriteHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

// AthSimulation doesn't contain the muon-container, so we can't
// really build the tool, but it is simpler to build an empty tool
// than to exclude the tool completely from the AthSimulation build.
#ifndef SIMULATIONBASE
#include <xAODMuon/MuonContainer.h>
#endif

namespace asg
{
  /// \brief a tool used to unit test AnaToolHandle
  ///
  /// This allows to unit test the various capabilities of
  /// stand-alone data handles in a controlled fashion.

  struct DataHandleTestTool : virtual public IDataHandleTestTool,
			 public AsgTool
  {
    ASG_TOOL_CLASS (DataHandleTestTool, IDataHandleTestTool)

    /// \brief standard constructor
  public:
    DataHandleTestTool (const std::string& val_name);

    /// \brief standard destructor
  public:
    ~DataHandleTestTool ();

  public:
    StatusCode initialize () override;

  public:
    void runTest ATLAS_NOT_THREAD_SAFE () override;

  public:
#ifndef SIMULATIONBASE
    SG::ReadHandleKey<xAOD::MuonContainer> m_readKey {this, "readKey", "Muons", "regular read key"};
    SG::ReadHandleKey<xAOD::MuonContainer> m_readKeyEmpty {this, "readKeyEmpty", "", "regular read key (empty by default)"};
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_readDecorKey {this, "readDecorKey", "Muons.pt", "read decor key"};
    SG::ReadDecorHandleKey<xAOD::MuonContainer> m_readDecorKeyEmpty {this, "readDecorKeyEmpty", "", "read decor key (empty by default)"};
    SG::ReadHandleKeyArray<xAOD::MuonContainer> m_readKeyArray {this, "readKeyArray", {}, "array read key"};
    SG::WriteHandleKey<xAOD::MuonContainer> m_writeKey {this, "writeKey", "", "regular write key"};
    SG::WriteDecorHandleKey<xAOD::MuonContainer> m_writeDecorKey {this, "writeDecorKey", "", "write decor key"};
    SG::WriteDecorHandleKey<xAOD::MuonContainer> m_writeDecorKeyExisting {this, "writeDecorKeyExisting", "", "write decor key (existing)"};
#endif
    bool m_readFailure {false};
    bool m_readArray {false};
    bool m_readDecorFailure {false};
    std::string m_doWriteName;
    std::string m_doWriteDecorName;
    std::string m_doWriteDecorNameExisting;
  };
}

#endif
