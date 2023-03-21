#Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import csv
import sys
import os
import datetime
import shutil
sys.path.append("../common")
from parseMapping import *

def fix_result_file():
  if not os.path.exists("template.dat"):
    create_reference()
  fix_module_id()
  count_bad_fittings()
  module_list = recover_zero()
  all_recover = copy_from_ref()
  sort_module(module_list, all_recover)
  create_summary()
  #check_diff()


#------------- recovery equations ---------------------------------------------
def avg(a, b):
  return 1/2*(a + b)

# recovery equation for the threshold (use values in same FE)
def recover_thr(nth_FE, pixel_type, value_type, recover_list, data):
  cure = data[nth_FE][pixel_type][value_type]
  # bring value from same FEs
  if data[nth_FE][pixel_type][value_type] == 0 and data[nth_FE]["normal"][value_type] + data[nth_FE]["long"][value_type] + data[nth_FE]["ganged"][value_type] >= 6000:
    if pixel_type == "normal":
      cure = avg(data[nth_FE]["long"][value_type], data[nth_FE]["ganged"][value_type])
    if pixel_type == "long":
      cure = avg(data[nth_FE]["normal"][value_type], data[nth_FE]["ganged"][value_type])
    if pixel_type == "ganged":
      cure = avg(data[nth_FE]["long"][value_type], data[nth_FE]["normal"][value_type])
  # bring value from different FEs
  if cure == 0:
    if recover_list == []:
      cure = 0.0
    else:
      cure = sum(recover_list)/len(recover_list)
  return round(cure, 1)

# recovery equation for other than the threshold (use values in different FEs)
def recover_diff(nth_FE, pixel_type, parameter, recover_list, data):
  cure = data[nth_FE][pixel_type][parameter]
  if cure == 0 or cure == -28284.3:
    if recover_list == []:
      cure = 0.0
    else:
      cure = sum(recover_list)/len(recover_list)
  return float('%.6g' % cure)


#---------------- Create reference file -------------
def create_reference():
  template = open('template.dat', 'w')
  sort_rec = open("PixCalib-DATA-RUN2-UPD4-21.log","r")
  contents_sort = sort_rec.read()
  elements_sort = contents_sort.splitlines()
  last3 = len(elements_sort)
  head3 = 0
  n_new = 0
  print(last3)
  sort_dict = {}
  golden_dict = {}
  #imported function reading the csv in common
  module_dict = create_mapping('../common/mapping.csv')
  #python 3 syntax for unpacking keys to a list
  golden_modules=[*module_dict]

  # seek the latest run
  run_numbers = []
  while head3 < last3:
    line3 = elements_sort[head3]
    if not line3.find("[") == 0:
      head3 += 1
      continue
    # seek the tail of the block
    tail3 = head3 + 1

    while tail3 < last3:
      tmp3 = elements_sort[tail3]
      if tmp3.find("[") == 0 or tail3 == last3:
        break
      else:
        tail3 += 1

    # the head line ( e.g. [400506,0] - [2147483647,4294967295) (1) [data (String4k) : -4,0,1,0 )
    first_module = elements_sort[head3].split()[3].split('(')[1].split(')')[0]
    if first_module == '0':
        run_number = elements_sort[head3].split()[0].split(',')[0].split('[')[1]
        run_numbers.append(int(run_number))
    head3 = tail3
  latest_run = max(run_numbers)

  # create reference file
  head3 = 0
  while head3 < last3:
    line3 = elements_sort[head3]
    # seek the head of the blocks
    if not line3.find("[") == 0:
      head3 += 1
      continue
    # seek the tail of the block
    tail3 = head3 + 1

    while tail3 < last3:
      tmp3 = elements_sort[tail3]
      if tmp3.find("[") == 0 or tail3 == last3:
        break
      else:
        tail3 += 1

    # the head line ( e.g. [400506,0] - [2147483647,4294967295) (1) [data (String4k) : -4,0,1,0 )
    get_latest_history = elements_sort[head3].split()[0]
    if get_latest_history == '[' + str(latest_run) + ',0]':
      module_num = elements_sort[head3].split()[3].split('(')[1].split(')')[0]
      module_name = golden_modules[int(module_num)]
      print(module_name)
      template.write(module_name)
      template.write('\n')
      for i in range(tail3 - head3 - 2):
        template.write(elements_sort[head3 + i + 1])
        template.write('\n')


  #  print( "processing module", module3 )
    head3 = tail3



#------------- Count number of bad fits by summary file ----------------------------

