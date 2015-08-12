#ifndef LEADERELECTION_AGENTCYCLES_H
#define LEADERELECTION_AGENTCYCLES_H

#include "alg/algorithmwithflags.h"

class System;

namespace LeaderElectionAgentCycles
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
    SegmentComparison,
    CoinFlip,
    SolitudeVerification
};

enum class TokenType {
    CandidacyAnnounce = 0,
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

class LeaderElectionAgentCyclesFlag : public Flag
{
public:
    LeaderElectionAgentCyclesFlag();
    LeaderElectionAgentCyclesFlag(const LeaderElectionAgentCyclesFlag& other);

    State state;
    std::array<Token, 7> tokens;
};

class LeaderElectionAgentCycles : public AlgorithmWithFlags<LeaderElectionAgentCyclesFlag>
{
private:
    class LeaderElectionAgent
    {
    public:
        LeaderElectionAgent();

        LeaderElectionAgentCycles* alg;
        State state;
        Subphase subphase;
        int local_id; // 1, 2, or 3
        int agentDir, nextAgentDir, prevAgentDir;
        bool waitingForTransferAck, gotAnnounceBeforeAck; // coin flipping information
        bool createdLead, sawUnmatchedToken; // solitude verification information
        int generateVectorDir;
        bool testingBorder; // border test information

        void setState(const State _state);
        void execute(LeaderElectionAgentCycles* _alg);

    protected:
        bool canSendToken(TokenType type, int dir) const;
        void sendToken(TokenType type, int dir, int value);
        int peekAtToken(TokenType type, int dir) const;
        Token receiveToken(TokenType type, int dir);
        void tokenCleanup();

        int encodeVector(std::pair<int, int> vector) const;
        std::pair<int, int> decodeVector(int code);
        std::pair<int, int> augmentDirVector(std::pair<int, int> vector, const int offset);
        int addNextBorder(int currentSum);
    };

public:
    LeaderElectionAgentCycles(const State _state);
    LeaderElectionAgentCycles(const LeaderElectionAgentCycles& other);
    virtual ~LeaderElectionAgentCycles();

    static std::shared_ptr<System> instance(const unsigned int size);
    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;

    const std::map<std::string, int> colors = {
        {"null", -1}, // no color
        {"black", 0},
        {"dark grey", 0x999999},
        {"grey", 0x666666},
        {"red", 0xff0000},
        {"gold", 0xff9900},
        {"green", 0x00ff00}
    };

protected:
    void setState(const State _state);
    void updateParticleState();
    void updateBorderColors();
    bool hasAgentInState(State state) const;
    int getNextAgentDir(const int agentDir) const;
    int getPrevAgentDir(const int agentDir) const;
    int numNeighbors() const;

    State state;
    std::array<LeaderElectionAgent, 3> agents;
};

}

#endif // LEADERELECTION_AGENTCYCLES_H
