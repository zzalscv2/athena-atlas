# This file was automatically created by FeynRules 2.3.1
# Mathematica version: 10.1.0  for Mac OS X x86 (64-bit) (March 24, 2015)
# Date: Wed 14 Oct 2015 11:32:28



from object_library import all_parameters, Parameter


from function_library import complexconjugate, re, im, csc, sec, acsc, asec, cot

# This is a default parameter object representing 0.
ZERO = Parameter(name = 'ZERO',
                 nature = 'internal',
                 type = 'real',
                 value = '0.0',
                 texname = '0')

# User-defined parameters.
cabi = Parameter(name = 'cabi',
                 nature = 'external',
                 type = 'real',
                 value = 0.227736,
                 texname = '\\theta _c',
                 lhablock = 'CKMBLOCK',
                 lhacode = [ 1 ])

ReXLut = Parameter(name = 'ReXLut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReX}_{\\text{Lut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 1 ])

ImXLut = Parameter(name = 'ImXLut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImX}_{\\text{Lut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 2 ])

ReXRut = Parameter(name = 'ReXRut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReX}_{\\text{Rut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 3 ])

ImXRut = Parameter(name = 'ImXRut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImX}_{\\text{Rut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 4 ])

ReXLct = Parameter(name = 'ReXLct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReX}_{\\text{Lct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 5 ])

ImXLct = Parameter(name = 'ImXLct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImX}_{\\text{Lct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 6 ])

ReXRct = Parameter(name = 'ReXRct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReX}_{\\text{Rct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 7 ])

ImXRct = Parameter(name = 'ImXRct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImX}_{\\text{Rct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 8 ])

ReKLut = Parameter(name = 'ReKLut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReK}_{\\text{Lut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 9 ])

ImKLut = Parameter(name = 'ImKLut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImK}_{\\text{Lut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 10 ])

ReKRut = Parameter(name = 'ReKRut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReK}_{\\text{Rut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 11 ])

ImKRut = Parameter(name = 'ImKRut',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImK}_{\\text{Rut}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 12 ])

ReKLct = Parameter(name = 'ReKLct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReK}_{\\text{Lct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 13 ])

ImKLct = Parameter(name = 'ImKLct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImK}_{\\text{Lct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 14 ])

ReKRct = Parameter(name = 'ReKRct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ReK}_{\\text{Rct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 15 ])

ImKRct = Parameter(name = 'ImKRct',
                   nature = 'external',
                   type = 'real',
                   value = 0.,
                   texname = '\\text{ImK}_{\\text{Rct}}',
                   lhablock = 'NEWCOUP',
                   lhacode = [ 16 ])

ReZetaLut = Parameter(name = 'ReZetaLut',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ReZeta}_{\\text{Lut}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 17 ])

ImZetaLut = Parameter(name = 'ImZetaLut',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ImZeta}_{\\text{Lut}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 18 ])

ReZetaRut = Parameter(name = 'ReZetaRut',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ReZeta}_{\\text{Rut}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 19 ])

ImZetaRut = Parameter(name = 'ImZetaRut',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ImZeta}_{\\text{Rut}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 20 ])

ReZetaLct = Parameter(name = 'ReZetaLct',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ReZeta}_{\\text{Lct}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 21 ])

ImZetaLct = Parameter(name = 'ImZetaLct',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ImZeta}_{\\text{Lct}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 22 ])

ReZetaRct = Parameter(name = 'ReZetaRct',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ReZeta}_{\\text{Rct}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 23 ])

ImZetaRct = Parameter(name = 'ImZetaRct',
                      nature = 'external',
                      type = 'real',
                      value = 0.,
                      texname = '\\text{ImZeta}_{\\text{Rct}}',
                      lhablock = 'NEWCOUP',
                      lhacode = [ 24 ])

ReEtaLut = Parameter(name = 'ReEtaLut',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ReEta}_{\\text{Lut}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 25 ])

ImEtaLut = Parameter(name = 'ImEtaLut',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ImEta}_{\\text{Lut}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 26 ])

ReEtaRut = Parameter(name = 'ReEtaRut',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ReEta}_{\\text{Rut}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 27 ])

ImEtaRut = Parameter(name = 'ImEtaRut',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ImEta}_{\\text{Rut}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 28 ])

ReEtaLct = Parameter(name = 'ReEtaLct',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ReEta}_{\\text{Lct}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 29 ])

ImEtaLct = Parameter(name = 'ImEtaLct',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ImEta}_{\\text{Lct}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 30 ])

