/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// files produced by this will need to be installed in /eos/atlas/atlascerngroupdisk/asg-calib/ZdcAnalysis

// default values below are utilized here only for testing purposes

void ZDC_fake_calib(int run=400000, float s0m0=100, float s0m1=110, float s0m2=120, float s0m3=130, float s1m0=500, float s1m1=510, float s1m2=520, float s1m3=530)
{
  TGraph g_s0m0;
  TGraph g_s0m1;
  TGraph g_s0m2;
  TGraph g_s0m3;
  TGraph g_s1m0;
  TGraph g_s1m1;
  TGraph g_s1m2;
  TGraph g_s1m3;
  for (int i =0;i<5000;i++)
    {
      g_s0m0.SetPoint(i,i,s0m0);
      g_s0m1.SetPoint(i,i,s0m1);
      g_s0m2.SetPoint(i,i,s0m2);
      g_s0m3.SetPoint(i,i,s0m3);
      g_s1m0.SetPoint(i,i,s1m0);
      g_s1m1.SetPoint(i,i,s1m1);
      g_s1m2.SetPoint(i,i,s1m2);
      g_s1m3.SetPoint(i,i,s1m3);
    }

  TString srun = TString::Itoa(run,10);
  g_s0m0.SetName("ZDC_Gcalib_run"+srun+"_s0_m0");
  g_s0m1.SetName("ZDC_Gcalib_run"+srun+"_s0_m1");
  g_s0m2.SetName("ZDC_Gcalib_run"+srun+"_s0_m2");
  g_s0m3.SetName("ZDC_Gcalib_run"+srun+"_s0_m3");
  g_s1m0.SetName("ZDC_Gcalib_run"+srun+"_s1_m0");
  g_s1m1.SetName("ZDC_Gcalib_run"+srun+"_s1_m1");
  g_s1m2.SetName("ZDC_Gcalib_run"+srun+"_s1_m2");
  g_s1m3.SetName("ZDC_Gcalib_run"+srun+"_s1_m3");

  TSpline3 s_s0m0("ZDC_Ecalib_run"+srun+"_s0_m0",&g_s0m0);
  TSpline3 s_s0m1("ZDC_Ecalib_run"+srun+"_s0_m1",&g_s0m1);
  TSpline3 s_s0m2("ZDC_Ecalib_run"+srun+"_s0_m2",&g_s0m2);
  TSpline3 s_s0m3("ZDC_Ecalib_run"+srun+"_s0_m3",&g_s0m3);
  TSpline3 s_s1m0("ZDC_Ecalib_run"+srun+"_s1_m0",&g_s1m0);
  TSpline3 s_s1m1("ZDC_Ecalib_run"+srun+"_s1_m1",&g_s1m1);
  TSpline3 s_s1m2("ZDC_Ecalib_run"+srun+"_s1_m2",&g_s1m2);
  TSpline3 s_s1m3("ZDC_Ecalib_run"+srun+"_s1_m3",&g_s1m3);

  s_s0m0.SetName(s_s0m0.GetTitle());
  s_s0m1.SetName(s_s0m1.GetTitle());
  s_s0m2.SetName(s_s0m2.GetTitle());
  s_s0m3.SetName(s_s0m3.GetTitle());
  s_s1m0.SetName(s_s1m0.GetTitle());
  s_s1m1.SetName(s_s1m1.GetTitle());
  s_s1m2.SetName(s_s1m2.GetTitle());
  s_s1m3.SetName(s_s1m3.GetTitle());
  
  TFile f("ZdcCalib_Run"+srun+".root","RECREATE");
  g_s0m0.Write();s_s0m0.Write();
  g_s0m1.Write();s_s0m1.Write();
  g_s0m2.Write();s_s0m2.Write();
  g_s0m3.Write();s_s0m3.Write();
  g_s1m0.Write();s_s1m0.Write();
  g_s1m1.Write();s_s1m1.Write();
  g_s1m2.Write();s_s1m2.Write();
  g_s1m3.Write();s_s1m3.Write();
  f.Close();
}
