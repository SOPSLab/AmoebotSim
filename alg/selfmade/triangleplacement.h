/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

//system used for defining a triangle set of particles.

#ifndef TRIANGLEPLACEMENT_H
#define TRIANGLEPLACEMENT_H

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"


class TriangleParticle:public AmoebotParticle{
    public:
        //states that a particle can be in
        enum class State{
            COORDINATOR,
            FOLLOWER
        };

        //constructor for TriangleParticle
        TriangleParticle(Node& head, const int globalTailDir,
                         const int orientation, AmoebotSystem& system,
                         const int sideLen, State state);

        //what to be done during activation
        void activate() override;

        //set color of the particle
        int headMarkColor() const override;
        int tailMarkColor() const override;


        QString inspectionText() const override;

        TriangleParticle& nbrAtLabel(int label) const;

    protected:
        //private vars
        const int sideLen;
        State _state;

        //add tokens

    private:
        friend class TriangleSystem;

};

class TriangleSystem:public AmoebotSystem{
    public:
        TriangleSystem(int sideLen = 6);


};


#endif // TRIANGLEPLACEMENT_H