ReEtaRct = Parameter(name = 'ReEtaRct',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ReEta}_{\\text{Rct}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 31 ])

ImEtaRct = Parameter(name = 'ImEtaRct',
                     nature = 'external',
                     type = 'real',
                     value = 0.,
                     texname = '\\text{ImEta}_{\\text{Rct}}',
                     lhablock = 'NEWCOUP',
                     lhacode = [ 32 ])

ReLambdaLut = Parameter(name = 'ReLambdaLut',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ReLambda}_{\\text{Lut}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 33 ])

ImLambdaLut = Parameter(name = 'ImLambdaLut',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ImLambda}_{\\text{Lut}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 34 ])

ReLambdaRut = Parameter(name = 'ReLambdaRut',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ReLambda}_{\\text{Rut}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 35 ])

ImLambdaRut = Parameter(name = 'ImLambdaRut',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ImLambda}_{\\text{Rut}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 36 ])

ReLambdaLct = Parameter(name = 'ReLambdaLct',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ReLambda}_{\\text{Lct}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 37 ])

ImLambdaLct = Parameter(name = 'ImLambdaLct',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ImLambda}_{\\text{Lct}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 38 ])

ReLambdaRct = Parameter(name = 'ReLambdaRct',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ReLambda}_{\\text{Rct}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 39 ])

ImLambdaRct = Parameter(name = 'ImLambdaRct',
                        nature = 'external',
                        type = 'real',
                        value = 0.,
                        texname = '\\text{ImLambda}_{\\text{Rct}}',
                        lhablock = 'NEWCOUP',
                        lhacode = [ 40 ])

aEWM1 = Parameter(name = 'aEWM1',
                  nature = 'external',
                  type = 'real',
                  value = 127.9,
                  texname = '\\text{aEWM1}',
                  lhablock = 'SMINPUTS',
                  lhacode = [ 1 ])

Gf = Parameter(name = 'Gf',
               nature = 'external',
               type = 'real',
               value = 0.0000116637,
               texname = 'G_f',
               lhablock = 'SMINPUTS',
               lhacode = [ 2 ])

aS = Parameter(name = 'aS',
               nature = 'external',
               type = 'real',
               value = 0.1184,
               texname = '\\alpha _s',
               lhablock = 'SMINPUTS',
               lhacode = [ 3 ])

ymdo = Parameter(name = 'ymdo',
                 nature = 'external',
                 type = 'real',
                 value = 0.00504,
                 texname = '\\text{ymdo}',
                 lhablock = 'YUKAWA',
                 lhacode = [ 1 ])

ymup = Parameter(name = 'ymup',
                 nature = 'external',
                 type = 'real',
                 value = 0.00255,
                 texname = '\\text{ymup}',
                 lhablock = 'YUKAWA',
                 lhacode = [ 2 ])

yms = Parameter(name = 'yms',
                nature = 'external',
                type = 'real',
                value = 0.101,
                texname = '\\text{yms}',
                lhablock = 'YUKAWA',
                lhacode = [ 3 ])

ymc = Parameter(name = 'ymc',
                nature = 'external',
                type = 'real',
                value = 1.27,
                texname = '\\text{ymc}',
                lhablock = 'YUKAWA',
                lhacode = [ 4 ])

ymb = Parameter(name = 'ymb',
                nature = 'external',
                type = 'real',
                value = 4.7,
                texname = '\\text{ymb}',
                lhablock = 'YUKAWA',
                lhacode = [ 5 ])

ymt = Parameter(name = 'ymt',
                nature = 'external',
                type = 'real',
                value = 172,
                texname = '\\text{ymt}',
                lhablock = 'YUKAWA',
                lhacode = [ 6 ])

yme = Parameter(name = 'yme',
                nature = 'external',
                type = 'real',
                value = 0.000511,
                texname = '\\text{yme}',
                lhablock = 'YUKAWA',
                lhacode = [ 11 ])

ymm = Parameter(name = 'ymm',
                nature = 'external',
                type = 'real',
                value = 0.10566,
                texname = '\\text{ymm}',
                lhablock = 'YUKAWA',
                lhacode = [ 13 ])

ymtau = Parameter(name = 'ymtau',
                  nature = 'external',
                  type = 'real',
                  value = 1.777,
                  texname = '\\text{ymtau}',
                  lhablock = 'YUKAWA',
                  lhacode = [ 15 ])