def count_bad_fittings():
  summary   = open(path_to_summary, "r+")
  contents_sum = summary.read()
  elements_sum = contents_sum.splitlines()
  lastsum = len(elements_sum)
  headsum = 0

  #print(summary_line)
  while headsum < lastsum:
    linesum = elements_sum[headsum]
    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not linesum.find("L") == 0 and not linesum.find("D") == 0:
      headsum += 1
      continue
    # seek the tail of the block
    tailsum = headsum + 1

    while tailsum < lastsum:
      tmpsum = elements_sum[tailsum]
      if tmpsum.find("L") == 0 or tmpsum.find("D") == 0 or tailsum == lastsum:
        break
      else:
        tailsum += 1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    modulesum = elements_sum[headsum]

    for i in range(tailsum - headsum -1):
      if elements_sum[headsum + i + 1 ] != "[ ]":
        if modulesum[0] != "D":
          bad_num[modulesum[:2]] += 1
        else:
          bad_num["Disk"] += 1

    headsum = tailsum
  summary.close()

#---------------- Fix ID for 3D Module -------------
def fix_module_id():
  golden_modules = []
  golden_hash = []
  #imported function
  module_dict = create_mapping('../common/mapping.csv')
  #produce a list of keys (module names)
  for (k,v) in module_dict.items():
    golden_modules.append(k)
    #produces string of format ': 2035 2,2,47,0'
    golden_hash.append(convertToOldFormat(v))
  golden_dict = {}
  for i, item in enumerate(golden_modules):
    golden_dict[item] = i

  sort_rec = open(input_data,"r")
  contents_sort = sort_rec.read()
  elements_sort = contents_sort.splitlines()
  last3 = len(elements_sort)
  head3 = 0
  n_new = 0

  outputput = open(input_data, "w")

  sort_dict = {}
  while head3 < last3:
    line3 = elements_sort[head3]
    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not line3.find("L") == 0 and not line3.find("D") == 0:
      head3 += 1
      continue
    # seek the tail of the block
    tail3 = head3 + 1

    while tail3 < last3:
      tmp3 = elements_sort[tail3]
      if tmp3.find("L") == 0 or tmp3.find("D") == 0 or tail3 == last3:
        break
      else:
        tail3 += 1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    module3 = elements_sort[head3].split(":")[0].strip()
    elements_sort[head3] = module3
    try:
      sort_num = golden_dict[module3]
      for i in range(tail3-head3):
        outputput.write( elements_sort[head3 + i] + "\n" if i==0 else elements_sort[head3 + i] + "\n")
    except:
        outputput.write(elements_sort[head3] + "_1" + "\n" )
        outputput.write(elements_sort[head3 + 1] + "\n")
        outputput.write(elements_sort[head3] + "_2" + "\n" )
        outputput.write("I0 " + elements_sort[head3 + 2][3:] + "\n")

  #  print( "processing module", module3 )
    head3 = tail3

  outputput.close()



