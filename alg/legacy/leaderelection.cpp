#include <set>

#include "alg/legacy/leaderelection.h"
#include "alg/legacy/legacysystem.h"
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
    comparingSegment(false), absorbedActiveToken(false), isCoveredCandidate(false), gotAnnounceInCompare(false),
    waitingForTransferAck(false), gotAnnounceBeforeAck(false),
    createdLead(false), sawUnmatchedToken(false), generateVectorDir(-1),
    testingBorder(false)
{}

void LeaderElection::LeaderElectionAgent::setState(const State _state)
{
    state = _state;
    if(state == State::Idle || state == State::Leader || state == State::Finished) {
        alg->borderPointColors.at(agentDir) = -1;
    } else if(state == State::Demoted) {
        alg->borderPointColors.at(agentDir) = QColor("dimgrey").rgb();
    } else if(state == State::Candidate) {
        setSubphase(subphase); // set color dependent on the candidate subphase
    } else if(state == State::SoleCandidate) {
        alg->borderPointColors.at(agentDir) = QColor("lime").rgb();
    }
}

void LeaderElection::LeaderElectionAgent::setSubphase(const Subphase _subphase)
{
    subphase = _subphase;
    if(subphase == Subphase::SegmentComparison) {
        alg->borderPointColors.at(agentDir) = QColor("red").rgb();
    } else if(subphase == Subphase::CoinFlip) {
        alg->borderPointColors.at(agentDir) = QColor("orange").rgb();
    } else if (subphase == Subphase::SolitudeVerification) {
        alg->borderPointColors.at(agentDir) = QColor("deepskyblue").darker().rgb();
    }
}

