      subroutine init_processes
      use openloops, only: set_parameter
      implicit none
      include "nlegborn.h"
      include "pwhg_flst.h"
      include "pwhg_st.h"
      include 'pwhg_ckm.h' 
      include 'pwhg_rad.h' 
c      include 'pwhg_physpar.h'
      include 'pwhg_res.h'
      integer i, int(maxprocreal), ickm
      integer nmaxres_real,nmaxres_born,dim_integ
      integer iun
      real * 8 powheginput
      logical openloopsreal,openloopsvirtual
      common/copenloopsreal/openloopsreal,openloopsvirtual
      integer channel
      real * 8 brtau2brmu, brtau2bre, brmu2bre

      openloopsreal = powheginput("#openloopsreal") /= 0
      openloopsvirtual = powheginput("#openloopsvirtual") /= 0

      do i=1,maxprocreal
         int(i)=i
      enddo

c     The following is used in building the resonances
      ickm = 0   !   0,1,2 for general, ckm_cabibbo, ckm_diag
      ckm_diag = .false.
      ckm_cabibbo = .true.
      if(ickm.eq.2) then
         ckm_diag = .true.
      elseif(ickm.eq.1) then
         ckm_cabibbo = .true.
      endif


CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C    Set here the number of light flavours
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
      if (powheginput("#MSbarscheme").eq.0) then
         st_nlight=4
      else
         st_nlight=5
      endif

      call init_couplings

      call init_processes_born
      call init_processes_real

c     set electroweak and strong couplings os the Born amplitude (i.e. the powers of g and g_s)
      res_powew=4
      res_powst=2
      if(powheginput("#nores") /= 1) then
         call build_resonance_histories
      endif

      call newunit(iun)
      open(unit=iun, file='DetailedFlavList.txt', status='unknown')
      write(iun,*) ' ***********   FINAL POWHEG    ***************'
      write(iun,*) ' ***********     BORN          ***************'
      do i=1,flst_nborn
         write(iun,'(a,100i4)')'                  ',int(1:flst_bornlength(i))
         write(iun,'(a,i3,a,100i4)')'flst_born(',i,')   =',flst_born(1:flst_bornlength(i),i)
         write(iun,'(a,i3,a,100i4)')'flst_bornres(',i,')=',flst_bornres(1:flst_bornlength(i),i)
         write(iun,*)
      enddo   
      write(iun,*) 'max flst_bornlength: ',maxval(flst_bornlength(1:flst_nborn))
   


      write(iun,*) ' ***********     REAL          ***************'
      do i=1,flst_nreal
         write(iun,'(a,100i4)')'                  ',int(1:flst_reallength(i))
         write(iun,'(a,i3,a,100i4)')'flst_real(',i,')   =',flst_real(1:flst_reallength(i),i)
         write(iun,'(a,i3,a,100i4)')'flst_realres(',i,')=',flst_realres(1:flst_reallength(i),i)
         write(iun,*)
      enddo   
      write(iun,*) 'max flst_reallength: ',maxval(flst_reallength(1:flst_nreal))

      call buildresgroups(flst_nborn,nlegborn,flst_bornlength,
     1     flst_born,flst_bornres,flst_bornresgroup,flst_nbornresgroup)


      call buildresgroups(flst_nreal,nlegreal,flst_reallength,
     1     flst_real,flst_realres,flst_realresgroup,flst_nrealresgroup)

      write(iun,*) 
      write(iun,*) '*********   SUMMARY  *********'
      write(iun,*) 'set nlegborn to: ',maxval(flst_bornlength(1:flst_nborn))
      write(iun,*) 'set nlegreal to: ',maxval(flst_reallength(1:flst_nreal))
      
      nmaxres_real = maxval(flst_realres(1:flst_reallength(flst_nreal),1:flst_nreal)) - 2
      nmaxres_born = maxval(flst_bornres(1:flst_bornlength(flst_nborn),1:flst_nborn)) - 2
      write(iun,*) 'max number of resonances: ',max(nmaxres_real,nmaxres_born)
      
