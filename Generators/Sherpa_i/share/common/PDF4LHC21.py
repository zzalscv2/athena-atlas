evgenConfig.tune = "PDF4LHC21"

import os
if not os.environ["SHERPAVER"].startswith('3.'):

  raise Exception("Please use the NNPDF3.0nnlo base fragment for Sherpa v2.")

## Nominal PDF settings
genSeq.Sherpa_i.BaseFragment += """
PDF_LIBRARY: LHAPDFSherpa
USE_PDF_ALPHAS: 1
PDF_SET: PDF4LHC21_40_pdfas
"""

## Enable scale and PDF variations by default
genSeq.Sherpa_i.BaseFragment += """
PDF_VARIATIONS:
- PDF4LHC21_40_pdfas*
- MSHT20nnlo_as118
- CT18NNLO_as_0118
- NNPDF31_nnlo_as_0118_hessian
- NNPDF40_nnlo_as_01180_hessian
- CT18ANNLO
- CT18XNNLO
- CT18ZNNLO
"""

