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
                    agents.at(agentNum).agentDir = dir;
                    agents.at(agentNum).nextAgentDir = getNextAgentDir(dir);
                    agents.at(agentNum).prevAgentDir = getPrevAgentDir(dir);
                    setAgentState(dir, State::Candidate);

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
                // NOTE: this only considers the subphase for coin flipping for now, will evolve as more things get added
                if(agent->state == State::Candidate) {
                    qDebug() << "Criteria for creating announcement: " << (!agent->waitingForTransferAck) << ", " << (outFlags[agent->nextAgentDir].announceToken == -1) << ", " << (!inFlags[agent->nextAgentDir]->receivedAnnounceToken);

                    // PART 1: perform tasks related to consuming/passing announcements
                    if(inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        // if the next agent has read the announcement I created, then I can destroy my copy
                        outFlags[agent->nextAgentDir].announceToken = -1;
                    } else if(inFlags[agent->prevAgentDir]->announceToken == -1) {
                        // if the previous agent has destroyed its copy of the announcement I consumed, then I reset my signal
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = false;
                    } else if(inFlags[agent->prevAgentDir]->announceToken != -1 && !outFlags[agent->prevAgentDir].receivedAnnounceToken &&
                            outFlags[agent->prevAgentDir].ackToken == -1 && !inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
                        if(agent->waitingForTransferAck) {
                           agent->gotAnnounceBeforeAck = true;
                        }
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = true;
                        outFlags[agent->prevAgentDir].ackToken = 1;
                    }

                    // PART 2: perform tasks related to consuming/passing acknowledgements
                    if(inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if the previous agent has read the acknowledgement I created, then I can destroy my copy
                        outFlags[agent->prevAgentDir].ackToken = -1;
                    } else if(inFlags[agent->nextAgentDir]->ackToken == -1) {
                        // if the next agent has destroyed its copy of the acknowledgement I consumed, then I reset my signal
                        outFlags[agent->nextAgentDir].receivedAckToken = false;
                    } else if(inFlags[agent->nextAgentDir]->ackToken != -1 && !outFlags[agent->nextAgentDir].receivedAckToken) {
                        // if there is an acknowledgement waiting to be received by me and I am ready to read, consume the acknowledgement
                        if(!agent->gotAnnounceBeforeAck) {
                            setAgentState(agent->agentDir, State::Demoted);
                        }
                        agent->waitingForTransferAck = false;
                        agent->gotAnnounceBeforeAck = false;
                        outFlags[agent->nextAgentDir].receivedAckToken = true;
                        if(agent->state == State::Demoted) {
                            continue; // if this agent has performed a state change, then it shouldn't perform any more computations
                        }
                    }

                    // PART 3: if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                    if(!agent->waitingForTransferAck && randBool() && outFlags[agent->nextAgentDir].announceToken == -1 && !inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        outFlags[agent->nextAgentDir].announceToken = 1;
                        agent->waitingForTransferAck = true;
                    }
                } else if(agent->state == State::Demoted) {
                    // PART 1: perform tasks related to passing announcements forward
                    if(outFlags[agent->nextAgentDir].announceToken == -1 && inFlags[agent->prevAgentDir]->announceToken != -1 &&
                            !outFlags[agent->prevAgentDir].receivedAnnounceToken && !inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        // if there is an announcement waiting to be passed that I have not already read and the next agent is ready, then pass it on
                        outFlags[agent->nextAgentDir].announceToken = inFlags[agent->prevAgentDir]->announceToken;
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = true;
                    } else if(inFlags[agent->nextAgentDir]->receivedAnnounceToken) {
                        // if the next agent has read from me, then I can destroy my copy
                        outFlags[agent->nextAgentDir].announceToken = -1;
                    } else if(inFlags[agent->prevAgentDir]->announceToken == -1) {
                        // if the previous agent has destroyed its copy, then I reset my signal
                        outFlags[agent->prevAgentDir].receivedAnnounceToken = false;
                    }

                    // PART 2: perform tasks related to passing acknowledgements backward
                    if(outFlags[agent->prevAgentDir].ackToken == -1 && inFlags[agent->nextAgentDir]->ackToken != -1 &&
                            !outFlags[agent->nextAgentDir].receivedAckToken && !inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if there is an acknowledgement waiting to be passed that I have not already read and the previous agent is ready, then pass it on
                        outFlags[agent->prevAgentDir].ackToken = inFlags[agent->nextAgentDir]->ackToken;
                        outFlags[agent->nextAgentDir].receivedAckToken = true;
                    } else if(inFlags[agent->prevAgentDir]->receivedAckToken) {
                        // if the previous agent has read from me, then I can destroy my copy
                        outFlags[agent->prevAgentDir].ackToken = -1;
                    } else if(inFlags[agent->nextAgentDir]->ackToken == -1) {
                        // if the next agent has destroyed its copy, then I reset my signal
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
//    const static int announceTokenColor = 0xff0000;
    const static int ackTokenColor = 0xff0000;
    const static int defaultColor = 0x666666;

    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->state != State::Idle) {
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

