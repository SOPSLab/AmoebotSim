#include <set>
#include <QDebug>

#include "leaderelection_agentcycles.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace LeaderElectionAgentCycles
{

LeaderElectionAgentCyclesFlag::LeaderElectionAgentCyclesFlag()
{
    int typenum = 0;
    for(auto token = tokens.begin(); token != tokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }
}

LeaderElectionAgentCyclesFlag::LeaderElectionAgentCyclesFlag(const LeaderElectionAgentCyclesFlag& other) :
    Flag(other),
    state(other.state),
    tokens(other.tokens)
{}

LeaderElectionAgentCycles::LeaderElectionAgent::LeaderElectionAgent() :
    alg(nullptr),
    state(State::Idle),
    waitingForTransferAck(false), gotAnnounceBeforeAck(false)
{}

void LeaderElectionAgentCycles::LeaderElectionAgent::setState(const State _state)
{
    state = _state;
    if(state == State::Idle) {
        alg->borderPointColors.at(agentDir) = -1;
    } else if(state == State::Candidate) {
        alg->borderPointColors.at(agentDir) = 0xff0000;
    } else if(state == State::Demoted) {
        alg->borderPointColors.at(agentDir) = 0x999999;
    } else { // state == State::Leader
        alg->borderPointColors.at(agentDir) = 0x00ff00;
    }
}

bool LeaderElectionAgentCycles::LeaderElectionAgent::canSendToken(TokenType type, int dir) const
{
    return (alg->outFlags[dir].tokens.at((int) type).value == -1 && !alg->inFlags[dir]->tokens.at((int) type).receivedToken);
}

void LeaderElectionAgentCycles::LeaderElectionAgent::sendToken(TokenType type, int dir, int value)
{
    Q_ASSERT(canSendToken(type, dir));
    alg->outFlags[dir].tokens.at((int) type).value = value;
}

int LeaderElectionAgentCycles::LeaderElectionAgent::peekAtToken(TokenType type, int dir) const
{
    if(alg->outFlags[dir].tokens.at((int) type).receivedToken) {
        // if this agent has already read this token, don't peek the same value again
        return -1;
    } else {
        return alg->inFlags[dir]->tokens.at((int) type).value;
    }
}

Token LeaderElectionAgentCycles::LeaderElectionAgent::receiveToken(TokenType type, int dir)
{
    Q_ASSERT(peekAtToken(type, dir) != -1);
    alg->outFlags[dir].tokens.at((int) type).receivedToken = true;
    return alg->inFlags[dir]->tokens.at((int) type);
}

void LeaderElectionAgentCycles::LeaderElectionAgent::tokenCleanup()
{
    // destroy copies of tokens if they've already been read
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::CandidacyAnnounce).receivedToken) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::CandidacyAnnounce).value = -1;
    }
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::CandidacyAck).receivedToken) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::CandidacyAck).value = -1;
    }
    // reset received token flags if the old copy has been destroyed
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::CandidacyAnnounce).value == -1) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::CandidacyAnnounce).receivedToken = false;
    }
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::CandidacyAck).value == -1) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::CandidacyAck).receivedToken = false;
    }
}

LeaderElectionAgentCycles::LeaderElectionAgentCycles(const State _state) :
    state(_state)
{
    setState(_state);
}

LeaderElectionAgentCycles::LeaderElectionAgentCycles(const LeaderElectionAgentCycles &other) :
    AlgorithmWithFlags(other),
    state(other.state),
    agents(other.agents)
{}

LeaderElectionAgentCycles::~LeaderElectionAgentCycles()
{}

