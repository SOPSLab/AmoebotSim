#include <QDebug>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <deque>
#include <set>

#include "alg/legacy/legacyparticle.h"
#include "alg/legacy/legacysystem.h"
#include "alg/legacy/universalcoating.h"
#include "helper/universalcoatinghelper.h"

namespace UniversalCoating {

UniversalCoatingFlag::UniversalCoatingFlag() :
  followIndicator(false),
  expFollowIndicator(false),
  contrFollowIndicator(false),
  Lnumber(-1),
  NumFinishedNeighbors(0),
  seedBound(false),
  buildBorder(false),
  id(-1),
  acceptPositionTokens(false),
  onSurface(false),
  block(false),
  expLocIndex(-1),
  contrLocIndex(-1),
  status("") {
    for (int dirIndex = 0; dirIndex < 6; ++dirIndex) {
      for (int tokenIndex = 0; tokenIndex < 16; ++tokenIndex) {
        expLocData.tokens[dirIndex][tokenIndex].type = (TokenType)tokenIndex;
        expLocData.tokens[dirIndex][tokenIndex].value = -1;
        expLocData.tokens[dirIndex][tokenIndex].receivedToken = false;

        contrLocData.tokens[dirIndex][tokenIndex].type = (TokenType)tokenIndex;
        contrLocData.tokens[dirIndex][tokenIndex].value = -1;
        contrLocData.tokens[dirIndex][tokenIndex].receivedToken = false;
      }
    }

    expLocData.electionRole = ElectionRole::Demoted;
    contrLocData.electionRole = ElectionRole::Demoted;

    expLocData.electionSubphase = ElectionSubphase::SegmentComparison;
    contrLocData.electionSubphase = ElectionSubphase::SegmentComparison;

    expLocData.parentIndex = -1;
    contrLocData.parentIndex = -1;

    expLocData.switches.at((int)SwitchVariable::generateVectorDir) = -1;
    for(uint i = 0; i < expLocData.switches.size(); ++i) {
        expLocData.switches.at(i) = 0;
    }
    contrLocData.switches.at((int)SwitchVariable::generateVectorDir) = -1;
    for(uint i = 0; i < contrLocData.switches.size(); ++i) {
        contrLocData.switches.at(i) = 0;
    }
 }

UniversalCoatingFlag::UniversalCoatingFlag(const UniversalCoatingFlag& other)
  : Flag(other),
  phase(other.phase),
  contractDir(other.contractDir),
  followIndicator(other.followIndicator),
  expFollowIndicator(other.expFollowIndicator),
  contrFollowIndicator(other.contrFollowIndicator),
  Lnumber(other.Lnumber),
  NumFinishedNeighbors(other.NumFinishedNeighbors),
  seedBound(other.seedBound),
  buildBorder(other.buildBorder),
  id(other.id),
  acceptPositionTokens(other.acceptPositionTokens),
  onSurface(other.onSurface),
  block(other.block),
  expLocIndex(other.expLocIndex),
  contrLocIndex(other.contrLocIndex),
  expLocData(other.expLocData),
  contrLocData(other.contrLocData),
  status(other.status) {}

UniversalCoating::UniversalCoating(const Phase _phase) {
  setPhase(_phase);
  Lnumber = -1;
  downDir = -1;
  leftDir = -1;
  rightDir = -1;
  NumFinishedNeighbors = 0;
  reachedSeedBound = false;
  pullDir = -1;
  parentStage = -1;
  childStage = -1;
  superLeader = false;
  borderPasses = 0;
  id = -1;
  sentBorder = false;
  startedRetired = false;
  currentExpansionDir = -1;
  followDir = -1;

  parentTokensIndex = -1;
  followTokensIndex = -1;
  parentOutIndex = -1;
  followOutIndex = -1;
  cleanHeadLocData();
  cleanTailLocData();
  cleanContrLocData();

  currentLocData = tailLocData;
  parentLocData = tailLocData;
  followLocData = tailLocData;
}

UniversalCoating::UniversalCoating(const UniversalCoating& other)
  : AlgorithmWithFlags(other),
  phase(other.phase),
  followDir(other.followDir),
  Lnumber(other.Lnumber),
  downDir(other.downDir),
  leftDir(other.leftDir),
  rightDir(other.rightDir),
  NumFinishedNeighbors(other.NumFinishedNeighbors),
  reachedSeedBound(other.reachedSeedBound),
  pullDir(other.pullDir),
  parentStage(other.parentStage),
  childStage(other.childStage),
  borderPasses(other.borderPasses),
  id(other.id),
  headLocData(other.headLocData),
  tailLocData(other.tailLocData),
  contrLocData(other.contrLocData),
  sentBorder(other.sentBorder),
  startedRetired(other.startedRetired),
  currentExpansionDir(other.currentExpansionDir),
  currentLocData(other.currentLocData),
  parentLocData(other.parentLocData),
  followLocData(other.followLocData),
  parentTokensIndex(other.parentTokensIndex),
  followTokensIndex(other.followTokensIndex),
  parentOutIndex(other.parentOutIndex),
  followOutIndex(other.followOutIndex) {}

UniversalCoating::~UniversalCoating() {}

std::shared_ptr<LegacySystem> UniversalCoating::instance(const int staticParticlesRadius, const int numParticles, const float holeProb) {
  std::shared_ptr<LegacySystem> system = std::make_shared<LegacySystem>();
  std::set<Node> occupied;
  const int hexRadius = staticParticlesRadius;

  // grow hexagon of given radius
  system->insertParticle(LegacyParticle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), Node(0, 0)));
  occupied.insert(Node(0, 0));
  std::set<Node> layer1, layer2;
  layer1.insert(Node(0, 0));
  for(int i = 1; i < hexRadius; i++) {
    for(auto n : layer1) {
      for(int dir = 0; dir < 6; dir++) {
        auto neighbor = n.nodeInDir(dir);
        if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
          layer2.insert(neighbor);
        }
      }
    }

    for(auto n : layer2) {
      system->insertParticle(LegacyParticle(std::make_shared<UniversalCoating>(Phase::Static), randDir(), n));
      occupied.insert(n);
    }

    layer1 = layer2;
    layer2.clear();
  }

  // approach 1: use entire layer as candidate set
  // determine candidate set by "growing an additional layer"
  std::set<Node> candidates;
  for(auto n : layer1) {
    for(int dir = 0; dir < 6; dir++) {
      auto neighbor = n.nodeInDir(dir);
      if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
        candidates.insert(neighbor);
      }
    }
  }

  // approach 2: pick only one seed to build the particle structure
  //    std::set<Node> candidates;
  //    for(auto n : layer1) {
  //      for(int dir = 0; dir < 6; dir++) {
  //        auto neighbor = n.nodeInDir(dir);
  //        if(occupied.find(neighbor) == occupied.end() && layer1.find(neighbor) == layer1.end()) {
  //          layer2.insert(neighbor);
  //        }
  //      }
  //    }
  //    {
  //      int randIndex = randInt(0, layer2.size());
  //      auto randIt = layer2.cbegin();
  //      while(randIndex > 0) {
  //        randIt++;
  //        randIndex--;
  //      }
  //      candidates.insert(*randIt);
  //    }

  // add inactive particles
  int particleID = 0;
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
      std::shared_ptr<UniversalCoating> newParticle= std::make_shared<UniversalCoating>(Phase::Inactive);
      newParticle->id = particleID;
      particleID++;
      system->insertParticle(LegacyParticle(newParticle, randDir(), randomCandidate));
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

  int weak  = getWeakLowerBound(*system.get());
  int strong = getStrongLowerBound(*system.get());
  system->setWeakBound(weak);
  system->setStrongBound(strong);
  return system;
}

