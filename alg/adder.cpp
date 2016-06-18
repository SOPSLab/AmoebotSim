#include <set>

#include <QtGlobal>

#include "alg/adder.h"

#include <QDebug>
AdderParticle::AdderParticle(const Node head,
                             const int globalTailDir,
                             const int orientation,
                             AmoebotSystem& system,
                             State state)
    : AmoebotParticle(head, globalTailDir, orientation, system),
      state(state),
      constructionDir(-1),
      moveDir(-1),
      followDir(-1)
{
    if(state == State::Seed) {
        constructionDir = 0;
        for(int i =0; i<counterGoal; i++)
        {
            putToken(std::make_shared<CarryToken>());
        }


    }
}

void AdderParticle::activate()
{

    if(state == State::Seed) {
        qDebug()<<"seed.";
        index = 0;
        nextLabel = labelOfFirstNeighborInState({State::Active});
        if(nextLabel<0 || nextLabel>=6)
            return;
        if ( neighborAtLabel(nextLabel).hasSpace() && countTokens<CarryToken>()>0)
        {
            neighborAtLabel(nextLabel).putToken(takeToken<CarryToken>());
        }
        return;
    } else if(state == State::Idle) {


            if(hasNeighborInState({State::Seed}))
            {

                prevLabel = labelOfFirstNeighborInState({State::Seed});
                nextLabel = (prevLabel+3)%6;
                state = State::Active;
                followDir = nextLabel;
                index = neighborAtLabel(prevLabel).index+1;
                qDebug()<<"index: "<<index<< " activated: "<<prevLabel<<","<<nextLabel;

            }
            else if(hasNeighborInState({State::Active}))
            {
                prevLabel = labelOfFirstNeighborInState({State::Active});
                nextLabel = (prevLabel+3)%6;
                state = State::Active;
                followDir = nextLabel;
                index = neighborAtLabel(prevLabel).index+1;
                if(!hasNeighborAtLabel(nextLabel))
                {
                    nextLabel = -1;
                }
                qDebug()<<"index: "<<index<< " activated: "<<prevLabel<<","<<nextLabel;


            }

        }
    else if (state == State::Active)
    {
        qDebug()<<"index: "<<index<<" start: ";
        if(prevLabel<0 || prevLabel>=6)
        {
            return;
        }


         if(countTokens<CarryToken>() >1 && nextLabel>=0 && nextLabel <6)
        {
            if(neighborAtLabel(nextLabel).hasSpace())
            {
                neighborAtLabel(nextLabel).putToken(takeToken<CarryToken>());
                takeToken<CarryToken>();//trash it
            }

        }
         displayVal = countTokens<CarryToken>();

      }



}
bool AdderParticle::hasSpace()
{
    if(countTokens<CarryToken>()<2)
        return true;

    return false;
}



int AdderParticle::headMarkColor() const
{
    switch(state) {
    case State::Seed:   return 0x00ff00;
    case State::Idle:   return -1;
    case State::Follow: return 0x0000ff;
    case State::Lead:   return 0xff0000;
    case State::Finish: return 0x000000;
    case State::Active: return 0x0000ff;
    }

    return -1;
}

int AdderParticle::headMarkDir() const
{
    if(state == State::Lead) {
        return moveDir;
    } else if(state == State::Seed || state == State::Finish) {
        return constructionDir;
    } else if(state == State::Follow || state== State::Active) {
        return followDir;
    }
    return -1;
}

int AdderParticle::tailMarkColor() const
{
    return headMarkColor();
}

QString AdderParticle::inspectionText() const
{
    QString text;
    text += "head: (" + QString::number(head.x) + ", " + QString::number(head.y) + ")\n";
    text += "orientation: " + QString::number(orientation) + "\n";
    text += "prevLabel: " + QString::number(prevLabel) + "\n";
    text += "nextLabel: " + QString::number(nextLabel) + "\n";
    text += "state: ";
    text += [this](){
        switch(state) {
        case State::Seed:   return "seed";
        case State::Idle:   return "idle";
        case State::Active: return "active";
        }
    }();
      text += "\nnumCarryTokens: " + QString::number(countTokens<CarryToken>());
    text += "\nDisplay: "+QString::number(displayVal)+"\n";
    return text;
}

