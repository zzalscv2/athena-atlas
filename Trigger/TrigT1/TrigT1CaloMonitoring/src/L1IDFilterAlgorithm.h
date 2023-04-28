
class L1IDFilterAlgorithm : public AthMonitorAlgorithm {

  public:
    L1IDFilterAlgorithm( const std::string& name, ISvcLocator* pSvcLocator ) : AthMonitorAlgorithm(name,pSvcLocator) {}
    virtual ~L1IDFilterAlgorithm()=default;

    virtual StatusCode fillHistograms(const EventContext& ctx) const override {
        // drop top 8 bits (ecrid) and check rest (L1ID) for multiple of 200
        setFilterPassed( ( (GetEventInfo(ctx)->extendedLevel1ID()&0xffffff) % 200) == 0, ctx );
        if (filterPassed(ctx)) ATH_MSG_INFO("Event ok " << GetEventInfo(ctx)->eventNumber()); // note: could also use: ctx.eventID().event_number()
        return StatusCode::SUCCESS;
    }


};