// wrapper to capture motion for token storage moving stuff before actually returning it
Movement UniversalCoating::execute() {
  if(hasNeighborInPhase(Phase::Static)) {
    for(int i = 0; i < 10; ++i) {
      outFlags[i].onSurface = true;
    }
  } else {
    for(int i = 0; i < 10; ++i) {
      outFlags[i].onSurface = false;
    }
  }

  if(isContracted()) {
    for(int i = 0; i < 10; ++i) {
      outFlags[i].block = false;
    }
  }

  for(int i = 0; i < 10; ++i) {
    outFlags[i].id = id;
  }

  // vars for determining if to return empty
  startedRetired = (phase == Phase::retiredLeader);

  if(phase == Phase::LayerRoot || phase == Phase::retiredLeader ||
     hasNeighborInPhase(Phase::LayerRoot) ||
     hasNeighborInPhase(Phase::retiredLeader) ||
     phase == Phase::LayerFollow) {

      if(phase==Phase::retiredLeader)// && !hasNeighborInPhase(Phase::Static))
      {
          //&&qDebug()<<"start retired";

          getLeftDir();
          Q_ASSERT(leftDir >=0 && leftDir<=5);
          int  newNumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
          //&&qDebug()<<"counted";
          if(newNumFinishedNeighbors!=NumFinishedNeighbors)
          {
              NumFinishedNeighbors = newNumFinishedNeighbors;
              setNumFinishedNeighbors(NumFinishedNeighbors);
              Q_ASSERT(NumFinishedNeighbors>=0 && NumFinishedNeighbors<=2);
              // setPhase(Phase::retiredLeader);
              return Movement(MovementType::Idle);
          }

          if(!sentBorder)
          {
              for(int label= 0; label<10;label++)
              {

                  if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                  {
                      int neighborsInMyLayer = countGreenNeighbors(Phase::retiredLeader,Lnumber);
                      //&&qDebug()<<"on border has neighbors: "<<neighborsInMyLayer;
                      if(neighborsInMyLayer == 2)
                      {
                          int buildDir = (headMarkDir+3)%6;
                          int count = 0;
                          while((neighborIsInPhase(buildDir,Phase::retiredLeader)||neighborIsInPhase(buildDir,Phase::Static)) && count<6)
                          {
                              // //&&qDebug()<<"check: "<<buildDir<<" "neighborIsInPhase(buildDir,Phase::retiredLeader)<<" "<<neighborIsInPhase(buildDir,Phase::Static)
                              buildDir = (buildDir+1)%6;
                              count++;
                          }
                          //&&qDebug()<<"building border: "<<headMarkDir<<" "<<buildDir;
                          outFlags[buildDir].buildBorder =true;
                          sentBorder = true;
                          setPhase(Phase::retiredLeader);//for coloring
                          return Movement(MovementType::Idle);//to force changes to save


                      }
                  }
              }
          }
      }
      if(phase ==Phase::retiredLeader && startedRetired && !hasNeighborInPhase(Phase::Root) && !hasNeighborInPhase(Phase::Follow))
      {
          return Movement(MovementType::Empty);
      }
      //&&qDebug()<<"call to subexecute";
      Movement movement = subExecute();
      return movement;
  }
  //determine surfaceParent, the direction toward the next particle counterclockwise around the surface, and surfaceFollow, the direction toward the previous
  int surfaceFollow = getSurfaceFollowDir();
  int surfaceParent = getSurfaceParentDir();

  if(isExpanded() && hasNeighborInPhase(Phase::Static) && headLocData.parentIndex==-1)
  {

      int parentFlag = getSurfaceParentDir();
      headLocData.parentIndex = outFlags[parentFlag].expLocIndex;

  }
  if(surfaceParent<0 && !hasNeighborInPhase(Phase::Static) )
  {

      surfaceParent = locDataParentToLabel();
  }

  bool hasParent = false;
  bool hasFollow = false;
  if(surfaceParent!=-1 && inFlags[surfaceParent]!=nullptr)
  {

      std::string parentStatus = inFlags[surfaceParent]->status.c_str();
      if(inFlags[surfaceParent]->isContracted())//if contracted, will read from contracted channel regardless
      {
          parentStatus ="C";
      }
      QString qstr(parentStatus.c_str());

      //&&qDebug()<<"parent: "<<surfaceParent<<" id:"<<inFlags[surfaceParent]->id<<" status: "<<qstr<<" contracted? "<<inFlags[surfaceParent]->isContracted();
      hasParent = true;
  }
  if(surfaceFollow!=-1 && inFlags[surfaceFollow]!=nullptr)
  {
      std::string followStatus = inFlags[surfaceFollow]->status.c_str();
      if(inFlags[surfaceFollow]->isContracted())//if contracted, will read from contracted channel regardless
      {
          followStatus ="C";
      }
      QString qstr(followStatus.c_str());

      //&&qDebug()<<"follow: "<<surfaceFollow<<" id:"<<inFlags[surfaceFollow]->id<<" status: "<<qstr<<" contracted? "<<inFlags[surfaceFollow]->isContracted();
      hasFollow = true;
  }



  //subExecute contains all the actions for complaint coating and layering after leader election and returns the movement the particle should return
  Movement movement= subExecute();
  //Leader election isn't effected by the particle changing follow dir or preparing expansion or anything from subExecute; just relies on surfaceParent/follow
  if(surfaceFollow!=-1 && inFlags[surfaceFollow]!=nullptr && surfaceParent!=-1 && inFlags[surfaceParent]!=nullptr && isContracted()
     &&            inFlags[surfaceParent]->onSurface && inFlags[surfaceFollow]->onSurface &&
     movement.type ==MovementType::Idle)
  {
      Q_ASSERT(hasNeighborInPhase(Phase::Static));
      Q_ASSERT(inFlags[surfaceParent]->onSurface && inFlags[surfaceFollow]->onSurface);
      executeLeaderElection(surfaceParent,surfaceFollow);
  }

  if(phase!=Phase::Inactive&& !hasNeighborInPhase(Phase::Inactive) && phase!=Phase::Inactive)
  {
      executeComplaintPassing(locDataParentToLabel());

  }


  std::string status = "id: "+std::to_string(id);

  if(isContracted())
  {

      status+= " contracted: clean head,tail,expOut";
      cleanHeadLocData();
      cleanTailLocData();

      for(int i =0; i<10;i++)
      {
          outFlags[i].status = "C";
          outFlags[i].contrLocData = contrLocData;
          outFlags[i].expLocData = headLocData;//doesn't matter- clean
      }
      switch(movement.type)
      {
          case MovementType::Expand:
              //&&qDebug()<<"expand: "<<currentExpansionDir;
              status+=" head = expOut of parent, tail = contr, update expOut";
              //put flags where they will be after expansion happens- separate channel from flags with current information about contracted particle
              prepLocForExpand(locDataParentToLabel(),currentExpansionDir);
              for(auto it = headLabelsAfterExpansion(currentExpansionDir).cbegin(); it != headLabelsAfterExpansion(currentExpansionDir).cend(); ++it) {
                  auto label = *it;
                  outFlags[label].status += "H";
                  outFlags[label].expLocData = headLocData;

              }
              for(auto it = tailLabelsAfterExpansion(currentExpansionDir).cbegin(); it != tailLabelsAfterExpansion(currentExpansionDir).cend(); ++it) {
                  auto label = *it;
                  outFlags[label].status += "T";
                  outFlags[label].expLocData = tailLocData;

              }

              break;
          case MovementType::Idle:

              break;
      }

      QString qstr(status.c_str());
      //&&qDebug()<<qstr;
  }
  if(isExpanded())
  {
      for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
          auto label = *it;
          outFlags[label].expLocData = headLocData;

      }
      for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
          auto label = *it;
          outFlags[label].status += "T";
          outFlags[label].expLocData = tailLocData;

      }
      status+=" expanded: clean contr,contrOut";
      cleanContrLocData();
      for(int i =0; i<10;i++)
      {
          outFlags[i].contrLocData = contrLocData;
      }
      switch(movement.type)
      {
          case MovementType::Contract:
              status +=" reg contract";
              if(tailOnSurface())
              {
                  status+= "tail on surface";
              }
              else
              {
                  status+=" tail not on surface";
              }
              status+=" contr=head, update contrOut";
              prepLocForContract();
              for(int i =0; i<10; i++)
              {
                  outFlags[i].contrLocData = contrLocData;
              }
              break;

          case MovementType::HandoverContract:
              status+=" contr=head, update contrOut";
              prepLocForContract();
              for(int i =0; i<10; i++)
              {
                  outFlags[i].contrLocData = contrLocData;
              }
              break;
          case MovementType::Idle:
              //head can execute because its between parent and tail
              if(tailOnSurface())
              {

              }
              else//tail off surface, head is maybe betweeen two particles on surface
              {

              }
              status+=", update expOut";
              break;
      }
      for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
          auto label = *it;
          outFlags[label].status = "H";
      }
      for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
          auto label = *it;
          outFlags[label].status ="T";
      }
      QString qstr(status.c_str());
      //&&qDebug()<<qstr;
  }

  //conditions for returning empty for termination of algorithm
  if(phase ==Phase::retiredLeader && !sentBorder && startedRetired && !hasNeighborInPhase(Phase::Root) && !hasNeighborInPhase(Phase::Follow))
      return Movement(MovementType::Empty);

  //&&qDebug()<<"movement: "<<(int)movement.type<<" on surface? "<<outFlags[0].onSurface;
  if(isExpanded())
  {
      //&&qDebug()<<"parent (off head): "<<headLocData.parentIndex<<" index: "<<locDataParentToLabel();
  }
  else
  {
      //&&qDebug()<<"parent (off contr): "<<contrLocData.parentIndex<<" index: "<<locDataParentToLabel();

  }
  //   //&&qDebug()<<"roles: H: "<<(int)headLocData.electionRole<<" T: "<<(int)tailLocData.electionRole<<" C: "<<(int)contrLocData.electionRole;
  /*//&&qDebug()<<"complaint tokens: ";
   if(isContracted())
   {
   for(int i =0; i<6;i++)
   {
   //&&qDebug()<<" i: "<<i<<" contrIndex: "<<outFlags[i].contrLocIndex<<" token: "
   <<contrLocData.tokens[outFlags[i].contrLocIndex][(int)TokenType::Complaint].value<<" "
   <<contrLocData.tokens[outFlags[i].contrLocIndex][(int)TokenType::Complaint].receivedToken;
   }
   }
   else
   {

   for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
   auto label = *it;
   //&&qDebug()<<" i: "<<label<<"(H) expIndex: "<<outFlags[label].expLocIndex<<" token: "
   <<headLocData.tokens[outFlags[label].expLocIndex][(int)TokenType::Complaint].value<<" "
   <<headLocData.tokens[outFlags[label].expLocIndex][(int)TokenType::Complaint].receivedToken;


   }
   for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
   auto label = *it;
   //&&qDebug()<<" i: "<<label<<"(T) expIndex: "<<outFlags[label].expLocIndex<<" token: "
   <<tailLocData.tokens[outFlags[label].expLocIndex][(int)TokenType::Complaint].value<<" "
   <<tailLocData.tokens[outFlags[label].expLocIndex][(int)TokenType::Complaint].receivedToken;
   }
   for(int i =0; i<6; i++)
   {
   //&&qDebug()<<"all head: i: "<<i<<" "<<headLocData.tokens[i][(int)TokenType::Complaint].value<<" "<<headLocData.tokens[i][(int)TokenType::Complaint].receivedToken;
   }
   for(int i =0; i<6; i++)
   {
   //&&qDebug()<<"all tail: i: "<<i<<" "<<tailLocData.tokens[i][(int)TokenType::Complaint].value<<" "<<tailLocData.tokens[i][(int)TokenType::Complaint].receivedToken;
   }
   }*/
  //colors for election role
  /*  if(isContracted())
   {
   if(contrLocData.electionRole==ElectionRole::Candidate)
   {
   headMarkColor = 0x00ff00;
   tailMarkColor = headMarkColor;
   }
   else if(contrLocData.electionRole == ElectionRole::Leader || contrLocData.electionRole == ElectionRole::SoleCandidate)
   {
   headMarkColor = 0xff0000;
   tailMarkColor = headMarkColor;
   }
   else
   {
   headMarkColor = 0x0000ff;
   tailMarkColor = headMarkColor;
   }
   }
   else
   {
   if(headLocData.electionRole == ElectionRole::Candidate)
   {
   headMarkColor = 0x00ff00;
   }
   else if(headLocData.electionRole == ElectionRole::Leader || headLocData.electionRole == ElectionRole::SoleCandidate)
   {
   headMarkColor = 0xff0000;
   }
   else
   {
   headMarkColor = 0x0000ff;
   }

   if(tailLocData.electionRole == ElectionRole::Candidate)
   {
   tailMarkColor = 0x00ff00;
   }
   else if(tailLocData.electionRole == ElectionRole::Leader || tailLocData.electionRole == ElectionRole::SoleCandidate)
   {
   tailMarkColor = 0xff0000;
   }
   else
   {
   tailMarkColor = 0x0000ff;
   }
   }*/
  if(isContracted()&& movement.type==MovementType::Idle && contrLocData.electionRole == ElectionRole::SoleCandidate)
  {
      int staticDir = firstNeighborInPhase(Phase::Static);
      int buildDir = (staticDir+3)%6;

      while(neighborIsInPhase(buildDir,Phase::Static)|| neighborIsInPhase(buildDir,Phase::retiredLeader) ||neighborIsInPhase(buildDir,Phase::Root))
      {
          buildDir = (buildDir+1)%6;
      }
      //&&qDebug()<<"building border: "<<headMarkDir<<" "<<buildDir;
      outFlags[buildDir].buildBorder =true;
      setPhase(Phase::retiredLeader);

      downDir= getDownDir();

      //Q_ASSERT(downDir!=-1);
      if(downDir == -1)
          return Movement(MovementType::Idle);
      else downDir=labelToDir(downDir);

      Lnumber = getLnumber();
      setLayerNumber(Lnumber);
      NumFinishedNeighbors = 2;
      setNumFinishedNeighbors(2);

      setPhase(Phase::retiredLeader);
  }
  return movement;
}

  Movement UniversalCoating::subExecute()
  {

      //&&qDebug()<<"subexecute layer follow";
      if(phase == Phase::Inactive) {
          Q_ASSERT(isContracted());
          for(int label = 0; label< 6; label++)
          {
              outFlags[label].contrLocIndex = label;
          }
          if(hasNeighborInPhase(Phase::Static)){
              setPhase(Phase::Root);
              downDir= getDownDir();

              //Q_ASSERT(downDir!=-1);
              if(downDir == -1)
                  return Movement(MovementType::Idle);
              else downDir=labelToDir(downDir);

              Lnumber = getLnumber();
              setLayerNumber(Lnumber);

              contrLocData.electionRole = ElectionRole::Candidate;
              int surfaceParent = getSurfaceParentDir();
              //&&qDebug()<<"set index r "<<surfaceParent;
              contrLocData.parentIndex = surfaceParent;

              setPhase(Phase::Root);
              return Movement(MovementType::Idle);
          }
          //otherwise, become follow
          auto label = std::max(firstNeighborInPhase(Phase::Follow), firstNeighborInPhase(Phase::Root));
          if(label != -1) {
              setPhase(Phase::Follow);
              followDir = labelToDir(label);
              setFollowIndicatorLabel(dirToHeadLabel(followDir));
              setFollowNumIndicatorLabel(followDir,true);
              headMarkDir = followDir;

              setPhase(Phase::Follow);
              contrLocData.tokens[followDir][(int)TokenType::Complaint].value = 1;//set complaint token forward to on
              contrLocData.parentIndex = followDir;
              //&&qDebug()<<"setIndex f "<<contrLocData.parentIndex;
              return Movement(MovementType::Idle);
          }
      } else if(phase == Phase::Follow) {

          if( isContracted() && (hasNeighborInPhase(Phase::LayerRoot) || hasNeighborInPhase(Phase::retiredLeader)))//|| hasNeighborInPhase(Phase::LayerFollow)
          {
              if(hasNeighborInPhase(Phase::Static) && inFlags[followDir]->phase == Phase::retiredLeader)
              {
                  setPhase(Phase::LayerRoot);
                  downDir= getDownDir();
                  if(downDir!=-1)
                  {
                      downDir=labelToDir(downDir);

                      Lnumber = getLnumber();
                      setLayerNumber(Lnumber);
                      int moveDir = getMoveDir();
                      setPhase(Phase::retiredLeader);
                      getLeftDir();
                      Q_ASSERT(leftDir>=0 && leftDir<=5);
                      NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                      //&&qDebug()<<"num fin neighbors: "<<NumFinishedNeighbors<<" id: "<<id;
                      setNumFinishedNeighbors(2);
                      headMarkDir = downDir;//-1;
                      setPhase(Phase::retiredLeader);
                      return Movement(MovementType::Idle);
                  }

              }
              else if(hasNeighborInPhase(Phase::retiredLeader) )
              {
                  if(isContracted())
                  {
                      setPhase(Phase::LayerRoot);
                      downDir= getDownDir();

                      //Q_ASSERT(downDir!=-1);
                      if(downDir == -1)
                          return Movement(MovementType::Idle);
                      else downDir=labelToDir(downDir);

                      Lnumber = getLnumber();
                      setLayerNumber(Lnumber);
                      unsetFollowIndicator();
                      setPhase(Phase::LayerRoot);
                  }
                  return Movement(MovementType::Idle);
              }
              else if(!hasNeighborInPhase(Phase::Static))
              {
                  auto label = std::max(firstNeighborInPhase(Phase::LayerFollow), firstNeighborInPhase(Phase::LayerRoot));
                  if(label != -1) {
                      setPhase(Phase::LayerFollow);
                      //  followDir = labelToDir(label);
                      setFollowIndicatorLabel(followDir);
                      setFollowNumIndicatorLabel(followDir,true);
                      headMarkDir = followDir;

                      setPhase(Phase::LayerFollow);
                      return Movement(MovementType::Idle);
                  }
              }
          }
          else if(hasNeighborInPhase(Phase::LayerFollow) && !hasNeighborInPhase(Phase::Static))
          {
              auto label = std::max(firstNeighborInPhase(Phase::LayerFollow), firstNeighborInPhase(Phase::LayerRoot));
              if(label != -1) {
                  setPhase(Phase::LayerFollow);
                  // followDir = labelToDir(label);
                  setFollowIndicatorLabel(followDir);
                  setFollowNumIndicatorLabel(followDir,true);
                  headMarkDir = followDir;

                  setPhase(Phase::LayerFollow);
                  return Movement(MovementType::Idle);
              }
          }
          if(isExpanded())
          {
              setFollowIndicatorLabel(dirToHeadLabel(followDir));
              setFollowNumIndicatorLabel(dirToHeadLabel(followDir),false);

              if(hasNeighborInPhase(Phase::Static) && outFlags[0].block)//curently blocking- check about unblocking
              {
                  bool offSurfaceNeighbor = false;
                  for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it)
                  {
                      auto label = *it;
                      if(inFlags[label]!=nullptr && !neighborIsInPhase(label,Phase::Inactive) && !inFlags[label]->onSurface && locFollowFromLabel(label))
                      {
                          offSurfaceNeighbor = true;
                      }
                  }

                  if(!offSurfaceNeighbor)
                  {
                      //&&qDebug()<<"follow unblocking";
                      for(int i =0; i<10;i++)
                      {
                          outFlags[i].block = false;
                      }
                      return Movement(MovementType::Idle);
                  }

              }
              if(hasNeighborInPhase(Phase::Inactive)  || hasLocationFollower()) {
                  //&&qDebug()<<"tail follow indicator"<<tailReceivesFollowIndicator()<<" location follower?"<< hasLocationFollower()<<" all expanded? "<<allLocationFollowersExpanded();
                  if(allLocationFollowersExpanded())
                  {
                      return Movement(MovementType::Idle);
                  }
                  return Movement(MovementType::HandoverContract, tailContractionLabel());
              }
              else {
                  //&&qDebug()<<"check for follow tokens; has retired neighbor? "<<hasNeighborInPhase(Phase::retiredLeader);
                  if(!hasNeighborInPhase(Phase::retiredLeader))
                  {
                      for(int i =0; i<6; i++)
                      {
                          if(tailLocData.tokens[i][(int)TokenType::Complaint].value == 1  )//|| (tailLocData.tokens[i][(int)TokenType::Complaint].receivedToken))
                          {
                              //&&qDebug()<<"follow tail has tokens.";
                              return Movement(MovementType::Idle);

                          }
                      }
                  }
                  return Movement(MovementType::Contract, tailContractionLabel());
              }
          }
          else
          {
              //&&qDebug()<<"contr follow";
              //&&qDebug()<<"followDir: "<<followDir;
              //  setFollowIndicatorLabel(followDir);
              //&&qDebug()<<"following phase: "<<(int)inFlags[followDir]->phase<<" id: "<<inFlags[followDir]->id;
              if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {

                  if(hasNeighborInPhase(Phase::Static))
                  {
                      for(int i =0; i<6;i++)
                      {
                          if(inFlags[i]!=nullptr && !inFlags[i]->onSurface && locFollowFromLabel(i))
                          {
                              for(int i =0; i<10; i++)
                              {
                                  outFlags[i].block =true;
                              }
                              //&&qDebug()<<"expanding- follow set block";
                          }
                      }
                      //check if neighbor handover is with is blocking, and if so, don't go
                      if(inFlags[followDir]!=nullptr && inFlags[followDir]->block)
                      {
                          //&&qDebug()<<"follow blocked";
                          return Movement(MovementType::Idle);

                      }
                  }

                  int expansionDir = followDir;
                  currentExpansionDir = expansionDir;
                  setContractDir(expansionDir);
                  followDir = updatedFollowDir();
                  headMarkDir = followDir;
                  auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                  Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                  setFollowIndicatorLabel(temp);
                  setFollowNumIndicatorLabel(temp,false);
                  setFollowNumIndicatorLabel(followDir,true);


                  return Movement(MovementType::Expand, expansionDir);


              }
          }
      } else if(phase == Phase::Root) {
          if(isExpanded())
          {

              if(hasNeighborInPhase(Phase::Static) && outFlags[0].block)//curently blocking- check about unblocking
              {
                  bool offSurfaceNeighbor = false;
                  for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
                      auto label = *it;

                      if(inFlags[label]!=nullptr && !neighborIsInPhase(label,Phase::Inactive) && !inFlags[label]->onSurface && locFollowFromLabel(label))
                      {
                          offSurfaceNeighbor = true;
                      }
                  }
                  if(!offSurfaceNeighbor)
                  {
                      //&&qDebug()<<"root unblocking";
                      for(int i =0; i<10;i++)
                      {
                          outFlags[i].block = false;
                      }
                      return Movement(MovementType::Idle);
                  }

              }

              if(hasNeighborInPhase(Phase::Inactive) || hasLocationFollower()) {
                  if(allLocationFollowersExpanded())
                  {
                      return Movement(MovementType::Idle);
                  }
                  return Movement(MovementType::HandoverContract, tailContractionLabel());
              } else {
                  // return Movement(MovementType::Contract, tailContractionLabel());
              }
          }
          else
          {
              downDir= getDownDir();

              if(downDir == -1)
                  return Movement(MovementType::Idle);
              else downDir=labelToDir(downDir);

              Lnumber = getLnumber();
              setLayerNumber(Lnumber);
              int moveDir = getMoveDir();



              setContractDir(moveDir);
              headMarkDir = downDir;
              setPhase(Phase::Root);

              if(hasNeighborInPhase(Phase::Static))
              {
                  for(int i =0; i<6;i++)
                  {
                      if(inFlags[i]!=nullptr)
                      {
                          //&&qDebug()<<i<<" "<<inFlags[i]->onSurface<<locFollowFromLabel(i)<<inFlags[i]->id;
                      }
                      if(inFlags[i]!=nullptr && !inFlags[i]->onSurface && locFollowFromLabel(i))
                      {
                          for(int i =0; i<10; i++)
                          {
                              outFlags[i].block =true;
                          }
                          //&&qDebug()<<"expanding- root set block";
                      }
                  }
                  //check if neighbor handover is with is blocking, and if so, don't go
                  if(inFlags[moveDir]!=nullptr && inFlags[moveDir]->block)
                  {
                      //&&qDebug()<<"root blocked";
                      return Movement(MovementType::Idle);

                  }
              }
              currentExpansionDir = moveDir;
              if(neighborIsInPhase(moveDir,Phase::Follow)||neighborIsInPhase(moveDir,Phase::Root))
              {
                  followDir = labelToDir(moveDir);
                  setFollowIndicatorLabel(followDir);
                  setFollowNumIndicatorLabel(followDir,true);

                  headMarkDir = followDir;

                  setPhase(Phase::Follow);
                  return Movement(MovementType::Idle);
              }
              if(parentOutIndex>-1)
              {
                  if(currentLocData.tokens[parentOutIndex][(int)TokenType::Complaint].value ==1)//if sending a complaint forward- and we know this is contracted
                  {
                      contrLocData.tokens[parentOutIndex][(int)TokenType::Complaint].value = -1;
                      currentLocData.tokens[parentOutIndex][(int)TokenType::Complaint].value = -1;
                      //if at least 1 neighbor is not on the surface, set block. Only neighbors on surface obey block for expanding.
                      for(int i =0; i<6;i++)
                      {
                          if(inFlags[i]!=nullptr && !inFlags[i]->onSurface && locFollowFromLabel(i))
                          {
                              for(int i =0; i<10; i++)
                              {
                                  outFlags[i].block =true;
                              }
                              //&&qDebug()<<"expanding-root set block";
                          }
                      }

                      return Movement(MovementType::Expand, moveDir);
                  }
              }
              else
              {
                  return Movement(MovementType::Idle);
              }

          }
      }
      else if(phase == Phase::LayerRoot) {

          unsetFollowIndicator();

          //want to make sure locally everyone is ready to go to layering
          /*  if(hasNeighborInPhase(Phase::Follow) || hasNeighborInPhase(Phase::Root))
           {
           //&&qDebug()<<"waiting for root/follow";
           return Movement(MovementType::Idle);
           }*/

          //&&qDebug()<<"layerRoot: follow indicator?"<< tailReceivesFollowIndicator();
          if(tailReceivesFollowIndicator())
          {
              headMarkColor = 0x551A8B;
          }
          else
          {
              headMarkColor=0x000000;
          }
          if(isExpanded())
          {

              if(hasNeighborInPhase(Phase::Inactive)||tailReceivesFollowIndicator()) {

                  //&&qDebug()<<"follower/handover "<<tailReceivesFollowIndicator();
                  return Movement(MovementType::HandoverContract, tailContractionLabel());
              } else {
                  return Movement(MovementType::Contract, tailContractionLabel());
              }
          }
          else
          {

              //  if(headMarkDir>-1 && inFlags[headMarkDir]!=nullptr)
              //    //&&qDebug()<<"expanded parent? "<<inFlags[headMarkDir]->isExpanded();

              downDir= getDownDir();
              if(downDir == -1)
              {
                  //&&qDebug()<<"no down dir, root idling";
                  // setPhase(Phase::LayerFollow);
                  return Movement(MovementType::Idle);
              }
              else
                  downDir=labelToDir(downDir);

              Lnumber = getLnumber();
              setLayerNumber(Lnumber);
              int moveDir = getMoveDir();

              //&&qDebug()<<"move dir: "<<moveDir<<" down dir: "<<downDir<<" Layer: "<<Lnumber<<" ret neighbor: "<< neighborIsInPhaseandLayer(moveDir, Phase::retiredLeader, Lnumber);
              if(moveDir!=-1 && inFlags[moveDir]!=nullptr)
              {
                  //&&qDebug()<<"neighbor @movedir: "<<inFlags[moveDir]->id;
              }
              if ( neighborIsInPhaseandLayer(moveDir, Phase::retiredLeader, Lnumber))// ||(hasNeighborInPhase(Phase::Border)))//??aya checke Lnumber nemikhad? ghanunan na age movedir dorost amal karde bashe
              {

                  setPhase(Phase::retiredLeader);
                  getLeftDir();
                  Q_ASSERT(leftDir>=0 && leftDir<=5);
                  NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                  //&&qDebug()<<" returing; num fin neighbors: "<<NumFinishedNeighbors;
                  setNumFinishedNeighbors(NumFinishedNeighbors);
                  headMarkDir = downDir;//-1;

                  setPhase(Phase::retiredLeader);

                  return Movement(MovementType::Idle);
              }
              else
              {
                  for(int label= 0; label<10;label++)
                  {

                      if(inFlags[label]!=nullptr && inFlags[label]->buildBorder)
                      {


                          //  int borderBuildDir = (headMarkDir+3)%6;
                          //  while(neighborIsInPhase(borderBuildDir,Phase::Static) || neighborIsInPhase(borderBuildDir,Phase::StaticBorder))
                          //     borderBuildDir = (borderBuildDir+1)%6;
                          // outFlags[borderBuildDir].buildBorder= true;

                          //&&qDebug()<<"saw build border";
                          setPhase(Phase::retiredLeader);
                          //  getLeftDir();
                          //  Q_ASSERT(leftDir>=0 && leftDir<=5);
                          //  NumFinishedNeighbors = CountFinishedSides(leftDir, rightDir);
                          //  setNumFinishedNeighbors(NumFinishedNeighbors);
                          headMarkDir = downDir;//-1;
                          //&&qDebug()<<"attached to build border, layer #: "<<Lnumber;
                          return Movement(MovementType::Idle);
                      }
                  }
              }

              setContractDir(moveDir);
              headMarkDir = downDir;
              setPhase(Phase::LayerRoot);

              //&&qDebug()<<"lead expand";

              return Movement(MovementType::Expand, moveDir);

          }
      }
      else if (phase == Phase::LayerFollow)
      {
          //&&qDebug()<<"subexecute layer follow";

          if(hasNeighborInPhase(Phase::Follow)||hasNeighborInPhase(Phase::Root))
          {
              return Movement(MovementType::Idle);
          }


          if(isExpanded())
          {
              //&&qDebug()<<"set follow indicator dir to head";
              // setFollowIndicatorLabel(dirToHeadLabel(followDir));
              setFollowNumIndicatorLabel(followDir,true);
              if(hasNeighborInPhase(Phase::Inactive)  || tailReceivesFollowIndicator()) {

                  return Movement(MovementType::HandoverContract, tailContractionLabel());
              }
              else {


                  return Movement(MovementType::Contract, tailContractionLabel());
              }
          }

          else
          {

              if(hasNeighborInPhase(Phase::Root) || hasNeighborInPhase(Phase::Follow))
              {
                  return Movement(MovementType::Idle);
              }
              //&&qDebug()<<"follow dir: "<<followDir;
              //&&qDebug()<<"first root neighbor: "<<firstNeighborInPhase(Phase::LayerRoot);
              //&&qDebug()<<"first retired neighbor: "<<firstNeighborInPhase(Phase::retiredLeader);
              //&&qDebug()<<"first follow neighbor: "<<firstNeighborInPhase(Phase::LayerFollow);
              if(hasNeighborInPhase(Phase::Static)) {
                  setPhase(Phase::LayerRoot);
                  downDir= getDownDir();

                  if(downDir == -1)
                      return Movement(MovementType::Idle);
                  else downDir=labelToDir(downDir);

                  Lnumber = getLnumber();
                  setLayerNumber(Lnumber);
                  unsetFollowIndicator();

                  setPhase(Phase::LayerRoot);
                  return Movement(MovementType::Idle);
              }
              if(hasNeighborInPhase(Phase::retiredLeader)) {
                  setPhase(Phase::LayerRoot);
                  downDir= getDownDir();

                  if(downDir == -1)
                      return Movement(MovementType::Idle);
                  else downDir=labelToDir(downDir);

                  Lnumber = getLnumber();
                  setLayerNumber(Lnumber);
                  unsetFollowIndicator();

                  setPhase(Phase::LayerRoot);

                  return Movement(MovementType::Idle);
              }
              if(inFlags[followDir] == nullptr)
              {
                  followDir = firstNeighborInPhase(Phase::LayerFollow);
                  if(followDir ==-1)
                  {
                      followDir = firstNeighborInPhase(Phase::LayerRoot);
                      qDebug()<<"new follow dir from root";
                  }
                  setFollowIndicatorLabel(followDir);
                  qDebug()<<"new follow dir: "<<followDir;
                  setFollowNumIndicatorLabel(followDir,false);
                  qDebug()<<"null follow dir";

              }
              Q_ASSERT(inFlags[followDir] != nullptr);

              if(inFlags[followDir]->isExpanded() && !inFlags[followDir]->fromHead) {
                  int expansionDir = followDir;
                  setContractDir(expansionDir);
                  followDir = updatedFollowDir();
                  headMarkDir = followDir;
                  auto temp = dirToHeadLabelAfterExpansion(followDir, expansionDir);
                  //&&qDebug()<<"followindicator: "<<followDir<<" new: "<<temp;

                  Q_ASSERT(labelToDirAfterExpansion(temp, expansionDir) == followDir);
                  setFollowIndicatorLabel(temp);
                  setFollowNumIndicatorLabel(temp,false);

                  return Movement(MovementType::Expand, expansionDir);
              }
          }
      }
      return Movement(MovementType::Idle);

  }

  void UniversalCoating::prepLocForContract()
  {

      contrLocData  = headLocData;

      for(int i = 0; i<6; i++)
      {
          if( i == tailDir())//dir is toward tail
          {
              //use next dir -1 to set outflag correctly, sincen nothing is currently on tailDir
              int nextDirLabel  = dirToHeadLabel((i+1)%6);
              int nextValue = outFlags[nextDirLabel].expLocIndex;
              outFlags[i].contrLocIndex = (nextValue+5)%6;//backward around
          }
          else
          {
              outFlags[i].contrLocIndex = outFlags[dirToHeadLabel(i)].expLocIndex;
          }
      }
      for(int i =0; i<6; i++)
      {
          //&&qDebug()<<"i: "<<i<<" index: "<<outFlags[i].contrLocIndex;
      }
  }

  void UniversalCoating::prepLocForExpand(int parentDir, int expansionDir)
  {
      if(parentDir!=-1 && inFlags[parentDir]!=nullptr)
      {
          headLocData = inFlags[parentDir]->expLocData;
          tailLocData = contrLocData;
          int parentExpLocIndex = inFlags[parentDir]->expLocIndex;
          //&&qDebug()<<"parentExpLocIndex: "<<parentExpLocIndex;
          //start at 1 to avoid trying to get a label from a dir that points to the particles own head/tail
          int revExpDir = (expansionDir+3)%6;
          for(int i =1; i<6; i++)
          {

              int newLabel = dirToHeadLabelAfterExpansion((revExpDir+i)%6,expansionDir);
              outFlags[newLabel].expLocIndex =(parentExpLocIndex + i)%6;

          }

      }
      else//make head new- clean headlocData and expLocIndices based on dirs (this could actually be arbitrary)
      {
          //&&qDebug()<<"new head";
          cleanHeadLocData();
          tailLocData = contrLocData;
          for(int i = 0; i<6; i++)//starting at 1 and adding to expansionDir avoids actual expansionDir
          {
              if(i != (expansionDir+3)%6)//if not the direction that will point at tail
              {
                  int currentLabel = dirToHeadLabelAfterExpansion(i,expansionDir);
                  outFlags[currentLabel].expLocIndex = i;
              }

          }
      }
      //prep tail either way
      for(int i =1; i<6; i++)
      {
          int newLabel = dirToTailLabelAfterExpansion((expansionDir+i)%6,expansionDir);
          //&&qDebug()<<"i: "<<i<<" new label: "<<newLabel;
          if(newLabel!=-1)//not between head and tail
          {
              outFlags[newLabel].expLocIndex = outFlags[(expansionDir+i)%6].contrLocIndex;
              //&&qDebug()<<"contr: "<<outFlags[(expansionDir+i)%6].contrLocIndex<<"from dir: "<<(int)((expansionDir+i)%6);
          }
      }

      for(int i =0; i<10; i++)
      {
          //&&qDebug()<<"i: "<<i<<" index: "<<outFlags[i].expLocIndex;
      }
  }

  bool UniversalCoating::canSendTokenForward(TokenType type) const
  {
      return (currentLocData.tokens[parentOutIndex][(int)type].value==-1 && !parentLocData.tokens[parentTokensIndex][(int)type].receivedToken);
  }
  bool UniversalCoating::canSendTokenBackward(TokenType type) const
  {
      return (currentLocData.tokens[followOutIndex][(int)type].value==-1 && !followLocData.tokens[followTokensIndex][(int)type].receivedToken);
  }
  bool UniversalCoating::canSendToken(TokenType type, bool isNextDir) const
  {
      if(isNextDir)
          return canSendTokenForward(type);
      else
          return canSendTokenBackward(type);
  }

  void UniversalCoating::sendTokenForward(TokenType type, int value)
  {
      if(type!=TokenType::Complaint)
      {
          Q_ASSERT(canSendTokenForward(type));
      }
      //&&qDebug()<<"send fwd: "<<(int)type<<" value "<<value;
      currentLocData.tokens[parentOutIndex][(int)type].value = value;
  }

  void UniversalCoating::sendTokenBackward(TokenType type, int value)
  {
      if(type!=TokenType::Complaint)
      {
          Q_ASSERT(canSendTokenBackward(type));
      }
      //&&qDebug()<<"send bkwd: "<<(int)type<<" value "<<value;
      currentLocData.tokens[followOutIndex][(int)type].value = value;
  }
  void UniversalCoating::sendToken(TokenType type,bool isNextDir, int value)
  {
      if(isNextDir)//is next dir
          sendTokenForward(type,value);
      else//is back dir
          sendTokenBackward(type,value);
  }

  int UniversalCoating::peekAtToken(TokenType type, bool isNextDir) const
  {
      if(isNextDir)
      {
          return peekAtParentToken(type);
      }
      else//prev
      {
          return peekAtFollowToken(type);
      }
  }
  int UniversalCoating::peekAtFollowToken(TokenType type) const
  {
      if(currentLocData.tokens[followOutIndex][(int)type].receivedToken)
      {
          return -1;
      }
      else
      {
          return followLocData.tokens[followTokensIndex][(int)type].value;
      }
  }

