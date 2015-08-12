#include <set>
#include <QDebug>

#include "leaderelection.h"
#include "sim/particle.h"
#include "sim/system.h"

namespace LeaderElection
{

LeaderElectionFlag::LeaderElectionFlag()
{
    int typenum = 0;
    for(auto token = tokens.begin(); token != tokens.end(); ++token) {
        token->type = (TokenType) typenum;
        token->value = -1;
        token->receivedToken = false;
        ++typenum;
    }
}

LeaderElectionFlag::LeaderElectionFlag(const LeaderElectionFlag& other) :
    Flag(other),
    state(other.state),
    tokens(other.tokens)
{}

LeaderElection::LeaderElectionAgent::LeaderElectionAgent() :
    alg(nullptr),
    state(State::Idle),
    waitingForTransferAck(false), gotAnnounceBeforeAck(false),
    createdLead(false), sawUnmatchedToken(false), generateVectorDir(-1),
    testingBorder(false)
{}

void LeaderElection::LeaderElectionAgent::setState(const State _state)
{
    state = _state;
    if(state == State::Idle || state == State::Demoted || state == State::Leader || state == State::Finished) {
        alg->borderPointColors.at(agentDir) = alg->colors.at("no color");
    } else if(state == State::Candidate) {
        setSubphase(subphase); // set color dependent on the candidate subphase
    } else if(state == State::SoleCandidate) {
        alg->borderPointColors.at(agentDir) = alg->colors.at("green");
    } else {
        Q_ASSERT(false); // an agent shouldn't be any other state
    }
}

void LeaderElection::LeaderElectionAgent::setSubphase(const Subphase _subphase)
{
    subphase = _subphase;
    if(subphase == Subphase::SegmentComparison) {
        // TODO: eventually needs a color
    } else if(subphase == Subphase::CoinFlip) {
        alg->borderPointColors.at(agentDir) = alg->colors.at("red");
    } else if (subphase == Subphase::SolitudeVerification) {
        alg->borderPointColors.at(agentDir) = alg->colors.at("dark blue");
    }
}

void LeaderElection::LeaderElectionAgent::execute(LeaderElection *_alg)
{
    if(state != State::Idle && state != State::Finished) {
        alg = _alg; // simulator stuff, this is necessary for any of the token functions to work
        tokenCleanup(); // clean token remnants before doing new actions

        if(state == State::Candidate) {
            // this first block of tasks should be performed by any candidate, regardless of subphase
            // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
            if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
                receiveToken(TokenType::CandidacyAnnounce, prevAgentDir);
                sendToken(TokenType::CandidacyAck, prevAgentDir, 1);
                paintBackSegment("grey");
                if(waitingForTransferAck) {
                   gotAnnounceBeforeAck = true;
                }
            }
            // if there is a solitude lead token waiting to be put into lane 2, put it there if it doesn't pass a vector token
            if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                int leadValue = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
                if(leadValue / 100 == 1) { // if the lead is on lap 1, append this candidate's local id
                    sendToken(TokenType::SolitudeLeadL2, prevAgentDir, (1000 * local_id) + leadValue);
                    paintBackSegment("blue");
                } else { // if the lead is on lap 2, just pass on the value
                    sendToken(TokenType::SolitudeLeadL2, prevAgentDir, leadValue);
                    paintBackSegment("grey");
                }
            }
            // if there is a vector waiting to be put into lane 2, put it there
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
            }