#------------- recover the partially "0" component ----------------------------
def recover_zero():
  test_data = open(input_data, "r")
  result    = open("partial_recover.dat","w+")
  contents_part = test_data.read()
  test_data.close()
  elements_part = contents_part.splitlines()

  para_lists = {
    "normal" : { "threshold" : [], "sigma" : [], "noise" : [], "intime" : [] },
    "long"   : { "threshold" : [], "sigma" : [], "noise" : [], "intime" : [] },
    "ganged" : { "threshold" : [], "sigma" : [], "noise" : [], "intime" : [] },
    "fit_normal" : { "A" : [], "E" : [], "C" : [] },
    "fit_longGanged" : { "A" : [], "E" : [], "C" : [] },
    "quality/unused" : {"fit_quality" : [], "unused" : [] }
  }

  recover_list = []
  module_list = []

  # parsing data file
  last = len(elements_part)
  head = 0
  module_num = 0

  n_line = 0

  print(last)

  while head < last:
    line = elements_part[head]
    result.write(elements_part[head])
    result.write("\n")

    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not line.find("L") == 0 and not line.find("D") == 0:
      head += 1
      continue
    # seek the tail of the block
    tail = head + 1
    while tail < last:
      tmp = elements_part[tail]
      if tmp.find("L") == 0 or tmp.find("D") == 0 or tail == last:
        break
      else:
        tail += 1

    module_num +=1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    module = elements_part[head]
    print( "processing module", module )
    module_list.append(module)

    # rawBlock is the block lines, corresponding to each Pixel Module
    # for IBL, there are either 2 or 1 FEs
    # else, there are 16 FEs
    rowBlock = elements_part[head+1:tail]

    # split each line of the block
    splitted = [ line.split() for line in rowBlock ]
    # number of FEs in a module
    n_FEs = len(splitted)

    # Structure variables adequately into the json format
    data = [ { "FE" : s[0],
               "normal" : { "threshold" : float(s[1]), "sigma" : float(s[2]),  "noise" : float(s[3]),  "intime" : float(s[4])  },
               "long"   : { "threshold" : float(s[5]), "sigma" : float(s[6]),  "noise" : float(s[7]),  "intime" : float(s[8])  },
               "ganged" : { "threshold" : float(s[9]), "sigma" : float(s[10]), "noise" : float(s[11]), "intime" : float(s[12]) },
               "fit_normal" : { "A" : float(s[13]), "E" : float(s[14]), "C" : float(s[15]) },
               "fit_longGanged" : { "A" : float(s[16]), "E" : float(s[17]), "C" : float(s[18]) },
               "quality/unused" : {"fit_quality" : float(s[19]), "unused" : float(s[20]) } }
               for s in splitted ]

    for i in range(len(data)):
      summary_text = ""
      check_fe = 0
      n_insert = head + i + 1

      pixtypes = data[i].keys()
      for pixtype in pixtypes:
        if pixtype == "FE":
          continue
        else:
          parameters = data[i][pixtype].keys()
          for parameter in parameters:
            if data[i][pixtype][parameter] == 0 or data[i][pixtype][parameter] == -28284.3:
              check_fe = check_fe + 1
              summary_text = summary_text + pixtype + " " + parameter + ", "
              error_num[pixtype + " " + parameter] = error_num[pixtype + " " + parameter] + 1

  #    summary_line[n_insert] = "I" + str(i) + ": [  ], " + summary_line[n_insert]
      if check_fe == 20:
        summary_line[n_insert] = "I" + str(i) + ": [ Missing all values for this FE. ], " + summary_line[n_insert]
        error_num["Number of FEs with all values zero"] += 1
      else:
        summary_line[n_insert] = "I" + str(i) + ": [ " + summary_text[:-2] + " ], " + summary_line[n_insert]



    # Make lists to recover dead channel using different FEs
    # normal threshold, sigma, noise, intime-thr
    normal_thr    = [data[i]["normal"]["threshold"] for i in range(n_FEs) if data[i]["normal"]["threshold"] != 0]
    normal_sigma  = [data[i]["normal"]["sigma"]     for i in range(n_FEs) if data[i]["normal"]["sigma"] != 0]
    normal_noise  = [data[i]["normal"]["noise"]     for i in range(n_FEs) if data[i]["normal"]["noise"] != 0]
    normal_intime = [data[i]["normal"]["intime"]    for i in range(n_FEs) if data[i]["normal"]["intime"] != 0]
    # long threshold, sigma, noise, intime-thr
    long_thr    = [data[i]["long"]["threshold"] for i in range(n_FEs) if data[i]["long"]["threshold"] != 0]
    long_sigma  = [data[i]["long"]["sigma"]     for i in range(n_FEs) if data[i]["long"]["sigma"] != 0]
    long_noise  = [data[i]["long"]["noise"]     for i in range(n_FEs) if data[i]["long"]["noise"] != 0]
    long_intime = [data[i]["long"]["intime"]    for i in range(n_FEs) if data[i]["long"]["intime"] != 0]
    # ganged threshold, sigma, noise, intime-thr
    ganged_thr    = [data[i]["ganged"]["threshold"]  for i in range(n_FEs) if data[i]["ganged"]["threshold"] != 0]
    ganged_sigma  = [data[i]["ganged"]["sigma"]      for i in range(n_FEs) if data[i]["ganged"]["sigma"] != 0]
    ganged_noise  = [data[i]["ganged"]["noise"]      for i in range(n_FEs) if data[i]["ganged"]["noise"] != 0 ]
    ganged_intime = [data[i]["ganged"]["intime"]     for i in range(n_FEs) if data[i]["ganged"]["intime"] != 0]
    # normal parameter A, B, C
    normal_A = [data[i]["fit_normal"]["A"] for i in range(n_FEs) if data[i]["fit_normal"]["A"] != 0]
    normal_B = [data[i]["fit_normal"]["E"] for i in range(n_FEs) if data[i]["fit_normal"]["E"] != 0 and data[i]["fit_normal"]["E"] != -28284.3]
    normal_C = [data[i]["fit_normal"]["C"] for i in range(n_FEs) if data[i]["fit_normal"]["C"] != 0]
    # long/ganged parameter A, B, C
    longGanged_A = [data[i]["fit_longGanged"]["A"] for i in range(n_FEs) if data[i]["fit_longGanged"]["A"] != 0]
    longGanged_B = [data[i]["fit_longGanged"]["E"] for i in range(n_FEs) if data[i]["fit_longGanged"]["E"] != 0 and data[i]["fit_longGanged"]["E"] != -28284.3]
    longGanged_C = [data[i]["fit_longGanged"]["C"] for i in range(n_FEs) if data[i]["fit_longGanged"]["C"] != 0]
    # fit quality/unused
    qty    = [data[i]["quality/unused"]["fit_quality"] for i in range(n_FEs) if data[i]["quality/unused"]["fit_quality"] != 0]
    unused = [data[i]["quality/unused"]["unused"]      for i in range(n_FEs) if data[i]["quality/unused"]["unused"] != 0]


    # recovered data list
    data_recover_part = [
       [splitted[i][0],
       recover_thr(i, "normal", "threshold", normal_thr, data), recover_diff(i, "normal", "sigma", normal_sigma, data), recover_diff(i, "normal", "noise", normal_noise, data), recover_diff(i, "normal", "intime", normal_intime, data),
       recover_thr(i, "long", "threshold", long_thr, data),     recover_diff(i, "long", "sigma", long_sigma, data),     recover_diff(i, "long", "noise", long_noise, data),     recover_diff(i, "long", "intime", long_intime, data),
       recover_thr(i, "ganged", "threshold", ganged_thr, data), recover_diff(i, "ganged", "sigma", ganged_sigma, data), recover_diff(i, "ganged", "noise", ganged_noise, data), recover_diff(i, "ganged", "intime", ganged_intime, data),
       recover_diff(i, "fit_normal", "A", normal_A, data),         recover_diff(i, "fit_normal", "E", normal_B, data),         recover_diff(i, "fit_normal", "C", normal_C, data),
       recover_diff(i, "fit_longGanged", "A", longGanged_A, data), recover_diff(i, "fit_longGanged", "E", longGanged_B, data), recover_diff(i, "fit_longGanged", "C", longGanged_C, data),
       recover_diff(i, "quality/unused", "fit_quality", qty, data), recover_diff(i, "quality/unused", "unused", unused, data)]
     for i in range(len(splitted))]

    recover_list.append(data_recover_part)

    # write recovered data to file
    writer = csv.writer(result, delimiter = " ")
    writer.writerows(data_recover_part)


    # Make parameter lists of modules for reccovering modules with all values "0"
    # normal threshold, sigma, noise, intime-threshold
    para_lists["normal"]["threshold"].append([recover_thr(i, "normal", "threshold", normal_thr, data) for i in range(len(splitted))])
    para_lists["normal"]["sigma"].append([recover_thr(i, "normal", "sigma", normal_sigma, data) for i in range(len(splitted))])
    para_lists["normal"]["noise"].append([recover_diff(i, "normal", "noise", normal_noise, data) for i in range(len(splitted))])
    para_lists["normal"]["intime"].append([recover_diff(i, "normal", "intime", normal_intime, data) for i in range(len(splitted))])
    # long threshold, sigma, noise, intime-threshold
    para_lists["long"]["threshold"].append([recover_thr(i, "long", "threshold", long_thr, data) for i in range(len(splitted))])
    para_lists["long"]["sigma"].append([recover_thr(i, "long", "sigma", long_sigma, data) for i in range(len(splitted))])
    para_lists["long"]["noise"].append([recover_diff(i, "long", "noise", long_noise, data) for i in range(len(splitted))])
    para_lists["long"]["intime"].append([recover_diff(i, "long", "intime", long_intime, data) for i in range(len(splitted))])
    # ganged threshold, sigma, noise, intime-threshold
    para_lists["ganged"]["threshold"].append([recover_thr(i, "ganged", "threshold", ganged_thr, data) for i in range(len(splitted))])
    para_lists["ganged"]["sigma"].append([recover_thr(i, "ganged", "sigma", ganged_sigma, data) for i in range(len(splitted))])
    para_lists["ganged"]["noise"].append([recover_diff(i, "ganged", "noise", ganged_noise, data) for i in range(len(splitted))])
    para_lists["ganged"]["intime"].append([recover_diff(i, "ganged", "intime", ganged_intime, data) for i in range(len(splitted))])
    # normal pixel fit parameters
    para_lists["fit_normal"]["A"].append([recover_diff(i, "fit_normal", "A", normal_A, data) for i in range(len(splitted))])
    para_lists["fit_normal"]["E"].append([recover_diff(i, "fit_normal", "E", normal_B, data) for i in range(len(splitted))])
    para_lists["fit_normal"]["C"].append([recover_diff(i, "fit_normal", "C", normal_C, data) for i in range(len(splitted))])
    # long/ganged pixel fit parameters
    para_lists["fit_longGanged"]["A"].append([recover_diff(i, "fit_longGanged", "A", longGanged_A, data) for i in range(len(splitted))])
    para_lists["fit_longGanged"]["E"].append([recover_diff(i, "fit_longGanged", "E", longGanged_B, data) for i in range(len(splitted))])
    para_lists["fit_longGanged"]["C"].append([recover_diff(i, "fit_longGanged", "C", longGanged_C, data) for i in range(len(splitted))])
    # fit quality/unused
    para_lists["quality/unused"]["fit_quality"].append([recover_diff(i, "quality/unused", "fit_quality", qty, data) for i in range(len(splitted))])
    para_lists["quality/unused"]["unused"].append([recover_diff(i, "quality/unused", "unused", unused, data) for i in range(len(splitted))])

    head = tail

  result.close()

  return module_list

  # end of while head < last


