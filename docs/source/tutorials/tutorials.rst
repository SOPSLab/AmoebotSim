Tutorials
=========

These tutorials are for researchers 🧪 and developers 💻 wanting to implement distributed algorithms for programmable matter in AmoebotSim.
As a prerequisite, make sure you've followed the instructions in the :doc:`Installation Guide </install/install>` so that everything builds and runs correctly.
Additionally, you should be comfortable with how the `amoebot model <link-todo>`_ is defined.

All tutorials can be found in the ``alg/demo/`` directory of AmoebotSim so you can build and run them yourself.
Note that the coding style in these tutorials follows our :ref:`C++ Style Guide <cpp-style>` which we will not re-explain here.

.. todo::
  Add links.


Anatomy of AmoebotSim
---------------------

AmoebotSim is organized into seven main directories that loosely collect similar classes.

* ``alg/`` contains all classes defining the amoebot model's distributed algorithms. If you're a researcher or developer hoping to implement your own algorithms in AmoebotSim, this is where they'll live.

  * ``demo/`` is a subdirectory of ``alg/`` containing example algorithms that, coupled with the documentation in this Tutorials page, act as the demos that new AmoebotSim developers can learn from.

* ``core/`` contains the core elements of AmoebotSim, including the base classes for particles and particle systems.

* ``docs/`` contains the source files for this documentation.

* ``helper/`` contains utility classes that interface with various C++ libraries on behalf of the particle algorithms.

* ``main/`` contains the main driver and ``Application`` class, which is responsible for connecting the front end GUI to the back end implementation. Only developers modifying the GUI will need to work with this class.

* ``res/`` contains AmoebotSim's resources, including textures for graphics rendering, icons, GUI layout, etc.

* ``script/`` contains the classes used for interpreting and executing commands issued to AmoebotSim via its `JavaScript API <link-todo>`_.

* ``ui/`` contains the classes used for front end GUI rendering and visualization.

.. todo::
  Add links.

The most important classes for implemeting new distributed algorithms in AmoebotSim are:

* ``Node`` (in ``core/node.h``) describes the ``(x,y)`` nodes of the triangular lattice.

* ``Object`` (in ``core/object.h``) describes a single node of a static object.

* ``Particle`` (in ``core/particle.*``) describes the most basic form of a particle, essentially reducing this entity to the node(s) it occupies.

* ``LocalParticle`` (in ``core/localparticle.*``) is a child class of ``Particle`` that adds functions for observing and using local information. In particular, these functions focus on obscuring global directions from a particle so that everything it sees is from its own local perspective. There are also functions for "cheating" and accessing some level of global information, if desired; however, these are meant only for convenience and are not compliant with the assumptions of the amoebot model.

* ``AmoebotParticle`` (in ``core/amoebotparticle.*``) is a child class of ``LocalParticle`` that adds functions for particle activations, particle movements, and token passing. All particles running new algorithms inherit from this class.

* ``AmoebotSystem`` (in ``core/amoebotsystem.*``) is a glorified container of ``AmoebotParticles`` that keeps track of the particle system's size, position, and progress. All particle systems running new algorithms inherit from this class.


DiscoDemo: Your First Algorithm
-------------------------------

We'll be developing a fun (but not particularly useful) algorithm called **Disco** where particles will move within a boundary, changing colors as they go.
You can follow along with this tutorial by referencing the completed ``alg/demo/discodemo.*`` files in AmoebotSim; this tutorial shows how to develop this simulation from scratch.


.. _disco-pseudocode:

Algorithm Pseudocode
^^^^^^^^^^^^^^^^^^^^

All algorithms for the amoebot model are defined at the particle level, and all particles run an instance of the same algorithm (this is where the "distributed computing" perspective comes in).
So when we write algorithm pseudocode, we always write it from the perspective of a single particle, *not* from the system's perspective!

For **Disco**, every particle will keep a state ``color`` and a counter ``ctr`` that decrements each time it is activated.
When ``ctr = 0``, the particle will change its ``color``.
Here's the color-changing part of the pseudocode for a particle ``P``::

  P.ctr <- P.ctr - 1
  if (P.ctr is 0), then do:
    P.ctr <- CTR_MAX  // Reset the counter.
    P.color <- getRandColor()
  end if

