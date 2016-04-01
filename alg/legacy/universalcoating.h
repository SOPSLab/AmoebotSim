#ifndef   UNIVERSALCOATING
#define UNIVERSALCOATING

#include "alg/legacy/algorithmwithflags.h"
#include <QColor>

class LegacySystem;


namespace UniversalCoating
{
enum class Phase {
    Static,
    StaticBorder,
    Inactive,
    retiredLeader,
    Follow,
    Root,
    LayerRoot,
    LayerFollow
};

enum class ElectionSubphase {
    Wait = 0,
    SegmentComparison,
    CoinFlip,
    SolitudeVerification
};

enum class ElectionRole{
    Pipeline,
    Candidate,
    Demoted,
    Leader,
    SoleCandidate,
    None
};

enum class TokenType {
    SegmentLead = 0,
    PassiveSegment,
    ActiveSegment,
    PassiveSegmentClean,
    ActiveSegmentClean,
    FinalSegmentClean,
    CandidacyAnnounce,
    CandidacyAck,
    SolitudeLeadL1,
    SolitudeLeadL2,
    SolitudeVectorL1,
    SolitudeVectorL2,
    BorderTest,
    PosCandidate,
    PosCandidateClean,
    Complaint
};

enum class SwitchVariable
{
    comparingSegment =0,
    absorbedActiveToken,
    isCoveredCandidate,
    gotAnnounceInCompare,
    waitingForTransferAck,
    gotAnnounceBeforeAck,
    generateVectorDir,
    createdLead,
    sawUnmatchedToken,
    testingBorder,
    contractedFinished
};

typedef struct {
    TokenType type;
    int value;
    bool receivedToken;
} Token;

typedef struct {
   Token tokens[6][16];
    std::array<int,13> switches;
    ElectionSubphase electionSubphase;
    ElectionRole electionRole;
    int parentIndex;
} LocData;


class UniversalCoatingFlag : public Flag
{
public:
    UniversalCoatingFlag();
    UniversalCoatingFlag(const UniversalCoatingFlag& other);

    Phase phase;
    int contractDir;
    bool followIndicator;
    bool expFollowIndicator;
    bool contrFollowIndicator;
    int Lnumber;
    int NumFinishedNeighbors;
    bool seedBound;
    bool buildBorder;
    int id;
    bool acceptPositionTokens;
    bool onSurface;
    bool block;

   LocData contrLocData;
    LocData expLocData;
    int expLocIndex;//since all token directions visible everywhere, which to look at
    int contrLocIndex;
    std::string status;

};

class UniversalCoating : public AlgorithmWithFlags<UniversalCoatingFlag>
{
public:
    UniversalCoating(const Phase _phase);
    UniversalCoating(const UniversalCoating& other);
    virtual ~UniversalCoating();

    static std::shared_ptr<LegacySystem> instance(const int numStaticParticles, const int numParticles, const float holeProb);
    virtual Movement execute();
    virtual Movement subExecute();
    virtual bool isDeterministic() const;

    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone();
    virtual bool isStatic() const;
    virtual bool isRetired() const;


protected:
    void setPhase(const Phase _phase);

    bool canSendTokenForward(TokenType type) const;
    bool canSendTokenBackward(TokenType type) const;
    bool canSendToken(TokenType type, bool isNextDir) const;

    void sendTokenForward(TokenType type, int value);
   void sendTokenBackward(TokenType type, int value);
   void sendToken(TokenType type, bool isNextDir,int value);

    int peekAtFollowToken(TokenType type) const;
    int peekAtParentToken(TokenType type) const;
    int peekAtToken(TokenType type, bool isNextDir) const;

    int receiveTokenFromFollow(TokenType type);
    int receiveTokenFromParent(TokenType type);
    Token receiveToken(TokenType type, bool isNextDir);


    void prepLocForExpand(int parent, int expansionDir);
    void prepLocForContract();

    bool neighborIsInPhase(const int label, const Phase _phase) const;
    int firstNeighborInPhase(const Phase _phase) const;
    bool hasNeighborInPhase(const Phase _phase) const;

    int countGreenNeighbors(const Phase _phase, const int L) const;
    void setLayerNumber(const int _Lnumber);
    void setNumFinishedNeighbors(const int _NumFinishedNeighbors);
    int CountFinishedSides(const int _leftDir, const int _rightDir) const;
    int countRetiredareFinished(const int _Lnumber) const;
    void getLeftDir();

    int firstNeighborInPhaseandLayer(const Phase _phase, const int L) const;
    bool neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const;


    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator();
    void setFollowIndicatorLabel(const int label);
    void setFollowNumIndicatorLabel(int label, bool contr);

    bool tailReceivesFollowIndicator() const;
    bool hasExpandedFollow();
    int getMoveDir() const;
    int getDownDir() const;
    bool getLnumber1() const;
    int getLnumber() const;


    void executeComplaintPassing(int parent);

    void executeLeaderElection(int parent,int follow);
    void subExecuteLeaderElection(int parentDir, int followDir);

    void paintFrontSegment(const int color);
    void paintBackSegment(const int color);

    void tokenCleanup();
    void complaintTokenCleanup();
    void electionTokenCleanup();
    void performPassiveClean(const int region);
    void performActiveClean(const int region1);

    int encodeVector(std::pair<int, int> vector) const;
    std::pair<int, int> decodeVector(int code);
    std::pair<int, int> augmentDirVector(std::pair<int, int> vector, const int offset);
    int addNextBorder(int currentSum, int prevAgentDir, int nextAgentDir);

    void printTokens(LocData ldata);

    void cleanHeadLocData();//also used to make clean initial
    void cleanTailLocData();
    void cleanContrLocData();

    bool expandedOnSurface();
    bool tailOnSurface();
    int getSurfaceFollowDir();
    int getSurfaceParentDir();

    int locDataParentToLabel();
    bool hasLocationFollower();
    bool allLocationFollowersExpanded();
    bool locFollowFromLabel(int label);
    Phase phase;
    int followDir;
    int Lnumber;
    int downDir;
    int leftDir;
    int rightDir;
    int NumFinishedNeighbors;
    bool reachedSeedBound;
    int pullDir;
    int parentStage;
    int childStage;
    bool superLeader;
    int borderPasses;
    int id;
    bool sentBorder;
    bool startedRetired;
    int currentExpansionDir;

    //leader election

    LocData headLocData;
    LocData tailLocData;
    LocData contrLocData;

    LocData currentLocData;
    LocData parentLocData;
    LocData followLocData;
    int parentTokensIndex;
    int followTokensIndex;
    int parentOutIndex;
    int followOutIndex;
    //in execute these get set, temp used
  //  LocData self;
   // std::array<Token, 16> followTokens;
   // std::array<Token, 16> parentTokens;
};
}


#endif // C

