#ifndef GENERALSHAPEFORMATION_H
#define GENERALSHAPEFORMATION_H


#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"
#include <stack>


class GSFParticle:public AmoebotParticle{
    public:
        //states that a particle can be in
        enum class State{
            COORDINATOR,
            CHAIN_COORDINATOR,
            CHAIN_FOLLOWER
        };

        //constructor for TriangleParticle
        GSFParticle(Node& head, const int globalTailDir,
                    const int orientation, AmoebotSystem& system,
                    const int sideLen, int triangleDirection, State state, unsigned int level,
                    unsigned int ldrlabel, int depth);

        //what to be done during activation
        void activate() override;

        // Chain Primitive Phase
        // Allows a chain of particles to move along a certain path
        void chain_activate();
        void chain_handleChainToken();

        //set color of the particle
        int headMarkColor() const override;
        int headMarkDir() const override;
        int tailMarkColor() const override;


        QString inspectionText() const override;

        GSFParticle& nbrAtLabel(int label) const;


protected:
        //private vars
        const int _initialSideLen;
        int _triangleDirection; // Direction of the left leg of the triangle as seen from the leader
        State _state;

        unsigned int _level = 0;
        int _ldrlabel = -1;
        int _depth = -1;

        //used for chain movement
        bool _sent_pull = false;

        //add tokens

        // TRIANGLE EXPAND tokens:

        // CHAIN tokens:

        // Used to initialize a triangle expansions from a coordinator
        // _left => expand the triangle at the left leg, looking at the triangle with the leader on top.
        // !_left => expand the triangle at the right leg, looking at the triangle with the leader on top.
        // _initiated <=> the triangle expansion has been initiated
        struct triangle_expand_TriggerExpandToken :public Token {bool _left; bool _initated = false;};

        // Used to inform all particles at a side of the triangle of the initiated expansion
        struct triangle_expand_ExpandToken :public Token {int _level; int _movementdir; int _dirpassed; int _confirmationdir;};

        // Used to confirm the coordinator of a succesful triangle expansion, from the last particlein the triangle edge
        struct triangle_expand_ConfirmExpandToken :public Token {int _dirpassed;};

        // debug token:
        struct chain_MovementInitToken :public Token {std::stack<int> L; int _lifetime; int _dirpassed;
                                               bool _contract; int _level;};
        //used to initiate chain movement
        //L: the path that the chain should follow
        //_contract: whether or not the chain should be fully contracted at the end
        struct chain_ChainToken : public Token {std::stack<int> L; bool _contract;};

        //_final: whether the final chain should be contracted (set by chain)
        struct chain_ContractToken : public Token{bool _final;};

        //Sets the depth of all tokens in the chain
        //_passeddir: the direction from which the token was passed. Used to
        // set what particle should be followed
        //_depth: location of the particle in the chain
        struct chain_DepthToken : public Token{int _passeddir; int _depth; int _level;};

        //used for confirming whether the chain is contracted when _final = true
        // in chain token
        struct chain_ConfirmContractToken : public Token{};

        struct triangle_shift_TriggerShiftToken: public Token{int _dir; bool _initiated;};
        struct triangle_shift_ShiftToken: public Token{int _level; int _left; int _dirpassed;};
        struct triangle_shift_CoordinatorToken: public Token{int _dirpassed; int _shiftdir;};


    private:
        friend class GSFSystem;

        void chain_handleContractToken();
        void chain_handleMovementInitToken();
        void chain_handleDepthToken();
        void chain_handleConfirmContractToken();
        void triangle_expand_activate();

        void triangle_shift_activate();

        void triangle_shift_coordinatorActivate();
        void triangle_shift_particleActivate();

        void triangle_expand_coordinatorActivate();
        void triangle_expand_particleActivate();
        void triangle_expand_createMovementInitToken(std::shared_ptr<triangle_expand_ExpandToken> expandToken);
        void triangle_expand_forwardExpandToken(std::shared_ptr<triangle_expand_ExpandToken> expandToken);
        void triangle_expand_handleTriggerExpandToken();
        void triangle_expand_handleConfirmExpandToken();
};

class GSFSystem:public AmoebotSystem{
    public:
        GSFSystem(int sideLen = 6, QString expanddir="l");


private:
        void initializeTriangle(int sideLen, Node current, int dir);
};


#endif // GENERALSHAPEFORMATION_H