c     add (-1) for overall azimuthal rotation of the event around the beam axis
      dim_integ = (flst_numfinal+1)*3 - 4 + 2 + max(nmaxres_real,nmaxres_born)
      write(iun,*) 'set ndiminteg to: ',dim_integ 


      write(iun,*) 'flst_nbornresgroup ',flst_nbornresgroup 
      write(iun,*) 'flst_nrealresgroup ',flst_nrealresgroup

      write(iun,*) '*********   END SUMMARY  *********'
      write(iun,*)

      close(iun)


c      do i=1,flst_nborn
c         write(*,*) flst_bornresgroup(i)
c      enddo
c      write(*,*) 'flst_nbornresgroup ',flst_nbornresgroup 
c      do i=1,flst_nreal
c         write(*,*) flst_realresgroup(i)
c      enddo
c      write(*,*) 'flst_nrealresgroup ',flst_nrealresgroup 


cccccccccccccccccccccccccOpenLoops Initcccccccccccccccccccccccccccccccccc

!     relocatable proclib
      if (powheginput("#relativeProclibPath").eq.1) then
         call set_parameter("install_path", "./")
      endif

! increase phase-space tolerance
      call set_parameter("psp_tolerance", 0.1)

! Coupling order: order_ew -> NLO QCD
      call set_parameter("order_ew", res_powew)

! make sure complex mass scheme is switched on
      call set_parameter("use_cms",1)

! select EW input scheme: MZ, MW, alpha (ew_scheme=2, default) VS. MZ, MW, Gmu (ew_scheme=1)
      if (powheginput('#ewscheme')>0) then
        call set_parameter("ew_scheme", powheginput('#ewscheme'))
      end if

! set number of active flavours in aS renormalization:
! if minnf_alphasrun is set to a value larger then the number of massless
! quark flavours, also the corresponding heavy quarks are assumed to contribute
! to the running of the strong coupling (above the respective thresholds).
! see: http://openloops.hepforge.org/parameters.html
      call set_parameter("minnf_alphasrun", st_nlight)

! write stability log files
      if (powheginput('#openloops-stability')>0) then
        call set_parameter("stability_log", 2)
      end if


! Initialize OpenLooos
      call openloops_init
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc

cccccc Here we double the cross section in case channel=2, correct it for
c      the correct branching ratios in case tau leptonic final state is 
c      involved, and set it to the sum of all possible weights in the case
c      of channel=7
      channel = powheginput('#channel')
      if (channel < 0) channel = 0
      brtau2brmu = powheginput('#br_tau/br_mu')
      if (brtau2brmu < 0) brtau2brmu = 0.999270413
      brtau2bre = powheginput('#br_tau/br_e')
      if (brtau2bre < 0) brtau2bre = 0.99926856
      brmu2bre = brtau2bre / brtau2brmu
c        in which case both e-mu+ and e+mu- channels will be generated
c        simply by adding e+mu- channel with the same amplitude 
c        (by flipping a signs in an already generated event with the 
c        probability of 0.5)
      select case (channel)
        case (2)
          rad_branching = 2d0
        case (3)
          rad_branching = brtau2brmu
        case (4)
          rad_branching = brtau2brmu
        case (5)
          rad_branching = brtau2bre
        case (6)
          rad_branching = brtau2bre
        case (7)
          rad_branching = 2d0 + 2d0*brtau2brmu + 2d0*brtau2bre
          rad_branching = rad_branching + 1d0/brmu2bre + brmu2bre
          rad_branching = rad_branching + brtau2brmu*brtau2bre
      end select
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
                
      end





      subroutine init_processes_born
      implicit none
      include "nlegborn.h"
      include "pwhg_flst.h"


      flst_bornlength = 8
