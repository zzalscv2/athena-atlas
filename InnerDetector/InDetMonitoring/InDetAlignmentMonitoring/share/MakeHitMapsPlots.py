#
for i in range(nFiles):    

    PIXBAR_HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "pixels", "BAR", True, 0, "measurements")
    outputFileName =  outputDir+"/"+"PIXBAR_HitMaps_File_"+str(i)+"."+oFext
    outputFileName =  outputDir+"/"+"PIXBAR_HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(PIXBAR_HitMaps, outputFileName, "#eta ring", "#phi stave", "Hits", 
                0.2, 0.96, "#mum",canvasText,makeOutput)

    PIXECA_HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "pixels", "ECA")
    outputFileName =  outputDir+"/"+"PIXECA_HitMaps_File_"+str(i)+"."+oFext
    outputFileName =  outputDir+"/"+"PIXECA_HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(PIXECA_HitMaps, outputFileName, "", "", "Hits", 
                0.2, 0.96, "#mum",canvasText,makeOutput,"PIX","ECA")

    PIXECC_HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "pixels", "ECC")
    outputFileName =  outputDir+"/"+"PIXECC_HitMaps_File_"+str(i)+"."+oFext
    outputFileName =  outputDir+"/"+"PIXECC_HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")     
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(PIXECC_HitMaps, outputFileName, "", "", "Hits", 
                0.2, 0.96, "#mum",canvasText,makeOutput,"PIX","ECC")

    # SCT barrel side 0 and side 1
    SCTBAR_s0HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "SCT", "BAR", True, 0)
    outputFileName =  outputDir+"/"+"SCTBAR_s0HitMaps_File_"+str(i)+"."+oFext
    outputFileName =  outputDir+"/"+"SCTBAR_s0HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(SCTBAR_s0HitMaps, outputFileName, "#eta ring", "#phi sector", "Hits", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","BAR")

    SCTBAR_s1HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "sct", "BAR", True, 1)
    outputFileName =  outputDir+"/"+"SCTBAR_s1HitMaps_File_"+str(i)+"."+oFext
    outputFileName =  outputDir+"/"+"SCTBAR_s1HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(SCTBAR_s1HitMaps, outputFileName, "#eta ring", "#phi sector", "Hits", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","BAR")

    # SCT ECA side 0 and sie 1
    SCTECA_s0HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "sct", "ECA", True, 0)
    outputFileName = outputDir+"/"+"SCTECA_s0HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(SCTECA_s0HitMaps, outputFileName, "", "", "Hits", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","ECA")

    SCTECA_s1HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "sct", "ECA", True, 1)
    outputFileName = outputDir+"/"+"SCTECA_s1HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(SCTECA_s1HitMaps, outputFileName, "", "", "Hits", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","ECA")

    # SCT ECC side 0 and sie 1
    outputFileName = outputDir+"/"+"SCTECC_s0HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    SCTECC_s0HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "sct", "ECC", True, 0)
    DrawHitMaps(SCTECC_s0HitMaps, outputFileName, "", "", "Hits", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","ECA")

    outputFileName = outputDir+"/"+"SCTECC_s1HitMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    SCTECC_s1HitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "sct", "ECC", True, 1)
    DrawHitMaps(SCTECC_s1HitMaps, outputFileName, "", "", "Hits", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","ECA")

    #
    # HIT EFFICIENCY PER MODULE AND PLANE
    #
    # SCT barrel side 0 and side 1
    SCTBAR_s0HitEffMaps = MakeHitEffMaps(hitEffDir, legendTitles, rootFiles, i, "SCT", "BAR", True, 0)
    outputFileName =  outputDir+"/"+"SCTBAR_s0HitEffMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(SCTBAR_s0HitEffMaps, outputFileName, "#eta ring", "#phi sector", "Hit eff.", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","BAR", 3)

    SCTBAR_s1HitEffMaps = MakeHitEffMaps(hitEffDir, legendTitles, rootFiles, i, "sct", "BAR", True, 1)
    outputFileName =  outputDir+"/"+"SCTBAR_s1HitEffMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(SCTBAR_s1HitEffMaps, outputFileName, "#eta ring", "#phi sector", "Hit eff.", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"SCT","BAR", 3)

    # pixel barrel hit efficiency map
    PIXBAR_HitEffMaps = MakeHitEffMaps(hitEffDir, legendTitles, rootFiles, i, "PIX", "BAR", True, 0)
    outputFileName =  outputDir+"/"+"PIXBAR_HitEffMaps_File_"+legendTitles[i]+"."+oFext
    outputFileName = outputFileName.replace(" ","_")    
    outputFileName = outputFileName.replace("(","_")    
    outputFileName = outputFileName.replace(")","_")    
    DrawHitMaps(PIXBAR_HitEffMaps, outputFileName, "#eta ring", "#phi sector", "Hit eff.", 
                0.2, 0.96, "#mum",canvasText, makeOutput,"PIX","BAR", 3)

    if (userExtended):
        print " <MakeHitMapsPlots> Extended = True "
        PIXBAR_ExpectedHitMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "pixels", "BAR", True, 0, "hits")
        outputFileName =  outputDir+"/"+"PIXBAR_ExpectedHitMaps_File_"+str(i)+"."+oFext
        outputFileName =  outputDir+"/"+"PIXBAR_ExpectedHitMaps_File_"+legendTitles[i]+"."+oFext
        outputFileName = outputFileName.replace(" ","_")    
        outputFileName = outputFileName.replace("(","_")    
        outputFileName = outputFileName.replace(")","_")    
        DrawHitMaps(PIXBAR_ExpectedHitMaps, outputFileName, "#eta ring", "#phi stave", "Expected hits", 
                    0.2, 0.96, "#mum",canvasText,makeOutput)
        
        PIXBAR_HoleMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "pixels", "BAR", True, 0, "holes")
        outputFileName =  outputDir+"/"+"PIXBAR_HoleMaps_File_"+str(i)+"."+oFext
        outputFileName =  outputDir+"/"+"PIXBAR_HoleMaps_File_"+legendTitles[i]+"."+oFext
        outputFileName = outputFileName.replace(" ","_")    
        outputFileName = outputFileName.replace("(","_")    
        outputFileName = outputFileName.replace(")","_")    
        DrawHitMaps(PIXBAR_HoleMaps, outputFileName, "#eta ring", "#phi stave", "Holes", 
                    0.2, 0.96, "#mum",canvasText,makeOutput)
        
        PIXBAR_OutliersMaps = MakeHitMaps(hitEffDir, legendTitles, rootFiles, i, "pixels", "BAR", True, 0, "outliers")
        outputFileName =  outputDir+"/"+"PIXBAR_OutlierMaps_File_"+str(i)+"."+oFext
        outputFileName =  outputDir+"/"+"PIXBAR_OutlierMaps_File_"+legendTitles[i]+"."+oFext
        outputFileName = outputFileName.replace(" ","_")    
        outputFileName = outputFileName.replace("(","_")    
        outputFileName = outputFileName.replace(")","_")    
        DrawHitMaps(PIXBAR_OutliersMaps, outputFileName, "#eta ring", "#phi stave", "Outliers", 
                    0.2, 0.96, "#mum",canvasText,makeOutput)


# reset the plots style
execfile("AtlasStyle.py")