MZ = Parameter(name = 'MZ',
               nature = 'external',
               type = 'real',
               value = 91.1876,
               texname = '\\text{MZ}',
               lhablock = 'MASS',
               lhacode = [ 23 ])

Me = Parameter(name = 'Me',
               nature = 'external',
               type = 'real',
               value = 0.000511,
               texname = '\\text{Me}',
               lhablock = 'MASS',
               lhacode = [ 11 ])

MMU = Parameter(name = 'MMU',
                nature = 'external',
                type = 'real',
                value = 0.10566,
                texname = '\\text{MMU}',
                lhablock = 'MASS',
                lhacode = [ 13 ])

MTA = Parameter(name = 'MTA',
                nature = 'external',
                type = 'real',
                value = 1.777,
                texname = '\\text{MTA}',
                lhablock = 'MASS',
                lhacode = [ 15 ])

MU = Parameter(name = 'MU',
               nature = 'external',
               type = 'real',
               value = 0.00255,
               texname = 'M',
               lhablock = 'MASS',
               lhacode = [ 2 ])

MC = Parameter(name = 'MC',
               nature = 'external',
               type = 'real',
               value = 1.27,
               texname = '\\text{MC}',
               lhablock = 'MASS',
               lhacode = [ 4 ])

MT = Parameter(name = 'MT',
               nature = 'external',
               type = 'real',
               value = 172,
               texname = '\\text{MT}',
               lhablock = 'MASS',
               lhacode = [ 6 ])

MD = Parameter(name = 'MD',
               nature = 'external',
               type = 'real',
               value = 0.00504,
               texname = '\\text{MD}',
               lhablock = 'MASS',
               lhacode = [ 1 ])

MS = Parameter(name = 'MS',
               nature = 'external',
               type = 'real',
               value = 0.101,
               texname = '\\text{MS}',
               lhablock = 'MASS',
               lhacode = [ 3 ])

MB = Parameter(name = 'MB',
               nature = 'external',
               type = 'real',
               value = 4.7,
               texname = '\\text{MB}',
               lhablock = 'MASS',
               lhacode = [ 5 ])

MH = Parameter(name = 'MH',
               nature = 'external',
               type = 'real',
               value = 125,
               texname = '\\text{MH}',
               lhablock = 'MASS',
               lhacode = [ 25 ])

WZ = Parameter(name = 'WZ',
               nature = 'external',
               type = 'real',
               value = 2.4952,
               texname = '\\text{WZ}',
               lhablock = 'DECAY',
               lhacode = [ 23 ])

WW = Parameter(name = 'WW',
               nature = 'external',
               type = 'real',
               value = 2.085,
               texname = '\\text{WW}',
               lhablock = 'DECAY',
               lhacode = [ 24 ])

WT = Parameter(name = 'WT',
               nature = 'external',
               type = 'real',
               value = 1.50833649,
               texname = '\\text{WT}',
               lhablock = 'DECAY',
               lhacode = [ 6 ])

WH = Parameter(name = 'WH',
               nature = 'external',
               type = 'real',
               value = 0.00407,
               texname = '\\text{WH}',
               lhablock = 'DECAY',
               lhacode = [ 25 ])

aEW = Parameter(name = 'aEW',
                nature = 'internal',
                type = 'real',
                value = '1/aEWM1',
                texname = '\\alpha _{\\text{EW}}')

G = Parameter(name = 'G',
              nature = 'internal',
              type = 'real',
              value = '2*cmath.sqrt(aS)*cmath.sqrt(cmath.pi)',
              texname = 'G')

CKM1x1 = Parameter(name = 'CKM1x1',
                   nature = 'internal',
                   type = 'complex',
                   value = 'cmath.cos(cabi)',
                   texname = '\\text{CKM1x1}')

CKM1x2 = Parameter(name = 'CKM1x2',
                   nature = 'internal',
                   type = 'complex',
                   value = 'cmath.sin(cabi)',
                   texname = '\\text{CKM1x2}')

CKM1x3 = Parameter(name = 'CKM1x3',
                   nature = 'internal',
                   type = 'complex',
                   value = '0',
                   texname = '\\text{CKM1x3}')

CKM2x1 = Parameter(name = 'CKM2x1',
                   nature = 'internal',
                   type = 'complex',
                   value = '-cmath.sin(cabi)',
                   texname = '\\text{CKM2x1}')

