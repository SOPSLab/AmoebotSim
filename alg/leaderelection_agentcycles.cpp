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
    waitingForTransferAck(false), gotAnnounceBeforeAck(false),
    createdLead(false), isSoleCandidate(true), generateVectorDir(-1)
{}

void LeaderElectionAgentCycles::LeaderElectionAgent::setState(const State _state)
{
    state = _state;
    if(state == State::Idle) {
        alg->borderPointColors.at(agentDir) = -1; // no color
    } else if(state == State::Candidate) {
        alg->borderPointColors.at(agentDir) = 0xff0000; // Red
    } else if(state == State::SoleCandidate) {
        alg->borderPointColors.at(agentDir) = 0xff9900; // Gold
    } else if(state == State::Demoted) {
        alg->borderPointColors.at(agentDir) = 0x999999; // Grey
    } else { // state == State::Leader
        alg->borderPointColors.at(agentDir) = 0x00ff00; // Green
    }
}

void LeaderElectionAgentCycles::LeaderElectionAgent::execute(LeaderElectionAgentCycles* _alg)
{
    if(state != State::Idle) {
        alg = _alg; // simulator stuff, this is necessary for any of the token functions to work
        tokenCleanup(); // clean token remnants before doing new actions

        if(state == State::Candidate) {
            // NOTE: this first block of tasks should be performed by any candidate, regardless of subphase
            // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
            if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
                receiveToken(TokenType::CandidacyAnnounce, prevAgentDir);
                sendToken(TokenType::CandidacyAck, prevAgentDir, 1);
                if(waitingForTransferAck) {
                   gotAnnounceBeforeAck = true;
                }
            }
            // if there is a solitude lead token waiting to be put into lane 2, put it there if it doesn't pass a vector token
            if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                sendToken(TokenType::SolitudeLeadL2, prevAgentDir, receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value);
            }
            // if there is a vector waiting to be put into lane 2, put it there if it doesn't pass the solitude lead token
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeLeadL2, prevAgentDir)) {
                sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
            }

            // NOTE: the next block of tasks are dependent upon subphase
            if(subphase == Subphase::CoinFlip) {
                if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1) {
                    // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                    receiveToken(TokenType::CandidacyAck, nextAgentDir);
                    subphase = Subphase::SolitudeVerification;
                    if(!gotAnnounceBeforeAck) {
                        setState(State::Demoted);
                    }
                    waitingForTransferAck = false;
                    gotAnnounceBeforeAck = false;
                    return; // completed subphase and thus shouldn't perform any more operations in this round
                } else if(!waitingForTransferAck && randBool()) {
                    // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                    Q_ASSERT(canSendToken(TokenType::CandidacyAnnounce, nextAgentDir)); // there shouldn't be a call to make two announcements
                    sendToken(TokenType::CandidacyAnnounce, nextAgentDir, 1);
                    waitingForTransferAck = true;
                }
            } else if(subphase == Subphase::SolitudeVerification) {
                // if the agent needs to, generate a lane 1 vector token if doing so won't pass the lead token
                if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                        canSendToken(TokenType::SolitudeLeadL1, nextAgentDir)) {
                    sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
                    generateVectorDir = -1;
                }

                if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                    // consume all vector tokens that have not been matched, decide that solitude has failed
                    Q_ASSERT(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != 0); // matched tokens should have dropped
                    receiveToken(TokenType::SolitudeVectorL2, nextAgentDir);
                    isSoleCandidate = false;
                } else if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1) {
                    // if the lead token has returned, either put it back in lane 1 (lap 1) or consume it and decide solitude (lap 2)
                    if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) / 100 == 1) { // lead has just completed lap 1
                        qDebug() << "lead finished lap 1, criteria for going into lane 1 again:" << canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) << canSendToken(TokenType::SolitudeVectorL1, nextAgentDir);
                        if(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                            qDebug() << "put lead back in lane 1";
                            receiveToken(TokenType::SolitudeLeadL2, nextAgentDir);
                            sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 200); // lap 2, orientation no longer matters => 200
                        }
                    } else if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) / 100 == 2) { // lead has just completed lap 2
                        qDebug() << "lead finished lap 2";
                        receiveToken(TokenType::SolitudeLeadL2, nextAgentDir);
                        if(isSoleCandidate) { // if this is the only candidate on the border, go on to the inner/outer border test
                            setState(State::SoleCandidate);
                            subphase = Subphase::BorderDetection;
                            qDebug() << "set subphase to BorderDetection";
                        } else { // if solitude verification failed, then do another coin flip compeititon
                            subphase = Subphase::CoinFlip;
                            qDebug() << "reset subphase to CoinFlip";
                        }
                        createdLead = false;
                        isSoleCandidate = true;
                        return; // completed subphase and thus shouldn't perform any more operations in this round
                    } else { // TODO: temporary case, but we should be sure it never gets any other lap number
                        Q_ASSERT(false);
                    }
                } else if(!createdLead) {
                    // to begin the solitude verification, create a lead token with an orientation to communicate to everyone else
                    Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir)); // there shouldn't be a call to make two leads
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                    createdLead = true;
                    generateVectorDir = encodeVector(std::make_pair(1,0));

                    // TODO: what happens if the very next agent on the cycle is another candidate?
                }
            }
        } else if(state == State::Demoted) {
            // SUBPHASE: Coin Flipping
            // pass announcements forward
            if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAnnounce, nextAgentDir)) {
                sendToken(TokenType::CandidacyAnnounce, nextAgentDir, receiveToken(TokenType::CandidacyAnnounce, prevAgentDir).value);
            }
            // pass acknowledgements backward
            if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
                sendToken(TokenType::CandidacyAck, prevAgentDir, receiveToken(TokenType::CandidacyAck, nextAgentDir).value);
            }

            // SUBPHASE: Solitude Verification
            // pass lane 1 solitude lead token forward, and update its relative direction
            if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                int code = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
                if(code / 100 == 1) { // lead token is on its first lap, so update the lead direction as it's passed
                    std::pair<int, int> leadVector = decodeVector(code);
                    int offset = (nextAgentDir - ((prevAgentDir + 3) % 6) + 6) % 6;
                    generateVectorDir = encodeVector(augmentDirVector(leadVector, offset));
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + generateVectorDir); // lap 1 + new encoded direction vector
                } else { // lead token is on its second pass, just send it forward
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, code);
                }
            }
            // pass lane 2 solitude lead token backward, if doing so does not pass a lane 2 vector token
            if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                sendToken(TokenType::SolitudeLeadL2, prevAgentDir, receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value);
            }
            // if the agent needs to, generate a lane 1 vector token if doing so won't pass the lead token
            if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                    canSendToken(TokenType::SolitudeLeadL1, nextAgentDir)) {
                sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
                generateVectorDir = -1;
            }
            // perform token matching if there are incoming lane 1 and lane 2 vectors and the various passings are safe
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                if(canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) &&
                        canSendToken(TokenType::SolitudeVectorL2, prevAgentDir) && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir)) {
                    // decode vectors to do matching on
                    std::pair<int, int> vector1 = decodeVector(receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
                    std::pair<int, int> vector2 = decodeVector(receiveToken(TokenType::SolitudeVectorL2, nextAgentDir).value);
                    // do vector token matching; if the components cancel out, then update both sides
                    if(vector1.first + vector2.first == 0) {
                        vector1.first = 0; vector2.first = 0;
                    }
                    if(vector1.second + vector2.second == 0) {
                        vector1.second = 0; vector2.second = 0;
                    }
                    // only send the tokens if they are non-(0,0)
                    if(vector1 != std::make_pair(0,0)) {
                        sendToken(TokenType::SolitudeVectorL1, nextAgentDir, encodeVector(vector1));
                    }
                    if(vector2 != std::make_pair(0,0)) {
                        sendToken(TokenType::SolitudeVectorL2, prevAgentDir, encodeVector(vector2));
                    }
                } else {
                    qDebug() << "wanted to match, but couldn't because of blockage";
                }
            } else {
                // if there aren't both lanes of vectors, then pass lane 1 vectors forward (without passing the lead token)...
                if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                        canSendToken(TokenType::SolitudeLeadL1, nextAgentDir)) {
                    sendToken(TokenType::SolitudeVectorL1, nextAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
                }
                // ...and pass lane 2 vectors backward (without passing the lead token)
                if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir) &&
                        canSendToken(TokenType::SolitudeLeadL2, prevAgentDir)) {
                    sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL2, nextAgentDir).value);
                }
            }
        } else if(state == State::SoleCandidate) {
            Q_ASSERT(subphase == Subphase::BorderDetection);
            // TODO: Complete later.
        }
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
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::SolitudeLeadL1).receivedToken) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::SolitudeLeadL1).value = -1;
    }
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::SolitudeLeadL2).receivedToken) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::SolitudeLeadL2).value = -1;
    }
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::SolitudeVectorL1).receivedToken) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::SolitudeVectorL1).value = -1;
    }
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::SolitudeVectorL2).receivedToken) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::SolitudeVectorL2).value = -1;
    }
    // reset received token flags if the old copy has been destroyed
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::CandidacyAnnounce).value == -1) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::CandidacyAnnounce).receivedToken = false;
    }
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::CandidacyAck).value == -1) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::CandidacyAck).receivedToken = false;
    }
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::SolitudeLeadL1).value == -1) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::SolitudeLeadL1).receivedToken = false;
    }
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::SolitudeLeadL2).value == -1) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::SolitudeLeadL2).receivedToken = false;
    }
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::SolitudeVectorL1).value == -1) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::SolitudeVectorL1).receivedToken = false;
    }
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::SolitudeVectorL2).value == -1) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::SolitudeVectorL2).receivedToken = false;
    }
}

