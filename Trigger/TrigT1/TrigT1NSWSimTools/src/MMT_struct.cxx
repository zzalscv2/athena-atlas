/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigT1NSWSimTools/MMT_struct.h"
#include "MuonAGDDDescription/MMDetectorDescription.h"
#include "MuonAGDDDescription/MMDetectorHelper.h"
#include "MuonReadoutGeometry/MuonChannelDesign.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"

#include "AthenaKernel/getMessageSvc.h"

#include <stdexcept>
#include <utility>
#include <cmath>

using std::vector;
using std::string;
using std::map;
using std::pair;
using std::ostringstream;
using std::setprecision;
using std::setw;


const float MMTStructConst = 8192.;

std_align::std_align(int qcm,const ROOT::Math::XYZVector& trans,const ROOT::Math::XYZVector& ang):type(qcm),translate(trans),rotate(ang){
  if(type==0){
    translate=ROOT::Math::XYZVector();
    rotate=ROOT::Math::XYZVector();
  }
}

string std_align::par_title(int par_num,bool small_unit)const{
  string par_title("invalid");
  if(par_num==0) par_title="#Deltas [mm]";
  else if(par_num==1) par_title="#Deltaz [mm]";
  else if(par_num==2) par_title="#Deltat [mm]";
  else if(par_num==3) par_title="#gamma_{s}";
  else if(par_num==4) par_title="#beta_{z}";
  else if(par_num==5) par_title="#alpha_{t}";
  else{
    throw std::runtime_error("MMT_struct: Invalid parameter index");
  }
  if(par_num>2){
    if(small_unit) par_title+=" [mrad]";
    else par_title+=" [rad]";
  }
  return par_title;
}

string std_align::par_name(int par_num)const{
  string par_name("invalid");
  if(par_num==0) par_name="dts";
  else if(par_num==1) par_name="dtz";
  else if(par_num==2) par_name="dtt";
  else if(par_num==3) par_name="drs";
  else if(par_num==4) par_name="drz";
  else if(par_num==5) par_name="drt";
  else{
    throw std::runtime_error("MMT_struct: Invalid parameter index");
  }
  return par_name;
}

string std_align::par_title_val(int par_num)const{
  assert(par_num>=0 && par_num<parmax());
  ostringstream par_title;
  bool do_it=false;
  for(int i=0;i<6;i++){
    if(par_val(i)!=0) do_it=true;
  }
  if(!do_it) return "nominal";
  if(par_num==0) par_title << "#Deltas=" << par_val(0) << " mm";
  else if(par_num==1) par_title << "#Deltaz=" << par_val(1)<< " mm";
  else if(par_num==2) par_title << "#Deltat=" << par_val(2)<< " mm";
  else if(par_num==3) par_title << "#gamma_{s}=" << par_val(3) << " rad";
  else if(par_num==4) par_title << "#beta_{z}=" << par_val(4) << " rad";
  else if(par_num==5) par_title << "#alpha_{t}=" << par_val(5) << " rad";
  return par_title.str();
}

string std_align::par_name_val(int par_num)const{
  assert(par_num>=0 && par_num<parmax());
  ostringstream par_name;
  if(par_num==0) par_name << "dts";
  else if(par_num==1) par_name << "dtz";
  else if(par_num==2) par_name << "dtt";
  else if(par_num==3) par_name << "drs";
  else if(par_num==4) par_name << "drz";
  else if(par_num==5) par_name << "drt";
  par_name<<par_val(par_num);
  return par_name.str();
}

string std_align::print()const{
  if(type==0) return string("_NOM");//nominal
  string gordon;
  int maxpar=0;
  if(type==1){
    gordon+="_MIS";
    maxpar=parmax();
  }
  else if(type==2){
    gordon+="_COR";
    maxpar=parmax();
  }
  else if(type==3){
    gordon+="_3CR";
    maxpar=parmax();
  }
  for(int imal=0; imal<maxpar;imal++){
    ostringstream addme;
    if(par_val(imal)!=0){
      addme << "_" << par_name(imal) << par_val(imal);
    }
    gordon+=addme.str();
  }
  return gordon;
}

double std_align::par_val(int par_num) const{
  assert(par_num>=0 && par_num<parmax());
  double par_val=-999.;
  if(par_num==0) par_val=translate.X();
  else if(par_num==1) par_val=translate.Y();
  else if(par_num==2) par_val=translate.Z();
  else if(par_num==3) par_val=rotate.X();
  else if(par_num==4) par_val=rotate.Y();
  else if(par_num==5) par_val=rotate.Z();
  return par_val;
}

void std_align::set_val(int par_num,double par_val){
  assert(par_num>=0 && par_num<parmax());
  if(std::log10(std::abs(par_val))<-7) par_val=0;
  if(par_num==0) translate.SetX(par_val);
  else if(par_num==1) translate.SetY(par_val);
  else if(par_num==2) translate.SetZ(par_val);
  else if(par_num==3) rotate.SetX(par_val);
  else if(par_num==4) rotate.SetY(par_val);
  else if(par_num==5) rotate.SetZ(par_val);
}

string std_align::detail()const{
  string logan;
  if(type==0)return string("nominal");
  else if(type==1) logan+="MIS";
  else if(type==2) logan+="COR";
  for(int imal=0; imal<parmax();imal++){
    ostringstream addme;
    if(par_val(imal)!=0) addme << ", " << par_title(imal) << "=" << par_val(imal);
    logan+=addme.str();
  }
  if(logan.compare("")==0) logan+=" nominal";
  return logan;
}

bool std_align::is_nominal()const{
  for(int i=0;i<parmax();i++){
    if(par_val(i)!=0) return false;//sometimes doubles as zero get set to really small but not zero values...
  }
  return true;
}

gcm_key::gcm_key(int the_pt,int the_ct,int the_mis,int the_corr,int the_eta,int the_qt,int the_bg)
 :pt(the_pt),ct(the_ct),mis(the_mis),correct(the_corr),eta(the_eta),qt(the_qt),bgcode(the_bg){}

bool gcm_key::operator==(const gcm_key& rhs) const{
  if(this->pt==rhs.pt && this->ct==rhs.ct && this->mis==rhs.mis && this->correct==rhs.correct && this->eta==rhs.eta && this->qt==rhs.qt && this->bgcode==rhs.bgcode) return true;
  return false;
}

bool gcm_key::operator!=(const gcm_key& rhs) const{
  return !(*this==rhs);
}
bool gcm_key::operator<(const gcm_key& rhs) const{
  if(this->pt<rhs.pt) return true;
  else if(this->pt==rhs.pt && this->ct<rhs.ct) return true;
  else if(this->pt==rhs.pt && this->ct==rhs.ct && this->mis<rhs.mis) return true;
  else if(this->pt==rhs.pt && this->ct==rhs.ct && this->mis==rhs.mis && this->correct<rhs.correct) return true;
  else if(this->pt==rhs.pt && this->ct==rhs.ct && this->mis==rhs.mis && this->correct==rhs.correct && this->eta<rhs.eta) return true;
  else if(this->pt==rhs.pt && this->ct==rhs.ct && this->mis==rhs.mis && this->correct==rhs.correct && this->eta==rhs.eta && this->qt<rhs.qt) return true;
  else if(this->pt==rhs.pt && this->ct==rhs.ct && this->mis==rhs.mis && this->correct==rhs.correct && this->eta==rhs.eta && this->qt==rhs.qt && this->bgcode<rhs.bgcode) return true;
  return false;
}
bool gcm_key::operator<=(const gcm_key& rhs) const{
  return(*this<rhs || *this==rhs);
}
bool gcm_key::operator>(const gcm_key& rhs) const{
  return !(*this>=rhs);
}
bool gcm_key::operator>=(const gcm_key& rhs) const{
  return(*this>rhs || *this==rhs);
}

string gcm_key::print()const{
  ostringstream ricola;
  ricola << "(p" << pt << ",C" << ct << ",m" << mis << ",c" << correct << ",e" << eta << ",q" << qt << ",b" << bgcode << ")";
  return ricola.str();
}

void gcm_key::set_var(int var,int val){
  assert(var>=0 && var<varmax());
  if(var==0) pt=val;
  else if(var==1) ct=val;
  else if(var==2) mis=val;
  else if(var==3) correct=val;
  else if(var==4) eta=val;
  else if(var==5) qt=val;
  else if(var==6) bgcode=val;
}

