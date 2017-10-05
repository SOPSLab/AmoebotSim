#include <memory>

#include "alg/tokendemo.h"

TokenDemoParticle::TokenDemoParticle(const Node head,
                                     const int globalTailDir,
                                     const int orientation,
                                     AmoebotSystem& system,
                                     State state)
    : HexagonParticle(head, globalTailDir, orientation, system, state)
{
    if(state == State::Seed) {
        putToken(std::make_shared<RedToken>());
        putToken(std::make_shared<RedToken>());
        putToken(std::make_shared<RedToken>());
        putToken(std::make_shared<BlueToken>());
        putToken(std::make_shared<BlueToken>());
    }
}

void TokenDemoParticle::activate()
{
    HexagonParticle::activate();

    if(hasToken<Token>()) {
        if(isContracted() && (state == State::Seed || state == State::Finish)) {
            int label = labelOfFirstNeighborInState({State::Seed, State::Finish}, randDir());
            if(label != -1) {
                nbrAtLabel(label).putToken(takeToken<Token>());
            }
        }
    }
}

int TokenDemoParticle::headMarkColor() const
{
    if(hasToken<RedToken>() && hasToken<BlueToken>()) {
        return 0xff00ff;
    }

    if(hasToken<RedToken>()) {
        return 0xff0000;
    }

    if(hasToken<BlueToken>()) {
        return 0x0000ff;
    }

    if(state == State::Seed || state == State::Finish) {
        return 0x000000;
    } else {
        return -1;
    }
}

QString TokenDemoParticle::inspectionText() const
{
    QString text = HexagonParticle::inspectionText();
    text += "numRedTokens: " + QString::number(countTokens<RedToken>());
    text += "\n";
    text += "numBlueTokens: " + QString::number(countTokens<BlueToken>());
    return text;
}

TokenDemoParticle& TokenDemoParticle::nbrAtLabel(int label) const
{
    return AmoebotParticle::nbrAtLabel<TokenDemoParticle>(label);
}

TokenDemoSystem::TokenDemoSystem(int numParticles, float holeProb)
{
    insert(new TokenDemoParticle(Node(0, 0), -1, randDir(), *this, HexagonParticle::State::Seed));

    std::set<Node> occupied;
    occupied.insert(Node(0, 0));

    std::set<Node> candidates;
    for(int i = 0; i < 6; i++) {
        candidates.insert(Node(0, 0).nodeInDir(i));
    }

    // add inactive particles
    int numNonStaticParticles = 0;
    while(numNonStaticParticles < numParticles && !candidates.empty()) {
        // pick random candidate
        int randIndex = randInt(0, candidates.size());
        Node randomCandidate;
        for(auto it = candidates.begin(); it != candidates.end(); ++it) {
            if(randIndex == 0) {
                randomCandidate = *it;
                candidates.erase(it);
                break;
            } else {
                randIndex--;
            }
        }

        occupied.insert(randomCandidate);

        if(randBool(1.0f - holeProb)) {
            // only add particle if not a hole
            insert(new TokenDemoParticle(randomCandidate, -1, randDir(), *this, HexagonParticle::State::Idle));
            numNonStaticParticles++;

            // add new candidates
            for(int i = 0; i < 6; i++) {
                auto neighbor = randomCandidate.nodeInDir(i);
                if(occupied.find(neighbor) == occupied.end()) {
                    candidates.insert(neighbor);
                }
            }
        }
    }
}

bool TokenDemoSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    return false;
}
