#ifndef OURPAINT_APP_SNAP_SERVICE_H_
#define OURPAINT_APP_SNAP_SERVICE_H_

struct Request {
    ScreenLogicalPoint cursor;
    ProjectionSnapshot projection;

    PlacementContext placement;
    SourcePolicy sources;
    AcquisitionPolicy acquisition;

    std::vector<SourceSite> preferred;
    std::vector<core::sketch::EntityId> excludedEntities;
};

struct Result {
    InputStamp stamp;
    Placement rawPlacement;
    Placement resolvedPlacement;

    std::vector<RankedCandidate> candidates;
    std::optional<CandidateKey> active;

    FeedbackMeaning meaning;
    QueryStatus status;
    std::vector<SnapIssue> issues;
};

class SnapService {
public:
    explicit SnapService(const core::sketch::Sketch& sketch);

    void begin(const SessionPolicy& policy);
    Result update(const Request& request);

    void cycle(int direction);
    void setLocked(bool locked);
    void setSuppressed(bool suppressed);

    AcceptedInput captureChoice() const;
    void reset();
};


#endif // ! OURPAINT_APP_SNAP_SERVICE_H_