int gcm_key::get_var(int var)const{
  assert(var>=0 && var<varmax());
  if(var==0) return pt;
  else if(var==1) return ct;
  else if(var==2) return mis;
  else if(var==3) return correct;
  else if(var==4) return eta;
  else if(var==5) return qt;
  else if(var==6) return bgcode;
  return -999;
}

par_par::par_par(double the_h,int xct,int uvct,double uver,const string& set,bool ql,bool dlm,bool qbg,double the_qt,std_align mis,std_align cor,bool fill_tab,int cs,const string&pd,const string&tg):
  h(the_h),ctx(xct),ctuv(uvct),uverr(uver),setup(set),islarge(ql),q_dlm(dlm),genbg(qbg),qt(the_qt),misal(std::move(mis)),corr(std::move(cor)),fill_val(fill_tab),colskip(cs),pcrep_dir(pd),tag(tg) {}

string par_par::print_pars(const vector<int>&hide) const{
  vector<bool>sho(gcm_key().varmax(),true);
  for(int i=0;i<(int)hide.size();i++){
    if(hide[i]<0 || hide[i]>=gcm_key().varmax()) continue;
    sho[hide[i]]=false;
    sho[2]=false;
  }
  ostringstream mars;
  if(sho[6]) mars << "_h" << h;
  if(sho[1]) mars << "_ctx" << ctx << "_ctuv" << ctuv;
  if(sho[6]) mars << "_uverr" << uverr;
  mars << "_set" << setup << "_ql" << islarge << "_qdlm" << q_dlm;
  if(sho[6]) mars << "_qbg" << genbg;
  if(sho[5]) mars << "_qt" << qt;
  if(sho[2]) mars << misal.print() << corr.print();
  return mars.str();
}

string par_par::detail()const{
  return misal.detail()+"; "+corr.detail();
}

