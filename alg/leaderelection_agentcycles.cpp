#include <set>
#include <QDebug>

#include "leaderelection_agentcycles.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace LeaderElectionAgentCycles
{

LeaderElectionAgentCyclesFlag::LeaderElectionAgentCyclesFlag() :
    announceToken(-1), ackToken(-1),
    receivedAnnounceToken(false), receivedAckToken(false)
{}

LeaderElectionAgentCyclesFlag::LeaderElectionAgentCyclesFlag(const LeaderElectionAgentCyclesFlag& other) :
    Flag(other),
    state(other.state),
    announceToken(other.announceToken), ackToken(other.ackToken),
    receivedAnnounceToken(other.receivedAnnounceToken), receivedAckToken(other.receivedAckToken)
{}

LeaderElectionAgentCycles::LeaderElectionAgentCycles(const State _state) :
    state(_state),
    waitingForTransferAck(false),
    gotAnnounceBeforeAck(false)
{
    setState(_state);
}

LeaderElectionAgentCycles::LeaderElectionAgentCycles(const LeaderElectionAgentCycles &other) :
    AlgorithmWithFlags(other),
    state(other.state),
    agents(other.agents),
    waitingForTransferAck(other.waitingForTransferAck),
    gotAnnounceBeforeAck(other.gotAnnounceBeforeAck)
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
                    agents.at(agentNum).agentDir = dir;
                    agents.at(agentNum).nextAgentDir = getNextAgentDir(dir);
                    agents.at(agentNum).prevAgentDir = getPrevAgentDir(dir);
                    setAgentState(dir, State::Candidate);

                    // make agent borders
                    int tempDir = (agents.at(agentNum).nextAgentDir + 1) % 6;
                    while(tempDir != agents.at(agentNum).prevAgentDir) {
                        if((tempDir + 1) % 6 == agents.at(agentNum).prevAgentDir) {
                            borderColors.at(3 * tempDir + 1) = 0x333333;
                        } else {
                            borderColors.at(3 * tempDir + 2) = 0x333333;
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
                // NOTE: this only considers the subphase for coin flipping for now, will evolve as more things get added
                if(agent->state == State::Candidate) {
                    // PART 1: receiving a candidacy transfer announcement
                    if(inFlags[agent->prevAgentDir]->announceToken != -1) {
                        // if a candidate recieves a candidacy transfer announcement, it creates an acknowledgement to send back
                        if(waitingForTransferAck) {
                           gotAnnounceBeforeAck = true;
                        }
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = true;
                        outFlags[agent->prevAgentDir].ackToken = 1;
                        qDebug() << "ack token created" << randDir();
                    } else { // the candidate is not currently receiving a candidacy transfer announcement
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = false;
                    }
                    if(inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if the previous agent has read my acknowledgement, I can destroy my copy
                        outFlags[agent->prevAgentDir].ackToken = -1;
                    }

                    // PART 2: receiving an acknowledgement of my own candidacy transfer
                    if(inFlags[agent->nextAgentDir]->ackToken != -1) {
                        qDebug() << "ack token destroyed" << randDir();
                        if(!gotAnnounceBeforeAck) {
                            setAgentState(agent->agentDir, State::Demoted);
                        }
                        waitingForTransferAck = false;
                        gotAnnounceBeforeAck = false;
                        outFlags[agent->nextAgentDir].receivedAckToken = true;
                        return Movement(MovementType::Idle); // return due to the state change
                    } else {
                        outFlags[agent->nextAgentDir].receivedAckToken = false;
                    }

                    // PART 3: creating a new candidacy transfer announcement with 1/2 probability
                    if(!waitingForTransferAck && randBool()) {
                        waitingForTransferAck = true;
                        outFlags[agent->nextAgentDir].announceToken = 1;
                    }
                    if(inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        // if the next agent has read my announcement, I can destroy my copy
                        outFlags[agent->nextAgentDir].announceToken = -1;
                    }
                } else if(agent->state == State::Demoted) {
                    // handle announcement tokens (forward along the cycle)
                    if(inFlags[agent->prevAgentDir]->announceToken != -1 && outFlags[agent->nextAgentDir].announceToken == -1 && !inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        // if there is an announcement waiting to be passed and the next agent is not still reading, copy it
                        outFlags[agent->nextAgentDir].announceToken = inFlags[agent->prevAgentDir]->announceToken;
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = true;
                    } else if(inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        // if the next agent has already read my token, I can destroy my copy
                        outFlags[agent->nextAgentDir].announceToken = -1;
                    } else if(outFlags[agent->prevAgentDir].receivedAnnounceToken && inFlags[agent->prevAgentDir]->announceToken == -1) {
                        // if the previous agent has destroyed its token because I've copied it, reset my receivedAnnounceToken
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = false;
                    }

                    // handle acknowledgement tokens (reverse along the cycle)
                    if(inFlags[agent->nextAgentDir]->ackToken != -1 && outFlags[agent->prevAgentDir].ackToken == -1 && !inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if there is an acknowledgement waiting to be passed and the previous agent is not still reading, copy it
                        outFlags[agent->prevAgentDir].ackToken = inFlags[agent->nextAgentDir]->ackToken;
                        outFlags[agent->nextAgentDir].receivedAckToken = true;
                    } else if(inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if the previous agent has already read my token, I can destroy my copy
                        outFlags[agent->prevAgentDir].ackToken = -1;
                    } else if(outFlags[agent->nextAgentDir].receivedAckToken && inFlags[agent->nextAgentDir]->ackToken == -1) {
                        // if the next agent has destroyed its token because I've copied it, reset my receivedAckToken
                        outFlags[agent->nextAgentDir].receivedAckToken = false;
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
        headMarkColor = 0xff0000; tailMarkColor = 0xff0000; // Red
    } else if(state == State::Demoted) {
        headMarkColor = 0x999999; tailMarkColor = 0x999999; // Grey
    } else if(state == State::Leader) {
        headMarkColor = 0x00ff00; tailMarkColor = 0x00ff00; // Green
    } else { // state == State::Finished
        headMarkColor = 0x000000; tailMarkColor = 0x000000; // Black
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void LeaderElectionAgentCycles::setAgentState(const int agentDir, const State state)
{
    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->agentDir == agentDir) {
            it->state = state;
            if(it->state == State::Idle) {
                borderPointColors.at(it->agentDir) = -1;
            } else if(it->state == State::Candidate) {
                borderPointColors.at(it->agentDir) = 0xff0000;
            } else if(it->state == State::Demoted) {
                borderPointColors.at(it->agentDir) = 0x999999;
            } else { // it->state == State::Leader
                borderPointColors.at(it->agentDir) = 0x00ff00;
            }

            break;
        }
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
    const static int announceTokenColor = 0xff0000;
    const static int ackTokenColor = 0x0000ff;
    const static int defaultColor = 0x333333;

    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->state != State::Idle) {
    //        if(outFlags[it->nextAgentDir].announceToken != -1) {
    //            borderColors.at() = announceTokenColor;
    //        }

            // color acknowledgement tokens
            int paintColor = (outFlags[it->prevAgentDir].ackToken != -1) ? ackTokenColor : defaultColor;
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

