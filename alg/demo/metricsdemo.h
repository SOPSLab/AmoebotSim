/* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
 * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
 * notice can be found at the top of main/main.cpp. */

// Defines the particle system and composing particles for the MetricsDemo code
// tutorial. MetricsDemo demonstrates how to add custom "counts" and "measures"
// to an algorithm, enabling the capture of quantitative data. The pseudocode is
// available in the docs:
// [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#metricsdemo-capturing-data].

#ifndef AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H_
#define AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H_

#include <QString>

#include "core/amoebotparticle.h"
#include "core/amoebotsystem.h"

class MetricsDemoParticle : public AmoebotParticle {
  friend class PercentRedMeasure;

 public:
  enum class State {
    Red,
    Orange,
    Yellow,
    Green,
    Blue,
    Indigo,
    Violet
  };

  // Constructs a new particle with a node position for its head, a global
  // compass direction from its head to its tail (-1 if contracted), an offset
  // for its local compass, a system that it belongs to, and a maximum value for
  // its counter.
  MetricsDemoParticle(const Node& head, const int globalTailDir,
                      const int orientation, AmoebotSystem& system,
                      const int counterMax);

  // Executes one particle activation.
  void activate() override;

  // Functions for altering the particle's color. headMarkColor() (resp.,
  // tailMarkColor()) returns the color to be used for the ring drawn around the
  // particle's head (resp., tail) node. In this demo, the tail color simply
  // matches the head color.
  int headMarkColor() const override;
  int tailMarkColor() const override;

  // Returns the string to be displayed when this particle is inspected; used to
  // snapshot the current values of this particle's memory at runtime.
  QString inspectionText() const override;

 protected:
  // Returns a random State.
  State getRandColor() const;

  // Member variables.
  State _state;
  int _counter;
  const int _counterMax;

 private:
  friend class MetricsDemoSystem;
};

class MetricsDemoSystem : public AmoebotSystem {
  friend class PercentRedMeasure;
  friend class MaxDistanceMeasure;

 public:
  // Constructs a system of the specified number of MetricsDemoParticles
  // enclosed by a hexagonal ring of objects.
  MetricsDemoSystem(unsigned int numParticles = 30, int counterMax = 5);
};

class PercentRedMeasure : public Measure {
 public:
  // Constructs a PercentRedMeasure by using the parent constructor and adding a
  // reference to the MetricsDemoSystem being measured.
  PercentRedMeasure(const QString name, const unsigned int freq,
                    MetricsDemoSystem& system);

  // Calculated the percentage of particles in the system in the Red state.
  double calculate() const final;

 protected:
  MetricsDemoSystem& _system;
};

class MaxDistanceMeasure : public Measure {
 public:
  // Constructs a MaxDistanceMeasure by using the parent constructor and adding
  // a reference to the MetricsDemoSystem being measured.
  MaxDistanceMeasure(const QString name, const unsigned int freq,
                     MetricsDemoSystem& system);

  // Calculates the largest Cartesian distance between any pair of particles in
  // the system.
  double calculate() const final;

 protected:
  MetricsDemoSystem& _system;
};

#endif  // AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H_