#------------------- recover the dead & missing module ------------------------

#....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

def diffmodule(data_diff, n_FE, pixel_type, para_type, data2):
  recover = data2[n_FE][pixel_type][para_type]
  if recover == 0.0 or recover == -28284.3:
    recover = data_diff[n_FE][pixel_type][para_type]
#    recover = 0
  return float('%.6g' % recover)

#....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......


def copy_from_ref():
  all_recover  = open(output_data, "w")
  partial_rec  = open("partial_recover.dat","r")
  contents_all = partial_rec.read()
  elements_all = contents_all.splitlines()


  # parsing data file
  last2 = len(elements_all)
  head2 = 0

  print(len(elements_all))

  while head2 < last2:
    line2 = elements_all[head2]
    all_recover.write(elements_all[head2])
    all_recover.write("\n")
    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not line2.find("L") == 0 and not line2.find("D") == 0:
      head2 += 1
      continue

    # seek the tail of the block
    tail2 = head2 + 1
    while tail2 < last2:
      tmp2 = elements_all[tail2]
      if tmp2.find("L") == 0 or tmp2.find("D") == 0 or tail2 == last2:
        break
      else:
        tail2 += 1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    module2 = elements_all[head2]
    print( "processing module", module2 )

    # rawBlock is the block lines, corresponding to each Pixel Module
    # for IBL, there are either 2 or 1 FEs
    # else, there are 16 FEs
    rowBlock2 = elements_all[head2+1:tail2]

    # split each line of the block
    splitted2 = [ line2.split() for line2 in rowBlock2 ]
    # number of FEs in a module
    n_FEs2 = len(splitted2)

    # Structure variables adequately into the json format
    data2 = [ { "FE" : s[0],
               "normal" : { "threshold" : float(s[1]), "sigma" : float(s[2]),  "noise" : float(s[3]),  "intime" : float(s[4])  },
               "long"   : { "threshold" : float(s[5]), "sigma" : float(s[6]),  "noise" : float(s[7]),  "intime" : float(s[8])  },
               "ganged" : { "threshold" : float(s[9]), "sigma" : float(s[10]), "noise" : float(s[11]), "intime" : float(s[12]) },
               "fit_normal" : { "A" : float(s[13]), "B" : float(s[14]), "C" : float(s[15]) },
               "fit_longGanged" : { "A" : float(s[16]), "B" : float(s[17]), "C" : float(s[18]) },
               "quality/unused" : {"fit_quality" : float(s[19]), "unused" : float(s[20]) } }
               for s in splitted2]

    diff_rec  = open("template.dat","r")
    contents_diff = diff_rec.read()
    elements_diff = contents_diff.splitlines()
    # parsing data file
    last_diff = len(elements_diff)
    head_diff = 0
    while head_diff < last_diff:
      line_diff = elements_diff[head_diff]
      # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
      if not line_diff.find("L") == 0 and not line_diff.find("D") == 0:
        head_diff += 1
        continue
      # seek the tail of the block
      tail_diff = head_diff + 1

      while tail_diff < last_diff:
        tmp_diff = elements_diff[tail_diff]
        if tmp_diff.find("L") == 0 or tmp_diff.find("D") == 0 or tail_diff == last_diff:
          break
        else:
          tail_diff += 1

      # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
      module_diff = elements_diff[head_diff].split()[0]
      if module_diff == module2:
        rowBlock_diff = elements_diff[head_diff+1:tail_diff]
        # split each line of the block
        splitted_diff = [ line_diff.split() for line_diff in rowBlock_diff ]
        data_diff = [ { "FE" : s[0],
                        "normal" : { "threshold" : float(s[1]), "sigma" : float(s[2]),  "noise" : float(s[3]),  "intime" : float(s[4])  },
                        "long"   : { "threshold" : float(s[5]), "sigma" : float(s[6]),  "noise" : float(s[7]),  "intime" : float(s[8])  },
                        "ganged" : { "threshold" : float(s[9]), "sigma" : float(s[10]), "noise" : float(s[11]), "intime" : float(s[12]) },
                        "fit_normal" : { "A" : float(s[13]), "B" : float(s[14]), "C" : float(s[15]) },
                        "fit_longGanged" : { "A" : float(s[16]), "B" : float(s[17]), "C" : float(s[18]) },
                        "quality/unused" : {"fit_quality" : float(s[19]), "unused" : float(s[20]) } }
                        for s in splitted_diff]
        break
      head_diff = tail_diff

    data_recover_all = [[splitted2[i][0],
                      int(diffmodule(data_diff, i, "normal", "threshold", data2)), int(diffmodule(data_diff, i, "normal", "sigma", data2)), int(diffmodule(data_diff, i, "normal", "noise", data2)), int(diffmodule(data_diff, i, "normal", "intime", data2)),
                      int(diffmodule(data_diff, i, "long", "threshold", data2)),   int(diffmodule(data_diff, i, "long", "sigma", data2)),   int(diffmodule(data_diff, i, "long", "noise", data2)),   int(diffmodule(data_diff, i, "long", "intime", data2)),
                      int(diffmodule(data_diff, i, "ganged", "threshold", data2)), int(diffmodule(data_diff, i, "ganged", "sigma", data2)), int(diffmodule(data_diff, i, "ganged", "noise", data2)), int(diffmodule(data_diff, i, "ganged", "intime", data2)),
                      '{:.6g}'.format(diffmodule(data_diff, i, "fit_normal", "A", data2)),     '{:.6g}'.format(diffmodule(data_diff, i, "fit_normal", "B", data2)),     '{:.6g}'.format(diffmodule(data_diff, i, "fit_normal", "C", data2)),
                      '{:.6g}'.format(diffmodule(data_diff, i, "fit_longGanged", "A", data2)), '{:.6g}'.format(diffmodule(data_diff, i, "fit_longGanged", "B", data2)), '{:.6g}'.format(diffmodule(data_diff, i, "fit_longGanged", "C", data2)),
                      diffmodule(data_diff, i, "quality/unused", "fit_quality", data2), diffmodule(data_diff, i, "quality/unused", "unused", data2)]
                      for i in range(len(splitted2))]

    n_insert = head2 - 8 - 6
    #summary_line[n_insert] = module2 + ": \n" #+ ": [ Parameters were recovered by using previous scan ] \n"

    # write recovered data to file
    writer = csv.writer(all_recover, delimiter = " ")
    writer.writerows(data_recover_all)

    head2 = tail2
  return all_recover

  # end of while head2 < last2