void LeaderElection::LeaderElectionAgent::execute(LeaderElection *_alg)
{
    if(state != State::Idle && state != State::Finished) {
        alg = _alg; // simulator stuff, this is necessary for any of the token functions to work
        tokenCleanup(); // clean token remnants before doing new actions

        if(state == State::Candidate) {
            /* this first block of tasks should be performed by any candidate, regardless of subphase */
            // if there is an incoming active segment cleaning token, consume it
            if(peekAtToken(TokenType::ActiveSegmentClean, nextAgentDir) != -1) {
                performActiveClean(2); // clean the front side only
                receiveToken(TokenType::ActiveSegmentClean, nextAgentDir);
            }
            // if there is an incoming passive segment cleaning token, consume it
            if(peekAtToken(TokenType::PassiveSegmentClean, prevAgentDir) != -1) {
                performPassiveClean(1); // clean the back side only
                receiveToken(TokenType::PassiveSegmentClean, prevAgentDir);
                paintBackSegment(QColor("dimgrey").rgb());
            }
            // if there is an incoming segment lead token, consume it and generate the final passive token
            if(peekAtToken(TokenType::SegmentLead, prevAgentDir) != -1 && canSendToken(TokenType::PassiveSegment, prevAgentDir)) {
                receiveToken(TokenType::SegmentLead, prevAgentDir);
                sendToken(TokenType::PassiveSegment, prevAgentDir, 2); // 2 => last passive/active token
                paintBackSegment(QColor("dimgrey").rgb());
            }
            // if there is an incoming active segment token, either absorb it and acknowledge covering or pass it backward
            if(peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) {
                if(!absorbedActiveToken) {
                    if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // absorbing the last active token
                        sendToken(TokenType::FinalSegmentClean, nextAgentDir, 2);
                    } else { // this candidate is now covered
                        Q_ASSERT(canSendToken(TokenType::ActiveSegmentClean, prevAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                        sendToken(TokenType::PassiveSegmentClean, nextAgentDir, 1);
                        performPassiveClean(2); // clean the front side only
                        paintFrontSegment(QColor("dimgrey").rgb());
                        sendToken(TokenType::ActiveSegmentClean, prevAgentDir, 1);
                        performActiveClean(1); // clean the back side only
                        absorbedActiveToken = true;
                        isCoveredCandidate = true;
                        setState(State::Demoted);
                        return; // completed subphase and thus shouldn't perform any more operations in this round
                    }
                } else if(canSendToken(TokenType::ActiveSegment, prevAgentDir) && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                    sendToken(TokenType::ActiveSegment, prevAgentDir, receiveToken(TokenType::ActiveSegment, nextAgentDir).value);
                }
            }

            // if there is an announcement waiting to be received by me and it is safe to create an acknowledgement, consume the announcement
            if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
                receiveToken(TokenType::CandidacyAnnounce, prevAgentDir);
                sendToken(TokenType::CandidacyAck, prevAgentDir, 1);
                paintBackSegment(QColor("dimgrey").rgb());
                if(waitingForTransferAck) {
                    gotAnnounceBeforeAck = true;
                } else if(comparingSegment) {
                    gotAnnounceInCompare = true;
                }
            }

            // if there is a solitude lead token waiting to be put into lane 2, put it there if it doesn't pass a vector token
            if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                int leadValue = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
                if(leadValue / 100 == 1) { // if the lead is on lap 1, append this candidate's local id
                    sendToken(TokenType::SolitudeLeadL2, prevAgentDir, (1000 * local_id) + leadValue);
                    paintBackSegment(QColor("deepskyblue").rgb());
                } else { // if the lead is on lap 2, just pass on the value
                    sendToken(TokenType::SolitudeLeadL2, prevAgentDir, leadValue);
                    paintBackSegment(QColor("dimgrey").rgb());
                }
            }
            // if there is a vector waiting to be put into lane 2, put it there
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                sendToken(TokenType::SolitudeVectorL2, prevAgentDir, receiveToken(TokenType::SolitudeVectorL1, prevAgentDir).value);
            }

            /* the next block of tasks are dependent upon subphase */
            if(subphase == Subphase::SegmentComparison) {
                if(peekAtToken(TokenType::PassiveSegment, nextAgentDir) != -1 && canSendToken(TokenType::ActiveSegment, prevAgentDir)) {
                    // if there is an incoming passive token, pass it on as an active token
                    int passiveValue = receiveToken(TokenType::PassiveSegment, nextAgentDir).value;
                    sendToken(TokenType::ActiveSegment, prevAgentDir, passiveValue);
                    if(passiveValue == 2) {
                        paintFrontSegment(QColor("dimgrey").rgb());
                    }
                } else if(peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1) {
                    // if there is an incoming final cleaning token, consume it and proceed according to its value
                    int finalCleanValue = receiveToken(TokenType::FinalSegmentClean, prevAgentDir).value;
                    if(finalCleanValue == 0 && !gotAnnounceInCompare) { // if this candidate did not cover any tokens and was not transferred candidacy, demote
                        setState(State::Demoted);
                    } else {
                        setSubphase(Subphase::CoinFlip);
                    }
                    comparingSegment = false;
                    gotAnnounceInCompare = false;
                    return; // completed subphase and thus shouldn't perform any more operations in this round
                } else if(!comparingSegment) {
                    // begin segment comparison by generating a segment lead token
                    Q_ASSERT(canSendToken(TokenType::SegmentLead, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                    sendToken(TokenType::SegmentLead, nextAgentDir, 1);
                    paintFrontSegment(QColor("red").rgb());
                    comparingSegment = true;
                }
            } else if(subphase == Subphase::CoinFlip) {
                if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1) {
                    // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                    receiveToken(TokenType::CandidacyAck, nextAgentDir);
                    paintFrontSegment(QColor("dimgrey").rgb());
                    setSubphase(Subphase::SolitudeVerification);
                    if(!gotAnnounceBeforeAck) {
                        setState(State::Demoted);
                    }
                    waitingForTransferAck = false;
                    gotAnnounceBeforeAck = false;
                    return; // completed subphase and thus shouldn't perform any more operations in this round
                } else if(!waitingForTransferAck && randBool()) {
                    // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                    Q_ASSERT(canSendToken(TokenType::CandidacyAnnounce, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two announcements
                    sendToken(TokenType::CandidacyAnnounce, nextAgentDir, 1);
                    paintFrontSegment(QColor("orange").rgb());
                    waitingForTransferAck = true;
                }
            } else if(subphase == Subphase::SolitudeVerification) {
                // if the agent needs to, generate a lane 1 vector token
                if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
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
                        if(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                                canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                            int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                            sendToken(TokenType::SolitudeLeadL1, nextAgentDir, (leadValue / 1000) * 1000 + 200); // lap 2, orientation no longer matters => 200
                            paintFrontSegment(QColor("deepskyblue").lighter().rgb());
                        }
                    } else if((peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) % 1000) / 100 == 2) { // lead has just completed lap 2
                        int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                        paintFrontSegment(QColor("dimgrey").rgb());
                        if(!sawUnmatchedToken && (leadValue / 1000) == local_id) { // if it did not consume an unmatched token and it assures it's matching with itself, go to inner/outer test
                            setState(State::SoleCandidate);
                        } else { // if solitude verification failed, then do another coin flip compeititon
                            setSubphase(Subphase::SegmentComparison);
                        }
                        createdLead = false;
                        sawUnmatchedToken = false;
                        return; // completed subphase and thus shouldn't perform any more operations in this round
                    }
                } else if(!createdLead) {
                    // to begin the solitude verification, create a lead token with an orientation to communicate to its segment
                    Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                             canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two leads
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                    paintFrontSegment(QColor("deepskyblue").darker().rgb());
                    createdLead = true;
                    generateVectorDir = encodeVector(std::make_pair(1,0));
                }
            }
        } else if(state == State::SoleCandidate) {
            if(!testingBorder) { // begin the inner/outer border test
                Q_ASSERT(canSendToken(TokenType::BorderTest, nextAgentDir));
                sendToken(TokenType::BorderTest, nextAgentDir, addNextBorder(0));
                paintFrontSegment(-1);
                testingBorder = true;
            } else if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1) { // test is complete
                int borderSum = receiveToken(TokenType::BorderTest, prevAgentDir).value;
                paintBackSegment(-1);
                if(borderSum == 1) { // outer border, agent becomes the leader
                    setState(State::Leader);
                } else if(borderSum == 4) { // inner border, demote agent and set to finished
                    setState(State::Finished);
                }
                testingBorder = false;
                return; // completed subphase and thus shouldn't perform any more operations in this round
            }
        } else if(state == State::Demoted) {
            // SUBPHASE: Segment Comparison
            // pass passive segment cleaning tokens forward, and perform cleaning
            if(peekAtToken(TokenType::PassiveSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                performPassiveClean(3); // clean the full segment
                sendToken(TokenType::PassiveSegmentClean, nextAgentDir, receiveToken(TokenType::PassiveSegmentClean, prevAgentDir).value);
                paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
            }
            // pass active segment cleaning tokens backward, and perform cleaning
            if(peekAtToken(TokenType::ActiveSegmentClean, nextAgentDir) != -1 && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                performActiveClean(3); // clean the full segment
                sendToken(TokenType::ActiveSegmentClean, prevAgentDir, receiveToken(TokenType::ActiveSegmentClean, nextAgentDir).value);
            }
            // if there is an incoming segment lead token, pass it forward and generate a passive token
            if(peekAtToken(TokenType::SegmentLead, prevAgentDir) != -1 && canSendToken(TokenType::SegmentLead, nextAgentDir) &&
                    canSendToken(TokenType::PassiveSegment, prevAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                sendToken(TokenType::SegmentLead, nextAgentDir, receiveToken(TokenType::SegmentLead, prevAgentDir).value);
                sendToken(TokenType::PassiveSegment, prevAgentDir, 1); // 1 => usual passive/active token
                paintFrontSegment(QColor("red").rgb()); paintBackSegment(QColor("red").rgb());
            }
            // pass passive tokens backward
            if(peekAtToken(TokenType::PassiveSegment, nextAgentDir) != -1 && canSendToken(TokenType::PassiveSegment, prevAgentDir)) {
                int passiveValue = receiveToken(TokenType::PassiveSegment, nextAgentDir).value;
                sendToken(TokenType::PassiveSegment, prevAgentDir, passiveValue);
                if(passiveValue == 2) {
                    paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
                }
            }
            // either absorb active tokens or pass them backward (but don't pass an active cleaning token)
            if(peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) {
                // absorb the token if possible
                if(!absorbedActiveToken) {
                    if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // if absorbing the final active token, generate the final cleaning token
                        sendToken(TokenType::FinalSegmentClean, nextAgentDir, 0); // the final segment cleaning token begins as having not seen covered candidates
                    } else {
                        absorbedActiveToken = true;
                    }
                } else if(canSendToken(TokenType::ActiveSegment, prevAgentDir) && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                    // pass active token backward if doing so does not pass the active cleaning token
                    sendToken(TokenType::ActiveSegment, prevAgentDir, receiveToken(TokenType::ActiveSegment, nextAgentDir).value);
                }
            }
            // pass final cleaning token forward, perform cleaning, and check for covered candidates
            if(peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::FinalSegmentClean, nextAgentDir)) {
                int finalCleanValue = receiveToken(TokenType::FinalSegmentClean, prevAgentDir).value;
                if(finalCleanValue != 2 && isCoveredCandidate) {
                    finalCleanValue = 1;
                }
                absorbedActiveToken = false;
                isCoveredCandidate = false;
                sendToken(TokenType::FinalSegmentClean, nextAgentDir, finalCleanValue);
            }

            // SUBPHASE: Coin Flipping
            // pass announcements forward
            if(peekAtToken(TokenType::CandidacyAnnounce, prevAgentDir) != -1 && canSendToken(TokenType::CandidacyAnnounce, nextAgentDir) &&
                    canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                sendToken(TokenType::CandidacyAnnounce, nextAgentDir, receiveToken(TokenType::CandidacyAnnounce, prevAgentDir).value);
                paintFrontSegment(QColor("orange").rgb()); paintBackSegment(QColor("orange").rgb());
            }
            // pass acknowledgements backward
            if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1 && canSendToken(TokenType::CandidacyAck, prevAgentDir)) {
                sendToken(TokenType::CandidacyAck, prevAgentDir, receiveToken(TokenType::CandidacyAck, nextAgentDir).value);
                paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
            }

            // SUBPHASE: Solitude Verification
            // pass lane 1 solitude lead token forward
            if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                int code = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
                if(code / 100 == 1) { // lead token is on its first lap, so update the lead direction as it's passed
                    std::pair<int, int> leadVector = decodeVector(code);
                    int offset = (nextAgentDir - ((prevAgentDir + 3) % 6) + 6) % 6;
                    generateVectorDir = encodeVector(augmentDirVector(leadVector, offset));
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + generateVectorDir); // lap 1 + new encoded direction vector
                    paintFrontSegment(QColor("deepskyblue").darker().rgb()); paintBackSegment(QColor("deepskyblue").darker().rgb());
                } else { // lead token is on its second pass, just send it forward
                    sendToken(TokenType::SolitudeLeadL1, nextAgentDir, code);
                    paintFrontSegment(QColor("deepskyblue").lighter().rgb()); paintBackSegment(QColor("deepskyblue").lighter().rgb());
                }
            }
            // pass lane 2 solitude lead token backward, if doing so does not pass a lane 2 vector token
            if(peekAtToken(TokenType::SolitudeLeadL2, nextAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
                    canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
                int leadValue = receiveToken(TokenType::SolitudeLeadL2, nextAgentDir).value;
                sendToken(TokenType::SolitudeLeadL2, prevAgentDir, leadValue);
                if((leadValue % 1000) / 100 == 1) { // first lap
                    paintFrontSegment(QColor("deepskyblue").rgb()); paintBackSegment(QColor("deepskyblue").rgb());
                } else { // second lap
                    paintFrontSegment(QColor("dimgrey").rgb()); paintBackSegment(QColor("dimgrey").rgb());
                }
            }
            // if the agent needs to, generate a lane 1 vector token
            if(generateVectorDir != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                sendToken(TokenType::SolitudeVectorL1, nextAgentDir, generateVectorDir);
                generateVectorDir = -1;
            }
            // perform token matching if there are incoming lane 1 and lane 2 vectors and the various passings are safe
            if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                if(canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL2, prevAgentDir) &&
                        canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
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
                if(peekAtToken(TokenType::SolitudeVectorL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                        canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
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
                paintFrontSegment(-1); paintBackSegment(-1);
                setState(State::Finished);
            }
        }
    }
}

