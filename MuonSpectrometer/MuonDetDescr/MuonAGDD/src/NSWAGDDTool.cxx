/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "NSWAGDDTool.h"

#include "MuonAGDDToolHelper.h"
#include "AGDDControl/AGDDController.h"
#include "AGDDControl/AGDD2GeoModelBuilder.h"
#include "AGDDControl/IAGDD2GeoSvc.h"
#include "AGDDModel/AGDDParameterStore.h"
#include "AGDDKernel/AGDDDetector.h"
#include "AGDDKernel/AGDDDetectorStore.h"

#include <fstream>

using namespace MuonGM;

NSWAGDDTool::NSWAGDDTool(const std::string& type, const std::string& name, const IInterface* parent) :
    AGDDToolBase(type,name,parent),
    m_outFileInName(""),
    m_outPREsqlName("") {
}

StatusCode NSWAGDDTool::initialize ATLAS_NOT_THREAD_SAFE ()
{
	ATH_CHECK(AGDDToolBase::initialize());
	ATH_MSG_INFO("NSWAGDDTool::initialize");

	if( m_xmlFiles.size() == 1 && m_writeDBfile )
	{
		std::size_t found = m_xmlFiles[0].find_last_of('/');
		m_outFileInName = m_xmlFiles[0].substr(found+1);
	}
	else if ( m_writeDBfile ) ATH_MSG_ERROR("writing data base files currently only supported if just a single input XML is given!");

	m_outFileName = "Out.AmdcOracle.AM." + m_outFileType + "temp.data";
	m_outPREsqlName = "Out.AmdcOracle.AM." + m_outFileType + ".PREsql";

	if (m_DBFileName.empty()) {
		m_DBFileName = "Generated_" + m_outFileType + "_pool.txt";
	}

        static std::once_flag init;
        std::call_once (init, [&]()
          {
            MuonAGDDToolHelper theHelper;
            theHelper.setAGDDtoGeoSvcName(m_agdd2GeoSvcName);
            theHelper.SetNSWComponents();
          });

        ATH_CHECK(construct());
	return StatusCode::SUCCESS;
}

// Base class method is also marked not thread-safe.
// Uses unsafe function UseGeoModelDetector
StatusCode NSWAGDDTool::construct ATLAS_NOT_THREAD_SAFE () 
{
	ATH_MSG_INFO(name()<<"::construct()");
	
        IAGDDtoGeoSvc::LockedController controller = m_svc->getController();
	MuonAGDDToolHelper theHelper;
	theHelper.setAGDDtoGeoSvcName(m_agdd2GeoSvcName);
	if (!m_readAGDD)
	{
		ATH_MSG_INFO(" trying to parse files ");
		controller->ParseFiles();
	}
	else
	{
		ATH_MSG_INFO(" trying to parse data base content ");
		std::string AGDDfile = theHelper.GetAGDD(m_dumpAGDD, m_outFileType, m_DBFileName);
		controller->ParseString(AGDDfile);
	}
	
	if (m_printSections) 
	{
		ATH_MSG_INFO("\t Printing all sections ");
		controller->PrintSections();
	}
	
	controller->UseGeoModelDetector("Muon");
	controller->BuildAll();
	
	// part needed to build the NSW RO geometry
	
	ATH_MSG_INFO("\t Building NSW Readout Geometry ");
	bool testRet=MuonAGDDToolHelper::BuildMScomponents();
	if (!testRet)
	{
		ATH_MSG_ERROR("something went wrong building the RO geometry!!! ");
		return StatusCode::FAILURE;
	}

	if(m_writeDBfile)
	{
		// build model before writing blob - if Athena crashes the XML is not good and should not become a blob
		ATH_MSG_INFO("\t-- attempting to write output to "<< m_outFileName );
		if( !m_outFileName.empty() )
		{
			if(!controller->WriteAGDDtoDBFile( m_outFileName ))
			{
				ATH_MSG_ERROR("\t-- something went wrong during writing AGDD file - crashing" );
				return StatusCode::FAILURE;
			}
			else {
				ATH_MSG_INFO("\t-- AGDD successfully dumped to "<< m_outFileName);
			}
			if( !WritePREsqlFile() )
			{
				ATH_MSG_ERROR("\t-- something went wrong during writing PREsql file - crashing" );
				return StatusCode::FAILURE;
			}
			else {
				ATH_MSG_INFO("\t-- AGDD successfully wrote PREsql file "<< m_outPREsqlName);
			}
		}
		else {
			ATH_MSG_ERROR("\t-- no output file name provided - crashing " );
			return StatusCode::FAILURE;
		}
	}

	controller->Clean();
	
	return StatusCode::SUCCESS;
}

