#ifndef GENERALSHAPEFORMATION_H
#define GENERALSHAPEFORMATION_H


#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"


class GSFParticle:public AmoebotParticle{
    public:
        //states that a particle can be in
        enum class State{
            COORDINATOR,
            FOLLOWER
        };

        //constructor for TriangleParticle
        GSFParticle(Node& head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         const int sideLen, State state);

        //what to be done during activation
        void activate() override;

        // Chain Primitive Phase
        // Allows a chain of particles to move along a certain path
        void activateChain();


        //set color of the particle
        int headMarkColor() const override;
        int tailMarkColor() const override;


        QString inspectionText() const override;

        GSFParticle& nbrAtLabel(int label) const;

    protected:
        //private vars
        const int _initialSideLen;
        State _state;

        //add tokens

    private:
        friend class GSFSystem;

};

class GSFSystem:public AmoebotSystem{
    public:
        GSFSystem(int sideLen = 6);


};


#endif // GENERALSHAPEFORMATION_H