MMT_Parameters::MMT_Parameters(par_par inputParams, char wedgeSize, const MuonGM::MuonDetectorManager* detManager)
  : AthMessaging(Athena::getMessageSvc(), "MMT_Parameters") {
  if(inputParams.misal.is_nominal()) inputParams.misal.type=0;
  //can still do sim_corrections for nominal
  if(inputParams.corr.is_nominal() && inputParams.corr.type==2) inputParams.corr.type=0;
  n_etabins=2;
  n_etabins=10;
  n_phibins=10;
  dtheta_cut=0.016;//16 mrad dtheta cut (VMM limitation...may revise)
  diag=true;
  fill0=false;
  //stuff pulled from the inputParams
  h=inputParams.h;
  CT_x=inputParams.ctx;
  CT_uv=inputParams.ctuv;
  uv_error=inputParams.uverr;
  setup=(inputParams.setup);
  islarge=inputParams.islarge;
  dlm_new=inputParams.q_dlm;
  chargeThreshold=inputParams.qt;
  genbg=inputParams.genbg;
  misal=inputParams.misal;
  misalign=(misal.type==1);
  correct=inputParams.corr;
  ybins=8;
  n_stations_eta=(dlm_new ? 2 : 4);
  val_tbl=inputParams.fill_val;
  // Get the modules for each multiplet, the sector and side arguement (third and fifth argument, respectively) shouldn't matter
  // since the algorithm computes locally
  char side   = 'A';
  sector = wedgeSize;
  MMDetectorHelper aHelper;
  MMDetectorDescription* mm_top_mult1    = aHelper.Get_MMDetector(sector, 2, 5, 1, side); // Parameters: sector, eta, phi, layer/multiplet
  MMDetectorDescription* mm_top_mult2    = aHelper.Get_MMDetector(sector, 2, 5, 2, side);
  MMDetectorDescription* mm_bottom_mult1 = aHelper.Get_MMDetector(sector, 1, 5, 1, side);
  MMDetectorDescription* mm_bottom_mult2 = aHelper.Get_MMDetector(sector, 1, 5, 2, side);

  MMReadoutParameters roParam_top_mult1    = mm_top_mult1->GetReadoutParameters();
  MMReadoutParameters roParam_top_mult2    = mm_top_mult2->GetReadoutParameters();
  MMReadoutParameters roParam_bottom_mult1 = mm_bottom_mult1->GetReadoutParameters();
  MMReadoutParameters roParam_bottom_mult2 = mm_bottom_mult2->GetReadoutParameters();

  //Needed to get the max channels for the deadZone in the x planes... a little annoying...
  //Only changes for small or large and per module. Phi, layer, etc don't matter (just set to 1)
  std::string wedgeString = (wedgeSize=='L' ? "MML" : "MMS");

  if(!islarge){
    ATH_MSG_WARNING("We haven't configured the small wedge parameters yet!  Go bother the developer...kindly...this will likely crash now!\n");
    //return; TODO: is this still the case?
  }
  //y = vertical distance from beamline
  //z = into wedge along beamline
  //x = horizontal distance from beam looking down

  m_pitch = roParam_bottom_mult1.stripPitch;
  m_lWidth = mm_top_mult1->lWidth();
  m_innerRadiusEta1 = roParam_bottom_mult1.distanceFromZAxis;
  m_innerRadiusEta2 = roParam_top_mult1.distanceFromZAxis;
  m_missedBottomEta = roParam_bottom_mult1.nMissedBottomEta;
  m_missedBottomStereo = roParam_bottom_mult1.nMissedBottomStereo;
  if (sector == 'L') {
    ATH_MSG_DEBUG("LM1 \n" <<
                 "\t\t\t\t Total Strips: " << roParam_bottom_mult1.tStrips << " with pitch: " << roParam_bottom_mult1.stripPitch << "\n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_bottom_mult1.nMissedTopEta << " - BottomEta: " << roParam_bottom_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_bottom_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_bottom_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_bottom_mult1->lWidth() << ", " << mm_bottom_mult1->sWidth() << ", " << mm_bottom_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_bottom_mult1.zpos << " - " << roParam_bottom_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_bottom_mult1.dlStereoTop << " " << roParam_bottom_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Bottom, Top, Height) : (" << roParam_bottom_mult1.activeBottomLength << ", " << roParam_bottom_mult1.activeTopLength << ", " << roParam_bottom_mult1.activeH << ")");
  } else if (sector == 'S') {
    ATH_MSG_DEBUG("SM1 \n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_bottom_mult1.nMissedTopEta << " - BottomEta: " << roParam_bottom_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_bottom_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_bottom_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t Total Strips: " << roParam_bottom_mult1.tStrips << " with pitch: " << roParam_bottom_mult1.stripPitch << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_bottom_mult1->lWidth() << ", " << mm_bottom_mult1->sWidth() << ", " << mm_bottom_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_bottom_mult1.zpos << " - " << roParam_bottom_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_bottom_mult1.dlStereoTop << " " << roParam_bottom_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Bottom, Top, Height) : (" << roParam_bottom_mult1.activeBottomLength << ", " << roParam_bottom_mult1.activeTopLength << ", " << roParam_bottom_mult1.activeH << ")");
  }

  if (sector == 'L') {
    ATH_MSG_DEBUG("LM2 \n" <<
                 "\t\t\t\t Total Strips: " << roParam_top_mult1.tStrips << " with pitch: " << roParam_top_mult1.stripPitch << "\n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_top_mult1.nMissedTopEta << " - BottomEta: " << roParam_top_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_top_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_top_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_top_mult1->lWidth() << ", " << mm_top_mult1->sWidth() << ", " << mm_top_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_top_mult1.zpos << " - " << roParam_top_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_top_mult1.dlStereoTop << " " << roParam_top_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Top, Bottom, Height) : (" << roParam_top_mult1.activeTopLength << ", " << roParam_top_mult1.activeBottomLength << ", " << roParam_top_mult1.activeH << ")");
  } else if (sector == 'S') {
    ATH_MSG_DEBUG("SM2 \n" <<
                 "\t\t\t\t KO strips TopEta: " << roParam_top_mult1.nMissedTopEta << " - BottomEta: " << roParam_top_mult1.nMissedBottomEta << "\n" <<
                 "\t\t\t\t KO strips TopStereo: " << roParam_top_mult1.nMissedTopStereo << " - BottomStereo: " << roParam_top_mult1.nMissedBottomStereo << "\n" <<
                 "\t\t\t\t (Top, Bottom, Length): (" << mm_top_mult1->lWidth() << ", " << mm_top_mult1->sWidth() << ", " << mm_top_mult1->Length() << ")\n" <<
                 "\t\t\t\t Zpos - Distance from ZAxis: " << roParam_top_mult1.zpos << " - " << roParam_top_mult1.distanceFromZAxis << "\n" <<
                 "\t\t\t\t dlStereoTop/Bottom: " << roParam_top_mult1.dlStereoTop << " " << roParam_top_mult1.dlStereoBottom << "\n" <<
                 "\t\t\t\t Active area --> (Top, Bottom, Height) : (" << roParam_top_mult1.activeTopLength << ", " << roParam_top_mult1.activeBottomLength << ", " << roParam_top_mult1.activeH << ")");
    for (const auto &angle : roParam_top_mult1.stereoAngle) ATH_MSG_DEBUG("Stereo angle: " << angle);
  }

  ////////////  Define the large wedge /////////////////
  w1=double(mm_top_mult1->lWidth());   //top
  w2=double(mm_top_mult1->lWidth()*1./(std::cos(roParam_top_mult1.stereoAngle[3])));  //determined by 33deg angle   //shelves part
  w3=double(mm_bottom_mult1->sWidth());//582.3);  //bottom
  h1=double(roParam_top_mult1.roLength+roParam_bottom_mult1.roLength+5.0); //how tall wedge is at w1, ie total height

  wedge_opening_angle = 33.;  //degree

  z_large=vector<vector<double > >(ybins,vector<double >(setup.length(),0.));

  // retrieve the z-position of the planes
  z_nominal.clear();
  int eta = 1;
  for (const auto& pos: MM_firststrip_positions(detManager, wedgeString, eta)){
    z_nominal.push_back(double(pos.z()));
  }

  if(z_nominal.size() != setup.size()){
    ATH_MSG_WARNING( "Number of planes in setup is "<< setup.size()
              << ", but we have a nominal "     << z_nominal.size() << " planes.");
    throw std::runtime_error("MMT_Parameters: Invalid number of planes");
  }

  mid_plane_large=0.;
  for(unsigned int iz=0; iz<z_nominal.size(); iz++) mid_plane_large += z_nominal[iz];
  mid_plane_large /= z_nominal.size();

  vector<int> xp=q_planes("x"), up=q_planes("u"), vp=q_planes("v");
  mid_plane_large_X=0.;
  for(unsigned int ix=0;ix<xp.size();ix++) mid_plane_large_X += z_nominal[xp[ix]];
  mid_plane_large_X /= 1.*xp.size();

  mid_plane_large_UV=0.;
  for(unsigned int iu=0;iu<up.size();iu++) mid_plane_large_UV+=z_nominal[up[iu]];
  for(unsigned int iv=0;iv<vp.size();iv++) mid_plane_large_UV+=z_nominal[vp[iv]];
  mid_plane_large_UV /= 1.*(up.size()+vp.size());

  H=double(roParam_bottom_mult1.distanceFromZAxis);//982.); //bottom of wedge to the beamline
  Hnom=double(roParam_bottom_mult1.distanceFromZAxis);//982.); //bottom of wedge to the beamline


  strip_width = double(roParam_top_mult1.stripPitch);  // 0.5;
  stereo_degree = double(TMath::RadToDeg()*roParam_top_mult1.stereoAngle[2]); //0.75 //3 in degrees!
  double degree=roParam_top_mult1.stereoAngle[2];
  vertical_strip_width_UV = strip_width/std::cos(degree);
  ybases=vector<vector<double > >(setup.size(), vector<double >(n_stations_eta,0.));

  //gposx for X1; gpos-lpos X2,UV1,UV2
  vector<double > xeta,ueta,veta;

  // MM_firststrip_positions returns the position for phi sector 1.
  // => for the small sectors, rotate this by -1/16 of a rotation to make our lives easier.
  // in this coordinate basis, x is up/down the wedge (radial), and y is left/right (phi).
  float cos_rotation = std::cos(-2*M_PI / 16.0);
  float sin_rotation = std::sin(-2*M_PI / 16.0);
  float x_rotated = 0.0;
  float y_rotated = 0.0;

  float st_angle = 0.0;

  float radial_pos      = 0;
  float radial_pos_xx_1 = 0, radial_pos_xx_2 = 0;
  float radial_pos_uv_1 = 0, radial_pos_uv_2 = 0;
  
  // FIXME: variable called 'eta' already defined above 
  for (unsigned int eta = 1; eta <= 2; eta++){
    unsigned int layer = 1;
    for (const auto& pos: MM_firststrip_positions(detManager, wedgeString, eta)){

      if (wedgeString=="MMS"){
        x_rotated = pos.x()*cos_rotation - pos.y()*sin_rotation;
        y_rotated = pos.x()*sin_rotation + pos.y()*cos_rotation;
      }
      else{
        x_rotated = pos.x();
        y_rotated = pos.y();
      }

      if      (is_u(layer)) st_angle = -1*std::abs(stereo_degree);
      else if (is_v(layer)) st_angle =    std::abs(stereo_degree);
      else                  st_angle = 0;

      // walk from the center of the strip to the position of the strip at the center of the wedge.
      // NB: for X-planes, this is simply the center of the strip: tan(0) = 0.
      radial_pos = std::abs(x_rotated - y_rotated*std::tan(st_angle * M_PI/180.0));

      if (is_x(layer) && eta==1) radial_pos_xx_1 = radial_pos;
      else if (is_x(layer) && eta==2) radial_pos_xx_2 = radial_pos;
      if (is_u(layer) && eta==1) radial_pos_uv_1 = radial_pos;
      else if (is_u(layer) && eta==2) radial_pos_uv_2 = radial_pos;

      layer++;
    }
  }

  // store radial positions as fixed point
  std::vector< double > radial_pos_xx = {double(radial_pos_xx_1), double(radial_pos_xx_2)};
  std::vector< double > radial_pos_uv = {double(radial_pos_uv_1), double(radial_pos_uv_2)};
  ybases[0] = radial_pos_xx;
  ybases[1] = radial_pos_xx;
  ybases[2] = radial_pos_uv;
  ybases[3] = radial_pos_uv;
  ybases[4] = radial_pos_uv;
  ybases[5] = radial_pos_uv;
  ybases[6] = radial_pos_xx;
  ybases[7] = radial_pos_xx;

  //now put in the positions at `ly spaced points for a y dependent z
  z_large=vector<vector<double > >(ybins,z_nominal);
  double pitch_f=1.*std::sin(correct.rotate.X())*h1/ybins, bumper_up=0.0;
  //double pitch_b = 0;
  ATH_MSG_DEBUG("Specs: correct.rotate.X()=" << correct.rotate.X() << ",correct.translate.Z()=" << correct.translate.Z() << ",pitch_f=" << pitch_f);
  for(int iy=0;iy<ybins;iy++){
    //z axis in misalignment line points in the opposite direction....TOO LATE! (right handed coordinate system the way we expect for x and y makes this happen)
    double over_f=pitch_f*(iy+bumper_up)-correct.translate.Z();
    double over_b=0;//pitch_b*(iy+bumper_up)+correct.dzb;
    ATH_MSG_DEBUG("iy=" << iy << "over_f=" << over_f << ",over_b=" << over_b);
    for(int jp=0;jp<8;jp++){
      ATH_MSG_DEBUG("z_large[" << iy << "][" << jp << "]" << z_large[iy][jp] << "--->");
      if(jp<4) z_large[iy][jp]+=over_f;
      else z_large[iy][jp]+=over_b;
      ATH_MSG_DEBUG(z_large[iy][jp]);
    }
  }

  //////// TABLE GENERATORS ///////////////  //size of cartesian steps
  h_mx = 0.0001; // 0.005;  //0.001;
  h_my = 0.0001; //0.005;  //0.001;
  m_y_max = ( (Hnom+h1)/z_nominal.front() );
  m_y_min = ( H/z_nominal.back() );
  m_x_max = (w2/2.)/z_nominal.front();
  m_x_min = (w2/-2.)/z_nominal.back(); //-2;

  n_x = std::ceil((m_x_max - m_x_min)/h_mx);
  n_y = std::ceil((m_y_max - m_y_min)/h_my);
  /////////////////////////////////////////

  //////////  for cut applications  /////////////////
  double tol = 0;  //0.02;

  //BLC had some interesting bounds...let's do the ones that make sense to me
  minimum_large_theta = std::atan(H/z_nominal.back()) + tol;
  maximum_large_theta = std::atan(std::sqrt(std::pow( (Hnom+h1),2) + 0.25*std::pow(w1,2))/z_nominal.back()) - tol;
  minimum_large_phi = -M_PI/180.0*0.5*wedge_opening_angle + tol;
  maximum_large_phi = M_PI/180.0*(0.5*wedge_opening_angle) - tol;
  ///////////////////////////////////////////////////

  double phiseg= ((maximum_large_phi-minimum_large_phi)/n_phibins);
  m_phibins.clear();
  for(int i=0;i<=n_phibins;i++)  m_phibins.push_back(  (minimum_large_phi+phiseg*i)   );
  double etalo=-std::log(std::tan(0.5*maximum_large_theta));
  double etahi=-std::log(std::tan(0.5*minimum_large_theta));
  /*
  bool custom=false;
  if(custom){
    //these custom eta bins have the top station as one bin (since that has normal shape for the phi resolutions)
    //and separates the old part from gap to 2.5 (where the simulation used to be limited) and then one bin from 2.5 to max (new stuff)
    m_etabins.clear();
    m_etabins.push_back(etalo);
    double etamid=std::asinh(  (z_nominal.front()/ybases.front()[1] )  );
    double intermid=(2.5-etamid)/8.;
    for(int i=0;i<8;i++) m_etabins.push_back(etamid+intermid*i);
    m_etabins.push_back(2.5);
    m_etabins.push_back(etahi);
    n_etabins=m_etabins.size()-1;
  }
  else */
  if(n_etabins==static_cast<int>(xeta.size())){
    m_etabins.push_back(etalo);
    for(int i=n_etabins-1;i>=0;i--){
      m_etabins.push_back(std::asinh( (z_nominal.front()/xeta[i]) ));
    }
  }
  else{
    double increment=(etahi-etalo)/n_etabins*1.;
    for(int i=0;i<=n_etabins;i++) m_etabins.push_back(etalo+increment*i);
  }

  ///////   Rough ROI  ////////////
  n_theta_rois = 32;
  n_phi_rois = 16;
  //////////////////////////////

  /////// Front Filter /////////////
  double theta_max=maximum_large_theta;
  double theta_min=minimum_large_theta;

  slope_max = double(std::tan(theta_max));
  slope_min = double(std::tan(theta_min));

  CT_u = 0;
  CT_v = 0;
  CT = CT_x + CT_uv;
  x_error = .0035;//h*0.5;
  BC_window = 2;
  //////////////////////////////////
  //tables--with parameters set, we can now generate them
  if(val_tbl){
    //Reference for local slope least squares fit
    Local_Slope_A_B();

    //cartesian slopes to phi and theta reference table
    Slope_Components_ROI();

    //Delta theta reference table to avoid division operation
    Delta_theta_optimization_LG();

    //simulation-based correction fill...if available
    if(correct.type==3) fill_crep_table(inputParams.pcrep_dir,inputParams.tag);
    fill_yzmod();
  }
}

