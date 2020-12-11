/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

//system used for defining a triangle set of particles.

#ifndef TRIANGLEPLACEMENT_H
#define TRIANGLEPLACEMENT_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"
#include <stack>


class TriangleParticle:public AmoebotParticle{
    public:
        //states that a particle can be in
        enum class State{
            COORDINATOR,
            CHAIN_COORDINATOR,
            FOLLOWER
        };

        //constructor for TriangleParticle
        TriangleParticle(Node& head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         const int sideLen, State state, unsigned int level,
                         unsigned int ldrlabel, int depth);

        //what to be done during activation
        void activate() override;

        void chain();

        //set color of the particle
        int headMarkColor() const override;
        int headMarkDir() const override;
        int tailMarkColor() const override;


        QString inspectionText() const override;

        TriangleParticle& nbrAtLabel(int label) const;

    protected:
        //private vars
        const int sideLen;
        State _state;
//        unsigned int _nbrLabel;
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
        
        //_final: whether the final chain should be contracted (used byd 
        struct ContractToken : public Token{bool _final;};
        struct DepthToken : public Token{int _passeddir; int _depth;};
        struct ConfirmContractToken : public Token{};

    private:
        friend class TriangleSystem;

};

class TriangleSystem:public AmoebotSystem{
    public:
        TriangleSystem(int sideLen = 6);


};


#endif // TRIANGLEPLACEMENT_H
