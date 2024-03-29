/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCOMBINEPATTERNTOOLS_MUONCOMBINEPATTERNTOOL_H
#define MUONCOMBINEPATTERNTOOLS_MUONCOMBINEPATTERNTOOL_H


#include "AthenaBaseComps/AthAlgTool.h" //base class
#include "GaudiKernel/ServiceHandle.h"  //member
#include "MuonHoughPatternEvent/MuonHoughMathUtils.h" //member
#include "MuonIdHelpers/IMuonIdHelperSvc.h"  //template parameter
#include "MuonRecToolInterfaces/IMuonCombinePatternTool.h" //base class
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"


class MuonCombinePatternTool : public AthAlgTool, virtual public Muon::IMuonCombinePatternTool {
private:
    struct ChamberInfo {
        ChamberInfo() = default;
        int neta{0};
        int nphi{0};
        int ninside{0};
        int noutside{0};
        int ninsidePat{0};
        int noutsidePat{0};
        double phiMin{FLT_MAX};
        double phiMax{-FLT_MAX};
    };
    using IdChMap =  std::map<Identifier, ChamberInfo>;

public:
    using PrdPatternPair = std::pair<std::unique_ptr<Muon::MuonPrdPattern>, std::unique_ptr<Muon::MuonPrdPattern>>;

    using CandPrdPatPtr = std::shared_ptr<const Muon::MuonPrdPattern>;
    using CandidatePatPair = std::pair<CandPrdPatPtr, CandPrdPatPtr>;

    MuonCombinePatternTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~MuonCombinePatternTool() = default;

    StatusCode initialize() override;

    /** Combines phi and eta pattern collection into a new combined pattern
     * collection */
    std::unique_ptr<MuonPrdPatternCollection> combineEtaPhiPatterns(
        const MuonPrdPatternCollection& phiPatternCollection, const MuonPrdPatternCollection& etaPatternCollection,
        const EtaPhiHitAssocMap& phiEtaHitAssMap) const override;

    /** Combines phi and eta pattern into a new combined pattern */
    std::unique_ptr<Muon::MuonPrdPattern> makeCombinedPattern(const Muon::MuonPrdPattern& phipattern, 
                                                              const Muon::MuonPrdPattern& etapattern) const override;

    /** converts MuonPrdPatterns into MuonPatternCombinationCollection
     * MuonPatternCombinationCollection are default output for PatternFinder */
    virtual std::unique_ptr<MuonPatternCombinationCollection> makePatternCombinations(const MuonPrdPatternCollection& muonpatterns) const override;

private:
    /** make combined pattern from all candidates, removes duplicates with phi
     * when no overlap with eta pattern */
    std::unique_ptr<MuonPrdPatternCollection> makeCombinedPatterns(std::vector<CandidatePatPair>& candidates) const;

    /** is pattern1 a complete subset of other pattern2? */
    static bool subset(const Muon::MuonPrdPattern* pattern1, const Muon::MuonPrdPattern* pattern2);

    /** is candidate1 a complete subset of other candidate2? */
    static bool subset(std::pair<PrepDataSet, PrepDataSet>& candidate1,
                       std::pair<PrepDataSet, PrepDataSet>& candidate2);


    /** clean phi pattern, similar as in MuonHoughPatternTool, used for newly
     * created phi patterns based on hit association */
    std::unique_ptr<Muon::MuonPrdPattern> cleanPhiPattern(std::unique_ptr<Muon::MuonPrdPattern> phipattern) const;

    /** clean combined pattern, remove outliers */
    std::unique_ptr<Muon::MuonPrdPattern> cleanupCombinedPattern(const Muon::MuonPrdPattern& combinedpattern) const;

    /** split patterns in two at point closest to IP in rphi */
    std::vector<PrdPatternPair> splitPatterns2D(const Muon::MuonPrdPattern* phipattern,
                                                const Muon::MuonPrdPattern* etapattern) const;

