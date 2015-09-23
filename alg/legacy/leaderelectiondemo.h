#ifndef   LEADERELECTIONDEMO
#define LEADERELECTIONDEMO

#include "alg/legacy/algorithmwithflags.h"

class LegacySystem;

namespace LeaderElectionDemo
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

class LeaderElectionDemoFlag : public Flag
{
public:
    LeaderElectionDemoFlag();
    LeaderElectionDemoFlag(const LeaderElectionDemoFlag& other);

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
    int activeTokenValue;
    int subPhase;
    int tokenD1;
    int tokenD2;
    int tokenD3;
    int tokenCurrentDir;
    int ownTokenValue;
    bool buildBorder;
    int xT;//x and y for orientation- actives move around to match with passive
    int yT;
    bool xTAccept;
    bool yTAccept;
    int xB;//x and y for orientation- actives move around to match with passive
    int yB;
    bool xBAccept;
    bool yBAccept;
    int xP;//pasive x,y for matching with active x,y
    int yP;
    bool finishedVectors;
    bool unmatchedVector;
    bool changeToken;
    bool handleVectors;


};

class LeaderElectionDemo : public AlgorithmWithFlags<LeaderElectionDemoFlag>
{
public:
    LeaderElectionDemo(const Phase _phase);
    LeaderElectionDemo(const LeaderElectionDemo& other);
    virtual ~LeaderElectionDemo();

    static std::shared_ptr<LegacySystem> instance();


    virtual Movement execute();
    virtual std::shared_ptr<Algorithm> blank() const override;
    virtual std::shared_ptr<Algorithm> clone() override;
    virtual bool isDeterministic() const;
    virtual bool isStatic() const;

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
    void newHandleVectors(int currentSubPhase);
    void oldHandleElectionTokens();
    void clearHeldToken();
    void setTokenCoordinates(int recDir);
    void processTokenCoordinates(int parent,int follower);
    void processXTokenCoordinates(int parent,int follower);
    void processYTokenCoordinates(int parent,int follower);

    void clearVectorToken();
    void setTokenValue(int val);


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
    bool parentChanged;
    int id;
    bool consumeOwnPassive;

};
}

#endif // C