#------------- sort modules  ----------------------------------------

def read_header_and_get_macro(header_filepath):
  with open(header_filepath, mode='r') as f:
    lst = [s.strip() for s in f.readlines()]
  comment_flg = False
  for l in lst:
    items = l.split()
    if len(items) != 0 and items[0] == "/*":
      comment_flg = True
      continue
    if comment_flg == True:
      if len(items) != 0 and items[0] == "*/":
        comment_flg = False
      continue
    if len(items) < 2 or items[0] != "#define":
      continue
    if items[1] in globals():
      if items[2] == 'true':
        globals()[items[1]] = True
      elif items[2] == 'false':
        globals()[items[1]] = False
      else:
        try:
          globals()[items[1]] = float(items[2])
        except ValueError:
          try:
            globals()[items[1]] = int(items[2])
          except ValueError:
            globals()[items[1]] = items[2]

def sort_module(module_list, all_recover):
  golden_modules = []
  golden_hash = []
   #imported function reading the csv in common
  module_dict = create_mapping('../common/mapping.csv')
  for (k,v) in module_dict.items():
    golden_modules.append(k)
    #produces string of format ': 2035 2,2,47,0'
    golden_hash.append(convertToOldFormat(v))
  
  list_sa = list(set(golden_modules) - set(module_list))

  all_recover.close()

  copy_rec  = open("template.dat","r")
  contents_copy = copy_rec.read()
  elements_copy = contents_copy.splitlines()

  last3 = len(elements_copy)
  head3 = 0
  n_new = 0

  while head3 < last3:
    line3 = elements_copy[head3]
    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not line3.find("L") == 0 and not line3.find("D") == 0:
      head3 += 1
      continue
    # seek the tail of the block
    tail3 = head3 + 1

    while tail3 < last3:
      tmp3 = elements_copy[tail3]
      if tmp3.find("L") == 0 or tmp3.find("D") == 0 or tail3 == last3:
        break
      else:
        tail3 += 1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    module3 = elements_copy[head3].split(":")[0].strip()
    print( "processing module", module3 )
    if module3 in list_sa:
      with open(output_data, "a") as f:
        for i in range(tail3 - head3):
          f.writelines(elements_copy[head3 + i] + "\n")
      summary_line.append(module3 + ": [ Scans were not performed for this module and copied from previous scan ]\n")
      if module3[0] != "D":
        not_scan[module3[:2]] += 1
      else:
        not_scan["Disk"] += 1
    else:
      print("The result for this module exists")
    head3 = tail3


  golden_dict = {}
  for i, item in enumerate(golden_modules):
    golden_dict[item] = i

  sort_rec = open(output_data,"r")
  contents_sort = sort_rec.read()
  elements_sort = contents_sort.splitlines()
  last3 = len(elements_sort)
  head3 = 0
  n_new = 0

  sort_dict = {}
  while head3 < last3:
    line3 = elements_sort[head3]
    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not line3.find("L") == 0 and not line3.find("D") == 0:
      head3 += 1
      continue
    # seek the tail of the block
    tail3 = head3 + 1

    while tail3 < last3:
      tmp3 = elements_sort[tail3]
      if tmp3.find("L") == 0 or tmp3.find("D") == 0 or tail3 == last3:
        break
      else:
        tail3 += 1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    module3 = elements_sort[head3].split(":")[0].strip()
    elements_sort[head3] = module3
    sort_num = golden_dict[module3]
    sort_dict[sort_num] = [ elements_sort[head3 + i] + golden_hash[sort_num] + "\n" if i==0 else elements_sort[head3 + i] + "\n" for i in range(tail3-head3)]

    print( "processing module", module3 )
    head3 = tail3

  print("sorting in hash ID...")
  with open(output_data, "w") as f:
    for i in range(len(golden_modules)):
      for item in sort_dict[i]:
        f.writelines(item)