int UniversalCoating::peekAtParentToken(TokenType type) const {
  if(currentLocData.tokens[parentOutIndex][(int) type].receivedToken) {
    return -1;
  } else {
    return parentLocData.tokens[parentTokensIndex][(int) type].value;
  }
}

Token UniversalCoating::receiveToken(TokenType type, bool isNextDir) {
  Q_ASSERT(peekAtToken(type, isNextDir) != -1);
  if(isNextDir) {
    currentLocData.tokens[parentOutIndex][(int) type].receivedToken = true;
    return parentLocData.tokens[parentTokensIndex][(int) type];
  } else {  // previous dir
    currentLocData.tokens[followOutIndex][(int)type].receivedToken = true;
    return followLocData.tokens[followTokensIndex][(int)type];
  }
}

int UniversalCoating::receiveTokenFromFollow(TokenType type) {
  currentLocData.tokens[followOutIndex][(int) type].receivedToken = true;
  return followLocData.tokens[followTokensIndex][(int) type].value;
}

int UniversalCoating::receiveTokenFromParent(TokenType type) {
  currentLocData.tokens[parentOutIndex][(int) type].receivedToken = true;
  return parentLocData.tokens[parentTokensIndex][(int) type].value;
}


  void UniversalCoating::electionTokenCleanup()
  {

      //&&qDebug()<<"election token cleanup: ";
      for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
          TokenType::CandidacyAnnounce, TokenType::SolitudeLeadL1, TokenType::SolitudeVectorL1, TokenType::BorderTest})
      {
          Token inFromParent = parentLocData.tokens[parentTokensIndex][(int)tokenType];
          Token inFromFollow = followLocData.tokens[followTokensIndex][(int)tokenType];

          if(inFromParent.receivedToken) {
              currentLocData.tokens[parentOutIndex][(int)tokenType].value = -1;

          }
          if(inFromFollow.value == -1) {
              currentLocData.tokens[followOutIndex][(int)tokenType].receivedToken = false;

          }
      }
      for(auto tokenType : {TokenType::PassiveSegment, TokenType::ActiveSegment, TokenType::ActiveSegmentClean, TokenType::CandidacyAck,
          TokenType::SolitudeLeadL2, TokenType::SolitudeVectorL2}) {
              Token inFromParent = parentLocData.tokens[parentTokensIndex][(int)tokenType];
              Token inFromFollow = followLocData.tokens[followTokensIndex][(int)tokenType];
              if(inFromFollow.receivedToken) {
                  currentLocData.tokens[followOutIndex][(int)tokenType].value = -1;

              }
              if(inFromParent.value == -1) {
                  currentLocData.tokens[parentOutIndex][(int)tokenType].receivedToken = false;

              }
          }

  }

  void UniversalCoating::complaintTokenCleanup()
  {
      //&&qDebug()<<"token cleanup: ";
      int type = (int)TokenType::Complaint;

      Token outForward = currentLocData.tokens[parentOutIndex][type];
      Token outBackward = currentLocData.tokens[followOutIndex][type];
      Token inFromParent = parentLocData.tokens[parentTokensIndex][type];
      Token inFromFollow = followLocData.tokens[followTokensIndex][type];
      if(inFromFollow.value == -1)
      {
          currentLocData.tokens[followOutIndex][type].receivedToken = false;
      }

      if(inFromParent.receivedToken && parentTokensIndex>-1)
      {
          //&&qDebug()<<"in from parent, clear token";
          currentLocData.tokens[parentOutIndex][type].value = -1;
      }


  }

  void UniversalCoating::tokenCleanup()
  {
      /*  for(auto tokenType : {TokenType::SegmentLead, TokenType::PassiveSegmentClean, TokenType::FinalSegmentClean,
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
       }*/
  }

  std::shared_ptr<Algorithm> UniversalCoating::blank() const
  {
      return std::make_shared<UniversalCoating>(Phase::Inactive);
  }

  std::shared_ptr<Algorithm> UniversalCoating::clone()
  {
      return std::make_shared<UniversalCoating>(*this);
  }

  bool UniversalCoating::isDeterministic() const
  {
      return true;
  }

  bool UniversalCoating::isStatic() const
  {
      if(phase==Phase::Static)
          return true;
      return false;
  }

  bool UniversalCoating::isRetired() const
  {
      if(phase==Phase::retiredLeader)
          return true;
      return false;
  }

  void UniversalCoating::setPhase(const Phase _phase)
  {
      phase = _phase;
      for(int label = 0; label < 10; label++) {
          outFlags[label].phase = phase;
      }

      if(phase == Phase::retiredLeader){

          headMarkColor = 0x00ff00;
          tailMarkColor = 0x00ff00;

          bool inBorder = false;
          bool outBorder = false;
          for(int i =0; i<6; i++)
          {
              if(inFlags[i]!=nullptr && inFlags[i]->buildBorder)
                  inBorder = true;
              if(outFlags[i].buildBorder)
                  outBorder = true;
          }
          if(inBorder && outBorder)
          {

              headMarkColor = 0x666666;
              tailMarkColor = 0x666666;
          }
          else if(inBorder)
          {
              //  headMarkColor = 0xD3D3D3;
              //  tailMarkColor = 0xD3D3D3;
          }
          else if(outBorder)
          {
              headMarkColor = 0xd6d4d4;
              tailMarkColor = 0xd6d4d4;
          }
      } else if(phase == Phase::Root) {
          headMarkColor = 0xff0000; tailMarkColor = 0xff0000;
      }

      else if(phase == Phase::Inactive) {
          headMarkColor = -1;
          tailMarkColor = -1;
      } else if(phase == Phase::StaticBorder)
      {
          headMarkColor = 0xd3d3d3;
          tailMarkColor = 0xd3d3d3;

      }
      else  if(phase == Phase::Follow)
      {
          headMarkColor = 0xFFA500;
          tailMarkColor = 0xFFA500;
      }
      else if (phase == Phase::LayerFollow)
      {
          headMarkColor = 0xFFFF7F;
          tailMarkColor = 0xFFFF7F;
      }
      else

      { // phase == Phase::Static
          headMarkColor = 0x000000;
          tailMarkColor = 0x000000;
      }
  }

  bool UniversalCoating::neighborIsInPhase(const int label, const Phase _phase) const
  {
      Q_ASSERT(0 <= label && label <= 9);
      return (inFlags[label] != nullptr && inFlags[label]->phase == _phase);
  }

  bool UniversalCoating::neighborIsInPhaseandLayer(const int label, const Phase _phase, const int L) const
  {
      Q_ASSERT(isContracted());

      return (inFlags[label] != nullptr && inFlags[label]->phase == _phase && inFlags[label]->Lnumber == L);
  }

  int UniversalCoating::firstNeighborInPhase(const Phase _phase) const
  {

      for(int label = 0; label < 10; label++) {
          if(neighborIsInPhase(label, _phase)) {
              return label;
          }
      }
      return -1;
  }

  int UniversalCoating::firstNeighborInPhaseandLayer(const Phase _phase, const int L) const
  {
      for(int label = 0; label < 10; label++) {
          if(neighborIsInPhaseandLayer(label, _phase, L)){
              return label;
          }
      }
      return -1;
  }
  bool UniversalCoating::hasNeighborInPhase(const Phase _phase) const
  {
      return (firstNeighborInPhase(_phase) != -1);
  }

  void UniversalCoating::setContractDir(const int contractDir)
  {
      for(int label = 0; label < 10; label++) {
          outFlags[label].contractDir = contractDir;
      }
  }

  int UniversalCoating::updatedFollowDir() const
  {
      int contractDir = inFlags[followDir]->contractDir;
      int offset = (followDir - inFlags[followDir]->dir + 9) % 6;
      int tempFollowDir = (contractDir + offset) % 6;
      Q_ASSERT(0 <= tempFollowDir && tempFollowDir <= 5);
      return tempFollowDir;
  }

  void UniversalCoating::unsetFollowIndicator()
  {
      for(int i = 0; i < 10; i++) {
          outFlags[i].expFollowIndicator=false;
          outFlags[i].contrFollowIndicator=false;
          outFlags[i].followIndicator = false;
      }
  }
  void UniversalCoating::setFollowNumIndicatorLabel(int label, bool contr)
  {
      if(contr)
      {
          for(int i = 0; i < 10; i++) {
              outFlags[i].contrFollowIndicator = (i == label);
          }
      }
      else {
          for(int i = 0; i < 10; i++) {
              outFlags[i].expFollowIndicator = (i == label);
          }

      }
  }

  void UniversalCoating::setFollowIndicatorLabel(const int label)
  {
      for(int i = 0; i < 10; i++) {
          outFlags[i].followIndicator = (i == label);
      }

  }

  bool UniversalCoating::tailReceivesFollowIndicator() const
  {
      for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
          auto label = *it;
          if(inFlags[label] != nullptr) {
              //&&qDebug()<<label<<" check for follower phase: "<<(int)inFlags[label]->phase<<" id: "<<inFlags[label]->id<<"on? "<<inFlags[label]->followIndicator<<" exp: "<<inFlags[label]->expFollowIndicator<<" contr: "<<inFlags[label]->contrFollowIndicator;
              if(
                 (inFlags[label]->expFollowIndicator && inFlags[label]->isExpanded())
                 || (inFlags[label]->contrFollowIndicator && inFlags[label]->isContracted())) {
                  return true;
              }
          }
      }
      return false;
  }
  bool UniversalCoating::hasExpandedFollow()
  {
      for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
          auto label = *it;
          if(inFlags[label] != nullptr) {
              if(inFlags[label]->followIndicator && inFlags[label]->isExpanded()) {
                  return true;
              }
          }
      }
      return false;
  }

  int UniversalCoating::getMoveDir() const
  {
      Q_ASSERT(isContracted());
      Q_ASSERT(Lnumber!=-1);
      int label= -1;
      if(Lnumber %2 == 0)
      {
          label = firstNeighborInPhase(Phase::Static);
          if(label==-1)
              label = firstNeighborInPhase(Phase::StaticBorder);

          if(label != -1)
          {
              label = (label+5)%6;
              while (neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label,Phase::StaticBorder)){
                  label = (label+5)%6;
              }
          }
          else
          {
              label = firstNeighborInPhaseandLayer(Phase::retiredLeader, (Lnumber+1)%2);
              label = (label+5)%6;
              while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){

                  label = (label+5)%6;
              }
          }
      }
      else
      {
          label = firstNeighborInPhase(Phase::Static);
          if(label==-1)
              label = firstNeighborInPhase(Phase::StaticBorder);


          if(label != -1)
          {
              label = (label+1)%6;
              while (neighborIsInPhase(label, Phase::Static)  || neighborIsInPhase(label,Phase::StaticBorder)//  || neighborIsInPhase(label,Phase::Border)
                     ){
                  label = (label+1)%6;
              }
          }
          else
          {
              label = firstNeighborInPhaseandLayer(Phase::retiredLeader, (Lnumber+1)%2);
              label = (label+1)%6;
              while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){
                  label = (label+1)%6;
              }
          }
      }
      Q_ASSERT(0 <= label && label <= 5);
      return labelToDir(label);
  }

  int UniversalCoating::getDownDir() const
  {
      Q_ASSERT(isContracted());
      //Q_ASSERT(phase==Phase::LayerRoot);

      int label= -1;//??
      if(hasNeighborInPhase(Phase::Static)) {
          label = firstNeighborInPhase(Phase::Static);

      }
      else
      {
          int ZeroLayerGreens=0;
          int OneLayerGreens = 0;
          if(hasNeighborInPhase(Phase::retiredLeader))
          {
              ZeroLayerGreens = countGreenNeighbors(Phase::retiredLeader, 0);
              OneLayerGreens =  countGreenNeighbors(Phase::retiredLeader, 1);
              //&&qDebug()<<"counting greens: "<<ZeroLayerGreens<<" "<<OneLayerGreens;
              if(ZeroLayerGreens>0 && OneLayerGreens==0) //down is obviously towards that only layer of retired leaders
              {
                  label= firstNeighborInPhaseandLayer(Phase::retiredLeader, 0);
                  Q_ASSERT(label>=0 && label<=5);
              }
              else if(ZeroLayerGreens==0 && OneLayerGreens>0)
              {
                  label = firstNeighborInPhaseandLayer(Phase::retiredLeader, 1);
                  Q_ASSERT(label>=0 && label<=5);

              }else //from each layer there are some retireds. So we need to decide based on f
              {
                  //&&qDebug()<<"more counting: "<<countRetiredareFinished(0)<<" "<<countRetiredareFinished(1);
                  if(countRetiredareFinished(0) == ZeroLayerGreens)
                  {
                      label= firstNeighborInPhaseandLayer(Phase::retiredLeader, 0);
                  }
                  else if(countRetiredareFinished(1) == OneLayerGreens)
                  {
                      label = firstNeighborInPhaseandLayer(Phase::retiredLeader, 1);
                  }
                  else label= -1;
              }
          }
      }
      return label;
  }

  int UniversalCoating::getLnumber() const
  {
      Q_ASSERT(isContracted());
      Q_ASSERT(downDir != -1);
      Q_ASSERT(inFlags[downDir] != nullptr);
      if(inFlags[downDir] != nullptr)
      {
          if(inFlags[downDir]->phase == Phase::Static)
              return 0;
          else
              return (inFlags[downDir]->Lnumber +1)%2;

      }

      return 0; // RG: previously, the function could reach this point without returning anything which caused a compiler warning
  }

  void UniversalCoating::getLeftDir()
  {
      Q_ASSERT(isContracted());
      Q_ASSERT(phase == Phase::retiredLeader);
      Q_ASSERT(downDir != -1);

      int label= downDir;
      int leftside= -1, rightside= -1;
      if(neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label, Phase::StaticBorder))
      {
          label = (label+5)%6;
          while (neighborIsInPhase(label, Phase::Static)|| neighborIsInPhase(label, Phase::StaticBorder)){
              label = (label+5)%6;
          }
          leftside= label;
      }
      else
      {
          label= downDir;
          if(neighborIsInPhase(label, Phase::retiredLeader))
          {
              label = (label+5)%6;
              while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){
                  label = (label+5)%6;
              }
              leftside= label;
          }
      }

      label= downDir;
      if(neighborIsInPhase(label, Phase::Static)|| neighborIsInPhase(label, Phase::StaticBorder))
      {
          label = (label+1)%6;
          while (neighborIsInPhase(label, Phase::Static) || neighborIsInPhase(label, Phase::StaticBorder)){
              label = (label+1)%6;
          }
          rightside= label;
      }
      else
      {   label= downDir;
          if(neighborIsInPhase(label, Phase::retiredLeader))
          {
              label = (label+1)%6;
              while (neighborIsInPhaseandLayer(label, Phase::retiredLeader, (Lnumber+1)%2)){
                  label = (label+1)%6;
              }
              rightside= label;
          }
      }

      leftDir = labelToDir(leftside);
      rightDir= labelToDir(rightside);
  }

  int UniversalCoating::countGreenNeighbors(const Phase _phase, const int L) const
  {
      int count=0;
      for(int dir = 0; dir < 10; dir++) {
          if(neighborIsInPhaseandLayer(dir, _phase, L) ) {
              count++;
          }
      }
      return count;
  }
  void UniversalCoating::setLayerNumber(const int _Lnumber){
      Q_ASSERT(_Lnumber==0 || _Lnumber==1);
      for(int label = 0; label < 10; label++){
          outFlags[label].Lnumber= _Lnumber;
      }
  }

  int UniversalCoating::CountFinishedSides(const int _leftDir, const int _rightDir) const
  {
      int count=0;
      Q_ASSERT(_leftDir != -1);
      //I have not put leftdit into my outflags but I am accessing thme from my inflags!
      if(neighborIsInPhase(_leftDir, Phase::retiredLeader))
          count++;
      if(neighborIsInPhase(_rightDir, Phase::retiredLeader))
          count++;

      return count;
  }

  int UniversalCoating::countRetiredareFinished(const int _Lnumber) const
  {
      //&&qDebug()<<"count for layer: "<<_Lnumber;
      int countFinished =0;
      for(int dir = 0; dir < 10; dir++) {
          if(neighborIsInPhaseandLayer(dir, Phase::retiredLeader, _Lnumber))
          {
              //&&qDebug()<<"finished neighbors: id: "<<inFlags[dir]->id<<" count: "<<inFlags[dir]->NumFinishedNeighbors;
          }
          if(neighborIsInPhaseandLayer(dir, Phase::retiredLeader, _Lnumber) && inFlags[dir]->NumFinishedNeighbors == 2) {
              countFinished++; //number of retired leaders p has with layer number =0

          }
      }

      return countFinished;
  }

  void UniversalCoating::setNumFinishedNeighbors(const int _NumFinishedNeighbors)
  {
      for(int i = 0; i < 10; i++) {
          outFlags[i].NumFinishedNeighbors = _NumFinishedNeighbors;
      }
  }
  void UniversalCoating::executeLeaderElection(int parent, int follow)
  {
      Q_ASSERT(parent!=-1 && follow !=-1);
      Q_ASSERT(isContracted());
      Q_ASSERT(inFlags[parent]!=nullptr && inFlags[follow]!=nullptr);
      Q_ASSERT(inFlags[parent]->id!=-1 && inFlags[follow]->id!=-1);
      parentTokensIndex = -1;
      parentOutIndex = -1;
      followTokensIndex = -1;
      followOutIndex = -1;

      currentLocData = contrLocData;
      parentOutIndex = outFlags[parent].contrLocIndex;

      if(inFlags[parent]->isContracted())
      {
          parentLocData = inFlags[parent]->contrLocData;
          parentTokensIndex = inFlags[parent]->contrLocIndex;
          //&&qDebug()<<"get parentLocData from: "<<parent<<" id: "<<inFlags[parent]->id<<"parentTokensIndex: "<<parentTokensIndex;
      }
      else
      {
          parentLocData = inFlags[parent]->expLocData;
          parentTokensIndex = inFlags[parent]->expLocIndex;
      }

      followOutIndex = outFlags[follow].contrLocIndex;
      if(inFlags[follow]->isContracted())
      {
          followLocData = inFlags[follow]->contrLocData;
          followTokensIndex = inFlags[follow]->contrLocIndex;
          //&&qDebug()<<"get followLocData from: "<<follow<<" id: "<<inFlags[follow]->id<<"followTokensIndex: "<<followTokensIndex;
      }
      else
      {
          followLocData = inFlags[follow]->expLocData;
          followTokensIndex = inFlags[follow]->expLocIndex;
      }
      Q_ASSERT(parentTokensIndex!=-1);
      Q_ASSERT(followTokensIndex!=-1);
      Q_ASSERT(parentOutIndex!=-1);
      Q_ASSERT(followOutIndex!=-1);


      electionTokenCleanup();
      subExecuteLeaderElection(parent,follow);
      contrLocData = currentLocData;


  }
  void UniversalCoating::subExecuteLeaderElection(int parentDir, int followDir)
  {
      //&&qDebug()<<"sub LE";
      bool prevAgentDir = false;
      bool nextAgentDir  = true;


      if(currentLocData.electionRole == ElectionRole::Candidate) {
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
              if(currentLocData.switches.at((int)SwitchVariable::absorbedActiveToken) == 0) {
                  if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // absorbing the last active token
                      sendToken(TokenType::FinalSegmentClean, nextAgentDir, 2);
                  } else { // this candidate is now covered
                      Q_ASSERT(canSendToken(TokenType::ActiveSegmentClean, prevAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                      sendToken(TokenType::PassiveSegmentClean, nextAgentDir, 1);
                      performPassiveClean(2); // clean the front side only
                      paintFrontSegment(QColor("dimgrey").rgb());
                      sendToken(TokenType::ActiveSegmentClean, prevAgentDir, 1);
                      performActiveClean(1); // clean the back side only
                      currentLocData.switches.at((int)SwitchVariable::absorbedActiveToken) = 1;
                      currentLocData.switches.at((int)SwitchVariable::isCoveredCandidate) = 1;
                      //&&qDebug()<<"demoted";
                      currentLocData.electionRole  = ElectionRole::Demoted;
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
              if(currentLocData.switches.at((int)SwitchVariable::waitingForTransferAck) == 1) {
                  currentLocData.switches.at((int)SwitchVariable::gotAnnounceBeforeAck) = 1;
              } else if( currentLocData.switches.at((int)SwitchVariable::comparingSegment)==1) {
                  currentLocData.switches.at((int)SwitchVariable::gotAnnounceInCompare) = 1;
              }
          }

          // if there is a solitude lead token waiting to be put into lane 2, put it there if it doesn't pass a vector token
          if(peekAtToken(TokenType::SolitudeLeadL1, prevAgentDir) != -1 && canSendToken(TokenType::SolitudeLeadL2, prevAgentDir) &&
             canSendToken(TokenType::SolitudeVectorL2, prevAgentDir)) {
              int leadValue = receiveToken(TokenType::SolitudeLeadL1, prevAgentDir).value;
              if(leadValue / 100 == 1) { // if the lead is on lap 1, append this candidate's local id
                  sendToken(TokenType::SolitudeLeadL2, prevAgentDir, (1000 * 1) + leadValue);
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
          if(currentLocData.electionSubphase == ElectionSubphase::SegmentComparison) {
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
                  if(finalCleanValue == 0 &&  currentLocData.switches.at((int)SwitchVariable::gotAnnounceInCompare)==0) { // if this candidate did not cover any tokens and was not transferred candidacy, demote
                      //&&qDebug()<<"demote 2";
                      //  currentLocData.electionRole = ElectionRole::Demoted;
                      //moved:
                      currentLocData.electionSubphase = ElectionSubphase::CoinFlip;

                  } else {
                      currentLocData.electionSubphase = ElectionSubphase::CoinFlip;
                  }
                  currentLocData.switches.at((int)SwitchVariable::comparingSegment) = 0;
                  currentLocData.switches.at((int)SwitchVariable:: gotAnnounceInCompare) = 0;
                  return; // completed subphase and thus shouldn't perform any more operations in this round
              } else if( currentLocData.switches.at((int)SwitchVariable::comparingSegment)==0) {
                  // begin segment comparison by generating a segment lead token
                  Q_ASSERT(canSendToken(TokenType::SegmentLead, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir));
                  sendToken(TokenType::SegmentLead, nextAgentDir, 1);
                  paintFrontSegment(QColor("red").rgb());
                  currentLocData.switches.at((int)SwitchVariable::comparingSegment) = 1;
              }
          } else if(currentLocData.electionSubphase == ElectionSubphase::CoinFlip) {
              if(peekAtToken(TokenType::CandidacyAck, nextAgentDir) != -1) {
                  // if there is an acknowledgement waiting, consume the acknowledgement and proceed to the next subphase
                  receiveToken(TokenType::CandidacyAck, nextAgentDir);
                  paintFrontSegment(QColor("dimgrey").rgb());
                  currentLocData.electionSubphase= ElectionSubphase::SolitudeVerification;
                  if( currentLocData.switches.at((int)SwitchVariable::gotAnnounceBeforeAck)==0) {
                      currentLocData.electionRole = ElectionRole::Demoted;
                  }
                  currentLocData.switches.at((int)SwitchVariable::waitingForTransferAck) = 0;
                  currentLocData.switches.at((int)SwitchVariable:: gotAnnounceBeforeAck) = 0;
                  return; // completed subphase and thus shouldn't perform any more operations in this round
              } else if( currentLocData.switches.at((int)SwitchVariable::waitingForTransferAck)==0 && randBool()) {
                  //&&qDebug()<<"won coin flip";
                  // if I am not waiting for an acknowlegdement of my previous announcement and I win the coin flip, announce a transfer of candidacy
                  Q_ASSERT(canSendToken(TokenType::CandidacyAnnounce, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two announcements
                  sendToken(TokenType::CandidacyAnnounce, nextAgentDir, 1);
                  paintFrontSegment(QColor("orange").rgb());
                  currentLocData.switches.at((int)SwitchVariable::waitingForTransferAck )= 1;
              }
              else
              {
                  //&&qDebug()<<"lost coin flip?";
              }

          } else if(currentLocData.electionSubphase == ElectionSubphase::SolitudeVerification) {
              // if the agent needs to, generate a lane 1 vector token
              if( currentLocData.switches.at((int)SwitchVariable::generateVectorDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                  sendToken(TokenType::SolitudeVectorL1, nextAgentDir,  currentLocData.switches.at((int)SwitchVariable::generateVectorDir));
                  currentLocData.switches.at((int)SwitchVariable:: generateVectorDir) = -1;
              }

              if(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != -1) {
                  // consume all vector tokens that have not been matched, decide that solitude has failed
                  Q_ASSERT(peekAtToken(TokenType::SolitudeVectorL2, nextAgentDir) != 0); // matched tokens should have dropped
                  receiveToken(TokenType::SolitudeVectorL2, nextAgentDir);
                  currentLocData.switches.at((int)SwitchVariable::sawUnmatchedToken )= 1;
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
                      if( currentLocData.switches.at((int)SwitchVariable::sawUnmatchedToken)==0 && (leadValue / 1000) == 1) { // if it did not consume an unmatched token and it assures it's matching with itself, go to inner/outer test
                          //&&qDebug()<<"sole candidate set";
                          currentLocData.electionRole = ElectionRole::SoleCandidate;
                      } else { // if solitude verification failed, then do another coin flip compeititon
                          currentLocData.electionSubphase = ElectionSubphase::SegmentComparison;
                      }
                      currentLocData.switches.at((int)SwitchVariable::createdLead )= 0;
                      currentLocData.switches.at((int)SwitchVariable::sawUnmatchedToken )= 0;
                      return; // completed subphase and thus shouldn't perform any more operations in this round
                  }
              } else if( currentLocData.switches.at((int)SwitchVariable::createdLead)==0) {
                  // to begin the solitude verification, create a lead token with an orientation to communicate to its segment
                  Q_ASSERT(canSendToken(TokenType::SolitudeLeadL1, nextAgentDir) && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) &&
                           canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)); // there shouldn't be a call to make two leads
                  sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + encodeVector(std::make_pair(1,0))); // lap 1 in direction (1,0)
                  paintFrontSegment(QColor("deepskyblue").darker().rgb());
                  currentLocData.switches.at((int)SwitchVariable::createdLead) = 1;
                  currentLocData.switches.at((int)SwitchVariable:: generateVectorDir )= encodeVector(std::make_pair(1,0));
              }
          }
      } /*else if(currentLocData.electionRole == ElectionRole::SoleCandidate) {
         if( currentLocData.switches.at((int)SwitchVariable::testingBorder)==0) { // begin the inner/outer border test
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
         }*/ else if(currentLocData.electionRole == ElectionRole::Demoted) {
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
                 if( currentLocData.switches.at((int)SwitchVariable::absorbedActiveToken)==0) {
                     if(receiveToken(TokenType::ActiveSegment, nextAgentDir).value == 2) { // if absorbing the final active token, generate the final cleaning token
                         sendToken(TokenType::FinalSegmentClean, nextAgentDir, 0); // the final segment cleaning token begins as having not seen covered candidates
                     } else {
                         currentLocData.switches.at((int)SwitchVariable:: absorbedActiveToken) = 1;
                     }
                 } else if(canSendToken(TokenType::ActiveSegment, prevAgentDir) && canSendToken(TokenType::ActiveSegmentClean, prevAgentDir)) {
                     // pass active token backward if doing so does not pass the active cleaning token
                     sendToken(TokenType::ActiveSegment, prevAgentDir, receiveToken(TokenType::ActiveSegment, nextAgentDir).value);
                 }
             }
             // pass final cleaning token forward, perform cleaning, and check for covered candidates
             if(peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1 && canSendToken(TokenType::FinalSegmentClean, nextAgentDir)) {
                 int finalCleanValue = receiveToken(TokenType::FinalSegmentClean, prevAgentDir).value;
                 if(finalCleanValue != 2 &&  currentLocData.switches.at((int)SwitchVariable::isCoveredCandidate)==1) {
                     finalCleanValue = 1;
                 }
                 currentLocData.switches.at((int)SwitchVariable::absorbedActiveToken) = 0;
                 currentLocData.switches.at((int)SwitchVariable::isCoveredCandidate) = 0;
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
                     int offset = (parentDir - ((followDir + 3) % 6) + 6) % 6;
                     currentLocData.switches.at((int)SwitchVariable::generateVectorDir) = encodeVector(augmentDirVector(leadVector, offset));
                     sendToken(TokenType::SolitudeLeadL1, nextAgentDir, 100 + currentLocData.switches.at((int)SwitchVariable:: generateVectorDir)); // lap 1 + new encoded direction vector
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
             if( currentLocData.switches.at((int)SwitchVariable::generateVectorDir) != -1 && canSendToken(TokenType::SolitudeVectorL1, nextAgentDir) && canSendToken(TokenType::PassiveSegmentClean, nextAgentDir)) {
                 sendToken(TokenType::SolitudeVectorL1, nextAgentDir,  currentLocData.switches.at((int)SwitchVariable::generateVectorDir));
                 currentLocData.switches.at((int)SwitchVariable::generateVectorDir) = -1;
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
             /* if(peekAtToken(TokenType::BorderTest, prevAgentDir) != -1 && canSendToken(TokenType::BorderTest, nextAgentDir)) {
              int borderSum = addNextBorder(receiveToken(TokenType::BorderTest, prevAgentDir).value);
              sendToken(TokenType::BorderTest, nextAgentDir, borderSum);
              paintFrontSegment(-1); paintBackSegment(-1);
              setState(State::Finished);
              }*/
         }
  }

void UniversalCoating::paintFrontSegment(const int color) {
    /*  for(int i = 0; i<18; i++)
     {
     if(i!=(3 * ( headMarkDir+2)%6 + 1))
     .at(i) = -1;
     }
     int tempDir =( headMarkDir+2)%6;
     while(tempDir != (headMarkDir + 1) % 6) {
     if((tempDir - 1 + 6) % 6 != headMarkDir) {
     borderColors.at((3 * tempDir - 1 + 18) % 18) = color;
     }
     tempDir = (tempDir - 1 + 6) % 6;
     }*/
    // borderColors.at()
}

  void UniversalCoating::paintBackSegment(const int color) {
      if(!hasNeighborInPhase(Phase::Static)) return;
      int downdir = firstNeighborInPhase(Phase::Static);

     if(neighborIsInPhase((downdir+1)%6,Phase::Static)){
          downdir=(downdir+1)%6;
         // while (neighborIsInPhase(downdir,Phase::Static))
          //    downdir=(downdir+1)%6;
      }
      else if (neighborIsInPhase((downdir+5)%6,Phase::Static)){
         downdir=(downdir+5)%6;
         // while (neighborIsInPhase(downdir,Phase::Static))
         //     downdir=(downdir+5)%6;
      }

    //  borderColors.at(3 * ( downdir+2)%6 + 2) = color;
     for(int i =0; i<18;i++)
         borderColors.at(i) = QColor("white").rgb();
      borderColors.at(downdir*3+1)=color;
  }

  void UniversalCoating::executeComplaintPassing(int parent)
  {

      //Q_ASSERT(parent!=-1);
      if(parent==-1)
      {
          qDebug()<<"-1 parent";
          return;
      }
      parentTokensIndex = -1;
      parentOutIndex = -1;
      followTokensIndex = -1;
      followOutIndex = -1;
      if(isContracted())
      {
          currentLocData = contrLocData;
          bool hasParent = false;
          parentOutIndex = outFlags[parent].contrLocIndex;//even if no parent, set this
          if(parent!=-1 && inFlags[parent]!=nullptr)
          {
              hasParent = true;
              if(inFlags[parent]->isContracted())
              {
                  parentLocData = inFlags[parent]->contrLocData;
                  parentTokensIndex = inFlags[parent]->contrLocIndex;
                  //&&qDebug()<<"get parentLocData from: "<<parent<<" id: "<<inFlags[parent]->id<<"parentTokensIndex: "<<parentTokensIndex;
              }
              else
              {
                  parentLocData = inFlags[parent]->expLocData;
                  parentTokensIndex = inFlags[parent]->expLocIndex;
              }

          }

          //find first follow with something between current and it going either way, if there is one
          bool foundFollow  =false;
          int followerDir;

          //find neighbors who are not static and not on surface
          bool followerOffSurface = false;
          for(int i =0; i<6;i++)
          {
              if(inFlags[i]!=nullptr && !neighborIsInPhase(i,Phase::Static) && !inFlags[i]->onSurface )
              {
                  int followersIndex = inFlags[i]->contrLocIndex;
                  LocData followLocData = inFlags[i]->contrLocData;
                  if(inFlags[i]->isExpanded() )
                  {
                      followersIndex = inFlags[i]->expLocIndex;
                      followLocData = inFlags[i]->expLocData;
                  }
                  //if someone behind has a loc parent that is the same as the index pointed at me
                  if(followLocData.parentIndex == followersIndex)
                  {
                      followerOffSurface = true;
                      break;

                  }
              }
          }
          for(int i =0; i<6; i++)
          {
              if(i!=parent && inFlags[i]!=nullptr && !neighborIsInPhase(i,Phase::Static) &&
                 (!followerOffSurface ||(followerOffSurface && !inFlags[i]->onSurface)))//either no follower off surface or this one is a follower off surface
              {
                  //info for outward stuff in this direction
                  int tokenIndex = outFlags[i].contrLocIndex;
                  Token outToken  = currentLocData.tokens[tokenIndex][(int)TokenType::Complaint];

                  //info for inward from this direction
                  int inTokenIndex =inFlags[i]->contrLocIndex;
                  LocData checkFollowData = inFlags[i]->contrLocData;
                  if(inFlags[i]->isExpanded())
                  {
                      inTokenIndex =inFlags[i]->expLocIndex;
                      checkFollowData = inFlags[i]->expLocData;
                  }
                  Token inToken = checkFollowData.tokens[inTokenIndex][(int)TokenType::Complaint];


                  if(outToken.value != -1 || outToken.receivedToken || inToken.value !=-1 || inToken.receivedToken)
                  {
                      followLocData = checkFollowData;
                      followTokensIndex = inTokenIndex;
                      followOutIndex = outFlags[i].contrLocIndex;
                      foundFollow = true;
                      followerDir = i;
                      //&&qDebug()<<"found follow: "<<followerDir<<" id: "<<inFlags[followerDir]->id;

                      break;
                  }
              }
          }


          //cleanup
          complaintTokenCleanup();
          //try to receive + send
          if(foundFollow)
          {
              //if peek at follow and can send
              if(peekAtFollowToken(TokenType::Complaint)!=-1 && (canSendTokenForward(TokenType::Complaint) || !hasParent))
              {
                  //send token(value = receive token)
                  int receiveValue = receiveTokenFromFollow(TokenType::Complaint);
                  sendTokenForward(TokenType::Complaint,receiveValue);
                  //&&qDebug()<<"received and send complaint: "<<receiveValue;
              }
          }
          contrLocData = currentLocData;
      }
      else//is expanded
      {
          //execute head:
          currentLocData = headLocData;
          bool hasParent = false;
          parentOutIndex = outFlags[parent].expLocIndex;//even if no parent, set this
          if(parent!=-1 && inFlags[parent]!=nullptr)
          {
              hasParent = true;
              if(inFlags[parent]->isContracted())
              {
                  parentLocData = inFlags[parent]->contrLocData;
                  parentTokensIndex = inFlags[parent]->contrLocIndex;
                  //&&qDebug()<<"get parentLocData(C) from: "<<parent<<" id: "<<inFlags[parent]->id<<"parentTokensIndex: "<<parentTokensIndex;
              }
              else
              {
                  parentLocData = inFlags[parent]->expLocData;
                  parentTokensIndex = inFlags[parent]->expLocIndex;
                  //&&qDebug()<<"get parentLocData(E) from: "<<parent<<" id: "<<inFlags[parent]->id<<"parentTokensIndex: "<<parentTokensIndex;

              }

          }

          //head prefers to receive from tail (ie follow = tail), otherwise looks
          followLocData = tailLocData;
          followTokensIndex = (outFlags[(dirToHeadLabel((tailDir()+1)%6)+9)%10].expLocIndex+1)%6;
          followOutIndex =(outFlags[dirToHeadLabel((tailDir()+1)%6)].expLocIndex +5)%6;
          //&&qDebug()<<"in value from tail: "<<tailLocData.tokens[followTokensIndex][(int)TokenType::Complaint].value<<" index: "<<followTokensIndex;

          //cleanup
          complaintTokenCleanup();
          //try to receive + send
          //if peek at follow and can send
          if(peekAtFollowToken(TokenType::Complaint)!=-1 && (canSendTokenForward(TokenType::Complaint) || !hasParent))
          {
              //send token(value = receive token)
              int receiveValue = receiveTokenFromFollow(TokenType::Complaint);
              sendTokenForward(TokenType::Complaint,receiveValue);
              //&&qDebug()<<"head received and send complaint: "<<receiveValue;
          }

          headLocData = currentLocData;

          //execute tail:
          parentTokensIndex = -1;
          parentOutIndex = -1;
          followTokensIndex = -1;
          followOutIndex = -1;

          currentLocData = tailLocData;
          //need: parentLocData, parentTOkensIndex, parentOutIndex
          parentLocData = headLocData;
          parentOutIndex = (outFlags[(dirToHeadLabel((tailDir()+1)%6)+9)%10].expLocIndex+1)%6;
          parentTokensIndex =(outFlags[dirToHeadLabel((tailDir()+1)%6)].expLocIndex +5)%6;


          bool foundFollow  =false;
          hasParent = true;
          int followerDir;

          //find neighbors who are not static and not on surface
          bool followerOffSurface = false;
          for(int i =0; i<6;i++)
          {
              if(inFlags[i]!=nullptr && !neighborIsInPhase(i,Phase::Static) && !inFlags[i]->onSurface )
              {
                  int followersIndex = inFlags[i]->contrLocIndex;
                  LocData followLocData = inFlags[i]->contrLocData;
                  if(inFlags[i]->isExpanded() )
                  {
                      followersIndex = inFlags[i]->expLocIndex;
                      followLocData = inFlags[i]->expLocData;
                  }
                  //if someone behind has a loc parent that is the same as the index pointed at me
                  if(followLocData.parentIndex == followersIndex)
                  {
                      followerOffSurface = true;
                      break;

                  }
              }
          }
          for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
              auto i = *it;

              if(i!=parent && inFlags[i]!=nullptr && !neighborIsInPhase(i,Phase::Static) &&
                 (!followerOffSurface ||(followerOffSurface && !inFlags[i]->onSurface)))
              {
                  //info for outward stuff in this direction
                  int tokenIndex = outFlags[i].expLocIndex;
                  Token outToken  = currentLocData.tokens[tokenIndex][(int)TokenType::Complaint];

                  //info for inward from this direction
                  int inTokenIndex =inFlags[i]->contrLocIndex;
                  LocData checkFollowData = inFlags[i]->contrLocData;
                  if(inFlags[i]->isExpanded())
                  {
                      inTokenIndex =inFlags[i]->expLocIndex;
                      checkFollowData = inFlags[i]->expLocData;
                  }
                  Token inToken = checkFollowData.tokens[inTokenIndex][(int)TokenType::Complaint];


                  if(outToken.value != -1 || outToken.receivedToken || inToken.value !=-1 || inToken.receivedToken)
                  {
                      followLocData = checkFollowData;
                      followTokensIndex = inTokenIndex;
                      followOutIndex = outFlags[i].expLocIndex;
                      foundFollow = true;
                      followerDir = i;
                      //&&qDebug()<<"Tail found follow: "<<followerDir<<" id: "<<inFlags[followerDir]->id;
                      break;
                  }
              }
          }
          //cleanup
          complaintTokenCleanup();
          //try to receive + send
          //if peek at follow and can send
          if(foundFollow)
          {
              if(peekAtFollowToken(TokenType::Complaint)!=-1 && (canSendTokenForward(TokenType::Complaint) || !hasParent))
              {
                  //send token(value = receive token)
                  int receiveValue = receiveTokenFromFollow(TokenType::Complaint);
                  sendTokenForward(TokenType::Complaint,receiveValue);
                  //&&qDebug()<<"tail received and send complaint: "<<receiveValue;
              }
          }
          else
          {
              //&&qDebug()<<"tail has no follow";

          }
          for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
              auto label = *it;
              if(inFlags[label]==nullptr)
              {
                  currentLocData.tokens[outFlags[label].expLocIndex][(int)TokenType::Complaint].receivedToken = false;
              }
          }
          tailLocData = currentLocData;

      }
  }

  void UniversalCoating::performPassiveClean(const int region)
  {
      int prevAgentDir = 0;
      int nextAgentDir = 1;
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

  void UniversalCoating::performActiveClean(const int region)
  {
      int prevAgentDir = 0;
      int nextAgentDir = 1;
      Q_ASSERT(1 <= region && region <= 3); // 1 => back, 2 => front, 3 => both
      if(region != 2 && peekAtToken(TokenType::FinalSegmentClean, prevAgentDir) != -1) { // back
          receiveToken(TokenType::FinalSegmentClean, prevAgentDir);
      }
      if(region != 1 && peekAtToken(TokenType::ActiveSegment, nextAgentDir) != -1) { // front
          receiveToken(TokenType::ActiveSegment, nextAgentDir);
      }
      currentLocData.switches.at((int)SwitchVariable::absorbedActiveToken) = 0;
  }
  int UniversalCoating::encodeVector(std::pair<int, int> vector) const
  {
      int x = (vector.first == -1) ? 2 : vector.first;
      int y = (vector.second == -1) ? 2 : vector.second;

      return (10 * x) + y;
  }

  std::pair<int, int> UniversalCoating::decodeVector(int code)
  {
      code = code % 100; // throw out the extra information for lap number
      int x = (code / 10 == 2) ? -1 : code / 10;
      int y = (code % 10 == 2) ? -1 : code % 10;

      return std::make_pair(x, y);
  }

  std::pair<int, int> UniversalCoating::augmentDirVector(std::pair<int, int> vector, const int offset)
  {
      static const std::array<std::pair<int, int>, 6> vectors = {std::make_pair(1,0), std::make_pair(0,1), std::make_pair(-1,1),
          std::make_pair(-1,0), std::make_pair(0,-1), std::make_pair(1,-1)};
      for(uint i = 0; i < vectors.size(); ++i) {
          if(vector == vectors.at(i)) {
              return vectors.at((i + offset) % 6);
          }
      }

      Q_ASSERT(false); // the desired vector should be one of the unit directions
      return std::make_pair(0,0);
  }

  int UniversalCoating::addNextBorder(int currentSum, int prevAgentDir, int nextAgentDir)
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

  void UniversalCoating::cleanHeadLocData()
  {
      for (int dirIndex =0; dirIndex<6; dirIndex++)
      {
          for(int tokenIndex= 0 ; tokenIndex<16; tokenIndex++)
          {
              headLocData.tokens[dirIndex][tokenIndex].type = (TokenType)tokenIndex;
              headLocData.tokens[dirIndex][tokenIndex].value = -1;
              headLocData.tokens[dirIndex][tokenIndex].receivedToken =false;

          }
      }


      headLocData.parentIndex = -1;
      headLocData.electionRole = ElectionRole::Demoted;
      headLocData.electionSubphase = ElectionSubphase::SegmentComparison;



      for (uint i = 0; i < headLocData.switches.size(); ++i)
      {
          headLocData.switches.at(i) = 0;
      }
      headLocData.switches.at((int)SwitchVariable::generateVectorDir)=-1;

  }
  void UniversalCoating::cleanTailLocData()
  {
      for (int dirIndex =0; dirIndex<6; dirIndex++)
      {
          for(int tokenIndex= 0 ; tokenIndex<16; tokenIndex++)
          {
              tailLocData.tokens[dirIndex][tokenIndex].type = (TokenType)tokenIndex;
              tailLocData.tokens[dirIndex][tokenIndex].value = -1;
              tailLocData.tokens[dirIndex][tokenIndex].receivedToken =false;

          }
      }

      tailLocData.parentIndex = -1;

      tailLocData.electionRole = ElectionRole::Demoted;

      tailLocData.electionSubphase = ElectionSubphase::SegmentComparison;


      for (uint i = 0; i < tailLocData.switches.size(); ++i)
      {
          tailLocData.switches.at(i) = 0;
      }
      tailLocData.switches.at((int)SwitchVariable::generateVectorDir)=-1;

  }
  void UniversalCoating::cleanContrLocData()
  {
      for (int dirIndex =0; dirIndex<6; dirIndex++)
      {
          for(int tokenIndex= 0 ; tokenIndex<16; tokenIndex++)
          {
              contrLocData.tokens[dirIndex][tokenIndex].type = (TokenType)tokenIndex;
              contrLocData.tokens[dirIndex][tokenIndex].value = -1;
              contrLocData.tokens[dirIndex][tokenIndex].receivedToken =false;

          }
      }

      contrLocData.parentIndex = -1;
      contrLocData.electionRole = ElectionRole::Demoted;

      contrLocData.electionSubphase = ElectionSubphase::SegmentComparison;


      for (uint i = 0; i < contrLocData.switches.size(); ++i) {
        contrLocData.switches.at(i) = 0;
      }
      contrLocData.switches.at((int)SwitchVariable::generateVectorDir)=-1;
  }

  bool UniversalCoating::expandedOnSurface()
  {
      bool tailOnSurface = false;
      bool headOnSurface = false;
      for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
          auto label = *it;
          if(neighborIsInPhase(label,Phase::Static))
              tailOnSurface = true;
      }
      for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
          auto label = *it;
          if(neighborIsInPhase(label,Phase::Static))
              headOnSurface = true;
      }
      return tailOnSurface && headOnSurface;
  }
  bool UniversalCoating::tailOnSurface()
  {
      bool tailOnSurface = false;
      for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
          auto label = *it;
          if(neighborIsInPhase(label,Phase::Static))
              tailOnSurface = true;
      }

      return tailOnSurface ;
  }
  int UniversalCoating::getSurfaceFollowDir()
  {
      auto surfaceFollower = firstNeighborInPhase(Phase::Static);
      if(surfaceFollower!=-1)
      {
          if(isExpanded())
          {
              while(neighborIsInPhase(surfaceFollower,Phase::Static))
              {
                  surfaceFollower = (surfaceFollower+1)%10;
              }
          }
          else
          {
              while(neighborIsInPhase(surfaceFollower,Phase::Static))
              {
                  surfaceFollower = (surfaceFollower+1)%6;
              }
          }
          return surfaceFollower;

      }
      return -1;
  }
  int UniversalCoating::getSurfaceParentDir()
  {
      auto surfaceParent = firstNeighborInPhase(Phase::Static);
      //&&qDebug()<<"first static neighbor: "<<surfaceParent;
      if(surfaceParent!=-1)
      {
          if(isExpanded())
          {
              while(neighborIsInPhase(surfaceParent,Phase::Static))
              {
                  surfaceParent = (surfaceParent+9)%10;
              }
          }
          else
          {
              while(neighborIsInPhase(surfaceParent,Phase::Static))
              {
                  surfaceParent = (surfaceParent+5)%6;
              }
          }
          return surfaceParent;

      }
      return -1;
  }
  int UniversalCoating::locDataParentToLabel()
  {

      if(isExpanded())
      {

          for(auto it = headLabels().cbegin(); it != headLabels().cend(); ++it) {
              auto label = *it;
              if(outFlags[label].expLocIndex == headLocData.parentIndex)
              {
                  return label;
              }

          }
          //label not returned- head is new or something
          Q_ASSERT(false);
          return -1;

      }
      else
      {
          for(int i =0; i<6; i++)
          {
              if(outFlags[i].contrLocIndex == contrLocData.parentIndex)
              {
                  return i;
              }
          }
          //&&qDebug()<<"didn't find label for locDataParent"<<contrLocData.parentIndex;

          return -1;
      }

  }