std::vector<Amg::Vector3D> MMT_Parameters::MM_firststrip_positions(const MuonGM::MuonDetectorManager* detManager, const std::string& wedge, int eta){

  std::vector<Amg::Vector3D> positions;
  positions.clear();

  Identifier strip_id;
  int phi = 1;
  int strip = 200;

  for (unsigned int mult = 1; mult <= n_multiplets; mult++) {
    for (unsigned int layer = 1; layer <= n_layers; layer++) {

      Amg::Vector3D pos(0.0, 0.0, 0.0);
      strip_id = detManager->mmIdHelper()->channelID(wedge, eta, phi, mult, layer, strip);
      const MuonGM::MMReadoutElement* readout = detManager->getMMReadoutElement(strip_id);
      readout->stripGlobalPosition(strip_id, pos);
      positions.push_back(pos);

      ATH_MSG_DEBUG( "global z-pos. (using MMReadoutElement) for"
                           << " wedge size "       << wedge
                           << " multiplet "        << mult
                           << " layer "            << layer
                           << " | z = "            << pos.z() );
    }
  }

   return positions;
}

int MMT_Parameters::is_x(int plane){ return (std::find(planes_x.begin(), planes_x.end(), plane) != planes_x.end()) ? 1 : 0; }
int MMT_Parameters::is_u(int plane){ return (std::find(planes_u.begin(), planes_u.end(), plane) != planes_u.end()) ? 1 : 0; }
int MMT_Parameters::is_v(int plane){ return (std::find(planes_v.begin(), planes_v.end(), plane) != planes_v.end()) ? 1 : 0; }

vector<int> MMT_Parameters::q_planes(const string& type) const{
  if(type.length()!=1) throw std::runtime_error("MMT_Parameters::q_planes: Invalid type");
  if(type.compare("x")!=0 && type.compare("u")!=0 && type.compare("v")!=0){
    ATH_MSG_WARNING("Unsupported plane type " << type << " in q_planes...aborting....\n");
    throw std::runtime_error("MMT_Parameters::q_planes: Unsupported plane type");
  }
  vector<int> q_planes;
  for(unsigned int ip=0;ip<setup.size();ip++){
    if(setup.compare(ip,1,type)==0) q_planes.push_back(ip);
  }
  return q_planes;
}

par_par MMT_Parameters::param_par() const{
  return par_par(h,CT_x,CT_uv,uv_error,setup,islarge,dlm_new,genbg,chargeThreshold,misal,correct,val_tbl);
}

double MMT_Parameters::y_from_eta_wedge(double eta,int plane)const{
  //assumes wedge geometry--average x^2, is 1/3 y^2*tan^2(stereo_degree), for eta/y correspondence
  double z=z_nominal[plane]; 
  double zeta=M_PI/180.0*(0.5*stereo_degree);
  return z*std::tan(2*std::atan(std::exp(-1.*eta)))/std::sqrt(1+std::tan(zeta)*std::tan(zeta)/3.);
}

double MMT_Parameters::eta_wedge_from_y(double y,int plane)const{
  double z=z_nominal[plane];
  double zeta=M_PI/180.0*(0.5*stereo_degree);
  return -1.*std::log(std::tan(0.5*std::atan(y/z*std::sqrt(1+std::tan(zeta)*std::tan(zeta)/3.))));
}

int MMT_Parameters::ybin(double y,int plane)const{

  double base=ybases[plane].front(), seg_len=h1/ybins;
  //if it's over, just keep it...for now--if it's below keep it, too: perhaps necessary for larger slope road sizes
  //update: DON'T keep the below ones....terrible things for drs
  int the_bin=ybases.front().size()-1;
  for(int i=0;i<=ybins;i++){
    if(y<(base+seg_len*i)) return i-1;
  }
  return the_bin;
}

//make the local slope ab for all ybins
//track percent in number of same bin
//maybe do a th2d scatter in bins for all/for a given ybin in plane 0

void MMT_Parameters::fill_full_Ak_Bk(){
  AB_k_local.clear();
  vector<int> x_planes=q_planes("x");
  int nx=x_planes.size();
  vector<int> variable_key(nx,-1);//a grid with all possible
  do{
    AB_k_local[variable_key] = ak_bk_hit_bins(variable_key);
  } while(!toggle_key(variable_key,-1,ybins-1));
}

//The problem is this: we don't a priori know how many X planes there will be (configured at run time)
//so, we don't know how many Ak, Bk constants we'll need (it's n_Xplanes * ybins)
//The idea, then is to run through the possible values like a key with n_Xplanes digits, each of which can take values in [-1,ybins-1]. (-1 is the no hit here value)
//We need to toggle through all possible values; the idea is to start with the lowest entry (-1,-1,...,-1) and increment right to left
//We start at the rightmost entry and check values until we get a value != highest possible value and then
// increment this up, resetting all values to the right of it to lo_value.
bool MMT_Parameters::toggle_key(vector<int>& key,int lo_value,int hi_value)const{
  for(int idex=(int)(key.size()-1); idex>=0; idex--){
    if(key[idex]==hi_value) continue;
    else{
      key[idex]++;
      for(unsigned int reset_dex=idex+1; reset_dex<key.size(); reset_dex++) key[reset_dex]=lo_value;
      return false;
    }
  }
  return true;
}