#------------- Create summary file  ----------------------------------------

def create_summary():
  template_summary = [
    "##===================================================================================\n",
    "## This file shows the way to recover data loss and calibration failure. \n",
    "## \n",
    "## How to recover parameters:\n",
    "## 1. If there is a data loss in output data:\n",
    "##   - Case1: Partial loss in a module \n",
    "##     - Threshold: Recover using average of same FE\n",
    "##     - Others:    Recover using average of different FEs\n",
    "## \n",
    "##   - Case2: All values loss in a module\n",
    "##     - Previous scan result is used for the recovery \n",
    "## \n",
    "## \n",
    "## 2. If there is a calibration failure:\n",
    "##   - Remove incorrect injected charge & refitting\n",
    "##   - Removed charges are listed in order of deletion\n",
    "## \n",
    "## \n",
    "## \n",
    "## Example of an output for a module: \n",
    "##   L0_B08_S1_A6_M2A: \n"
    "##   I2: [ normal threshold ], [ ] <--- Parameters that become '0' is listed \n",
    "##   I8: [ ], [ 30000 40000 ] <-------- Injected charges that removed is listed \n"
    "## \n"
    "## \n"
    "## Mapping convention for FE-I3 in the calibration.\n",
    "##     ^ \n",
    "##  phi| \n",
    "##  320| \n",
    "##     | FE15 FE14 FE13 ... FE9 FE8 \n",
    "##  160| \n",
    "##     | FE0  FE1  FE2  ... FE6 FE7 \n",
    "##     | \n",
    "##    0+------------------------------> \n",
    "##     0    18   36     ...   126 144  \n",
    "##                                  eta \n",
    "## \n"
    "##==================================================================================\n",
    "\n",
    "\n"
    ]




  template_summary.append("~~~~~~~~ Summary for the recovery ~~~~~~~~ \n")
  template_summary.append("1. Number of parameters that were 0\n")
  for parameter in error_num:
    template_summary.append(parameter + ": " + str(error_num[parameter]) + "\n")
  template_summary.append("\n")

  template_summary.append("2. Number of FEs that had bad fits\n")
  template_summary.append("IBL: " + str(bad_num["LI"]) + "\n")
  template_summary.append("B-Layer: " + str(bad_num["L0"]) + "\n")
  template_summary.append("Layer1: " + str(bad_num["L1"]) + "\n")
  template_summary.append("Layer2: " + str(bad_num["L2"]) + "\n")
  template_summary.append("Disk: " + str(bad_num["Disk"]) + "\n")
  template_summary.append("\n")

  template_summary.append("3. Number of modules that were not scanned\n")
  template_summary.append("IBL: " + str(not_scan["LI"]) + "\n")
  template_summary.append("B-Layer: " + str(not_scan["L0"]) + "\n")
  template_summary.append("Layer1: " + str(not_scan["L1"]) + "\n")
  template_summary.append("Layer2: " + str(not_scan["L2"]) + "\n")
  template_summary.append("Disk: " + str(not_scan["Disk"]) + "\n")

  template_summary.append("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n" + "\n")

  new_summary_line = []

  for item in summary_line:
    try:
  #    str(item.split(":")[1])
      if item.split(":")[1] != " [  ], [ ]\n":
        new_summary_line.append(item)
    except:
      new_summary_line.append(item)

  new_summary_line = template_summary + new_summary_line

  with open(path_to_summary, mode='w') as f:
    f.writelines(new_summary_line)

  # delete partial_recover.dat
  os.remove("partial_recover.dat")

  #print('number of "0" =', error_num)
  print( "processing done." )


