# This file was automatically created by FeynRules 2.0.21
# Mathematica version: 8.0 for Mac OS X x86 (64-bit) (October 5, 2011)
# Date: Mon 5 Jan 2015 18:13:09


from __future__ import division
from object_library import all_particles, Particle
import parameters as Param

import propagators as Prop

ve = Particle(pdg_code = 12,
              name = 've',
              antiname = 've~',
              spin = 2,
              color = 1,
              mass = Param.ZERO,
              width = Param.ZERO,
              texname = 've',
              antitexname = 've~',
              charge = 0,
              GhostNumber = 0)

ve__tilde__ = ve.anti()

vm = Particle(pdg_code = 14,
              name = 'vm',
              antiname = 'vm~',
              spin = 2,
              color = 1,
              mass = Param.ZERO,
              width = Param.ZERO,
              texname = 'vm',
              antitexname = 'vm~',
              charge = 0,
              GhostNumber = 0)

vm__tilde__ = vm.anti()

vt = Particle(pdg_code = 16,
              name = 'vt',
              antiname = 'vt~',
              spin = 2,
              color = 1,
              mass = Param.ZERO,
              width = Param.ZERO,
              texname = 'vt',
              antitexname = 'vt~',
              charge = 0,
              GhostNumber = 0)

vt__tilde__ = vt.anti()

e__minus__ = Particle(pdg_code = 11,
                      name = 'e-',
                      antiname = 'e+',
                      spin = 2,
                      color = 1,
                      mass = Param.Me,
                      width = Param.ZERO,
                      texname = 'e-',
                      antitexname = 'e+',
                      charge = -1,
                      GhostNumber = 0)

e__plus__ = e__minus__.anti()

mu__minus__ = Particle(pdg_code = 13,
                       name = 'mu-',
                       antiname = 'mu+',
                       spin = 2,
                       color = 1,
                       mass = Param.MM,
                       width = Param.ZERO,
                       texname = 'mu-',
                       antitexname = 'mu+',
                       charge = -1,
                       GhostNumber = 0)

mu__plus__ = mu__minus__.anti()

ta__minus__ = Particle(pdg_code = 15,
                       name = 'ta-',
                       antiname = 'ta+',
                       spin = 2,
                       color = 1,
                       mass = Param.MTA,
                       width = Param.ZERO,
                       texname = 'ta-',
                       antitexname = 'ta+',
                       charge = -1,
                       GhostNumber = 0)

ta__plus__ = ta__minus__.anti()

u = Particle(pdg_code = 2,
             name = 'u',
             antiname = 'u~',
             spin = 2,
             color = 3,
             mass = Param.MU,
             width = Param.ZERO,
             texname = 'u',
             antitexname = 'u~',
             charge = 2/3,
             GhostNumber = 0)

u__tilde__ = u.anti()

c = Particle(pdg_code = 4,
             name = 'c',
             antiname = 'c~',
             spin = 2,
             color = 3,
             mass = Param.MC,
             width = Param.ZERO,
             texname = 'c',
             antitexname = 'c~',
             charge = 2/3,
             GhostNumber = 0)

c__tilde__ = c.anti()

t = Particle(pdg_code = 6,
             name = 't',
             antiname = 't~',
             spin = 2,
             color = 3,
             mass = Param.MT,
             width = Param.WT,
             texname = 't',
             antitexname = 't~',
             charge = 2/3,
             GhostNumber = 0)

t__tilde__ = t.anti()

d = Particle(pdg_code = 1,
             name = 'd',
             antiname = 'd~',
             spin = 2,
             color = 3,
             mass = Param.MD,
             width = Param.ZERO,
             texname = 'd',
             antitexname = 'd~',
             charge = -1/3,
             GhostNumber = 0)

d__tilde__ = d.anti()

s = Particle(pdg_code = 3,
             name = 's',
             antiname = 's~',
             spin = 2,
             color = 3,
             mass = Param.MS,
             width = Param.ZERO,
             texname = 's',
             antitexname = 's~',
             charge = -1/3,
             GhostNumber = 0)

s__tilde__ = s.anti()

b = Particle(pdg_code = 5,
             name = 'b',
             antiname = 'b~',
             spin = 2,
             color = 3,
             mass = Param.MB,
             width = Param.ZERO,
             texname = 'b',
             antitexname = 'b~',
             charge = -1/3,
             GhostNumber = 0)

b__tilde__ = b.anti()

ghA = Particle(pdg_code = 9000001,
               name = 'ghA',
               antiname = 'ghA~',
               spin = -1,
               color = 1,
               mass = Param.ZERO,
               width = Param.ZERO,
               texname = 'ghA',
               antitexname = 'ghA~',
               charge = 0,
               GhostNumber = 1)

ghA__tilde__ = ghA.anti()

ghZ = Particle(pdg_code = 9000002,
               name = 'ghZ',
               antiname = 'ghZ~',
               spin = -1,
               color = 1,
               mass = Param.MZ,
               width = Param.ZERO,
               texname = 'ghZ',
               antitexname = 'ghZ~',
               charge = 0,
               GhostNumber = 1)

ghZ__tilde__ = ghZ.anti()

ghWp = Particle(pdg_code = 9000003,
                name = 'ghWp',
                antiname = 'ghWp~',
                spin = -1,
                color = 1,
                mass = Param.MW,
                width = Param.ZERO,
                texname = 'ghWp',
                antitexname = 'ghWp~',
                charge = 1,
                GhostNumber = 1)

