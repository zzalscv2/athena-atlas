/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

//          Copyright Nils Krumnack 2011.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// Please feel free to contact me (krumnack@iastate.edu) for bug
// reports, feature suggestions, praise and complaints.


//
// includes
//

#include <MultiDraw/Global.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <TFile.h>
#include <TH1.h>
#include <TString.h>
#include <TSystem.h>
#include <EventLoop/DirectDriver.h>
#include <EventLoop/Job.h>
#include <MultiDraw/AlgCFlow.h>
#include <MultiDraw/FormulaSvc.h>
#include <RootCoreUtils/ThrowMsg.h>
#include <RootCoreUtils/UnitTestDir.h>
#include <SampleHandler/SampleLocal.h>
#include <SampleHandler/SamplePtr.h>

//
// main program
//

using namespace MD;

int main ()
{
  RCU::UnitTestDir outputDir ("MultiDraw", "alg_cflow");
  TString output = outputDir.path() + "submit";

  try
  {
    TString input = "$ROOTCOREBIN/data/EventLoop/test_ntuple1.root";
    gSystem->ExpandPathName (input);
    std::string tree ("physics");

    EL::DirectDriver driver;
    std::unique_ptr<SH::SampleLocal> mysample (new SH::SampleLocal ("dataset"));
    mysample->add (input.Data());
    mysample->meta()->setString ("nc_tree", tree);
    SH::SamplePtr sample = mysample.release();

    EL::Job job;
    {
      SH::SampleHandler sh;
      sh.add (sample);
      job.sampleHandler (sh);
    }
    {
      TH1 *hist = new TH1F ("el_n", "el_n", 10, 0, 10);
      hist->GetXaxis()->SetBinLabel (1, "el_n>=0");
      hist->GetXaxis()->SetBinLabel (2, "el_n>=2");
      hist->GetXaxis()->SetBinLabel (3, "el_n>=0");
      job.algsAdd (new AlgCFlow (hist));
    }

    gSystem->Exec (("rm -rf " + output).Data());
    driver.submit (job, output.Data());
    TFile hist_file ((output + "/hist-dataset.root").Data(), "READ");
    TH1 *hist = dynamic_cast<TH1*>(hist_file.Get ("el_n"));
    if (hist == 0)
      RCU_THROW_MSG ("didn't find histogram el_n");
    float content [4] = {0, 10, 9, 9};
    for (int bin = 0, end = 4; bin != end; ++ bin)
    {
      if (hist->GetBinContent (bin) != content[bin])
      {
	std::ostringstream str;
	str << "bin content missmatch in bin " << bin
	    << " found " << hist->GetBinContent (bin)
	    << " expected " << content[bin];
	RCU_THROW_MSG (str.str());
      }
    }
  } catch (std::string& s)
  {
    std::cerr << "caught error: " << s << std::endl;
    return 1;
  } catch (...)
  {
    std::cerr << "caught unknown error" << std::endl;
    return 1;
  }
}