#------------- Check difference from a previous scan  ----------------------------------------

def funcTot(a, e, c, Q):
  ret = 9.9e10
  num = Q + e
  denom = Q + c
  if denom != 0.0:
    ret = a*(num/denom)
  return ret

def check_diff():
  result = open(output_data, "r")
  ref = open("template.dat" , "r")
  conf = open("conf.dat", "w")

  contents_result = result.read()
  elements_result = contents_result.splitlines()
  contents_ref = ref.read()
  elements_ref = contents_ref.splitlines()


  # parsing data file
  last2 = len(elements_result)
  head2 = 0
  charges = [6000, 20000, 30000]

  print(len(elements_result))

  while head2 < last2:
    line2 = elements_result[head2]
    # seek the head of the blocks ("L" : IBL, BLayer, L1L2. "D" : disk)
    if not line2.find("L") == 0 and not line2.find("D") == 0:
      head2 += 1
      continue

    # seek the tail of the block
    tail2 = head2 + 1
    while tail2 < last2:
      tmp2 = elements_result[tail2]
      if tmp2.find("L") == 0 or tmp2.find("D") == 0 or tail2 == last2:
        break
      else:
        tail2 += 1

    # the head line is the module name ( e.g. L0_B01_S2_A7_M2A )
    module2 = elements_result[head2]
    print( "processing module", module2 )

    # rawBlock is the block lines, corresponding to each Pixel Module
    # for IBL, there are either 2 or 1 FEs
    # else, there are 16 FEs
    rowBlock2 = elements_result[head2+1:tail2]

    # split each line of the block
    splitted2 = [ line2.split() for line2 in rowBlock2 ]
    # number of FEs in a module
    n_FEs2 = len(splitted2)

    # Structure variables adequately into the json format
    data_result = [ { "FE" : s[0],
               "normal" : { "threshold" : float(s[1]), "sigma" : float(s[2]),  "noise" : float(s[3]),  "intime" : float(s[4])  },
               "long"   : { "threshold" : float(s[5]), "sigma" : float(s[6]),  "noise" : float(s[7]),  "intime" : float(s[8])  },
               "ganged" : { "threshold" : float(s[9]), "sigma" : float(s[10]), "noise" : float(s[11]), "intime" : float(s[12]) },
               "fit_normal" : { "A" : float(s[13]), "B" : float(s[14]), "C" : float(s[15]) },
               "fit_longGanged" : { "A" : float(s[16]), "B" : float(s[17]), "C" : float(s[18]) },
               "quality/unused" : {"fit_quality" : float(s[19]), "unused" : float(s[20]) } }
               for s in splitted2]

    rowBlock_ref = elements_ref[head2+1:tail2]
    splitted_ref = [ line2.split() for line2 in rowBlock_ref ]

    data_ref = [ { "FE" : s[0],
               "normal" : { "threshold" : float(s[1]), "sigma" : float(s[2]),  "noise" : float(s[3]),  "intime" : float(s[4])  },
               "long"   : { "threshold" : float(s[5]), "sigma" : float(s[6]),  "noise" : float(s[7]),  "intime" : float(s[8])  },
               "ganged" : { "threshold" : float(s[9]), "sigma" : float(s[10]), "noise" : float(s[11]), "intime" : float(s[12]) },
               "fit_normal" : { "A" : float(s[13]), "B" : float(s[14]), "C" : float(s[15]) },
               "fit_longGanged" : { "A" : float(s[16]), "B" : float(s[17]), "C" : float(s[18]) },
               "quality/unused" : {"fit_quality" : float(s[19]), "unused" : float(s[20]) } }
               for s in splitted_ref]

    for i in range(len(data_result)):
      diff = []
      for charge in charges:
        result_tot = funcTot(data_result[i]["fit_normal"]["A"], data_result[i]["fit_normal"]["B"], data_result[i]["fit_normal"]["C"], charge)
        ref_tot = funcTot(data_ref[i]["fit_normal"]["A"], data_ref[i]["fit_normal"]["B"], data_ref[i]["fit_normal"]["C"], charge)
        if ref_tot != 0:
          diff.append(abs(result_tot - ref_tot))
      if diff and "L0" in module2:
        if max(diff) != 0:
          print(max(diff))
          conf.write(str(max(diff)) + "\n")

          if max(diff) > 2:
            print("hoge"+ module2 + " " + str(i))

    head2 = tail2