    /** split patterns in two at point closest to IP in 3D */
    std::vector<PrdPatternPair> splitPatterns3D(const Muon::MuonPrdPattern* phipattern,
                                                const Muon::MuonPrdPattern* etapattern) const;

    /** split patterns in two when crossing calorimeter at point closest to IP in
       3D (should be same as splitting at calorimeter) if not split empty vector
       is returned */
    std::vector<PrdPatternPair> splitPatternsCylinder(const Muon::MuonPrdPattern* phipattern, 
                                                      const Muon::MuonPrdPattern* etapattern) const;

    /** make combined phi pattern by associating phi hits to noncombined eta
     * pattern, return 0 if no phi measurements added, 2nd argument is if checking
     * that added phi hits are already on pattern (not necessary for uncombined
     * etapattern) */
    std::unique_ptr<Muon::MuonPrdPattern> makeAssPhiPattern(
        const Muon::MuonPrdPattern& pattern,
        const EtaPhiHitAssocMap& phiEtaHitAssMap,
        bool check = false) const;

    /** calculate new track parameters of match (only for cosmics!) returns [r0,
     * phi, rz0, theta]*/
    std::array<double,4> updateParametersForCosmics(const Muon::MuonPrdPattern& phipattern, 
                                                    const Muon::MuonPrdPattern& etapattern) const;

    /** calculate phi and r0 for cosmic patterns, phi estimate needs to be given
     */
    std::pair<double, double> calculateR0Phi(const Muon::MuonPrdPattern& phipattern, const Muon::MuonPrdPattern& etapattern,
                                             double phi_estimate = -M_PI_2) const;

    /** calculate rz0 for cosmic pattern */
    static double calculateRz0(const Muon::MuonPrdPattern& pattern, double phi, double theta);

    /** update patterns based on new track parameters (used only for cosmics)
     * builds 2 new prd patterns */
    PrdPatternPair updatePatternsForCosmics(const Muon::MuonPrdPattern& phipattern, 
                                                   const Muon::MuonPrdPattern& etapattern, 
                                                   const std::array<double,4>&  new_pars) const;

    /** adds eta,phi pair to candidate vector, also performs splitting and
     * associated pattern (only for cosmics!)*/    
    void addCandidate(const CandPrdPatPtr& etapattern, const CandPrdPatPtr& phipattern,
                     std::vector<CandidatePatPair>& candidates, bool add_asspattern,
                     const EtaPhiHitAssocMap& phiEtaHitAssMap) const;

    /** clean candidates from subsets or duplicates */
    static void cleanCandidates(std::vector<CandidatePatPair>& candidates);

    /** print out pattern hits */
    void printPattern(const Muon::MuonPrdPattern* muonpattern) const;

    /** object for use of mathematical formulas for trackmodels */
    MuonHoughMathUtils m_muonHoughMathUtils;

    /** distance cut in xy for hits */
    const double m_maximum_xydistance;
    /** distance cut in rz for hits */
    const double m_maximum_rzdistance;

    /** use cosmic settings */
    bool m_use_cosmics;

    /** split patterns (only for cosmics) */
    bool m_splitpatterns;

    /** don't discard any candidates based on gasgap assocation (true by default)
     */
    bool m_nodiscarding;

    /** take only best phi match as candidate or take all phi matches (false by
     * default, but true for cosmics) */
    bool m_bestphimatch;

    /** flip direction for cosmics after splitting as if coming from IP (false by
     * default) */
    bool m_flipdirectionforcosmics;

    bool m_useTightAssociation;
    unsigned int m_maxSizePhiPatternLoose;
    unsigned int m_maxSizeEtaPatternLoose;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    ToolHandle<Muon::MuonEDMPrinterTool> m_printer{this, "printerTool", "Muon::MuonEDMPrinterTool/MuonEDMPrinterTool",
                                                       "ToolHandle for EDM printing of segments"};
};

#endif  // MUONCOMBINEPATTERNTOOLS_MUONCOMBINEPATTERNTOOL_H
