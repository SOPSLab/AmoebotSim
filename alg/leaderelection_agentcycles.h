#ifndef LEADERELECTION_AGENTCYCLES_H
#define LEADERELECTION_AGENTCYCLES_H

#include "alg/algorithmwithflags.h"

class System;

namespace LeaderElectionAgentCycles
{

enum class State {
    Idle = 0,
    Candidate,
    Demoted,
    Leader,
    Finished
};

enum class TokenType {
    CandidacyAnnounce = 0,
    CandidacyAck
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
    std::array<Token, 2> tokens;
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
        int agentDir, nextAgentDir, prevAgentDir;
        bool waitingForTransferAck, gotAnnounceBeforeAck;

        void setState(const State _state);
        bool canSendToken(TokenType type, int dir) const;
        void sendToken(TokenType type, int dir, int value);
        int peekAtToken(TokenType type, int dir) const;
        Token receiveToken(TokenType type, int dir);
        void tokenCleanup();
    };

public:
    LeaderElectionAgentCycles(const State _state);
    LeaderElectionAgentCycles(const LeaderElectionAgentCycles& other);
    virtual ~LeaderElectionAgentCycles();

    static std::shared_ptr<System> instance(const unsigned int size);
    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;

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