std::shared_ptr<System> LeaderElectionAgentCycles::instance(const unsigned int size)
{
    std::shared_ptr<System> system = std::make_shared<System>();
    std::set<Node> occupied, baseComponent;
    std::deque<Node> queue;

    // define a square region of (2*size) particles, where every position has a 0.5 chance of being occupied
    int regionSize = sqrt(2.0 * size);
    for(int x = 0; x < regionSize; ++x) {
        for(int y = 0; y < regionSize; ++y) {
            if(randBool()) {
                occupied.insert(Node(x,y));
            }
        }
    }

    // randomly choose one of the occupied positions to grow a component from
    int baseIndex = randInt(0, occupied.size());
    int i = 0;
    Node base;
    for(auto it = occupied.begin(); it != occupied.end(); ++it) {
        if(i == baseIndex) {
            base = *it;
            occupied.erase(it);
            break;
        }
        ++i;
    }

    // perform a flooding search beginning at the base to discover the its connected component
    queue.push_back(base);
    baseComponent.insert(base);
    while(!queue.empty()) {
        Node n = queue.front();
        queue.pop_front();
        for(int dir = 0; dir < 6; ++dir) {
            Node neighbor = n.nodeInDir(dir);
            auto nodeIt = occupied.find(neighbor);
            if(nodeIt != occupied.end()) {
                queue.push_back(neighbor);
                baseComponent.insert(neighbor);
                occupied.erase(nodeIt);
            }
        }
    }

    // insert all particles in the base component into the system
    while(!baseComponent.empty()) {
        auto node = *baseComponent.begin();
        baseComponent.erase(baseComponent.begin());
        system->insert(Particle(std::make_shared<LeaderElectionAgentCycles>(State::Idle), randDir(), node, -1));
    }

    return system;
}

Movement LeaderElectionAgentCycles::execute()
{
    if(state == State::Finished || state == State::Leader) {
        return Movement(MovementType::Empty);
    } else if(state == State::Idle) {
        if(numNeighbors() == 0) { // a particle with no neighbors is the only one in the system and is thus the leader
            setState(State::Leader);
        } else if(numNeighbors() == 6) { // a surrounded particle is not on a border and will not compete
            setState(State::Demoted);
        } else { // create agents and borders in unoccupied components
            int agentNum = 0;
            for(int dir = 0; dir < 6; ++dir) {
                if(inFlags[dir] == nullptr && inFlags[(dir + 1) % 6] != nullptr) {
                    Q_ASSERT(agentNum < 3);

                    // record agent information
                    agents.at(agentNum).alg = this; // simulator stuffs
                    agents.at(agentNum).agentDir = dir;
                    agents.at(agentNum).nextAgentDir = getNextAgentDir(dir);
                    agents.at(agentNum).prevAgentDir = getPrevAgentDir(dir);
                    agents.at(agentNum).setState(State::Candidate);

                    // make agent borders
                    int tempDir = (agents.at(agentNum).nextAgentDir + 1) % 6;
                    while(tempDir != agents.at(agentNum).prevAgentDir) {
                        if((tempDir + 1) % 6 == agents.at(agentNum).prevAgentDir) {
                            borderColors.at(3 * tempDir + 1) = 0x666666;
                        } else {
                            borderColors.at(3 * tempDir + 2) = 0x666666;
                        }
                        tempDir = (tempDir + 1) % 6;
                    }
                    agentNum++;
                }
            }
            setState(State::Candidate);
        }
    } else { // particle is in a state where it may need to do something
        for(auto agent = agents.begin(); agent != agents.end(); ++agent) {
            if(agent->state != State::Idle) {
                agent->alg = this; // simulator stuff, this is necessary for any of the token functions to work
                agent->tokenCleanup(); // clean token remnants before doing new actions

                // NOTE: this only considers the subphase for coin flipping for now, will evolve as more things get added
                if(agent->state == State::Candidate) {
                    // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
                    if(agent->peekAtToken(TokenType::CandidacyAnnounce, agent->prevAgentDir) != -1 && agent->canSendToken(TokenType::CandidacyAck, agent->prevAgentDir)) {
                        agent->receiveToken(TokenType::CandidacyAnnounce, agent->prevAgentDir);
                        agent->sendToken(TokenType::CandidacyAck, agent->prevAgentDir, 1);
                        if(agent->waitingForTransferAck) {
                           agent->gotAnnounceBeforeAck = true;
                        }
                    }

                    // if there is an acknowledgement waiting to be received by me and I am ready to read, consume the acknowledgement
                    if(agent->peekAtToken(TokenType::CandidacyAck, agent->nextAgentDir) != -1) {
                        agent->receiveToken(TokenType::CandidacyAck, agent->nextAgentDir);
                        if(!agent->gotAnnounceBeforeAck) {
                            agent->setState(State::Demoted);
                        }
                        agent->waitingForTransferAck = false;
                        agent->gotAnnounceBeforeAck = false;
                        if(agent->state == State::Demoted) {
                            continue; // if this agent has performed a state change, then it shouldn't perform any more computations
                        }
                    }

                    // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                    if(!agent->waitingForTransferAck && randBool() && agent->canSendToken(TokenType::CandidacyAnnounce, agent->nextAgentDir)) {
                        agent->sendToken(TokenType::CandidacyAnnounce, agent->nextAgentDir, 1);
                        agent->waitingForTransferAck = true;
                    }
                } else if(agent->state == State::Demoted) {
                    // pass announcements forward
                    if(agent->peekAtToken(TokenType::CandidacyAnnounce, agent->prevAgentDir) != -1 && agent->canSendToken(TokenType::CandidacyAnnounce, agent->nextAgentDir)) {
                        agent->sendToken(TokenType::CandidacyAnnounce, agent->nextAgentDir, agent->receiveToken(TokenType::CandidacyAnnounce, agent->prevAgentDir).value);
                    }
                    // pass acknowledgements backward
                    if(agent->peekAtToken(TokenType::CandidacyAck, agent->nextAgentDir) != -1 && agent->canSendToken(TokenType::CandidacyAck, agent->prevAgentDir)) {
                        agent->sendToken(TokenType::CandidacyAck, agent->prevAgentDir, agent->receiveToken(TokenType::CandidacyAck, agent->nextAgentDir).value);
                    }
                }
            }
        }

        updateParticleState();
        updateBorderColors();
    }

    // regardless of what happens, particles shouldn't move
    return Movement(MovementType::Idle);
}