void LeaderElection::LeaderElectionAgent::paintFrontSegment(const int color)
{
    int tempDir = agentDir;
    while(tempDir != (nextAgentDir + 1) % 6) {
        if((tempDir - 1 + 6) % 6 != nextAgentDir) {
            alg->borderColors.at((3 * tempDir - 1 + 18) % 18) = color;
        }
        tempDir = (tempDir - 1 + 6) % 6;
    }
}

void LeaderElection::LeaderElectionAgent::paintBackSegment(const int color)
{
    alg->borderColors.at(3 * agentDir + 1) = color;
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
    for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
        TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest}) {
        if(alg->inFlags[nextAgentDir]->tokens.at((int) tokenType).receivedToken) {
            alg->outFlags[nextAgentDir].tokens.at((int) tokenType).value = -1;
        }
        if(alg->inFlags[prevAgentDir]->tokens.at((int) tokenType).value == -1) {
            alg->outFlags[prevAgentDir].tokens.at((int) tokenType).receivedToken = false;
        }
    }
    for(auto tokenType : {TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
        TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
        if(alg->inFlags[prevAgentDir]->tokens.at((int) tokenType).receivedToken) {
            alg->outFlags[prevAgentDir].tokens.at((int) tokenType).value = -1;
        }
        if(alg->inFlags[nextAgentDir]->tokens.at((int) tokenType).value == -1) {
            alg->outFlags[nextAgentDir].tokens.at((int) tokenType).receivedToken = false;
        }
    }
}