We'd also like our particles to "dance" around in the boundary.
We'll achieve this using the amoebot model's expansion and contraction movements.
However, the amoebot model doesn't allow multiple particles to be on the same node, so we need to be careful not to let particles expand into nodes that are already occupied:::

  if (P is contracted), then do:
    expandDir <- random direction in [0, 6)
    if (node in direction expandDir is empty), then do:
      expand towards expandDir
    end if
  else, do:  // P is expanded
    contract tail
  end if

Some caveats related to **Disco**'s compliance with the amoebot model:

- The amoebot model assumes that each particle only has constant-size memory, which has two implications for our algorithm: (1) ``CTR_MAX`` should be a constant value, e.g., "5", and (2) the number of colors that can be returned by ``getRandColor()`` must also be constant. We'll use {R, O, Y, G, B, I, V} (`what? <https://en.wikipedia.org/wiki/ROYGBIV>`_).
- Many algorithms for the amoebot model are designed to keep the particle system connected, but the particles running **Disco** will definitely disconnect from one another because they're moving in random directions. This is why we have the static boundary: to corral all those particles and keep them from diffusing into infinity!


Creating the Files
^^^^^^^^^^^^^^^^^^

All algorithms in AmoebotSim come in header/source file pairs, and live within the ``alg/`` directory.
The file naming convention is ``<algorithmname>.h`` and ``<algorithmname>.cpp``.
For example, the Compression algorithm is simulated in the files ``alg/compression.h`` and ``alg/compression.cpp``.
In our case, because our Disco algorithm is meant for demonstration, we will create its two files in the ``alg/demo/`` directory: ``alg/demo/discodemo.h`` and ``alg/demo/discodemo.cpp``.

Importantly, because this is a Qt project, we need to use Qt's *"Add New..."* dialog (shown below).
In addition to simply creating the files, this process automatically adds them to AmoebotSim's ``.pro`` file which indexes the project files for compilation.

First, right-click on the folder to add the files to (in our case, this is ``alg/demo/``). Select *"Add New..."*.

.. image:: graphics/disco1.jpg

Next, in the C++ tab, choose *"C++ Source File"*.
We don't use the *"C++ Class"* option because our classes do not always match the filenames (for reasons that will become clear in just a few steps).

.. image:: graphics/disco2.jpg

Specify the filename and click *"Next"*.

.. image:: graphics/disco3.jpg

The source file ``discodemo.cpp`` is now in the ``alg/demo/`` directory and has been added to the ``AmoebotSim.pro`` file's ``SOURCES`` list.

.. image:: graphics/disco4.jpg

Repeat these steps for the header file ``discodemo.h`` using the *"C++ Header File"* option in the *"Add New..."* dialog.


The Header File
^^^^^^^^^^^^^^^

This section assumes you've read the :ref:`C++ Style Guide <cpp-style>` in our development guide.
We begin by setting up the header file's structure, which includes the following elements:

- A `class comment <https://google.github.io/styleguide/cppguide.html#Class_Comments>`_ that includes the copyright notice, a brief description of the class(es) this file contains, any relevant publications/references, and this algorithm's command line signature (we'll leave the signature blank for now). **Disco** doesn't have a publication, but we'll add a reference to this code tutorial.

- `#define guards <https://google.github.io/styleguide/cppguide.html#The__define_Guard>`_ of the form ``<PROJECT>_<PATH>_<FILE>_<H>_``. In our case, this is ``AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_``.

- Any ``#includes`` grouped in order of standard C/C++ libraries, then any Qt libraries, and finally any AmoebotSim-specific headers. Each group is ordered alphabetically. For **Disco**, we only need the core ``AmoebotParticle`` and ``AmoebotSystem`` classes, which are used in essentially every algorithm.

- The two classes for **Disco**: a particle class ``DiscoDemoParticle`` that inherits from ``AmoebotParticle``, and a particle system class ``DiscoDemoSystem`` that inherits from ``AmoebotSystem``.

With all these elements in place, we have the following:

.. code-block:: c++

  /* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  // Defines the particle system and composing particles for the Disco code
  // tutorial, a first algorithm for new developers to AmoebotSim. Disco
  // demonstrates the basics of algorithm architecture, instantiating a particle
  // system, moving particles, and changing particles' states. The pseudocode is
  // available in the docs:
  // [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#discodemo-your-first-algorithm].
  //
  // Run on the simulator command line using discodemo(???).

  #ifndef AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_
  #define AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_

  #include "core/amoebotparticle.h"
  #include "core/amoebotsystem.h"

  class DiscoDemoParticle : public AmoebotParticle {

  };

  class DiscoDemoSystem : public AmoebotSystem {

  };

  #endif  // AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_

Next, we need to fill out our classes' member variables and functions.
It's helpful to be familiar with the parent classes' variables and functions when defining our own so we don't waste time implementing functionality that already exists.
Reviewing our :ref:`pseudocode <disco-pseudocode>`, every `DiscoDemoParticle` will need the following:

- A variable to store its state (color). We'll call this ``State _state``, where ``State`` is a custom type we'll define shortly.

- Variables to store its current counter value (``int _counter``) and the maximum counter value (``const int _counter_max``).

- The ability to generate a new color at random. We'll define a function with the signature ``State getRandColor() const``.

- The ability to check if it is contracted or expanded. These already exist as ``isContracted()`` and ``isExpanded()`` in ``Particle``, which is inherited by ``LocalParticle``, which is inherited by ``AmoebotParticle``, which is inherited by our ``DiscoDemoParticle``. So we don't need to implement these again.

- The ability to generate a random direction in [0,6). This already exists as ``randDir()`` in ``RandomNumberGenerator``, which is inherited by ``AmoebotParticle``, which is inherited by our ``DiscoDemoParticle``.

- The ability to check if a node in a given direction is unoccupied. This already exists as ``canExpand(int)`` in ``AmoebotParticle``.

- The ability to expand in a given direction and contract its tail. These already exist as ``expand(int)`` and ``contractTail()``, respectively, in ``AmoebotParticle``.

The variables and functions that we do need to implement are specific to ``DiscoDemoParticle``, so we can make them ``protected`` instead of ``public``.

.. code-block:: c++

  class DiscoDemoParticle : public AmoebotParticle {
   public:

   protected:
    // Returns a random State.
    State getRandColor() const;

    // Member variables.
    State _state;
    int _counter;
    const int _counterMax;

   private:
  };

For the ``public`` members, we need:

- A definition for the custom type ``State``. We'll use an `enumeration class <https://www.learncpp.com/cpp-tutorial/4-5a-enum-classes/>`_ to define a type-safe set of possible states; in our case, this is a set of colors.

- A constructor. Every class that inherits from ``AmoebotParticle`` should at least take a ``const Node head``, ``const int globalTailDir``, ``const int orientation``, and ``AmoebotSystem& system`` as inputs to its constructor, but can additionally take algorithm-specific information. For **Disco**, we'll additionally take a maximum counter value ``const int counterMax``.

- A function handling what a ``DiscoDemoParticle`` does when it's activated. This is achieved by overriding the ``activate()`` function from ``AmoebotParticle``.

- A function handling the visual color changes for a ``DiscoDemoParticle``'s head and tail nodes. This is achieved by overriding the ``headMarkColor()`` and ``tailMarkColor()`` functions from ``Particle``.

- A function handling the text that appears when inspecting a ``DiscoDemoParticle`` (see the :ref:`Controls <controls>` section of the usage guide). This is achieved by overriding the ``inspectionText()`` function from ``Particle``.

As a ``private`` declaration, we need to name ``DiscoDemoSystem`` as a ``friend`` class.
All together, we have:

.. code-block:: c++

  class DiscoDemoParticle : public AmoebotParticle {
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
    DiscoDemoParticle(const Node head, const int globalTailDir,
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
    // ...

   private:
    friend class DiscoDemoSystem;
  };

Finally, we need to define a constructor for ``DiscoDemoSystem``.
This constructor will take the desired number of particles in the system as well as the maximum counter value.
We also provide some default parameter values.

.. code-block:: c++

  class DiscoDemoSystem : public AmoebotSystem {
   public:
    // Constructs a system of the specified number of DiscoDemoParticles enclosed
    // by a hexagonal ring of objects.
    DiscoDemoSystem(unsigned int numParticles = 30, int counterMax = 5);
  };


The Source File
^^^^^^^^^^^^^^^

The source file has a fairly straightforward structure.
It begins with the copyright notice and an ``#include`` of the header file, and then simply lists the functions to be implemented with their scopes:

.. code-block:: c++

  /* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  #include "alg/demo/discodemo.h"

  DiscoDemoParticle::DiscoDemoParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system,
                                       const int counterMax) {}

  void DiscoDemoParticle::activate() {}

  int DiscoDemoParticle::headMarkColor() const {}

  int DiscoDemoParticle::tailMarkColor() const {}

  QString DiscoDemoParticle::inspectionText() const {}

  DiscoDemoParticle::State DiscoDemoParticle::getRandColor() const {}

  DiscoDemoSystem::DiscoDemoSystem(unsigned int numParticles, int counterMax) {}

We'll detail each function implementation in order.

``DiscoDemoParticle``'s constructor is fairly straightforward.
We can use an `initializer list <https://google.github.io/styleguide/cppguide.html#Constructor_Initializer_Lists>`_ to initialize ``_counter`` and ``_counterMax`` both to the maximum counter value.
We then set this particle's ``_state`` to a random initial color.

.. code-block:: c++

  DiscoDemoParticle::DiscoDemoParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system,
                                       const int counterMax)
      : AmoebotParticle(head, globalTailDir, orientation, system),
        _counter(counterMax),
        _counterMax(counterMax) {
    _state = getRandColor();
  }

The implementation of ``activate()`` simply follows the :ref:`pseudocode <disco-pseudocode>` we detailed before.
As an aside, AmoebotSim does not prohibit the ``activate()`` function from allowing a particle to do more than what is allowed by the amoebot model, such as letting a particle make more than one expansion or contraction in a single activation.
It is up to the algorithm designer and simulation developer to ensure no rules of the amoebot model are violated.

.. code-block:: c++

  void DiscoDemoParticle::activate() {
    // First decrement the particle's counter. If it's zero, reset the counter and
    // get a new color.
    _counter--;
    if (_counter == 0) {
      _counter = _counterMax;
      _state = getRandColor();
    }

    // Next, handle movement. If the particle is contracted, choose a random
    // direction to try to expand towards, but only do so if the node in that
    // direction is unoccupied. Otherwise, if the particle is expanded, simply
    // contract its tail.
    if (isContracted()) {
      int expandDir = randDir();
      if (canExpand(expandDir)) {
        expand(expandDir);
      }
    } else {  // isExpanded().
      contractTail();
    }
  }

The implementation of ``headMarkColor()`` uses the particle's ``_state`` (color) to decide what color to use when rendering its head node.
All colors are expressed in RGB format as 6-digit hexadecimal numbers: ``0x<rr><bb><gg>``. For example, the color red is ``0xff0000`` while the color black is ``0x000000``.
If no color (transparent) is desired, return ``-1``.

.. code-block:: c++

  int DiscoDemoParticle::headMarkColor() const {
    switch(_state) {
      case State::Red:    return 0xff0000;
      case State::Orange: return 0xff9000;
      case State::Yellow: return 0xffff00;
      case State::Green:  return 0x00ff00;
      case State::Blue:   return 0x0000ff;
      case State::Indigo: return 0x4b0082;
      case State::Violet: return 0xbb00ff;
    }

    return -1;
  }

The implementation of ``tailMarkColor()`` simply mirrors ``headMarkColor()``:

.. code-block:: c++

  int DiscoDemoParticle::tailMarkColor() const {
    return headMarkColor();
  }

The implementation of ``inspectionText()`` concatenates a series of strings describing the particle's global information (position, orientation, and tail direction) in addition to its local information (current state and counter value).
Here, a lambda function is used to encapsulate the switch statement because it is more concise, but could just as easily be implemented using a long ``if``/``else if``/``else`` chain.

.. code-block:: c++

  QString DiscoDemoParticle::inspectionText() const {
    QString text;
    text += "Global Info:\n";
    text += "  head: (" + QString::number(head.x) + ", "
                        + QString::number(head.y) + ")\n";
    text += "  orientation: " + QString::number(orientation) + "\n";
    text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
    text += "Local Info:\n";
    text += "  state: ";
    text += [this](){
      switch(_state) {
        case State::Red:    return "red\n";
        case State::Orange: return "orange\n";
        case State::Yellow: return "yellow\n";
        case State::Green:  return "green\n";
        case State::Blue:   return "blue\n";
        case State::Indigo: return "indigo\n";
        case State::Violet: return "violet\n";
      }
      return "no state\n";
    }();
    text += "  counter: " + QString::number(_counter);

    return text;
  }

The implementation of ``getRandColor()`` uses ``RandomNumberGenerator``'s ``randInt()`` function to choose a random index in [0,7) (where 7 is the number of states).
It then casts this index as a ``State``, effectively choosing a random color.
Note that although enumeration classes (like ``State``) are not ``ints``, they can be safely casted back and forth using ``static_cast``.

.. code-block:: c++

  DiscoDemoParticle::State DiscoDemoParticle::getRandColor() const {
    // Randomly select an integer and return the corresponding state via casting.
    return static_cast<State>(randInt(0, 7));
  }

Finally, we need to implement ``DiscoDemoSystem``'s constructor.
At a high level, the goal of this function is to create a closed boundary of ``Objects`` in the shape of a regular hexagon and then place the desired number of ``DiscoDemoParticles`` randomly inside that boundary.
Before diving into the details, there are several useful functions to be familiar with:

- ``insert()`` is defined by ``AmoebotSystem``. It takes as input a pointer to an ``Object`` or to an ``AmoebotParticle``. This is what's used to add ``Objects`` or ``DiscoDemoParticles`` to the ``DiscoDemoSystem``.

- ``nodeInDir()`` is defined by ``Node``. It returns the node adjacent to the one calling the function in the given global direction, where direction ``0`` is to the right and directions increase counterclockwise.

- ``randInt()`` and ``randDir()`` are both defined by ``RandomNumberGenerator``, and are used to get random values.

Let's first look at the code used to create the hexagonal boundary.

.. code-block:: c++

  // In order to enclose an area that's roughly 3.7x the # of particles using a
  // regular hexagon, the hexagon should have side length 1.4*sqrt(# particles).
  int sideLen = static_cast<int>(std::round(1.4 * std::sqrt(numParticles)));
  Node boundNode(0, 0);
  for (int dir = 0; dir < 6; ++dir) {
    for (int i = 0; i < sideLen; ++i) {
      insert(new Object(boundNode));
      boundNode = boundNode.nodeInDir(dir);
    }
  }

A brief primer on how AmoebotSim treats its coordinate system will be helpful to understand the rest of this code.
AmoebotSim assigns an ``(x,y)`` coordinate to each node on the triangular lattice.
The origin ``(0,0)`` is fixed, and from this point the x-axis increases to the right and decreases to the left while the y-axis increases to the up-right and decreases to the down-left.
Think of it as a usual Cartesian grid that's been squished to the right.

We need the boundary of our ``DiscoDemoSystem`` to be big enough so that the particles have reasonable room to "dance" around, but not so big that the dance floor feels empty.
Some elementary geometry would tell you that the area of a regular hexagon is ``3s^2 * sqrt(3) / 2``, where ``s`` is the side length.
So if we wanted our boundary to enclose an area ``C`` times the number of particles ``n``, some algebra shows us that the side length has to be ``sqrt(2Cn / (3sqrt(3)))``.
This is where the ``3.7x`` and ``1.4`` come from in the code above: setting ``C = 3.7`` means that ``s ~ 1.4 * sqrt(n)``.

Now that we know how long each side should be, we start at node ``(0,0)``.
The outer ``for`` loop controls the direction we're adding boundary nodes, while the inner ``for`` loop ensures we add the right number of boundary nodes to each side.
In words, these ``for`` loops add ``s`` boundary nodes starting at ``(0,0)`` and going right, then ``s`` nodes going up-right, then ``s`` nodes going up-left, and so on until the boundary is closed.

Since we started at ``(0,0)``, we have the following boundaries for our hexagon:

.. image:: graphics/discoboundary.png

All that remains is to choose a node ``(x,y)`` at random with ``-s < x < s`` and ``0 < y < 2s`` and place a particle there as long as the node is inside the boundary and unoccupied.
This process is repeated until the desired number of particles has been placed.

.. code-block:: c++

  // Let s be the bounding hexagon side length. When the hexagon is created as
  // above, the nodes (x,y) strictly within the hexagon have (i) -s < x < s,
  // (ii) 0 < y < 2s, and (iii) 0 < x+y < 2s. Choose interior nodes at random to
  // place particles, ensuring at most one particle is placed at each node.
  std::set<Node> occupied;
  while (occupied.size() < numParticles) {
    // First, choose an x and y position at random from the (i) and (ii) bounds.
    int x = randInt(-sideLen + 1, sideLen);
    int y = randInt(1, 2 * sideLen);
    Node node(x, y);

    // If the node satisfies (iii) and is unoccupied, place a particle there.
    if (0 < x + y && x + y < 2 * sideLen
        && occupied.find(node) == occupied.end()) {
      insert(new DiscoDemoParticle(node, -1, randDir(), *this, counterMax));
      occupied.insert(node);
    }
  }

Here, we use a ``std::set<Node> occupied`` to keep track of the nodes that are occupied by placed particles, and use the condition ``occupied.find(node) == occupied.end()`` to check that the node in question is not already occupied by a particle.
This sort of logic is fairly common in many other algorithms' particle system constructors.


Registering the Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^

With the header and source files completed, we're nearly done with the **Disco** simulation.
The last (small) bit of work to do is to register the **Disco** simulation with AmoebotSim so that **Disco** can be run from the GUI.
The first files we need to update are ``script/scriptinterface.h`` and ``script/scriptinterface.cpp``.
In ``script/scriptinterface.h``, we add the algorithm's *script signature*.
Except for in rare cases, an algorithm's script signature registered in ``ScriptInterface`` should have the same parameters as the algorithm's system constructor.
In our case, we have:

.. code-block:: c++

  // Demonstration algorithm instance commands. Documentation for foo() can be
  // found in alg/demo/foo.h.
  void discodemo(const int numParticles = 30, const int counterMax = 5);

At this point, we can also go back to ``alg/demo/discodemo.h`` and update the part of the class comment that describes the algorithm's signature on the command line:

.. code-block:: c++

  // ...
  //
  // Run on the simulator command line using discodemo(# particles, counter max).

  #ifndef AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_
  #define AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_

  // ...

Next, in ``script/scriptinterface.cpp``, we implement ``discodemo()``.
This essentially has two parts: parameter checking (to ensure we don't pass our algorithm bad parameters that might crash AmoebotSim) and instantiating the system (achieved using ``Simulator``'s ``setSystem()`` function).

.. code-block:: c++

  void ScriptInterface::discodemo(const int numParticles, const int counterMax) {
    if (numParticles <= 0) {
      log("# particles must be > 0", true);
    } else if (counterMax <= 0) {
      log("counterMax must be > 0", true);
    } else {
      sim.setSystem(std::make_shared<DiscoDemoSystem>(numParticles));
    }
  }

If we were to compile and run AmoebotSim after doing these steps, we would be able to instantiate a ``DiscoDemoSystem`` using AmoebotSim's command line by typing ``discodemo()`` (if we wanted the default parameter values), or something like ``discodemo(70, 3)`` (if we wanted to supply our own parameter values).

In order to get the **Disco** simulation to appear in the algorithm selection dropdown, however, we need to register the **Disco** simulation with ``AlgorithmList``, which is implemented in ``ui/alg.cpp``.
Navigate to the implementation of ``AlgorithmList``'s constructor, and observe how the existing algorithms are registered.

#. First, we use ``_algorithms.push_back(Algorithm(<algorithm name>, <algorithm signature>));`` to register the algorithm's name and signature. The name should be something human readable (in our case, *"Demo: Disco"*), but the signature must exactly match what we registered in ``ScriptInterface`` (in our case, *"discodemo"*).

#. We then register each parameter using ``_algorithms.back().addParameter(<parameter name>, <default value>);``. Once again, the parameter name should be something human readable (e.g., *"# Particles"*), and the default value should always be given as a string (e.g., *"30"*).

All together, we have:

.. code-block:: c++

  // ...

  AlgorithmList::AlgorithmList() {
    /* DEMO ALGORITHMS */

    // Demo: Disco, a first tutorial.
    _algorithms.push_back(new Algorithm("Demo: Disco", "discodemo"));
    _algorithms.back()->addParameter("# Particles", "30");
    _algorithms.back()->addParameter("Counter Max", "5");

    // ...

Compiling and running AmoebotSim after these steps will allow you to instantiate the **Disco** simulation using either the command line or the sidebar interface.

Congratulations, you've implemented your first simulation on AmoebotSim!

.. image:: graphics/discoanimation.gif


CoordinationDemo: Working Together
----------------------------------

.. todo::
  Coming soon!


TokenDemo: Communicating over Distance
--------------------------------------

.. todo::
  Coming soon!


.. _metrics-demo:

MetricsDemo: Capturing Data
---------------------------

.. todo::
  Coming soon!