#------------- main ----------------------------------------

if __name__ == "__main__":
  input_data  = sys.argv[1]
  output_data = sys.argv[2]
  today_date = datetime.datetime.now()
  today = today_date.strftime( '%Y%m%d' )
  path_to_summary = "./" + today + "_CalibSummary.txt"
  summary   = open(path_to_summary, "r+")
  summary_line = summary.readlines()
  error_num   = {
    "Number of FEs with all values zero": 0,
    "normal threshold": 0, "normal sigma": 0, "normal noise": 0, "normal intime": 0,
    "long threshold": 0,   "long sigma": 0,   "long noise": 0,   "long intime": 0,
    "ganged threshold": 0, "ganged sigma": 0, "ganged noise": 0, "ganged intime": 0,
    "fit_normal A": 0,      "fit_normal E": 0,      "fit_normal C": 0,
    "fit_longGanged A": 0, "fit_longGanged E": 0, "fit_longGanged C": 0,
    "quality/unused fit_quality": 0,"quality/unused unused": 0
  }

  bad_num = {
    "LI": 0,
    "L0": 0,
    "L1": 0,
    "L2": 0,
    "Disk": 0
  }
  not_scan = {
    "LI": 0,
    "L0": 0,
    "L1": 0,
    "L2": 0,
    "Disk": 0
  }

  ### Remove interrupting line
  sort_rec = open(input_data,"r")
  contents_sort = sort_rec.read()
  elements_sort = contents_sort.splitlines()
  template = open(input_data, 'w')
  for item in elements_sort:
      if '*********' in item:
          continue
      elif 'FUNCTION' in item:
          continue
      elif 'VERIFY' in item:
          continue
      else:
          template.write(item)
          template.write('\n')

  template.close()

  fix_result_file()

  ### re-write reference file
  shutil.copyfile(output_data, "template.dat")
