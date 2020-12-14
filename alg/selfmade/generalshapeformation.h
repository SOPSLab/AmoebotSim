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
            FOLLOWER
        };

        //constructor for TriangleParticle
        GSFParticle(Node& head, const int globalTailDir,
                    const int orientation, AmoebotSystem& system,
                    const int sideLen, State state, unsigned int level,
                    unsigned int ldrlabel, int depth);

        //what to be done during activation
        void activate() override;

        // Chain Primitive Phase
        // Allows a chain of particles to move along a certain path
        void chain_activate();
        void chain();

        //set color of the particle
        int headMarkColor() const override;
        int headMarkDir() const override;
        int tailMarkColor() const override;


        QString inspectionText() const override;

        GSFParticle& nbrAtLabel(int label) const;

    protected:
        //private vars
        const int _initialSideLen;
        State _state;

        unsigned int _level = 0;
        int _ldrlabel = -1;
        int _depth = -1;

        //used for chain movement
        bool _sent_pull = false;

        //add tokens
        struct MovementInitToken :public Token {std::stack<int> L; int _lifetime; int _dirpassed;
                                               bool _contract;};
        //used to initiate chain movement
        //L: the path that the chain should follow
        //_contract: whether or not the chain should be fully contracted at the end
        struct ChainToken : public Token {std::stack<int> L; bool _contract;};

        //_final: whether the final chain should be contracted (set by chain)
        struct ContractToken : public Token{bool _final;};

        //Sets the depth of all tokens in the chain
        //_passeddir: the direction from which the token was passed. Used to
        // set what particle should be followed
        //_depth: location of the particle in the chain
        struct DepthToken : public Token{int _passeddir; int _depth;};

        //used for confirming whether the chain is contracted when _final = true
        // in chain token
        struct ConfirmContractToken : public Token{};
    private:
        friend class GSFSystem;

};

class GSFSystem:public AmoebotSystem{
    public:
        GSFSystem(int sideLen = 6);


};


#endif // GENERALSHAPEFORMATION_H