            // the next block of tasks are dependent upon subphase
            if(subphase == Subphase::CoinFlip) {
                if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1) {
                    // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                    receiveToken(TokenType::CandidacyAck, nextAgentDir);
                    paintFrontSegment("grey");
                    setSubphase(Subphase::SolitudeVerification);
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
                    paintFrontSegment("red");
                    waitingForTransferAck = true;
                }
            } else if(subphase == Subphase::SolitudeVerification) {
                // if the agent needs to, generate a lane 1 vector token
                if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                    sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
                    generateVectorDir = -1;
                }

                if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                    // consume all vector tokens that have not been matched, decide that solitude has failed
                    Q_ASSERT(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != 0); // matched tokens should have dropped
                    receiveToken(TokenType::SolitudeVectorL2, nextAgentDir);
                    sawUnmatchedToken = true;
                } else if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1) {
                    // if the lead token has returned, either put it back in lane 1 (lap 1) or consume it and decide solitude (lap 2)
                    if((peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) % 1000) / 100 == 1) { // lead has just completed lap 1
                        if(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                            int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                            sendToken(TokenType::SolitudeLeadL1, nextAgentDir, (leadValue / 1000) * 1000 + 200); // lap 2, orientation no longer matters => 200
                            paintFrontSegment("light blue");
                        }
                    } else if((peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) % 1000) / 100 == 2) { // lead has just completed lap 2
                        int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                        paintFrontSegment("grey");
                        if(!sawUnmatchedToken && (leadValue / 1000) == local_id) { // if it did not consume an unmatched token and it assures it's matching with itself, go to inner/outer test
                            setState(State::SoleCandidate);
                        } else { // if solitude verification failed, then do another coin flip compeititon
                            setSubphase(Subphase::CoinFlip);
                        }
                        createdLead = false;
                        sawUnmatchedToken = false;
                        return; // completed subphase and thus shouldn't perform any more operations in this round
                    }
                } else if(!createdLead) {
                    // to begin the solitude verification, create a lead token with an orientation to communicate to its segment
                    Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)); // there shouldn't be a call to make two leads
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                    paintFrontSegment("dark blue");
                    createdLead = true;
                    generateVectorDir = encodeVector(std::make_pair(1,0));
                }
            }
        } else if(state == State::SoleCandidate) {
            if(!testingBorder) { // begin the inner/outer border test
                Q_ASSERT(canSendToken(TokenType::BorderTest, nextAgentDir));
                sendToken(TokenType::BorderTest, nextAgentDir, addNextBorder(0));
                paintFrontSegment("no color");
                testingBorder = true;
            } else if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1) { // test is complete
                int borderSum = receiveToken(TokenType::BorderTest, prevAgentDir).value;
                paintBackSegment("no color");
                if(borderSum == 1) { // outer border, agent becomes the leader
                    setState(State::Leader);
                } else if(borderSum == 4) { // inner border, demote agent and set to finished
                    setState(State::Finished);
                }
                testingBorder = false;
                return; // completed subphase and thus shouldn't perform any more operations in this round
            }
        } else if(state == State::Demoted) {
            // SUBPHASE: Coin Flipping
            // pass announcements forward
            if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAnnounce, nextAgentDir)) {
                sendToken(TokenType::CandidacyAnnounce, nextAgentDir, receiveToken(TokenType::CandidacyAnnounce, prevAgentDir).value);
                paintFrontSegment("red"); paintBackSegment("red");
            }
            // pass acknowledgements backward
            if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
                sendToken(TokenType::CandidacyAck, prevAgentDir, receiveToken(TokenType::CandidacyAck, nextAgentDir).value);
                paintFrontSegment("grey"); paintBackSegment("grey");
            }

            // SUBPHASE: Solitude Verification
            // pass lane 1 solitude lead token forward
            if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                int code = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
                if(code / 100 == 1) { // lead token is on its first lap, so update the lead direction as it's passed
                    std::pair<int, int> leadVector = decodeVector(code);
                    int offset = (nextAgentDir - ((prevAgentDir + 3) % 6) + 6) % 6;
                    generateVectorDir = encodeVector(augmentDirVector(leadVector, offset));
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + generateVectorDir); // lap 1 + new encoded direction vector
                    paintFrontSegment("dark blue"); paintBackSegment("dark blue");
                } else { // lead token is on its second pass, just send it forward
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, code);
                    paintFrontSegment("light blue"); paintBackSegment("light blue");
                }
            }
            // pass lane 2 solitude lead token backward, if doing so does not pass a lane 2 vector token
            if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                sendToken(TokenType::SolitudeLeadL2, prevAgentDir, leadValue);
                if((leadValue % 1000) / 100 == 1) { // first lap
                    paintFrontSegment("blue"); paintBackSegment("blue");
                } else { // second lap
                    paintFrontSegment("grey"); paintBackSegment("grey");
                }
            }
            // if the agent needs to, generate a lane 1 vector token
            if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
                generateVectorDir = -1;
            }
            // perform token matching if there are incoming lane 1 and lane 2 vectors and the various passings are safe
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                if(canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
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
                }
            } else {
                // if there aren't both lanes of vectors, then pass lane 1 vectors forward...
                if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir)) {
                    sendToken(TokenType::SolitudeVectorL1, nextAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
                }
                // ...and pass lane 2 vectors backward
                if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                    sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL2, nextAgentDir).value);
                }
            }

            // SUBPHASE: Inner/Outer Border Test
            if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1 && canSendToken(TokenType::BorderTest, nextAgentDir)) {
                int borderSum = addNextBorder(receiveToken(TokenType::BorderTest, prevAgentDir).value);
                sendToken(TokenType::BorderTest, nextAgentDir, borderSum);
                paintFrontSegment("no color"); paintBackSegment("no color");
                setState(State::Finished);
            }
        }
    }
}

