/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the Disco code
// tutorial, a first algorithm for new developers to AmoebotSim. Disco
// demonstrates the basics of algorithm architecture, instantiating a particle
// system, moving particles, and changing particles' states. The pseudocode is
// available in the docs:
// [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#discodemo-your-first-algorithm].

#ifndef AMOEBOTSIM_ALG_DEMO_DEBUG_H_
#define AMOEBOTSIM_ALG_DEMO_DEBUG_H_

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class DebugParticle : public AmoebotParticle {
    public:
    DebugParticle(const Node& head, const int globalTailDir,
                  const int orientation,
                  AmoebotSystem& system, int leaderLabel, int id);
    void activate() override;
    QString inspectionText() const override;

    DebugParticle& nbrAtLabel(int label) const;
    int headMarkDir() const override;
    int headMarkColor() const override;

    int tailMarkDir() const override;
    int tailMarkColor() const override;

    protected:
    int _leaderLabel, _id;
};

class DebugSystem : public AmoebotSystem {
public:
    DebugSystem();
};

#endif  // AMOEBOTSIM_ALG_DEMO_DEBUG_H_