AdderParticle& AdderParticle::neighborAtLabel(int label) const
{
    return AmoebotParticle::neighborAtLabel<AdderParticle>(label);
}

int AdderParticle::labelOfFirstNeighborInState(std::initializer_list<State> states, int startLabel) const
{
    auto propertyCheck = [&](const AdderParticle& p) {
        for(auto state : states) {
            if(p.state == state) {
                return true;
            }
        }
        return false;
    };

    return labelOfFirstNeighborWithProperty<AdderParticle>(propertyCheck, startLabel);
}

bool AdderParticle::hasNeighborInState(std::initializer_list<State> states) const
{
    return labelOfFirstNeighborInState(states) != -1;
}

int AdderParticle::constructionReceiveDir() const
{
    auto propertyCheck = [&](const AdderParticle& p) {
        return  isContracted() &&
                (p.state == State::Seed || p.state == State::Finish) &&
                pointsAtMe(p, p.constructionDir);
    };

    return labelOfFirstNeighborWithProperty<AdderParticle>(propertyCheck);
}

bool AdderParticle::canFinish() const
{
    return constructionReceiveDir() != -1;
}

void AdderParticle::updateConstructionDir()
{
    constructionDir = constructionReceiveDir();
    if(neighborAtLabel(constructionDir).state == State::Seed) {
        constructionDir = (constructionDir + 1) % 6;
    } else {
        constructionDir = (constructionDir + 2) % 6;
    }

    if(hasNeighborAtLabel(constructionDir) && neighborAtLabel(constructionDir).state == State::Finish) {
        constructionDir = (constructionDir + 1) % 6;
    }
}

void AdderParticle::updateMoveDir()
{
    moveDir = labelOfFirstNeighborInState({State::Seed, State::Finish});
    while(hasNeighborAtLabel(moveDir) && (neighborAtLabel(moveDir).state == State::Seed || neighborAtLabel(moveDir).state == State::Finish)) {
        moveDir = (moveDir + 5) % 6;
    }
}

bool AdderParticle::hasTailFollower() const
{
    auto propertyCheck = [&](const AdderParticle& p) {
        return  p.state == State::Follow &&
                pointsAtMyTail(p, p.dirToHeadLabel(p.followDir));
    };
    return labelOfFirstNeighborWithProperty<AdderParticle>(propertyCheck) != -1;
}

AdderSystem::AdderSystem(int numParticles, float holeProb)
{
    numParticles = 5;
    AdderParticle *newparticle = new AdderParticle(Node(0, 0), -1, randDir(), *this, AdderParticle::State::Seed);
    newparticle->index = 0;
    insert(newparticle);

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
        int randIndex = 1;//randInt(0, candidates.size());
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

        // if(randBool(1.0f - holeProb)) {
        // only add particle if not a hole
        insert(new AdderParticle(randomCandidate, -1,1, *this, AdderParticle::State::Idle));
        numNonStaticParticles++;

        // add new candidates
        for(int i = 0; i < 6; i++) {
            auto neighbor = randomCandidate.nodeInDir(i);
            if(occupied.find(neighbor) == occupied.end()) {
                candidates.insert(neighbor);
            }
        }
        // }
    }
}

bool AdderSystem::hasTerminated() const
{
#ifdef QT_DEBUG
    if(!isConnected(particles)) {
        return true;
    }
#endif

    for(auto p : particles) {
        auto hp = dynamic_cast<AdderParticle*>(p);
        if(hp->state != AdderParticle::State::Seed && hp->state != AdderParticle::State::Finish) {
            return false;
        }
    }

    return true;
}