int LeaderElectionAgentCycles::LeaderElectionAgent::encodeVector(std::pair<int, int> vector) const
{
    int x = (vector.first == -1) ? 2 : vector.first;
    int y = (vector.second == -1) ? 2 : vector.second;

    return (10 * x) + y;
}

std::pair<int, int> LeaderElectionAgentCycles::LeaderElectionAgent::decodeVector(int code)
{
    code = code % 100; // throw out the extra information for lap number
    int x = (code / 10 == 2) ? -1 : code / 10;
    int y = (code % 10 == 2) ? -1 : code % 10;

    return std::make_pair(x, y);
}

std::pair<int, int> LeaderElectionAgentCycles::LeaderElectionAgent::augmentDirVector(std::pair<int, int> vector, const int offset)
{
    static const std::array<std::pair<int, int>, 6> vectors = {std::make_pair(1,0), std::make_pair(0,1), std::make_pair(-1,1),
                                                               std::make_pair(-1,0), std::make_pair(0,-1), std::make_pair(1,-1)};
    for(auto i = 0; i < vectors.size(); ++i) {
        if(vector == vectors.at(i)) {
            return vectors.at((i + offset) % 6);
        }
    }

    Q_ASSERT(false); // the desired vector should be one of the unit directions
    return std::make_pair(0,0);
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
                    agents.at(agentNum).subphase = Subphase::CoinFlip; // TODO: eventually this needs to be Subphase::SegementComparison

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
            agent->execute(this);
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
    Q_ASSERT(_state != State::SoleCandidate); // the SoleCandidate state is for agents only, not particles

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
    const static int solitudeLeadColor = 0xff9900;
    const static int defaultColor = 0x666666;

    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->state != State::Idle) {
            // color acknowledgement tokens
            int paintColor;
            if(outFlags[it->prevAgentDir].tokens.at((int) TokenType::CandidacyAck).value != -1) {
                paintColor = ackTokenColor;
            } else if(outFlags[it->prevAgentDir].tokens.at((int) TokenType::SolitudeLeadL1).value != -1) {
                paintColor = solitudeLeadColor;
            } else {
                paintColor = defaultColor;
            }
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