void LeaderElection::LeaderElectionAgent::paintFrontSegment(std::string color)
{
    Q_ASSERT(alg->colors.find(color) != alg->colors.end());
    int tempDir = agentDir;
    while(tempDir != (nextAgentDir + 1) % 6) {
        if((tempDir - 1 + 6) % 6 != nextAgentDir) {
            alg->borderColors.at((3 * tempDir - 1 + 18) % 18) = alg->colors.at(color);
        }
        tempDir = (tempDir - 1 + 6) % 6;
    }
}

void LeaderElection::LeaderElectionAgent::paintBackSegment(std::string color)
{
    Q_ASSERT(alg->colors.find(color) != alg->colors.end());
    alg->borderColors.at(3 * agentDir + 1) = alg->colors.at(color);
}

bool LeaderElection::LeaderElectionAgent::canSendToken(TokenType type, int dir) const
{
    return (alg->outFlags[dir].tokens.at((int) type).value == -1 && !alg->inFlags[dir]->tokens.at((int) type).receivedToken);
}

void LeaderElection::LeaderElectionAgent::sendToken(TokenType type, int dir, int value)
{
    Q_ASSERT(canSendToken(type, dir));
    alg->outFlags[dir].tokens.at((int) type).value = value;
}

int LeaderElection::LeaderElectionAgent::peekAtToken(TokenType type, int dir) const
{
    if(alg->outFlags[dir].tokens.at((int) type).receivedToken) {
        // if this agent has already read this token, don't peek the same value again
        return -1;
    } else {
        return alg->inFlags[dir]->tokens.at((int) type).value;
    }
}

Token LeaderElection::LeaderElectionAgent::receiveToken(TokenType type, int dir)
{
    Q_ASSERT(peekAtToken(type, dir) != -1);
    alg->outFlags[dir].tokens.at((int) type).receivedToken = true;
    return alg->inFlags[dir]->tokens.at((int) type);
}

void LeaderElection::LeaderElectionAgent::tokenCleanup()
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
    if(alg->inFlags[nextAgentDir]->tokens.at((int) TokenType::BorderTest).receivedToken) {
        alg->outFlags[nextAgentDir].tokens.at((int) TokenType::BorderTest).value = -1;
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
    if(alg->inFlags[prevAgentDir]->tokens.at((int) TokenType::BorderTest).value == -1) {
        alg->outFlags[prevAgentDir].tokens.at((int) TokenType::BorderTest).receivedToken = false;
    }
}

int LeaderElection::LeaderElectionAgent::encodeVector(std::pair<int, int> vector) const
{
    int x = (vector.first == -1) ? 2 : vector.first;
    int y = (vector.second == -1) ? 2 : vector.second;

    return (10 * x) + y;
}

std::pair<int, int> LeaderElection::LeaderElectionAgent::decodeVector(int code)
{
    code = code % 100; // throw out the extra information for lap number
    int x = (code / 10 == 2) ? -1 : code / 10;
    int y = (code % 10 == 2) ? -1 : code % 10;

    return std::make_pair(x, y);
}

