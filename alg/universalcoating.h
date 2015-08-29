#ifndef   UNIVERSALCOATING
#define UNIVERSALCOATING

#include "alg/algorithmwithflags.h"
#include <QColor>

class System;

namespace UniversalCoating
{
enum class Phase {
    Static,
    Border,
    StaticBorder,
    Inactive,
    retiredLeader,
    Follow,
    Lead,
    Seed,
    Hold,
    Send,
    Wait,
    Normal,
    Leader,
    Finished


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
    PosCandidateClean
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
    contractedFinished,
    invalidTail

};

typedef struct {
    TokenType type;
    int value;
    bool receivedToken;
} Token;

typedef struct {
    std::array<Token, 15> forwardTokens;
    std::array<Token, 15> backTokens;
    std::array<int,13> switches;
    ElectionSubphase electionSubphase;
    ElectionRole electionRole;
} LocData;

class UniversalCoatingFlag : public Flag
{
public:
    UniversalCoatingFlag();
    UniversalCoatingFlag(const UniversalCoatingFlag& other);

    Phase phase;
    int contractDir;
    bool followIndicator;
    int Lnumber;
    int NumFinishedNeighbors;
    bool leadComplaint;
    bool seedBound;
    bool block;

    int tokenCurrentDir;
    bool isSendingToken;
    int ownTokenValue;
    bool buildBorder;
    int id;
    bool acceptPositionTokens;

    bool expandPrepped;
    LocData headLocData;
    LocData tailLocData;



};

class UniversalCoating : public AlgorithmWithFlags<UniversalCoatingFlag>
{

public:
    UniversalCoating(const Phase _phase);
    UniversalCoating(const UniversalCoating& other);
    virtual ~UniversalCoating();

    static std::shared_ptr<System> instance(const int numStaticParticles, const int numParticles, const float holeProb, const bool rectWidth, const bool rectLength);


    virtual Movement execute();
    virtual Movement subExecute();
    virtual bool isDeterministic() const;
    void paintFrontSegment(const int color);
    void paintBackSegment(const int color);
    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone();
    virtual bool isStatic() const;

protected:
    void setPhase(const Phase _phase);

    bool neighborIsInPhase(const int label, const Phase _phase) const;
    int firstNeighborInPhase(const Phase _phase) const;
    bool hasNeighborInPhase(const Phase _phase) const;

    int countGreenNeighbors(const Phase _phase, const int L) const;
    void setLayerNumber(const int _Lnumber);
    void setNumFinishedNeighbors(const int _NumFinishedNeighbors);
    int CountFinishedSides(const int _leftDir, const int _rightDir) const;
    int countRetiredareFinished(const int _Lnumber) const;
    void getLeftDir() ;

    int firstNeighborInPhaseandLayer(const Phase _phase, const int L) const;
    bool neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const;


    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator();
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;

    int getMoveDir() const;
    int getDownDir() const;
    bool getLnumber1() const;
    int getLnumber() const;

    bool inFrontOfLeadC() const;
    void setLeadComplaint(bool value);
    bool isSuperLeader();
    void setBlock();
    void expandedSetBlock();
    void updateParentStage();
    bool parentActivated();
    void updateChildStage();
    void updateNeighborStages();

    LocData handlePositionElection(LocData myData, LocData followData, LocData parentData);
    LocData ExecuteLeaderElection(LocData myData,std::array<Token, 15> followTokens,ElectionRole followRole,ElectionSubphase followSubphase,
                                  std::array<Token, 15>parentTokens,ElectionRole parentRole,ElectionSubphase parentSubphase);

    void setElectionRole(ElectionRole role);
    void setElectionSubphase(ElectionSubphase electionSubphase);

    bool canSendToken(TokenType type, bool toBack) const;
    void sendToken(TokenType type, bool toBack, int value);
    int peekAtToken(TokenType type, bool fromBack) const;
    Token receiveToken(TokenType type, bool fromBack);
    void tokenCleanup();

    void performPassiveClean(const int region);
    void performActiveClean(const int region1);

    int encodeVector(std::pair<int, int> vector) const;
    std::pair<int, int> decodeVector(int code);
    std::pair<int, int> augmentDirVector(std::pair<int, int> vector, const int offset);
    int addNextBorder(int currentSum, int prevAgentDir, int nextAgentDir);

    void printTokens(int prevAgentDir, int nextAgentDir);

    void cleanHeadLocData();//also used to make clean initial
    void cleanTailLocData();

    bool expandedOnSurface();

    Phase phase;
    int followDir;
    int Lnumber;
    int downDir;
    int leftDir;
    int rightDir;
    int NumFinishedNeighbors;
    bool reachedSeedBound;
    bool hasComplained;
    int pullDir;
    int holdCount;
    bool startedOffSurface;
    int parentStage;
    int childStage;
    int ownTokenValue;
    bool hasLost;
    bool superLeader;
    int borderPasses;
    int id;
    //leader election

    LocData headLocData;
    LocData tailLocData;

    //in execute these get set, temp used
    LocData self;
    std::array<Token, 15> followTokens;
    std::array<Token, 15> parentTokens;
};
}


#endif // C