ghWp__tilde__ = ghWp.anti()

ghWm = Particle(pdg_code = 9000004,
                name = 'ghWm',
                antiname = 'ghWm~',
                spin = -1,
                color = 1,
                mass = Param.MW,
                width = Param.ZERO,
                texname = 'ghWm',
                antitexname = 'ghWm~',
                charge = -1,
                GhostNumber = 1)

ghWm__tilde__ = ghWm.anti()

ghG = Particle(pdg_code = 9000005,
               name = 'ghG',
               antiname = 'ghG~',
               spin = -1,
               color = 8,
               mass = Param.ZERO,
               width = Param.ZERO,
               texname = 'ghG',
               antitexname = 'ghG~',
               charge = 0,
               GhostNumber = 1)

ghG__tilde__ = ghG.anti()

A = Particle(pdg_code = 22,
             name = 'A',
             antiname = 'A',
             spin = 3,
             color = 1,
             mass = Param.ZERO,
             width = Param.ZERO,
             texname = 'A',
             antitexname = 'A',
             charge = 0,
             GhostNumber = 0)

Z = Particle(pdg_code = 23,
             name = 'Z',
             antiname = 'Z',
             spin = 3,
             color = 1,
             mass = Param.MZ,
             width = Param.WZ,
             texname = 'Z',
             antitexname = 'Z',
             charge = 0,
             GhostNumber = 0)

W__plus__ = Particle(pdg_code = 24,
                     name = 'W+',
                     antiname = 'W-',
                     spin = 3,
                     color = 1,
                     mass = Param.MW,
                     width = Param.WW,
                     texname = 'W+',
                     antitexname = 'W-',
                     charge = 1,
                     GhostNumber = 0)

W__minus__ = W__plus__.anti()

G = Particle(pdg_code = 21,
             name = 'G',
             antiname = 'G',
             spin = 3,
             color = 8,
             mass = Param.ZERO,
             width = Param.ZERO,
             texname = 'G',
             antitexname = 'G',
             charge = 0,
             GhostNumber = 0)

H = Particle(pdg_code = 25,
             name = 'H',
             antiname = 'H',
             spin = 1,
             color = 1,
             mass = Param.MH,
             width = Param.WH,
             texname = '\\phi',
             antitexname = '\\phi',
             charge = 0,
             GhostNumber = 0)

phi0 = Particle(pdg_code = 250,
                name = 'phi0',
                antiname = 'phi0',
                spin = 1,
                color = 1,
                mass = Param.MZ,
                width = Param.ZERO,
                texname = 'phi0',
                antitexname = 'phi0',
                goldstone = True,
                charge = 0,
                GhostNumber = 0)

phi__plus__ = Particle(pdg_code = 251,
                       name = 'phi+',
                       antiname = 'phi-',
                       spin = 1,
                       color = 1,
                       mass = Param.MW,
                       width = Param.ZERO,
                       texname = '\\phi^+',
                       antitexname = '\\phi^-',
                       goldstone = True,
                       charge = 1,
                       GhostNumber = 0)

phi__minus__ = phi__plus__.anti()

xchi = Particle(pdg_code = 3000001,
                name = 'xchi',
                antiname = 'xchi',
                spin = 2,
                color = 1,
                mass = Param.Mxchi,
                width = Param.Wxchi,
                texname = 'xchi',
                antitexname = 'xchi',
                charge = 0,
                GhostNumber = 0)

sd = Particle(pdg_code = 1000001,
              name = 'sd',
              antiname = 'sd~',
              spin = 1,
              color = 3,
              mass = Param.Msd,
              width = Param.Wsd,
              texname = 'sd',
              antitexname = 'sd~',
              charge = -1/3,
              GhostNumber = 0)

sd__tilde__ = sd.anti()

ss = Particle(pdg_code = 1000003,
              name = 'ss',
              antiname = 'ss~',
              spin = 1,
              color = 3,
              mass = Param.Mss,
              width = Param.Wss,
              texname = 'ss',
              antitexname = 'ss~',
              charge = -1/3,
              GhostNumber = 0)

ss__tilde__ = ss.anti()

sb = Particle(pdg_code = 1000005,
              name = 'sb',
              antiname = 'sb~',
              spin = 1,
              color = 3,
              mass = Param.Msb,
              width = Param.Wsb,
              texname = 'sb',
              antitexname = 'sb~',
              charge = -1/3,
              GhostNumber = 0)

sb__tilde__ = sb.anti()

se = Particle(pdg_code = 1000011,
              name = 'se',
              antiname = 'se~',
              spin = 1,
              color = 1,
              mass = Param.Mse,
              width = Param.Wse,
              texname = 'se',
              antitexname = 'se~',
              charge = -1,
              GhostNumber = 0)

se__tilde__ = se.anti()

sm = Particle(pdg_code = 1000013,
              name = 'sm',
              antiname = 'sm~',
              spin = 1,
              color = 1,
              mass = Param.Msm,
              width = Param.Wsm,
              texname = 'sm',
              antitexname = 'sm~',
              charge = -1,
              GhostNumber = 0)

sm__tilde__ = sm.anti()

sta = Particle(pdg_code = 1000015,
               name = 'sta',
               antiname = 'sta~',
               spin = 1,
               color = 1,
               mass = Param.Msta,
               width = Param.Wsta,
               texname = 'sta',
               antitexname = 'sta~',
               charge = -1,
               GhostNumber = 0)

sta__tilde__ = sta.anti()