c     set the number of final-state paricles at the Born-level WITHOUT any resonance
      flst_numfinal=6


      flst_born(   1,   1)=          -1
      flst_born(   2,   1)=           1
      flst_born(   3,   1)=         -11
      flst_born(   4,   1)=          12
      flst_born(   5,   1)=          13
      flst_born(   6,   1)=         -14
      flst_born(   7,   1)=           5
      flst_born(   8,   1)=          -5

      flst_born(   1,   2)=           1
      flst_born(   2,   2)=          -1
      flst_born(   3,   2)=         -11
      flst_born(   4,   2)=          12
      flst_born(   5,   2)=          13
      flst_born(   6,   2)=         -14
      flst_born(   7,   2)=           5
      flst_born(   8,   2)=          -5
 
      flst_born(   1,   3)=          -2
      flst_born(   2,   3)=           2
      flst_born(   3,   3)=         -11
      flst_born(   4,   3)=          12
      flst_born(   5,   3)=          13
      flst_born(   6,   3)=         -14
      flst_born(   7,   3)=           5
      flst_born(   8,   3)=          -5
 
      flst_born(   1,   4)=           2
      flst_born(   2,   4)=          -2
      flst_born(   3,   4)=         -11
      flst_born(   4,   4)=          12
      flst_born(   5,   4)=          13
      flst_born(   6,   4)=         -14
      flst_born(   7,   4)=           5
      flst_born(   8,   4)=          -5
 
      flst_born(   1,   5)=          -4
      flst_born(   2,   5)=           4
      flst_born(   3,   5)=         -11
      flst_born(   4,   5)=          12
      flst_born(   5,   5)=          13
      flst_born(   6,   5)=         -14
      flst_born(   7,   5)=           5
      flst_born(   8,   5)=          -5
 
      flst_born(   1,   6)=           4
      flst_born(   2,   6)=          -4
      flst_born(   3,   6)=         -11
      flst_born(   4,   6)=          12
      flst_born(   5,   6)=          13
      flst_born(   6,   6)=         -14
      flst_born(   7,   6)=           5
      flst_born(   8,   6)=          -5
 
      flst_born(   1,   7)=          -3
      flst_born(   2,   7)=           3
      flst_born(   3,   7)=         -11
      flst_born(   4,   7)=          12
      flst_born(   5,   7)=          13
      flst_born(   6,   7)=         -14
      flst_born(   7,   7)=           5
      flst_born(   8,   7)=          -5
 
      flst_born(   1,   8)=           3
      flst_born(   2,   8)=          -3
      flst_born(   3,   8)=         -11
      flst_born(   4,   8)=          12
      flst_born(   5,   8)=          13
      flst_born(   6,   8)=         -14
      flst_born(   7,   8)=           5
      flst_born(   8,   8)=          -5
 
      flst_born(   1,   9)=           0
      flst_born(   2,   9)=           0
      flst_born(   3,   9)=         -11
      flst_born(   4,   9)=          12
      flst_born(   5,   9)=          13
      flst_born(   6,   9)=         -14
      flst_born(   7,   9)=           5
      flst_born(   8,   9)=          -5
 
      flst_nborn=           9
 
      end
 
 
 
      subroutine init_processes_real
      implicit none
      include "nlegborn.h"
      include "pwhg_flst.h"
 
      flst_reallength =            9
      flst_real(   1,   1)=          -1
      flst_real(   2,   1)=           1
      flst_real(   3,   1)=         -11
      flst_real(   4,   1)=          12
      flst_real(   5,   1)=          13
      flst_real(   6,   1)=         -14
      flst_real(   7,   1)=           5
      flst_real(   8,   1)=          -5
      flst_real(   9,   1)=           0
 
      flst_real(   1,   2)=          -1
      flst_real(   2,   2)=           0
      flst_real(   3,   2)=         -11
      flst_real(   4,   2)=          12
      flst_real(   5,   2)=          13
      flst_real(   6,   2)=         -14
      flst_real(   7,   2)=           5
      flst_real(   8,   2)=          -5
      flst_real(   9,   2)=          -1
 
      flst_real(   1,   3)=           1
      flst_real(   2,   3)=          -1
      flst_real(   3,   3)=         -11
      flst_real(   4,   3)=          12
      flst_real(   5,   3)=          13
      flst_real(   6,   3)=         -14
      flst_real(   7,   3)=           5
      flst_real(   8,   3)=          -5
      flst_real(   9,   3)=           0
 
      flst_real(   1,   4)=           1
      flst_real(   2,   4)=           0
      flst_real(   3,   4)=         -11
      flst_real(   4,   4)=          12
      flst_real(   5,   4)=          13
      flst_real(   6,   4)=         -14
      flst_real(   7,   4)=           5
      flst_real(   8,   4)=          -5
      flst_real(   9,   4)=           1
 
      flst_real(   1,   5)=          -2
      flst_real(   2,   5)=           2
      flst_real(   3,   5)=         -11
      flst_real(   4,   5)=          12
      flst_real(   5,   5)=          13
      flst_real(   6,   5)=         -14
      flst_real(   7,   5)=           5
      flst_real(   8,   5)=          -5
      flst_real(   9,   5)=           0
 
      flst_real(   1,   6)=          -2
      flst_real(   2,   6)=           0
      flst_real(   3,   6)=         -11
      flst_real(   4,   6)=          12
      flst_real(   5,   6)=          13
      flst_real(   6,   6)=         -14
      flst_real(   7,   6)=           5
      flst_real(   8,   6)=          -5
      flst_real(   9,   6)=          -2
 
      flst_real(   1,   7)=           2
      flst_real(   2,   7)=          -2
      flst_real(   3,   7)=         -11
      flst_real(   4,   7)=          12
      flst_real(   5,   7)=          13
      flst_real(   6,   7)=         -14
      flst_real(   7,   7)=           5
      flst_real(   8,   7)=          -5
      flst_real(   9,   7)=           0
 
      flst_real(   1,   8)=           2
      flst_real(   2,   8)=           0
      flst_real(   3,   8)=         -11
      flst_real(   4,   8)=          12
      flst_real(   5,   8)=          13
      flst_real(   6,   8)=         -14
      flst_real(   7,   8)=           5
      flst_real(   8,   8)=          -5
      flst_real(   9,   8)=           2
 
      flst_real(   1,   9)=          -4
      flst_real(   2,   9)=           4
      flst_real(   3,   9)=         -11
      flst_real(   4,   9)=          12
      flst_real(   5,   9)=          13
      flst_real(   6,   9)=         -14
      flst_real(   7,   9)=           5
      flst_real(   8,   9)=          -5
      flst_real(   9,   9)=           0
 
      flst_real(   1,  10)=          -4
      flst_real(   2,  10)=           0
      flst_real(   3,  10)=         -11
      flst_real(   4,  10)=          12
      flst_real(   5,  10)=          13
      flst_real(   6,  10)=         -14
      flst_real(   7,  10)=           5
      flst_real(   8,  10)=          -5
      flst_real(   9,  10)=          -4
 
      flst_real(   1,  11)=           4
      flst_real(   2,  11)=          -4
      flst_real(   3,  11)=         -11
      flst_real(   4,  11)=          12
      flst_real(   5,  11)=          13
      flst_real(   6,  11)=         -14
      flst_real(   7,  11)=           5
      flst_real(   8,  11)=          -5
      flst_real(   9,  11)=           0
 
      flst_real(   1,  12)=           4
      flst_real(   2,  12)=           0
      flst_real(   3,  12)=         -11
      flst_real(   4,  12)=          12
      flst_real(   5,  12)=          13
      flst_real(   6,  12)=         -14
      flst_real(   7,  12)=           5
      flst_real(   8,  12)=          -5
      flst_real(   9,  12)=           4
 
      flst_real(   1,  13)=          -3
      flst_real(   2,  13)=           3
      flst_real(   3,  13)=         -11
      flst_real(   4,  13)=          12
      flst_real(   5,  13)=          13
      flst_real(   6,  13)=         -14
      flst_real(   7,  13)=           5
      flst_real(   8,  13)=          -5
      flst_real(   9,  13)=           0
 
      flst_real(   1,  14)=          -3
      flst_real(   2,  14)=           0
      flst_real(   3,  14)=         -11
      flst_real(   4,  14)=          12
      flst_real(   5,  14)=          13
      flst_real(   6,  14)=         -14
      flst_real(   7,  14)=           5
      flst_real(   8,  14)=          -5
      flst_real(   9,  14)=          -3
 
      flst_real(   1,  15)=           3
      flst_real(   2,  15)=          -3
      flst_real(   3,  15)=         -11
      flst_real(   4,  15)=          12
      flst_real(   5,  15)=          13
      flst_real(   6,  15)=         -14
      flst_real(   7,  15)=           5
      flst_real(   8,  15)=          -5
      flst_real(   9,  15)=           0
 
      flst_real(   1,  16)=           3
      flst_real(   2,  16)=           0
      flst_real(   3,  16)=         -11
      flst_real(   4,  16)=          12
      flst_real(   5,  16)=          13
      flst_real(   6,  16)=         -14
      flst_real(   7,  16)=           5
      flst_real(   8,  16)=          -5
      flst_real(   9,  16)=           3
 
      flst_real(   1,  17)=           0
      flst_real(   2,  17)=          -1
      flst_real(   3,  17)=         -11
      flst_real(   4,  17)=          12
      flst_real(   5,  17)=          13
      flst_real(   6,  17)=         -14
      flst_real(   7,  17)=           5
      flst_real(   8,  17)=          -5
      flst_real(   9,  17)=          -1
 
      flst_real(   1,  18)=           0
      flst_real(   2,  18)=           1
      flst_real(   3,  18)=         -11
      flst_real(   4,  18)=          12
      flst_real(   5,  18)=          13
      flst_real(   6,  18)=         -14
      flst_real(   7,  18)=           5
      flst_real(   8,  18)=          -5
      flst_real(   9,  18)=           1
 
      flst_real(   1,  19)=           0
      flst_real(   2,  19)=          -2
      flst_real(   3,  19)=         -11
      flst_real(   4,  19)=          12
      flst_real(   5,  19)=          13
      flst_real(   6,  19)=         -14
      flst_real(   7,  19)=           5
      flst_real(   8,  19)=          -5
      flst_real(   9,  19)=          -2
 
      flst_real(   1,  20)=           0
      flst_real(   2,  20)=           2
      flst_real(   3,  20)=         -11
      flst_real(   4,  20)=          12
      flst_real(   5,  20)=          13
      flst_real(   6,  20)=         -14
      flst_real(   7,  20)=           5
      flst_real(   8,  20)=          -5
      flst_real(   9,  20)=           2
 
      flst_real(   1,  21)=           0
      flst_real(   2,  21)=          -4
      flst_real(   3,  21)=         -11
      flst_real(   4,  21)=          12
      flst_real(   5,  21)=          13
      flst_real(   6,  21)=         -14
      flst_real(   7,  21)=           5
      flst_real(   8,  21)=          -5
      flst_real(   9,  21)=          -4
 
      flst_real(   1,  22)=           0
      flst_real(   2,  22)=           4
      flst_real(   3,  22)=         -11
      flst_real(   4,  22)=          12
      flst_real(   5,  22)=          13
      flst_real(   6,  22)=         -14
      flst_real(   7,  22)=           5
      flst_real(   8,  22)=          -5
      flst_real(   9,  22)=           4
 
      flst_real(   1,  23)=           0
      flst_real(   2,  23)=          -3
      flst_real(   3,  23)=         -11
      flst_real(   4,  23)=          12
      flst_real(   5,  23)=          13
      flst_real(   6,  23)=         -14
      flst_real(   7,  23)=           5
      flst_real(   8,  23)=          -5
      flst_real(   9,  23)=          -3
 
      flst_real(   1,  24)=           0
      flst_real(   2,  24)=           3
      flst_real(   3,  24)=         -11
      flst_real(   4,  24)=          12
      flst_real(   5,  24)=          13
      flst_real(   6,  24)=         -14
      flst_real(   7,  24)=           5
      flst_real(   8,  24)=          -5
      flst_real(   9,  24)=           3
 
      flst_real(   1,  25)=           0
      flst_real(   2,  25)=           0
      flst_real(   3,  25)=         -11
      flst_real(   4,  25)=          12
      flst_real(   5,  25)=          13
      flst_real(   6,  25)=         -14
      flst_real(   7,  25)=           5
      flst_real(   8,  25)=          -5
      flst_real(   9,  25)=           0
 
      flst_nreal=          25
 
      return
      end
 