bool UniversalCoating::hasLocationFollower() {
  Q_ASSERT(isExpanded());
  for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
    auto label = *it;
    if(inFlags[label] != nullptr && !neighborIsInPhase(label, Phase::Static)) { // && inFlags[label]->isContracted())//someone there
      int followersIndex = inFlags[label]->contrLocIndex;
      LocData followLocData = inFlags[label]->contrLocData;
      if(inFlags[label]->isExpanded()) {
        followersIndex = inFlags[label]->expLocIndex;
        followLocData = inFlags[label]->expLocData;
      }

      // if someone behind has a loc parent that is the same as the index pointed at me
      if(followLocData.parentIndex == followersIndex) {
        return true;
      }
    }
  }

  return false;
}

bool UniversalCoating::allLocationFollowersExpanded() {
  Q_ASSERT(isExpanded());
  bool locFollowsExpanded = true;

  // find neighbors who are not static and not on surface
  bool followerOffSurface = false;
  for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
    auto label = *it;
    if(inFlags[label]!=nullptr && !neighborIsInPhase(label, Phase::Static) && !inFlags[label]->onSurface) {
      int followersIndex = inFlags[label]->contrLocIndex;
      LocData followLocData = inFlags[label]->contrLocData;
      if(inFlags[label]->isExpanded()) {
        followersIndex = inFlags[label]->expLocIndex;
        followLocData = inFlags[label]->expLocData;
      }

      // if someone behind has a loc parent that is the same as the index pointed at me
      if(followLocData.parentIndex == followersIndex) {
        followerOffSurface = true;
        break;
      }
    }
  }

  // if a follower is off the surface we want to take care of them, so check if
  // all followers off surface are expanded by returning true
  if(followerOffSurface) {
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
      auto label = *it;
      if(inFlags[label] != nullptr && !neighborIsInPhase(label, Phase::Static) && !inFlags[label]->onSurface) { //&& inFlags[label]->isContracted())//someone there
        int followersIndex = inFlags[label]->contrLocIndex;
        LocData followLocData = inFlags[label]->contrLocData;
        if(inFlags[label]->isExpanded()) {
          followersIndex = inFlags[label]->expLocIndex;
          followLocData = inFlags[label]->expLocData;
        }

        // if someone behind has a loc parent that is the same as the index (+
        // this block offsurface) pointed at me
        if(followLocData.parentIndex == followersIndex) {
          if(inFlags[label]->isContracted()) {
            locFollowsExpanded = false;
            break;
          }
        }
      }
    }
  } else {
    for(auto it = tailLabels().cbegin(); it != tailLabels().cend(); ++it) {
      auto label = *it;
      if(inFlags[label]!=nullptr && !neighborIsInPhase(label, Phase::Static)) {  //&& inFlags[label]->isContracted())//someone there
        int followersIndex = inFlags[label]->contrLocIndex;
        LocData followLocData = inFlags[label]->contrLocData;
        if(inFlags[label]->isExpanded()) {
          followersIndex = inFlags[label]->expLocIndex;
          followLocData = inFlags[label]->expLocData;
        }

        // if someone behind has a loc parent that is the same as the index pointed at me
        if(followLocData.parentIndex == followersIndex) {
          if(inFlags[label]->isContracted()) {
              locFollowsExpanded = false;
              break;
          }
        }
      }
    }
  }

  return locFollowsExpanded;
}

bool UniversalCoating::locFollowFromLabel(int label) {
  if(inFlags[label] == nullptr || inFlags[label]->phase == Phase::Static) {
    return false;
  }

  int followersIndex = inFlags[label]->contrLocIndex;
  LocData followLocData = inFlags[label]->contrLocData;
  if(inFlags[label]->isExpanded()) {
    followersIndex = inFlags[label]->expLocIndex;
    followLocData = inFlags[label]->expLocData;
  }

  // if someone behind has a loc parent that is the same as the index pointed at me
  return followLocData.parentIndex == followersIndex;
}

}  // namespace UniversalCoating
