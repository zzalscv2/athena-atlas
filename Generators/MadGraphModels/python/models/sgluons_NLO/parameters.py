# This file was automatically created by FeynRules 2.1.78
# Mathematica version: 9.0 for Mac OS X x86 (64-bit) (January 24, 2013)
# Date: Fri 17 Oct 2014 08:59:37



from object_library import all_parameters, Parameter


from function_library import complexconjugate, re, im, csc, sec, acsc, asec, cot

# This is a default parameter object representing 0.
ZERO = Parameter(name = 'ZERO',
                 nature = 'internal',
                 type = 'real',
                 value = '0.0',
                 texname = '0')

# This is a default parameter object representing the renormalization scale (MU_R).
MU_R = Parameter(name = 'MU_R',
                 nature = 'external',
                 type = 'real',
                 value = 91.188,
                 texname = '\\text{\\mu_r}',
                 lhablock = 'LOOP',
                 lhacode = [1])

# User-defined parameters.
g8g = Parameter(name = 'g8g',
                nature = 'external',
                type = 'real',
                value = 1.e-6,
                texname = 'g_{\\text{8g}}',
                lhablock = 'NPG8G',
                lhacode = [ 1 ])

g8Ld1x1 = Parameter(name = 'g8Ld1x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld1x1}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 1, 1 ])

g8Ld1x2 = Parameter(name = 'g8Ld1x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld1x2}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 1, 2 ])

g8Ld1x3 = Parameter(name = 'g8Ld1x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld1x3}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 1, 3 ])

g8Ld2x1 = Parameter(name = 'g8Ld2x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld2x1}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 2, 1 ])

g8Ld2x2 = Parameter(name = 'g8Ld2x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld2x2}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 2, 2 ])

g8Ld2x3 = Parameter(name = 'g8Ld2x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld2x3}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 2, 3 ])

g8Ld3x1 = Parameter(name = 'g8Ld3x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld3x1}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 3, 1 ])

g8Ld3x2 = Parameter(name = 'g8Ld3x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld3x2}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 3, 2 ])

g8Ld3x3 = Parameter(name = 'g8Ld3x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ld3x3}',
                    lhablock = 'NPG8LD',
                    lhacode = [ 3, 3 ])

g8Lu1x1 = Parameter(name = 'g8Lu1x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu1x1}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 1, 1 ])

g8Lu1x2 = Parameter(name = 'g8Lu1x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu1x2}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 1, 2 ])

g8Lu1x3 = Parameter(name = 'g8Lu1x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu1x3}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 1, 3 ])

g8Lu2x1 = Parameter(name = 'g8Lu2x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu2x1}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 2, 1 ])

g8Lu2x2 = Parameter(name = 'g8Lu2x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu2x2}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 2, 2 ])

g8Lu2x3 = Parameter(name = 'g8Lu2x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu2x3}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 2, 3 ])

g8Lu3x1 = Parameter(name = 'g8Lu3x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu3x1}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 3, 1 ])

g8Lu3x2 = Parameter(name = 'g8Lu3x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu3x2}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 3, 2 ])

g8Lu3x3 = Parameter(name = 'g8Lu3x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Lu3x3}',
                    lhablock = 'NPG8LU',
                    lhacode = [ 3, 3 ])

g8Rd1x1 = Parameter(name = 'g8Rd1x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd1x1}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 1, 1 ])

g8Rd1x2 = Parameter(name = 'g8Rd1x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd1x2}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 1, 2 ])

g8Rd1x3 = Parameter(name = 'g8Rd1x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd1x3}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 1, 3 ])

g8Rd2x1 = Parameter(name = 'g8Rd2x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd2x1}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 2, 1 ])

g8Rd2x2 = Parameter(name = 'g8Rd2x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd2x2}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 2, 2 ])

g8Rd2x3 = Parameter(name = 'g8Rd2x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd2x3}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 2, 3 ])

g8Rd3x1 = Parameter(name = 'g8Rd3x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd3x1}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 3, 1 ])

g8Rd3x2 = Parameter(name = 'g8Rd3x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd3x2}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 3, 2 ])

g8Rd3x3 = Parameter(name = 'g8Rd3x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Rd3x3}',
                    lhablock = 'NPG8RD',
                    lhacode = [ 3, 3 ])

g8Ru1x1 = Parameter(name = 'g8Ru1x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru1x1}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 1, 1 ])

g8Ru1x2 = Parameter(name = 'g8Ru1x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru1x2}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 1, 2 ])

g8Ru1x3 = Parameter(name = 'g8Ru1x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru1x3}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 1, 3 ])

g8Ru2x1 = Parameter(name = 'g8Ru2x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru2x1}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 2, 1 ])

g8Ru2x2 = Parameter(name = 'g8Ru2x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru2x2}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 2, 2 ])

g8Ru2x3 = Parameter(name = 'g8Ru2x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru2x3}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 2, 3 ])

g8Ru3x1 = Parameter(name = 'g8Ru3x1',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru3x1}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 3, 1 ])

g8Ru3x2 = Parameter(name = 'g8Ru3x2',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru3x2}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 3, 2 ])

g8Ru3x3 = Parameter(name = 'g8Ru3x3',
                    nature = 'external',
                    type = 'real',
                    value = 0.001,
                    texname = '\\text{g8Ru3x3}',
                    lhablock = 'NPG8RU',
                    lhacode = [ 3, 3 ])

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

MTA = Parameter(name = 'MTA',
                nature = 'external',
                type = 'real',
                value = 1.777,
                texname = '\\text{MTA}',
                lhablock = 'MASS',
                lhacode = [ 15 ])

MT = Parameter(name = 'MT',
               nature = 'external',
               type = 'real',
               value = 172,
               texname = '\\text{MT}',
               lhablock = 'MASS',
               lhacode = [ 6 ])

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

m8 = Parameter(name = 'm8',
               nature = 'external',
               type = 'real',
               value = 400,
               texname = '\\text{m8}',
               lhablock = 'MASS',
               lhacode = [ 9000001 ])

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

w8 = Parameter(name = 'w8',
               nature = 'external',
               type = 'real',
               value = 10,
               texname = '\\text{w8}',
               lhablock = 'DECAY',
               lhacode = [ 9000001 ])

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

muH = Parameter(name = 'muH',
                nature = 'internal',
                type = 'real',
                value = 'cmath.sqrt(lam*vev**2)',
                texname = '\\mu')

I1a33 = Parameter(name = 'I1a33',
                  nature = 'internal',
                  type = 'complex',
                  value = 'yb',
                  texname = '\\text{I1a33}')

I2a33 = Parameter(name = 'I2a33',
                  nature = 'internal',
                  type = 'complex',
                  value = 'yt',
                  texname = '\\text{I2a33}')

I3a33 = Parameter(name = 'I3a33',
                  nature = 'internal',
                  type = 'complex',
                  value = 'yt',
                  texname = '\\text{I3a33}')

I4a33 = Parameter(name = 'I4a33',
                  nature = 'internal',
                  type = 'complex',
                  value = 'yb',
                  texname = '\\text{I4a33}')