int MMT_Parameters::eta_bin(double theta) const{
  int ebin=-999;
  double eta=-std::log(std::tan(0.5*theta));
  if(theta==-999 || theta==-16) return -1;
  for(int i=0;i<=n_etabins;i++){
    if(eta<m_etabins[i]){
      ebin=i-1;
      break;
    }
  }
  //we want the histograms binned on truth eta
  //event selection eta cuts are done on theta_ip, not theta_pos
  if(ebin==-1) return 0;
  else if(ebin==-999) return n_etabins-1;
  return ebin;
}

string MMT_Parameters::eta_str(int eta) const{
  ostringstream what;
  what<<"_eta";
  if(eta>=0 && eta<n_etabins) what << m_etabins[eta] << "_" << m_etabins[eta+1];
  else what << "all";
  return what.str();
}

int MMT_Parameters::phi_bin(double phi) const{
  int pbin=-999;
  if(phi==-999 || phi==-16) return n_phibins*0.5;
  for(int i=0;i<=n_phibins;i++){
    if(phi<m_phibins[i]){
      pbin=i-1;
      break;
    }
  }
  //we want the histograms binned on truth phi
  //event selection eta cuts are done on phi_ip, not phi_pos
  if(pbin==-1) return 0;
  else if(pbin==-999) return n_phibins-1;
  return pbin;
}

string MMT_Parameters::phi_str(int phi) const{
  ostringstream what;
  what<<"_phi";
  if(phi>=0 && phi<n_phibins) what << m_phibins[phi] << "_" << m_phibins[phi+1];
  else what << "all";
  return what.str();
}

//calculate A_k and B_k
//these are NOT the constants defined in the note--firstly, they use an altered length scale for fixed point
//B_k is otherwise the same
//A_k is the average z
pair<double,double> MMT_Parameters::ak_bk_hit_bins(const vector<int>& hits)const{
  vector<int> x_planes=q_planes("x");
  assert(hits.size()==x_planes.size());
  int nhits=0;
  double sum_x=0,sum_xx=0;
  for(int ih=0;ih<(int)(hits.size());ih++){
    if(hits[ih]<0 || hits[ih]>=ybins) continue;
    double addme=z_large[hits[ih]][x_planes[ih]]/MMTStructConst;
    sum_x  += addme;
    sum_xx += addme*addme;
    nhits++;
  }
  double diff = nhits*sum_xx-sum_x*sum_x;
  if(std::abs(diff)<1e-10){
    ATH_MSG_WARNING("diff is too small: " << diff << " 1e-10 will be used instead");
    diff = 1e-10;
  }
  if(std::abs(sum_x)<1e-10){
    ATH_MSG_WARNING("sum_x is too small: " << sum_x << " 1e-10 will be used instead");
    sum_x = 1e-10;
  }
  return pair<double,double>(1.*nhits/sum_x, 1.*sum_x/diff);
}

void MMT_Parameters::fill_slims(){
  Ak_local_slim=vector<vector<vector<double > > >(11,vector<vector<double > >(ybins,vector<double >()));
  Bk_local_slim=vector<vector<vector<double > > >(11,vector<vector<double > >(ybins,vector<double >()));
  for(int xdex=0;xdex<11;xdex++){
    vector<bool>xhits(lcl_int_to_xhits(xdex));
    int ntru=0;
    for(int k=0; k<(int)xhits.size(); k++) ntru+=xhits[k];
    ATH_MSG_DEBUG("xdex=" << xdex << " (" << ntru << " hits)");
    for(int ybin=0; ybin<ybins; ybin++){
      ATH_MSG_DEBUG("\tybin=" << ybin );
      if(ybin==ybins-1) ntru=1;
      for(int which=0; which<ntru; which++){
 	ATH_MSG_DEBUG("\t\twhich=" << which);
	vector<int>indices(indices_to_key(xdex,ybin,which));
	for(int i=0;i<(int)indices.size();i++)  ATH_MSG_DEBUG(indices[i]);
	pair<double,double>howdy(ak_bk_hit_bins(indices));
 	ATH_MSG_DEBUG(setprecision(8) << "---akbk.first=" << howdy.first << ", second=" << howdy.second);
	Ak_local_slim[xdex][ybin].push_back(howdy.first);
	Bk_local_slim[xdex][ybin].push_back(howdy.second);
      }
    }
  }
}

void MMT_Parameters::fill_crep_table(const string&dir,const string&tag){
  par_par bill=param_par();
  bill.ctx=2;bill.ctuv=1;
  bill.corr.type=0;
  double fudge_factor=(correct.translate.Z()!=0||correct.rotate.X()!=0?0.5:1.);
  int nk=nsimmax_1d();
  ostringstream crep_nom;
  crep_nom<<dir<<(dir.substr(dir.length()-1)=="/"?"":"/")<<"pcrep"<<bill.print_pars()<<"_"<<tag<<".txt";
  std::ifstream crep(crep_nom.str().c_str());
  crep_table=vector<vector<vector<vector<float> > > >(n_etabins,vector<vector<vector<float> > >(n_phibins,vector<vector<float> >(nk,vector<float>(3,0))));
  if(crep.good()){
    for(int i=0;i<n_etabins;i++){
      string estr=eta_str(i);
      for(int j=0;j<n_phibins;j++){
	string pstr=phi_str(i);
	for(int k=0;k<nk;k++){
	string title;
        double the,phi,dth;
	crep>>title;
	if(title!=estr+pstr+index_to_hit_str(k)){
	  ATH_MSG_WARNING("Something's wrong with your simulation-based correct read-in...you want entries for " << estr+pstr+index_to_hit_str(k) << ", but you got "<< title << " in " << crep_nom.str());
          throw std::runtime_error("MMT_Parameters::fill_crep_table: Invalid configuration");
	}
	crep >> the >> phi >> dth;
	crep_table[i][j][k][0]=the*fudge_factor;
        crep_table[i][j][k][1]=phi;
        crep_table[i][j][k][2]=dth*fudge_factor;
	}
      }
    }
  }
}

void MMT_Parameters::fill_yzmod(){
  vector<int> x_planes=q_planes("x");//this tells us which planes are x planes
  int nxp=x_planes.size();
  ymod=vector<vector<vector<double > > >(n_etabins,vector<vector<double > >(n_phibins,vector<double >(nxp,double(0.))));
  zmod=vector<vector<vector<double > > >(n_etabins,vector<vector<double > >(n_phibins,vector<double >(nxp,double(0.))));
  double pbump=0.5*(maximum_large_phi-minimum_large_phi)/(n_phibins);
  for(int et=0;et<n_etabins;et++){
    double theta=2*std::atan(std::exp(-1.*m_etabins[et]));
    for(int ph=0;ph<n_phibins;ph++){
      double phi=m_phibins[ph]+pbump;
      for(int pl=0;pl<nxp;pl++){
        if(correct.type!=2 || x_planes[pl]>3) continue;//if we don't correct or if it's the back multiplet, just leave these as zero

        double yadd=0.;
        double zadd=0.;
        double zflt=z_nominal[x_planes[pl]];
        double x=zflt*std::tan(theta)*std::sin(phi);
        double yflt=zflt*std::tan(theta)*std::cos(phi);
        double yup=yflt-ybases[x_planes[pl]][0];

        double alpha=correct.rotate.Z();
        double beta=correct.rotate.Y();
        // double gamma=correct.rotate.X();
        //gamma--do here if necessary...problematic if so

        //beta angle--rotation around the y axis; the x coordinate or phi taken care of in correction to slope road limits in x in constructor
        //z is changed, and so y must be scaled (should it?): THIS DOESN'T WORK....
        // 	zadd-=1.*x*sin(beta)/MMTStructConst;yadd+=yup*zadd/zflt/MMTStructConst;
        zadd-=std::tan(beta)*std::tan(theta)*std::sin(phi)*zflt/MMTStructConst;
        yadd-=std::tan(beta)*std::tan(theta)*std::sin(phi)*yflt/MMTStructConst;

        //alpha angle--rotation around the z axis; z unchanged, y the expected rotation (final rotation taken care of at start)
        yadd+=((std::cos(alpha)-1.)*yup+x*std::sin(alpha))/MMTStructConst;

        //add the entries
        ymod[et][ph][pl]=double(yadd);
        zmod[et][ph][pl]=double(zadd);
      }
    }
  }
}