CKM2x2 = Parameter(name = 'CKM2x2',
                   nature = 'internal',
                   type = 'complex',
                   value = 'cmath.cos(cabi)',
                   texname = '\\text{CKM2x2}')

CKM2x3 = Parameter(name = 'CKM2x3',
                   nature = 'internal',
                   type = 'complex',
                   value = '0',
                   texname = '\\text{CKM2x3}')

CKM3x1 = Parameter(name = 'CKM3x1',
                   nature = 'internal',
                   type = 'complex',
                   value = '0',
                   texname = '\\text{CKM3x1}')

CKM3x2 = Parameter(name = 'CKM3x2',
                   nature = 'internal',
                   type = 'complex',
                   value = '0',
                   texname = '\\text{CKM3x2}')

CKM3x3 = Parameter(name = 'CKM3x3',
                   nature = 'internal',
                   type = 'complex',
                   value = '1',
                   texname = '\\text{CKM3x3}')

MW = Parameter(name = 'MW',
               nature = 'internal',
               type = 'real',
               value = 'cmath.sqrt(MZ**2/2. + cmath.sqrt(MZ**4/4. - (aEW*cmath.pi*MZ**2)/(Gf*cmath.sqrt(2))))',
               texname = 'M_W')

ee = Parameter(name = 'ee',
               nature = 'internal',
               type = 'real',
               value = '2*cmath.sqrt(aEW)*cmath.sqrt(cmath.pi)',
               texname = 'e')

sw2 = Parameter(name = 'sw2',
                nature = 'internal',
                type = 'real',
                value = '1 - MW**2/MZ**2',
                texname = '\\text{sw2}')

cw = Parameter(name = 'cw',
               nature = 'internal',
               type = 'real',
               value = 'cmath.sqrt(1 - sw2)',
               texname = 'c_w')

sw = Parameter(name = 'sw',
               nature = 'internal',
               type = 'real',
               value = 'cmath.sqrt(sw2)',
               texname = 's_w')

g1 = Parameter(name = 'g1',
               nature = 'internal',
               type = 'real',
               value = 'ee/cw',
               texname = 'g_1')

gw = Parameter(name = 'gw',
               nature = 'internal',
               type = 'real',
               value = 'ee/sw',
               texname = 'g_w')

vev = Parameter(name = 'vev',
                nature = 'internal',
                type = 'real',
                value = '(2*MW*sw)/ee',
                texname = '\\text{vev}')

lam = Parameter(name = 'lam',
                nature = 'internal',
                type = 'real',
                value = 'MH**2/(2.*vev**2)',
                texname = '\\text{lam}')

yb = Parameter(name = 'yb',
               nature = 'internal',
               type = 'real',
               value = '(ymb*cmath.sqrt(2))/vev',
               texname = '\\text{yb}')

yc = Parameter(name = 'yc',
               nature = 'internal',
               type = 'real',
               value = '(ymc*cmath.sqrt(2))/vev',
               texname = '\\text{yc}')

ydo = Parameter(name = 'ydo',
                nature = 'internal',
                type = 'real',
                value = '(ymdo*cmath.sqrt(2))/vev',
                texname = '\\text{ydo}')

ye = Parameter(name = 'ye',
               nature = 'internal',
               type = 'real',
               value = '(yme*cmath.sqrt(2))/vev',
               texname = '\\text{ye}')

ym = Parameter(name = 'ym',
               nature = 'internal',
               type = 'real',
               value = '(ymm*cmath.sqrt(2))/vev',
               texname = '\\text{ym}')

ys = Parameter(name = 'ys',
               nature = 'internal',
               type = 'real',
               value = '(yms*cmath.sqrt(2))/vev',
               texname = '\\text{ys}')

yt = Parameter(name = 'yt',
               nature = 'internal',
               type = 'real',
               value = '(ymt*cmath.sqrt(2))/vev',
               texname = '\\text{yt}')

ytau = Parameter(name = 'ytau',
                 nature = 'internal',
                 type = 'real',
                 value = '(ymtau*cmath.sqrt(2))/vev',
                 texname = '\\text{ytau}')

yup = Parameter(name = 'yup',
                nature = 'internal',
                type = 'real',
                value = '(ymup*cmath.sqrt(2))/vev',
                texname = '\\text{yup}')

muH = Parameter(name = 'muH',
                nature = 'internal',
                type = 'real',
                value = 'cmath.sqrt(lam*vev**2)',
                texname = '\\mu')

