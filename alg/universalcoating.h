#ifndef   UNIVERSALCOATING
#define UNIVERSALCOATING

#include "alg/algorithmwithflags.h"
class System;

namespace UniversalCoating
{
enum class Phase {
    Static,
    Border,
    Inactive,
    retiredLeader,
    Follow,
    Lead,
    Seed,
    Hold,
    Send,
    Wait,
    Normal,
};

class UniversalCoatingFlag : public Flag
{
public:
    UniversalCoatingFlag();
    UniversalCoatingFlag(const UniversalCoatingFlag& other);

public:
    Phase phase;
    int contractDir;
    bool followIndicator;
    int Lnumber;
    int NumFinishedNeighbors;
    bool leadComplaint;
    bool seedBound;
    bool block;
    int tokenValue;
    int tokenD1;
    int tokenD2;
    int tokenD3;
    int tokenCurrentDir;
    bool isSendingToken;
    int ownTokenValue;
    bool buildBorder;
};

class UniversalCoating : public AlgorithmWithFlags<UniversalCoatingFlag>
{
public:
    UniversalCoating(const Phase _phase);
    UniversalCoating(const UniversalCoating& other);
    virtual ~UniversalCoating();

    static std::shared_ptr<System> instance(const int numStaticParticles, const int numParticles, const float holeProb, const bool rectWidth, const bool rectLength);


    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> clone();
    virtual bool isDeterministic() const;

protected:
    void setPhase(const Phase _phase);

    bool neighborIsInPhase(const int label, const Phase _phase) const;
    int firstNeighborInPhase(const Phase _phase) const;
    bool hasNeighborInPhase(const Phase _phase) const;

    int countGreenNeighbors(const Phase _phase, const int L) const;
    void setLayerNumber(const int _Lnumber);
    void setNumFinishedNeighbors(const int _NumFinishedNeighbors);
    int CountFinishedSides(const int _leftDir, const int _rightDir) const;
    int countRetiredareFinished(const int _Lnumber) const;
    void getLeftDir() ;

    int firstNeighborInPhaseandLayer(const Phase _phase, const int L) const;
    bool neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const;


    void setContractDir(const int contractDir);
    int updatedFollowDir() const;

    void unsetFollowIndicator();
    void setFollowIndicatorLabel(const int label);
    bool tailReceivesFollowIndicator() const;

    int getMoveDir() const;
    int getDownDir() const;
    bool getLnumber1() const;
    int getLnumber() const;

    bool inFrontOfLeadC() const;
    void setLeadComplaint(bool value);
    bool isSuperLeader();
    void setBlock();
    void expandedSetBlock();
    void updateParentStage();
    bool parentActivated();
    void updateChildStage();
    void updateNeighborStages();

    void handleElectionTokens();
    void clearHeldToken();
    void updateTokenDirs(int recDir);
    void setSendingToken(bool value);
    void sendOwnToken();
    void newOwnToken();



protected:
    Phase phase;
    int followDir;
    int Lnumber;
    int downDir;
    int leftDir;
    int rightDir;
    int NumFinishedNeighbors;
    bool reachedSeedBound;
    bool hasComplained;
    int pullDir;
    int holdCount;
    bool startedOffSurface;
    int parentStage;
    int childStage;
    int ownTokenValue;
    bool hasLost;
    bool superLeader;
    int borderPasses;


};
}


#endif // C