std::pair<int, int> LeaderElection::LeaderElectionAgent::augmentDirVector(std::pair<int, int> vector, const int offset)
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

int LeaderElection::LeaderElectionAgent::addNextBorder(int currentSum)
{
    // adjust offset in modulo 6 to be compatible with modulo 5 computations
    int offsetMod6 = (prevAgentDir + 3) % 6 - nextAgentDir;
    if(4 <= offsetMod6 && offsetMod6 <= 5) {
        offsetMod6 -= 6;
    } else if(-5 <= offsetMod6 && offsetMod6 <= -3) {
        offsetMod6 += 6;
    }

    return (currentSum + offsetMod6 + 5) % 5;
}

LeaderElection::LeaderElection(const State _state) :
    state(_state)
{
    setState(_state);
}

LeaderElection::LeaderElection(const LeaderElection& other) :
    AlgorithmWithFlags(other),
    state(other.state),
    agents(other.agents)
{}

LeaderElection::~LeaderElection()
{}

std::shared_ptr<System> LeaderElection::instance(const unsigned int size)
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
        system->insert(Particle(std::make_shared<LeaderElection>(State::Idle), randDir(), node, -1));
    }

    return system;
}

Movement LeaderElection::execute()
{
    if(state == State::Finished || state == State::Leader) {
        return Movement(MovementType::Empty);
    } else if(state == State::Idle) {
        if(numNeighbors() == 0) { // a particle with no neighbors is the only one in the system and is thus the leader
            setState(State::Leader);
        } else if(numNeighbors() == 6) { // a surrounded particle is not on a border and will not compete
            setState(State::Finished);
        } else { // create agents and borders in unoccupied components
            int agentNum = 0;
            for(int dir = 0; dir < 6; ++dir) {
                if(inFlags[dir] == nullptr && inFlags[(dir + 1) % 6] != nullptr) {
                    Q_ASSERT(agentNum < 3);

                    // record agent information and paint agent borders
                    LeaderElectionAgent *agent = &agents.at(agentNum);
                    agent->alg = this; // simulator stuff
                    agent->local_id = agentNum + 1;
                    agent->agentDir = dir;
                    agent->nextAgentDir = getNextAgentDir(dir);
                    agent->prevAgentDir = getPrevAgentDir(dir);
                    agent->setState(State::Candidate);
                    agent->setSubphase(Subphase::CoinFlip); // TODO: eventually this needs to be Subphase::SegementComparison
                    agent->paintFrontSegment("grey");
                    agent->paintBackSegment("grey");

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
    }

    // regardless of what happens, particles shouldn't move
    return Movement(MovementType::Idle);
}

std::shared_ptr<Algorithm> LeaderElection::clone()
{
    return std::make_shared<LeaderElection>(*this);
}

bool LeaderElection::isDeterministic() const
{
    return true;
}

void LeaderElection::setState(const State _state)
{
    state = _state;
    if(state == State::Leader) {
        headMarkColor = colors.at("green"); tailMarkColor = colors.at("green");
    } else {
        headMarkColor = colors.at("no color"); tailMarkColor = colors.at("no color");
    }

    for(int i = 0; i < 10; i++) {
        outFlags[i].state = state;
    }
}

void LeaderElection::updateParticleState()
{
    if(hasAgentInState(State::Leader)) {
        setState(State::Leader);
    } else if(hasAgentInState(State::Candidate) || hasAgentInState(State::SoleCandidate)) {
        setState(State::Candidate);
    } else if(hasAgentInState(State::Demoted)) {
        setState(State::Demoted);
    } else { // all agents are either idle or finished
        setState(State::Finished);
    }
}

bool LeaderElection::hasAgentInState(const State state) const
{
    for(auto it = agents.begin(); it != agents.end(); ++it) {
        if(it->state == state) {
            return true;
        }
    }
    return false;
}

int LeaderElection::getNextAgentDir(const int agentDir) const
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

int LeaderElection::getPrevAgentDir(const int agentDir) const
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

int LeaderElection::numNeighbors() const
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