bool NSWAGDDTool::WritePREsqlFile() const
{

	std::ifstream outfile(m_outFileName.value().c_str(), std::ifstream::in | std::ifstream::binary);

	std::vector<std::string> newoutfilelines;
	std::string outfileline;
	while( getline(outfile, outfileline) )
		if( outfileline != "\n" && outfileline != "\r" && !outfileline.empty() )
		{
			const auto strBegin = outfileline.find_first_not_of(" \t");
			const auto strEnd = outfileline.find_last_not_of(" \t");
			const auto strRange = strEnd - strBegin + 1;
			if (strBegin != std::string::npos) outfileline = outfileline.substr(strBegin, strRange);
			newoutfilelines.push_back(outfileline);
		}
	outfile.close();

	std::ofstream newoutfile(m_outFileName.value().c_str(), std::ofstream::out | std::ofstream::trunc);
	for(auto it = newoutfilelines.begin(); it != newoutfilelines.end(); ++it)
	{
		if(it != newoutfilelines.begin()) newoutfile << "\n";
		newoutfile << *it;
	}
	newoutfile.close();
	outfile.open(m_outFileName.value().c_str(), std::ifstream::in | std::ifstream::binary);

	int fileSize = 0;
	if(outfile.is_open())
	{
		outfile.seekg(0, std::ios::end);
		fileSize = outfile.tellg();
		outfile.close();
	}
	else {
		ATH_MSG_ERROR("\t-- cannot get size of file " << m_outFileName );
		return false;
	}

	std::ofstream prefile;
	prefile.open (m_outPREsqlName.c_str());
	prefile << "insert into NSWD_data (\n";
	prefile << "NSWD_data_id,\n";
	prefile << "ACTVERS,\n";
	prefile << "ACTVNAME,\n";
	prefile << "ALGVERS,\n";
	prefile << "ALGVNAME,\n";
	prefile << "PASVERS,\n";
	prefile << "PASVNAME,\n";
	prefile << "FORMAT,\n";
	prefile << "FNAME,\n";
	prefile << "LENNSW,\n";
	prefile << "NLINE,\n";
	prefile << "data\n";
	prefile << ") values (XXX_DATA_ID_KOUNTER,\n";
	prefile << m_outFileActV << ",'" << m_outFileActN << "',";
	prefile << m_outFileAlgV << ",'" << m_outFileAlgN << "',";
	prefile << m_outFilePasV << ",'" << m_outFilePasN << "',\n";
	prefile << "'" << m_outFileForm <<"','" << m_outFileInName << "',"<< fileSize-1 << ","<< int( (fileSize + 2) / 4 )<<",\n";
	prefile << "empty_clob()\n";
	prefile << ");\n";
	prefile << "insert into NSWD_data2tag values (XXX_DATA2TAG_KOUNTER,XXX_DATA_ID_KOUNTER);\n";
	prefile << "DECLARE\n";
	prefile << "  lobloc  CLOB;\n";
	prefile << "  req     utl_http.req;\n";
	prefile << "  resp    utl_http.resp;\n";
	prefile << "  text    VARCHAR2(32767);\n";
	prefile << "  amount  INTEGER(10) := 0;\n";
	prefile << "  offset  INTEGER(10) := 0;\n";
	prefile << "  TRUE    BOOLEAN;\n";
	prefile << "BEGIN\n";
	prefile << "  SELECT data INTO lobloc\n";
	prefile << "  FROM   NSWD_data\n";
	prefile << "  WHERE  NSWD_data_id =  XXX_DATA_ID_KOUNTER FOR UPDATE;\n";
	prefile << "  offset := DBMS_LOB.GETLENGTH(lobloc)+2;\n";
	prefile << "     req := utl_http.begin_request(\n";
	prefile << "     'WEB_ADDRESS_FOR_TEMP_DATA_FILENSWDtemp.data');\n";
	prefile << "  resp := utl_http.get_response(req);\n";
	prefile << "  LOOP\n";
	prefile << "     text := ' ';\n";
	prefile << "     UTL_HTTP.READ_TEXT(resp, text, NULL);\n";
	prefile << "     /* DBMS_OUTPUT.PUT_LINE(text); */\n";
	prefile << "     amount := length(text);\n";
	prefile << "     DBMS_LOB.WRITEAPPEND(lobloc,amount,text);\n";
	prefile << "  END LOOP;\n";
	prefile << "    utl_http.end_response(resp);\n";
	prefile << "  EXCEPTION\n";
	prefile << "    WHEN utl_http.end_of_body\n";
	prefile << "    THEN utl_http.end_response(resp);\n";
	prefile << "END;\n";
	prefile << "/\n";
	prefile.close();

	return true;

}