std::shared_ptr<Algorithm> LeaderElectionAgentCycles::clone()
{
    return std::make_shared<LeaderElectionAgentCycles>(*this);
}

bool LeaderElectionAgentCycles::isDeterministic() const
{
    return true;
}

void LeaderElectionAgentCycles::setState(const State _state)
{
    state = _state;
    if(state == State::Idle) {
        headMarkColor = -1; tailMarkColor = -1; // No color
    } else if(state == State::Candidate) {
        headMarkColor = -1; tailMarkColor = -1; // Red
    } else if(state == State::Demoted) {
        headMarkColor = -1; tailMarkColor = -1; // Grey
    } else if(state == State::Leader) {
        headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
    } else { // state == State::Finished
        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void LeaderElectionAgentCycles::updateParticleState()
{
    if(hasAgentInState(State::Leader)) {
        setState(State::Leader);
    } else if(hasAgentInState(State::Candidate)) {
        setState(State::Candidate);
    } else { // agents all have either state == State::Idle or state == State::Demoted
        setState(State::Demoted);
    }
}

void LeaderElectionAgentCycles::updateBorderColors()
{
    const static int ackTokenColor = 0xff0000;
    const static int defaultColor = 0x666666;

    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->state != State::Idle) {
            // color acknowledgement tokens
            int paintColor = (outFlags[it->prevAgentDir].tokens.at((int) TokenType::CandidacyAck).value != -1) ? ackTokenColor : defaultColor;
            int dir = it->agentDir;
            while(dir != it->prevAgentDir) {
                if(dir == (it->prevAgentDir - 1 + 6) % 6) {
                    borderColors.at(3 * dir + 1) = paintColor;
                } else {
                    borderColors.at(3 * dir + 2) = paintColor;
                }
                dir = (dir + 1) % 6;
            }
        }
    }
}

bool LeaderElectionAgentCycles::hasAgentInState(const State state) const
{
    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->state == state) {
            return true;
        }
    }
    return false;
}

int LeaderElectionAgentCycles::getNextAgentDir(const int agentDir) const
{
    Q_ASSERT(inFlags[agentDir] == nullptr); // the agent direction should be an empty space

    // find the first occupied particle clockwise from the agent direction
    for(int offset = 1; offset < 6; ++offset) {
        if(inFlags[(agentDir - offset + 6) % 6] != nullptr) {
            return (agentDir - offset + 6) % 6;
        }
    }

    Q_ASSERT(false); // the particle should have some neighbor
    return -1;
}

int LeaderElectionAgentCycles::getPrevAgentDir(const int agentDir) const
{
    Q_ASSERT(inFlags[agentDir] == nullptr); // the agent direction should be an empty space

    // find the first occupied particle counter-clockwise from the agent direction
    for(int offset = 1; offset < 6; ++offset) {
        if(inFlags[(agentDir + offset) % 6] != nullptr) {
            return (agentDir + offset) % 6;
        }
    }

    Q_ASSERT(false); // the particle should have some neighbor
    return -1;
}

int LeaderElectionAgentCycles::numNeighbors() const
{
    int count = 0;
    for(int dir = 0; dir < 6; ++dir) {
        if(inFlags[dir] != nullptr) {
            ++count;
        }
    }

    return count;
}

}
