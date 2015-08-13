#ifndef LEADERELECTION_H
#define LEADERELECTION_H

#include "alg/algorithmwithflags.h"
#include <QColor>

class System;

namespace LeaderElection
{

enum class State {
    Idle = 0,
    Candidate,
    SoleCandidate,
    Demoted,
    Leader,
    Finished
};

enum class Subphase {
    SegmentComparison = 0,
    CoinFlip,
    SolitudeVerification
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
    BorderTest
};

typedef struct {
    TokenType type;
    int value;
    bool receivedToken;
} Token;

class LeaderElectionFlag : public Flag
{
public:
    LeaderElectionFlag();
    LeaderElectionFlag(const LeaderElectionFlag& other);

    State state;
    std::array<Token, 13> tokens;
};

class LeaderElection : public AlgorithmWithFlags<LeaderElectionFlag>
{
private:
    class LeaderElectionAgent
    {
    public:
        LeaderElectionAgent();

        LeaderElection* alg;
        State state;
        Subphase subphase;
        int local_id; // 1, 2, or 3
        int agentDir, nextAgentDir, prevAgentDir;
        bool comparingSegment, absorbedActiveToken, isCoveredCandidate, gotAnnounceInCompare; // segment comparison information
        bool waitingForTransferAck, gotAnnounceBeforeAck; // coin flipping information
        bool createdLead, sawUnmatchedToken; // solitude verification information
        int generateVectorDir;
        bool testingBorder; // border test information

        void setState(const State _state);
        void setSubphase(const Subphase _subphase);
        void execute(LeaderElection* _alg);
        void paintFrontSegment(const int color);
        void paintBackSegment(const int color);

    protected:
        bool canSendToken(TokenType type, int dir) const;
        void sendToken(TokenType type, int dir, int value);
        int peekAtToken(TokenType type, int dir) const;
        Token receiveToken(TokenType type, int dir);
        void tokenCleanup();

        void performPassiveClean(const int region);
        void performActiveClean(const int region1);

        int encodeVector(std::pair<int, int> vector) const;
        std::pair<int, int> decodeVector(int code);
        std::pair<int, int> augmentDirVector(std::pair<int, int> vector, const int offset);
        int addNextBorder(int currentSum);
    };

public:
    LeaderElection(const State _state);
    LeaderElection(const LeaderElection& other);
    virtual ~LeaderElection();

    static std::shared_ptr<System> instance(const unsigned int size);
    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;

protected:
    void setState(const State _state);
    void updateParticleState();
    bool hasAgentInState(State state) const;
    int getNextAgentDir(const int agentDir) const;
    int getPrevAgentDir(const int agentDir) const;
    int numNeighbors() const;

    State state;
    std::array<LeaderElectionAgent, 3> agents;
};

}

#endif // LEADERELECTION_H