void MMT_Parameters::index_key_test(){
  for(int xdex=0;xdex<11;xdex++){
    vector<bool>xhits(lcl_int_to_xhits(xdex));
    int ntru=0;
    for(int k=0; k<(int)xhits.size(); k++) ntru+=xhits[k];
    for(int ybin=0; ybin<ybins; ybin++){
      if(ybin==ybins-1) ntru=1;
      for(int which=0; which<ntru; which++){
	ATH_MSG_DEBUG("Let's start with [" << xdex << "][" << ybin << "][" << which << "] makes ");
	vector<int>key(indices_to_key(xdex,ybin,which));
	for(int i=0; i<(int)key.size(); i++) ATH_MSG_DEBUG(key[i] << " ");
	int x,y,w;
        key_to_indices(key,x,y,w);
	ATH_MSG_DEBUG("...and back to [" << x << "][" << y << "][" << w << "]  ");
      }
    }
  }
}


void MMT_Parameters::key_to_indices(const vector<int>& key,int& xdex,int& ybin,int& which)const{
  vector<bool> boogah;vector<int> was_hit;
  for(unsigned int i=0;i<key.size();i++){
    boogah.push_back(key[i]>=0 && key[i]<ybins);
    if(boogah.back()) was_hit.push_back(key[i]);
  }
  bool even=true;
  int dev_pos=0, dev_bin=-1;
  for(unsigned int i=0;i<was_hit.size();i++){
    if(was_hit[i]!=was_hit.front()){
      if(even){
	dev_pos=i;
	if(was_hit[i]!=was_hit.front()+1){
	  dev_pos=-2;
          break;
	}
	else dev_bin=was_hit[i];
      }
      else if(was_hit[i]!=dev_bin){
	dev_pos=-2;
        break;
      }
      even=false;
    }
  }
  if(dev_pos<0 || dev_pos>=(int)boogah.size()) which=0;//if the track isn't upward going, make the single bin assumption
  else which=dev_pos;
  xdex=xhits_to_lcl_int(boogah);
  if(was_hit.size()==0) was_hit.push_back(-999);
  ybin=was_hit.front();
}

vector<int> MMT_Parameters::indices_to_key(int xdex,int ybin,int which)const{
  vector<bool> xhits=lcl_int_to_xhits(xdex);
  vector<int> key(xhits.size(),-1);
  int count=0;
  int raise_it=(which==0?xhits.size():which);
  for(unsigned int ix=0;ix<xhits.size();ix++){
    if(xhits[ix]){
      key[ix]=ybin;
      if(count>=raise_it) key[ix]++;
      count++;
    }
  }
  return key;
}

void MMT_Parameters::Local_Slope_A_B(){
  fill_full_Ak_Bk();
  fill_slims();
}

void MMT_Parameters::Slope_Components_ROI(){
  //these are mm/mm i.e. not units of strip #

  Slope_to_ROI=vector<vector<vector<double > > >(n_y,vector<vector<double > >(n_x,vector<double >(2,double(0.))));
  for(int ix=0; ix<n_x; ix++){
    for(int jy=0; jy<n_y; jy++){
      int xdex=ix, ydex=jy+1;
      if(xdex==0) xdex++;

      double M_x = ( m_x_min + h_mx * xdex );
      double M_y = ( m_y_min + h_my * ydex );

      double theta = std::atan(std::sqrt(M_x*M_x+M_y*M_y));
      double phi= std::atan2(M_y,M_x);
      if(minimum_large_phi>phi || maximum_large_phi<phi) phi=999;
      if(minimum_large_theta>theta || maximum_large_theta<theta) theta=0;
      Slope_to_ROI[jy][ix][0] = theta;
      Slope_to_ROI[jy][ix][1] = phi;
    }
  }
  ATH_MSG_DEBUG( "return Slope_Components_ROI" );
}

void MMT_Parameters::Delta_theta_optimization_LG(){
  DT_Factors.clear();
  double a=1.;   //sin(pi/2+TMath::DegToRad(28/4));  //sin(phi);
  int number_LG_regions = 256;
  double LG_min = 0;
  double LG_max = 0.5;
  double LG_region_width = (LG_max - LG_min)/number_LG_regions;//LG_min=0 means e no neg slopes

  for(int ilgr=0; ilgr<number_LG_regions; ilgr++){
    vector<double > tmpVector;
    tmpVector.push_back(LG_min+LG_region_width *ilgr);//LG
    tmpVector.push_back(  double(1.) / (tmpVector.front()/a  +  a));//mult_factor
    DT_Factors.push_back(tmpVector);
  }
  ATH_MSG_DEBUG( "return Delta_theta_optimization_LG" );
}

int MMT_Parameters::xhits_to_lcl_int(const vector<bool>& xhits) const{
  if(xhits.size()!=4){
    ATH_MSG_WARNING("There should be 4 xplanes, only " << xhits.size() << " in the xhit vector given to local_slope_index()\n");
    throw std::runtime_error("MMT_Parameters::xhits_to_lcl_int: Invalid number of planes");
  }
  if(xhits[0] && xhits[1] && xhits[2] && xhits[3]) return 0;
  else if( xhits[0] &&  xhits[1] &&  xhits[2] && !xhits[3]) return 1;
  else if( xhits[0] &&  xhits[1] && !xhits[2] &&  xhits[3]) return 2;
  else if( xhits[0] && !xhits[1] &&  xhits[2] &&  xhits[3]) return 3;
  else if(!xhits[0] &&  xhits[1] &&  xhits[2] &&  xhits[3]) return 4;
  else if( xhits[0] &&  xhits[1] && !xhits[2] && !xhits[3]) return 5;
  else if( xhits[0] && !xhits[1] &&  xhits[2] && !xhits[3]) return 6;
  else if( xhits[0] && !xhits[1] && !xhits[2] &&  xhits[3]) return 7;
  else if(!xhits[0] &&  xhits[1] &&  xhits[2] && !xhits[3]) return 8;
  else if(!xhits[0] &&  xhits[1] && !xhits[2] &&  xhits[3]) return 9;
  else if(!xhits[0] && !xhits[1] &&  xhits[2] &&  xhits[3]) return 10;
  return -999;
}

int MMT_Parameters::nsimmax_1d()const{
  return std::pow(2.,setup.size());
}

int MMT_Parameters::bool_to_index(const vector<bool>&track)const{
  assert(track.size()==setup.size());
  int index=0;
  for(int plane=0; plane<(int)track.size(); plane++){
    if(!track[plane]) continue;
    index = index | int(std::pow(2.,plane));
  }
  return index;
}

vector<bool> MMT_Parameters::index_to_bool(int index)const{
  vector<bool>code(setup.size(),false);
  for(int i=0; i<(int)code.size(); i++) code[i]=(index&int(std::pow(2.,i)));
  return code;
}

string MMT_Parameters::index_to_hit_str(int index)const{
  return bool_to_hit_str(index_to_bool(index));
}

string MMT_Parameters::bool_to_hit_str(const vector<bool>&track)const{
  ostringstream ok;
  ok<<"_ht";
  for(int i=0; i<(int)track.size(); i++) ok << track[i];
  return ok.str();
}

vector<bool> MMT_Parameters::lcl_int_to_xhits(int lcl_int)const{
  vector<bool> xhits(4,true);
  if(lcl_int<0 || lcl_int>10){
    ATH_MSG_WARNING("Wherefore dost thou chooseth the hits of planes of X for thy lcl_int " << lcl_int << "?  'Tis not in [0,10]!");
    throw std::runtime_error("MMT_Parameters::lcl_int_to_xhits: invalid value for lcl_int");
  }
  if(lcl_int==0) return xhits;
  if(lcl_int==1 || lcl_int==5 || lcl_int==6 || lcl_int==8) xhits[3]=false;
  if(lcl_int==2 || lcl_int==5 || lcl_int==7 || lcl_int==9) xhits[2]=false;
  if(lcl_int==3 || lcl_int==6 || lcl_int==7 || lcl_int==10) xhits[1]=false;
  if(lcl_int==4 || lcl_int==8 || lcl_int==9 || lcl_int==10) xhits[0]=false;
  return xhits;
}

string MMT_Parameters::lcl_int_to_xhit_str(int lcl_int)const{
  vector<bool>hits=lcl_int_to_xhits(lcl_int);
  ostringstream out;
  out<<"_xht";
  for(int i=0;i<(int)hits.size();i++) out<<hits[i];
  return out.str();
}