void LeaderElection::LeaderElectionAgent::performPassiveClean(const int region)
{
    Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
    if(region != 2) { // back
        for(auto tokenType : {TokenType::SegmentLead, TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1}) {
            if(peekAtToken(tokenType, prevAgentDir) != -1) {
                receiveToken(tokenType, prevAgentDir);
            }
        }
    }
    if(region != 1) { // front
        for(auto tokenType : {TokenType::PassiveSegment, TokenType::CandidacyAck, TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
            if(peekAtToken(tokenType, nextAgentDir) != -1) {
                receiveToken(tokenType, nextAgentDir);
            }
        }
    }
}

void LeaderElection::LeaderElectionAgent::performActiveClean(const int region)
{
    Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
    if(region != 2 && peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1) { // back
        receiveToken(TokenType::FinalSegmentClean, prevAgentDir);
    }
    if(region != 1 && peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) { // front
        receiveToken(TokenType::ActiveSegment, nextAgentDir);
    }
    absorbedActiveToken = false;
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

std::shared_ptr<LegacySystem> LeaderElection::instance(const unsigned int size)
{
    std::shared_ptr<LegacySystem> system = std::make_shared<LegacySystem>();
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
        system->insertParticle(Particle(std::make_shared<LeaderElection>(State::Idle), randDir(), node, -1));
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
                    agent->setSubphase(Subphase::SegmentComparison);
                    agent->paintFrontSegment(QColor("dimgrey").rgb());
                    agent->paintBackSegment(QColor("dimgrey").rgb());

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

std::shared_ptr<Algorithm> LeaderElection::blank() const
{
    return std::make_shared<LeaderElection>(State::Idle);
}

std::shared_ptr<Algorithm> LeaderElection::clone()
{
    return std::make_shared<LeaderElection>(*this);
}

bool LeaderElection::isDeterministic() const
{
    return true;
}

bool LeaderElection::isStatic() const
{
    return false;
}

void LeaderElection::setState(const State _state)
{
    state = _state;
    if(state == State::Leader) {
        headMarkColor = QColor("lime").rgb(); tailMarkColor = QColor("lime").rgb();
    } else {
        headMarkColor = -1; tailMarkColor = -1;
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