evInf_entry::evInf_entry(int event,int pdg,double e,double p,double ieta,double peta,double eeta,double iphi,double pphi,double ephi,double ithe,double pthe,double ethe,double dth,
			 int trn,int mun,const ROOT::Math::XYZVector& tex,int troi,int antev,int postv,int nxh,int nuvh,int nxbg,int nuvbg,double adt,int difx,
			 int difuv,bool cut,bool bad)
  :athena_event(event),pdg_id(pdg),E(e),pt(p),eta_ip(ieta),eta_pos(peta),eta_ent(eeta),phi_ip(iphi),phi_pos(pphi),phi_ent(ephi),theta_ip(ithe),theta_pos(pthe),theta_ent(ethe),
   dtheta(dth),truth_n(trn),mu_n(mun),vertex(tex),truth_roi(troi),N_hits_preVMM(antev),N_hits_postVMM(postv),N_X_hits(nxh),N_UV_hits(nuvh),NX_bg_preVMM(nxbg),
   NUV_bg_preVMM(nuvbg),avg_drift_time(adt),max_less_min_hitsteps_X(difx),max_less_min_hitsteps_UV(difuv),pass_cut(cut),bad_wedge(bad) {}

void evInf_entry::print() const{
}

evFit_entry::evFit_entry(int event,double fthe,double fphi,double fdth,int roi,int xhit,int uvhit,int bgx,int bguv,double dth_nd,int hc,int tph,int bgph)
  :athena_event(event),fit_theta(fthe),fit_phi(fphi),fit_dtheta(fdth),fit_roi(roi),X_hits_in_fit(xhit),UV_hits_in_fit(uvhit),bg_X_fit(bgx),
   bg_UV_fit(bguv),dtheta_nodiv(dth_nd),hcode(hc),truth_planes_hit(tph),bg_planes_hit(bgph) {}

void evFit_entry::print()const{
}

hitData_key::hitData_key(int bct, double t, double gt, int vmm, int ev):BC_time(bct),time(t),gtime(gt),VMM_chip(vmm),event(ev) {}

bool hitData_key::operator==(const hitData_key& rhs) const{
  if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime==rhs.gtime && this->VMM_chip==rhs.VMM_chip && this->event==rhs.event) return true;
  return false;
}

bool hitData_key::operator!=(const hitData_key& rhs) const{
  return !(*this==rhs);
}
bool hitData_key::operator<(const hitData_key& rhs) const{
  if(this->BC_time<rhs.BC_time) return true;
  else if(this->BC_time==rhs.BC_time && this->time<rhs.time) return true;
  else if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime<rhs.gtime) return true;
  else if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime==rhs.gtime && this->VMM_chip<rhs.VMM_chip) return true;
  else if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime==rhs.gtime && this->VMM_chip==rhs.VMM_chip && this->event<rhs.event) return true;
  return false;
}

bool hitData_key::operator>(const hitData_key& rhs) const{
  if(this->BC_time>rhs.BC_time) return true;
  else if(this->BC_time==rhs.BC_time && this->time>rhs.time) return true;
  else if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime>rhs.gtime) return true;
  else if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime==rhs.gtime && this->VMM_chip>rhs.VMM_chip) return true;
  else if(this->BC_time==rhs.BC_time && this->time==rhs.time && this->gtime==rhs.gtime && this->VMM_chip==rhs.VMM_chip && this->event>rhs.event) return true;
  return false;
}

bool hitData_key::operator<=(const hitData_key& rhs) const{
  return(*this<rhs || *this==rhs);
}

bool hitData_key::operator>=(const hitData_key& rhs) const{
  return(*this>rhs || *this==rhs);
}

string hitData_key::hdr()const{
  ostringstream out;
  out << setw(12) << "BC_t" << setw(12) << "t" << setw(12) << "g_t" << setw(12) << "VMM" << setw(12) << "event";
  return out.str();
}
string hitData_key::str()const{
  ostringstream out;
  out << setw(12) << this->BC_time << setw(12) << this->time << setw(12) << this->gtime << setw(12) << this->VMM_chip << setw(12) << this->event;
  return out.str();
}
void hitData_key::print()const{
}


hitData_info::hitData_info(int pl,int station_eta,int strip,std::shared_ptr<MMT_Parameters> par,const ROOT::Math::XYZVector &tru,double tpos,double ppos):plane(pl){
  (void) tru;
  //The idea here is to calculate/assign a y and a z to a given hit based on its pl/station/strip, the geometry of the detector (in par), and misalignment based on position.
  //We start by assigning the plane dependent strip width (the stereo planes come in skew and so get divided by cos(stereo_angle)
  char schar=par->setup[plane];
  bool horizontal=(schar=='x'||schar=='X');
  double swidth=par->strip_width;
  int eta = std::abs(station_eta) -1;
  double base=par->ybases[plane][eta];
  if(!horizontal) swidth/=std::cos(M_PI/180.0*(par->stereo_degree));
  //Next, we initialize some constants--y will eventually be calculated as y=base+scale*(width*strip+delta_y(misalignment,correction)).
  //The correction portion of delta_y is done in in the calculations on the ybases object in par
  //yup, or "y up" is the portion of y above the base of the plane (i.e. in the detector)
  double delta_y=mis_dy(pl,par,tpos,ppos), yup_scale=1.;
  double yflt=-999;
  yflt=base+(strip*swidth+delta_y)*yup_scale;
  //Note that ybin should be calculated on the basis of a misaligned y, not a corrected one (to get the position on the wedge just right),
  //but the difference for this part of the calculation should be negligible (an effect of <~ misal/(bin length) ~ (5 mm/(3600 mm/15)) ~ %.
  int bin=par->ybin(yflt,plane);

  if(bin==-1) bin=0;
  //We have here both the "perfect" and binned z calculations/lookups; the commented out one is there for reference
  double zflt=par->z_large[bin][plane];
//   if(par->correct.type==2&&plane<4&&par->correct.rotate.X()!=0){
//     // double fltyup=(base+(strip*swidth+delta_y)-planebase);
//     double angle=par->correct.rotate.X();
// //     yflt=planebase+fltyup*cos(angle);
// //     zflt=zplane+0.5*fltyup*sin(angle)*cos(angle)*(1-tan(angle)*base/zplane)/(1-tan(angle)*1.*yflt/zplane);
// //   }
//   y = yflt / MMTStructConst;
//   z = zflt / MMTStructConst;
  y = yflt;
  z = zflt;
  slope =  (zflt!=0.) ? yflt / zflt : 0.;
}

double hitData_info::mis_dy(int plane,std::shared_ptr<MMT_Parameters> par,double tpos,double ppos)const{
  if(par->misal.type!=1 || plane>3) return 0.;
  double zplane=par->z_nominal[plane];
  double base=par->ybases[plane].front();

  // double s_z0=zplane;
  double s_x0=zplane*std::tan(tpos)*std::sin(ppos);
  double s_y0=zplane*std::tan(tpos)*std::cos(ppos);//initial position

  double hats_z0=std::cos(tpos);
  double hats_x0=std::sin(tpos)*std::sin(ppos);
  double hats_y0=std::sin(tpos)*std::cos(ppos);//muon track unit vector
  double zeta_y0=s_y0-base;//height in y in the wedge local coordinates--this is what we have to compare in the end
  double alpha=par->misal.rotate.Z();
  double beta=par->misal.rotate.Y();
  double gamma=par->misal.rotate.X();//rotation angles
  double ds=par->misal.translate.X();
  double dz=par->misal.translate.Y();
  double dt=-1.*par->misal.translate.Z();//t comes in -z
  double O_bxf=ds;
  double O_byf=base+dz;
  double O_bzf=zplane+dt;//position of bottom of the wedge in global coordinates; subtract this from the final y position (s_yf) for final zeta (comparison to add to y position)
  double yhat_x=-1.*std::sin(alpha)*std::cos(beta);
  double yhat_y=(std::cos(alpha)*std::cos(gamma)-std::sin(alpha)*std::sin(beta)*std::sin(gamma));
  double yhat_z=(std::cos(alpha)*std::sin(gamma)+std::sin(alpha)*std::sin(beta)*std::cos(gamma));//new y direction after rotations; horizontal case
  char schar=par->setup[plane];
  if(!(schar=='x'||schar=='X')){
    //if we're in a stereo plane, calculate different coefficients.
    double omega=M_PI/180.0*(par->stereo_degree);
    double pm=(schar=='u'||schar=='U'?1.:-1.);
    yhat_x=pm*std::cos(alpha)*std::cos(beta)*std::sin(omega)-std::sin(alpha)*std::cos(beta)*std::cos(omega);
    yhat_y=pm*std::sin(omega)*(std::sin(alpha)*std::cos(gamma)+std::cos(alpha)*std::sin(beta)*std::sin(gamma))+std::cos(omega)*(std::cos(alpha)*std::cos(gamma)-std::sin(alpha)*std::sin(beta)*std::sin(gamma));
    yhat_z=pm*std::sin(omega)*(std::sin(alpha)*std::sin(gamma)-std::cos(alpha)*std::sin(beta)*std::cos(gamma))+std::cos(omega)*(std::cos(alpha)*std::sin(gamma)+std::sin(alpha)*std::sin(beta)*std::cos(gamma));
    zeta_y0=pm*std::sin(omega)*s_x0+std::cos(omega)*(s_y0-base);
  }
  double kprime=(std::sin(beta)*O_bxf-std::cos(beta)*std::sin(gamma)*O_byf+std::cos(beta)*std::cos(gamma)*O_bzf)/(std::sin(beta)*hats_x0-std::cos(beta)*std::sin(gamma)*hats_y0+std::cos(beta)*std::cos(gamma)*hats_z0);
  double zeta_xf=kprime*hats_x0-O_bxf;
  double zeta_yf=kprime*hats_y0-O_byf;
  double zeta_zf=kprime*hats_z0-O_bzf;
  double zetayf_yhatf=zeta_xf*yhat_x+zeta_yf*yhat_y+zeta_zf*yhat_z;
  return zetayf_yhatf-zeta_y0;
}

hitData_info::hitData_info(int the_pl,double the_y,double the_z)
 : plane(the_pl),y(the_y),z(the_z){
  if(the_z==0 || the_z==-999) slope=-999;
  else slope=the_y/the_z;
}

string hitData_info::hdr()const{
  ostringstream out;
  out << setw(12) << "plane" << setw(12) << "y" << setw(12) << "z" << setw(12) << "slope";
  return out.str();
}
string hitData_info::str()const{
  ostringstream out;
  out << setprecision(4) << setw(12) << plane << setw(12) << y << setw(12) << z << setw(12) << slope;
  return out.str();
}
void hitData_info::print()const{
}

bool hitData_info::operator==(const hitData_info& rhs) const{
  if(this->plane==rhs.plane && this->y==rhs.y && this->z==rhs.z && this->slope==rhs.slope)return true;
  return false;
}

Hit::Hit(const hitData_key& k,const hitData_info&i):key(k),info(i) {}

bool Hit::operator==(const Hit& rhs) const{
  if(this->key==rhs.key && this->info==rhs.info)return true;
  return false;
}

void Hit::print_track(const vector<Hit>& track) const{
  (void) track;
}

void Hit::print() const{
}

hitData_entry::hitData_entry(int ev, double gt, double q, int vmm, int mmfe, int pl, int st, int est, int phi, int mult, int gg, double locX, double tr_the, double tru_phi,
		       bool q_tbg, int bct, double t, const ROOT::Math::XYZVector& tru, const ROOT::Math::XYZVector& rec,
		       double fit_the, double fit_ph, double fit_dth, double tru_dth,// double tru_thl, double tru_thg,
		       double mxg, double mug, double mvg, double mxl, double the_mx, double the_my, int the_roi):
  event(ev),gtime(gt),charge(q),VMM_chip(vmm),MMFE_VMM(mmfe),plane(pl),strip(st),station_eta(est),station_phi(phi),multiplet(mult),gasgap(gg),localX(locX),tru_theta_ip(tr_the),tru_phi_ip(tru_phi),truth_nbg(q_tbg),BC_time(bct),time(t),truth(tru),recon(rec),fit_theta(fit_the),fit_phi(fit_ph),fit_dtheta(fit_dth),tru_dtheta(tru_dth),
  /*tru_theta_local(tru_thl),tru_theta_global(tru_thg),*/M_x_global(mxg),M_u_global(mug),M_v_global(mvg),M_x_local(mxl),mx(the_mx),my(the_my),roi(the_roi) {}

Hit hitData_entry::entry_hit(std::shared_ptr<MMT_Parameters> par)const{
  return Hit(entry_key(),entry_info(par));
}
hitData_key hitData_entry::entry_key() const{
  return hitData_key(BC_time,time,gtime,VMM_chip,event);
}

hitData_info hitData_entry::entry_info(std::shared_ptr<MMT_Parameters> par)const{
  hitData_info spade(plane,station_eta,strip,par,recon,tru_theta_ip,tru_phi_ip);//truth or recon? doesn't matter too much--it's for misalignment
  return spade;
}
void hitData_entry::fit_fill(double fthe,double fphi, double fdth, double mxg, double mug, double mvg, double mxl, double M_x, double M_y, int king){
  this->fit_theta=fthe;
  this->fit_phi=fphi;
  this->fit_dtheta=fdth;
  this->M_x_global=mxg;
  this->M_u_global=mug;
  this->M_v_global=mvg;
  this->M_x_local=mxl;
  this->mx=M_x;
  this->my=M_y;
  this->roi=king;
}

void hitData_entry::print() const{
  // ATH_MSG_DEBUG( "%%%%%%%%%%%%%%%%HDST_ENTRY%%%%%%%%%%%%%%%%%%"<<endl
  //     <<"(Event,BC_time,time): ("<<event<<","<<BC_time<<","<<time<<"), "<<(truth_nbg?"truth":"bg")<<", charge: "<<charge<<endl
  //     <<"Wedge Coord---plane: "<<plane<<", strip: "<<strip<<", est: "<<station_eta<<", vmm: "<<VMM_chip<<", ip theta: "<<tru_theta_ip<<", ip phi: "<<tru_phi_ip<<endl
  //     <<"truth angles---dtheta: "<<tru_dtheta<<endl//", theta(loc): "<<tru_theta_local<<", theta(glo): "<<tru_theta_global<<endl
  //     <<"Truth vertex: \n");
  // truth.Print();
  // ATH_MSG_DEBUG( "Recon vertex: \n");
  // recon.Print();
  // ATH_MSG_DEBUG( "FIT---roi: "<<roi<<endl
  //     <<"        angles---theta: "<<fit_theta.getValue()<<", phi: "<<fit_phi.getValue()<<", dtheta: "<<fit_dtheta.getValue()<<endl
  //     <<"slopes, global---x: "<<M_x_global.getValue()<<", u: "<<M_u_global.getValue()<<", v: "<<M_v_global.getValue()<<endl
  //     <<"slopes,  other---x(loc): "<<M_x_local.getValue()<<", x(coord): "<<mx.getValue()<<", y(coord): "<<my.getValue()<<endl
  //     <<"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%");
}

finder_entry::finder_entry(bool the_is_hit, int the_clock,const Hit& the_k):
  is_hit(the_is_hit), clock(the_clock), hit(the_k) {}


bool finder_entry::operator==(const finder_entry& merp) const{
  if(merp.is_hit==this->is_hit && merp.clock==this->clock && this->hit==merp.hit) return true;
  else return false;
}
bool finder_entry::operator!=(const finder_entry& merp) const{
  return !(*this==merp);
}

ROI::ROI(double the_theta, double the_phi, double the_m_x, double the_m_y, int the_roi):
  theta(the_theta), phi(the_phi), m_x(the_m_x), m_y(the_m_y), roi(the_roi) {}


athena_header::athena_header(const ROOT::Math::PtEtaPhiEVector& par, int tpn, double etp, double ete, double php, double phe, int mun, const ROOT::Math::XYZVector& ver):
  the_part(par),trupart_n(tpn),etapos(etp),etaent(ete),phipos(php),phient(phe),muent_n(mun),vertex(ver) {}

digitWrapper::digitWrapper(const MmDigit* digit,
                           const std::string &stationName,
                           double tmpGTime,
                           const ROOT::Math::XYZVector& truthLPos,
                           const ROOT::Math::XYZVector& stripLPos,
                           const ROOT::Math::XYZVector& stripGPos
                           ):
  digit(digit),
  stName(stationName),
  gTime(tmpGTime),
  truth_lpos(truthLPos),
  strip_lpos(stripLPos),
  strip_gpos(stripGPos){}

track_address::track_address(int bct,bool big,int wed,int pl,int sh,const ROOT::Math::XYZVector& chr):
  BC(bct),islarge(big),wedge(wed),plane(pl),strip_hit(sh),cart_hit(chr) {}
