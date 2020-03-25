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

We'll be developing a fun (but not particularly useful) algorithm called **DiscoDemo** where particles will move within a boundary, changing colors as they go.
You can follow along with this tutorial by referencing the completed ``alg/demo/discodemo.*`` files in AmoebotSim; this tutorial shows how to develop this simulation from scratch.


.. _disco-pseudocode:

Algorithm Pseudocode
^^^^^^^^^^^^^^^^^^^^

All algorithms for the amoebot model are defined at the particle level, and all particles run an instance of the same algorithm (this is where the "distributed computing" perspective comes in).
So when we write algorithm pseudocode, we always write it from the perspective of a single particle, *not* from the system's perspective!

For **DiscoDemo**, every particle will keep a state ``color`` and a counter ``ctr`` that decrements each time it is activated.
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

Some caveats related to **DiscoDemo**'s compliance with the amoebot model:

- The amoebot model assumes that each particle only has constant-size memory, which has two implications for our algorithm: (1) ``CTR_MAX`` should be a constant value, e.g., "5", and (2) the number of colors that can be returned by ``getRandColor()`` must also be constant. We'll use {R, O, Y, G, B, I, V} (`what? <https://en.wikipedia.org/wiki/ROYGBIV>`_).
- Many algorithms for the amoebot model are designed to keep the particle system connected, but the particles running **DiscoDemo** will definitely disconnect from one another because they're moving in random directions. This is why we have the static boundary: to corral all those particles and keep them from diffusing into infinity!


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

- A `class comment <https://google.github.io/styleguide/cppguide.html#Class_Comments>`_ that includes the copyright notice, a brief description of the class(es) this file contains, any relevant publications/references, and this algorithm's command line signature (we'll leave the signature blank for now). **DiscoDemo** doesn't have a publication, but we'll add a reference to this code tutorial.

- `#define guards <https://google.github.io/styleguide/cppguide.html#The__define_Guard>`_ of the form ``<PROJECT>_<PATH>_<FILE>_<H>_``. In our case, this is ``AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_``.

- Any ``#includes`` grouped in order of standard C/C++ libraries, then any Qt libraries, and finally any AmoebotSim-specific headers. Each group is ordered alphabetically. For **DiscoDemo**, we only need the core ``AmoebotParticle`` and ``AmoebotSystem`` classes, which are used in essentially every algorithm.

- The two classes for **DiscoDemo**: a particle class ``DiscoDemoParticle`` that inherits from ``AmoebotParticle``, and a particle system class ``DiscoDemoSystem`` that inherits from ``AmoebotSystem``.

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

- A constructor. Every class that inherits from ``AmoebotParticle`` should at least take a ``const Node head``, ``const int globalTailDir``, ``const int orientation``, and ``AmoebotSystem& system`` as inputs to its constructor, but can additionally take algorithm-specific information. For **DiscoDemo**, we'll additionally take a maximum counter value ``const int counterMax``.

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

.. _disco-system-constructor:

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


.. _disco-register:

Registering the Algorithm
^^^^^^^^^^^^^^^^^^^^^^^^^

With the header and source files completed, we're nearly done with the **DiscoDemo** simulation.
The last (small) bit of work to do is to register **DiscoDemo** with AmoebotSim so it can be run from the GUI.
The first files we need to update are ``ui/algorithm.h`` and ``ui/algorithm.cpp``.
In ``ui/algorithm.h``, we add an ``Algorithm`` child class to represent **DiscoDemo** with a constructor and an ``instantiate()`` function.
The ``instantiate()`` function should have the same parameters as ``DiscoDemoSystem``'s constructor.

.. code-block:: c++

  // Demo: Disco, a first tutorial.
  class DiscoDemoAlg : public Algorithm {
    Q_OBJECT

   public:
    DiscoDemoAlg();

   public slots:
    void instantiate(const int numParticles = 30, const int counterMax = 5);
  };

In ``ui/algorithm.cpp``, we first implement the ``DiscoDemoAlg()`` constructor.
This first calls the parent constructor ``Algorithm(<name>, <signature>)``, which takes two parameters: a *human-readable name* for the algorithm to put in the algorithm selection dropdown, and an algorithm *signature* to be used internally by the simulator.
Here, we use *"Demo: Disco"* as the name and *"discodemo"* as the signature.
Next, we add a human-readable name and a default value for each of the algorithm's parameters using ``addParameter(<name>, <default value>)``; these parameters should match what was used in the ``instantiate()`` function.
Note that the default values should always be given as a string (e.g., *"30"*).

.. code-block:: c++

  DiscoDemoAlg::DiscoDemoAlg() : Algorithm("Demo: Disco", "discodemo") {
    addParameter("# Particles", "30");
    addParameter("Counter Max", "5");
  };

Next, we implement the ``instantiate()`` function.
This essentially has two parts: parameter checking (to ensure we don't pass our algorithm bad parameters that might crash AmoebotSim) and instantiating the system (achieved using ``Simulator``'s ``setSystem()`` function).
Here, we use ``log()`` to show error messages to the user if one of their parameters is bad.

.. code-block:: c++

  void DiscoDemoAlg::instantiate(const int numParticles, const int counterMax) {
    if (numParticles <= 0) {
      log("# particles must be > 0", true);
    } else if (counterMax <= 0) {
      log("counterMax must be > 0", true);
    } else {
      sim.setSystem(std::make_shared<DiscoDemoSystem>(numParticles));
    }
  }

One last addition to ``ui/algorithm.cpp``: we need to construct an instance of our newly defined ``DiscoDemoAlg`` class and add it to AmoebotSim's algorithm list.
This will add **DiscoDemo** to the algorithm selection dropdown.

.. code-block:: c++

  // ...

  AlgorithmList::AlgorithmList() {
    // Demo algorithms.
    _algorithms.push_back(new DiscoDemoAlg());

    // ...

At this point, we can go back to ``alg/demo/discodemo.h`` and update the part of the class comment that describes the algorithm's signature on the command line:

.. code-block:: c++

  // ...
  //
  // Run on the simulator command line using discodemo(# particles, counter max).

  #ifndef AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_
  #define AMOEBOTSIM_ALG_DEMO_DISCODEMO_H_

  // ...

Finally, in ``ui/parameterlistmodel.cpp``, we need to parse the values given by the user in the sidebar's parameter input boxes.
All parameter values are input as strings, but need to be cast to their correct data types as defined by ``instantiate()``.

.. code-block:: c++

  void ParameterListModel::createSystem(QString algName) {
    // ...

    if (signature == "discodemo") {
      dynamic_cast<DiscoDemoAlg*>(alg)->
          instantiate(params[0].toInt(), params[1].toInt());
    } else if (signature ==  // ...

Compiling and running AmoebotSim after these steps will allow you to instantiate the **DiscoDemo** simulation using either the command line or the sidebar interface.

Congratulations, you've implemented your first simulation on AmoebotSim!

.. image:: graphics/discoanimation.gif


CoordinationDemo: Working Together
----------------------------------

.. todo::
  Coming soon!


.. _token-demo:

TokenDemo: Communicating over Distance
--------------------------------------

In this tutorial, you will learn how to utilize token passing in AmoebotSim, including creating custom token types containing structured data.
We'll be developing **TokenDemo**, a simple algorithm where a ring (hexagon) of particles will pass tokens around, changing colors based on what tokens they are holding.
You can follow along with this tutorial by referencing the completed ``alg/demo/tokendemo.*`` files in AmoebotSim.
These instructions assume that you've read the previous tutorials and are familiar with AmoebotSim basics.


A Primer on Token Passing
^^^^^^^^^^^^^^^^^^^^^^^^^

In the amoebot model, a *token* is a constant-size piece of information that can be passed between particles for long-range communication.
Every token in AmoebotSim is derived from the base ``Token`` struct.
This base token contains no structured data, but it appears in the definitions of the core functions for handling tokens found in the ``AmoebotParticle`` class in ``core/amoebotparticle.h``.
Many of these functions are *templates*, which are used to restrict their scope to a specific token type.

.. cpp:function:: void putToken(std::shared_ptr<Token> token)

  Add the given token pointer to this particle's collection.

.. cpp:function:: template<class TokenType> \
                  std::shared_ptr<TokenType> peekAtToken()

  Get a reference to the first token in this particle's collection of the specified type.

.. cpp:function:: template<class TokenType> \
                  std::shared_ptr<TokenType> takeToken()

  Performs the same operation as ``peekAtToken()``, but additionally removes the returned reference from this particle's collection.

.. cpp:function:: template<class TokenType> \
                  int countTokens()

  Counts the number of tokens in this particle's collection of the specified type.

.. cpp:function:: template<class TokenType> \
                  bool hasToken()

  Checks whether this particle's collection contains at least one token of the specified type (equivalent to ``countTokens() > 0``).

.. tip::

  The four template functions also have overloaded versions that additionally take a custom property as input. For example, the overloaded version of ``countTokens()`` only counts the tokens of the specified type that also satisfy the input property. See ``core/amoebotparticle.h`` for more details.


Algorithm Description
^^^^^^^^^^^^^^^^^^^^^

The goal of **TokenDemo** is to pass tokens of two types, ``RedToken`` and ``BlueToken``, in opposite directions around a ring of particles.
To achieve this, each token will keep track of ``_passedFrom``, the direction it was last passed from.
If a token is being passed for the first time, its particle must choose a consistent passing direction for ``RedTokens`` and the opposite direction for ``BlueTokens``.
Every particle has exactly two neighbors on the ring, so as long as ``_passedFrom`` is properly maintained, continuing to pass a token in the same direction is straightforward.

Each token will also keep track of its ``_lifetime`` which is decremented each time it is passed.
Once its ``_lifetime`` is zero, the token should be deleted.


Setting Up the Files
^^^^^^^^^^^^^^^^^^^^

We begin by creating the ``alg/demo/tokendemo.h`` and ``alg/demo/tokendemo.cpp`` files and setting up their structure.
Just as with other new particle types, we inherit from ``AmoebotParticle`` and set up the necessary function overrides in ``alg/demo/tokendemo.h``.

.. code-block:: c++

  /* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  // Defines a particle system and composing particles for the TokenDemo code
  // tutorial. TokenDemo demonstrates token passing functionality, including
  // defining new token types, modifying token memory contents, and passing tokens
  // between particles. The description and tutorial is available in the docs:
  // [https://amoebotsim.rtfd.io/en/latest/tutorials/tutorials.html#tokendemo-communicating-over-distance].
  //
  // Run with tokendemo(#particles, tokenlifetime) on the simulator command line.

  #ifndef AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_
  #define AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_

  #include "core/amoebotparticle.h"
  #include "core/amoebotsystem.h"

  class TokenDemoParticle : public AmoebotParticle {
   public:
    // Constructs a new particle with a node position for its head, a global
    // compass direction from its head to its tail (-1 if contracted), an offset
    // for its local compass, and a system which it belongs to.
    TokenDemoParticle(const Node head, const int globalTailDir,
                      const int orientation, AmoebotSystem& system);

    // Executes one particle activation.
    void activate() override;

    // Returns the color to be used for the ring drawn around the head node. In
    // this case, it returns the color of the token(s) this particle is holding.
    int headMarkColor() const override;

    // Returns the string to be displayed when this particle is inspected; used
    // to snapshot the current values of this particle's memory at runtime.
    QString inspectionText() const override;

    // Gets a reference to the neighboring particle incident to the specified port
    // label. Crashes if no such particle exists at this label; consider using
    // hasNbrAtLabel() first if unsure.
    TokenDemoParticle& nbrAtLabel(int label) const;

   protected:
    // TODO: define token types.

   private:
    friend class TokenDemoSystem;
  };

  // ...

  #endif  // AMOEBOTSIM_ALG_DEMO_TOKENDEMO_H_

We next add the token type definitions that **TokenDemo** will use.
``DemoToken`` will serve as the base token struct for this algorithm, storing both the ``_passedFrom`` and ``_lifetime`` member variables.
``RedToken`` and ``BlueToken`` are derived from ``DemoToken``.

.. code-block:: c++

  // ...

   protected:
    // Token types. DemoToken is a general type that has two data members:
    // (i) _passedFrom, which denotes the direction from which the token was last
    // passed (initially -1, meaning it has not yet been passed), and (ii)
    // _lifetime, which is decremented each time the token is passed. The red and
    // blue tokens are two types of DemoTokens.
    struct DemoToken : public Token { int _passedFrom = -1; int _lifetime; };
    struct RedToken : public DemoToken {};
    struct BlueToken : public DemoToken {};

  // ...

As with other algorithms, the last addition to ``alg/demo/tokendemo.h`` is the declaration of a new particle system type inheriting from ``AmoebotSystem``.
Because this algorithm has a termination condition (when all tokens have died out), we include an override for ``hasTerminated()``.

.. code-block:: c++

  class TokenDemoSystem : public AmoebotSystem {
   public:
    // Constructs a system of TokenDemoParticles with an optionally specified size
    // (#particles) and token lifetime.
    TokenDemoSystem(int numParticles = 48, int lifetime = 100);

    // Returns true when the simulation has completed; i.e, when all tokens have
    // died out.
    bool hasTerminated() const override;
  };

We complete our setup with a skeleton of ``alg/demo/tokendemo.cpp``.

.. code-block:: c++

  /* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  #include "alg/demo/tokendemo.h"

  TokenDemoParticle::TokenDemoParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system) {}

  void TokenDemoParticle::activate() {}

  int TokenDemoParticle::headMarkColor() const {}

  QString TokenDemoParticle::inspectionText() const {}

  TokenDemoParticle& TokenDemoParticle::nbrAtLabel(int label) const {}

  TokenDemoSystem::TokenDemoSystem(int numParticles, int lifetime) {}

  bool TokenDemoSystem::hasTerminated() const {}


Function Implementations
^^^^^^^^^^^^^^^^^^^^^^^^

We'll now implement each function, working our way up from the simplest to the more complex.
As an advanced tutorial, we will skip the implementations of the ``TokenDemoParticle`` constructor and ``nbrAtLabel()`` since they are straightforward and do not involve token handling.
See ``alg/demo/tokendemo.cpp`` for more details.

The ``headMarkColor()`` function, as in other algorithms, uses a series of conditionals to decide what color to circle this particle with.
In **TokenDemo**, we color the particles based on the type of token(s) they're holding: a particle holding a ``RedToken`` is colored red, a particle holding a ``BlueToken`` is colored blue, and a particle holding both types is colored purple.
Here, we make use of the ``hasToken()`` function to check what types of tokens the particle is holding.

.. code-block:: c++

  int TokenDemoParticle::headMarkColor() const {
    if (hasToken<RedToken>() && hasToken<BlueToken>()) {
      return 0xff00ff;
    } else if (hasToken<RedToken>()) {
      return 0xff0000;
    } else if (hasToken<BlueToken>()) {
      return 0x0000ff;
    } else {
      return -1;
    }
  }

In addition to the usual global information, **TokenDemo**'s ``inspectionText()`` uses ``countTokens()`` to display the number of each token type the particle is holding.

.. code-block:: c++

  QString TokenDemoParticle::inspectionText() const {
    QString text;
    text += "Global Info:\n";
    text += "  head: (" + QString::number(head.x) + ", "
                        + QString::number(head.y) + ")\n";
    text += "  orientation: " + QString::number(orientation) + "\n";
    text += "  globalTailDir: " + QString::number(globalTailDir) + "\n\n";
    text += "Local Info:\n";
    text += "  # RedTokens: " + QString::number(countTokens<RedToken>()) + "\n";
    text += "  # BlueTokens: " + QString::number(countTokens<BlueToken>());

    return text;
  }

The ``hasTerminated()`` function in ``TokenDemoSystem`` stops the simulation when it evaluates to true.
We want **TokenDemo** to terminate after all its tokens have died out, since there is nothing more to do at that point.
This is best implemented as a for-loop over all particles, checking if any still hold a token using ``hasToken()``.
Note that we leverage the encapsulation of both colored token types by checking for ``DemoToken``.

.. code-block:: c++

  bool TokenDemoSystem::hasTerminated() const {
    for (auto p : particles) {
      auto tdp = dynamic_cast<TokenDemoParticle*>(p);
      if (tdp->hasToken<TokenDemoParticle::DemoToken>()) {
        return false;
      }
    }

    return true;
  }

We want the ``TokenDemoSystem`` constructor to instantiate a hexagonal ring of particles and then add some fixed number of tokens to the system.
To create the ring, we leverage the :ref:`hexagon building technique <disco-system-constructor>` introduced in **DiscoDemo**, but instead of placing objects, we place particles.
Using ``std::make_shared`` and ``putToken()``, we add five tokens of each color to the first particle; i.e., the one at ``(0,0)``.
We also initialize these token's ``_lifetime`` variables according to the input parameter.

.. code-block:: c++

  TokenDemoSystem::TokenDemoSystem(int numParticles, int lifetime) {
    Q_ASSERT(numParticles >= 6);

    // Instantiate a hexagon of particles.
    int sideLen = static_cast<int>(std::round(numParticles / 6.0));
    Node hexNode = Node(0, 0);
    for (int dir = 0; dir < 6; ++dir) {
      for (int i = 0; i < sideLen; ++i) {
        // Give the first particle five tokens of each color.
        if (hexNode.x == 0 && hexNode.y == 0) {
          auto firstP = new TokenDemoParticle(Node(0, 0), -1, randDir(), *this);
          for (int j = 0; j < 5; ++j) {
            auto redToken = std::make_shared<TokenDemoParticle::RedToken>();
            redToken->_lifetime = lifetime;
            firstP->putToken(redToken);
            auto blueToken = std::make_shared<TokenDemoParticle::BlueToken>();
            blueToken->_lifetime = lifetime;
            firstP->putToken(blueToken);
          }
          insert(firstP);
        } else {
          insert(new TokenDemoParticle(hexNode, -1, randDir(), *this));
        }

        hexNode = hexNode.nodeInDir(dir);
      }
    }
  }

We conclude with the ``activate()`` function for ``TokenDemoParticle``.
This is split into four main parts:

1. *Retrieving a token*. We first check if this particle is holding a token of either color by using ``hasToken<DemoToken>()``, again leveraging the encapsulation of both colored token types by ``DemoToken``. If this is the case, we use ``takeToken<DemoToken>()`` to take the first such token out of this particle's collection.

2. *Calculating where to pass the token*. The exact details of this calculation are beside the point of this token-passing tutorial, but there is an important detail. If a token has not yet been passed, then the particle holding it needs to consistently pass ``RedTokens`` in one direction and ``BlueTokens`` in the other. To check what type of token we're dealing with, we use ``std::dynamic_pointer_cast<type>(token)`` which will be non-empty if and only if ``token`` is of type ``type``.

3. *Updating* ``_passedFrom`` *according to how the token is about to be passed*. This involves a simple for-loop that checks which neighbor direction points at this particle. Once the correct direction is found, the token's ``_passedFrom`` variable is accessed and updated.

4. *Passing the token if and only if it has* ``_lifetime`` *remaining*. We access and check the token's ``_lifetime`` variable to see if we should pass it on. If ``_lifetime == 0``, then we simply do nothing. Since we used ``takeToken()`` to remove this token from the particle's collection, doing nothing means it will cease to exist at the end of this ``activate()`` function. Otherwise, if it does have ``_lifetime`` remaining, we decrement it and use ``putToken()`` to pass it to the desired neighbor.

.. code-block:: c++

  void TokenDemoParticle::activate() {
    if (hasToken<DemoToken>()) {
      std::shared_ptr<DemoToken> token = takeToken<DemoToken>();

      // Calculate the direction to pass this token.
      int passTo;
      if (token->_passedFrom == -1) {
        // This hasn't been passed yet; pass red and blue in opposite directions.
        int sweepLen = (std::dynamic_pointer_cast<RedToken>(token)) ? 1 : 2;
        // ...
      } else {
        // This has been passed before; pass continuing in the same direction.
        // ...
      }

      // Update the token's _passedFrom direction. Needs to point at this particle
      // from the perspective of the next neighbor.
      for (int nbrLabel = 0; nbrLabel < 6; nbrLabel++) {
        if (pointsAtMe(nbrAtLabel(passTo), nbrLabel)) {
          token->_passedFrom = nbrLabel;
          break;
        }
      }

      // If the token still has lifetime remaining, pass it on.
      if (token->_lifetime > 0) {
        token->_lifetime--;
        nbrAtLabel(passTo).putToken(token);
      }
    }
  }


Final Touches
^^^^^^^^^^^^^

As in the other tutorials, the last step is to :ref:`register the algorithm <disco-register>` in the same way we did with **DiscoDemo**.
Compiling and running AmoebotSim after completing these steps will allow you to instantiate the **TokenDemo** simulation using either the command line or the sidebar interface.
Well done!

.. image:: graphics/tokenanimation.gif

.. tip::

  If you're interested in more advanced examples of token passing, consider studying the implementations of the leader election algorithms in AmoebotSim.


.. _metrics-demo:

MetricsDemo: Capturing Data
---------------------------

In this tutorial, we'll be adding custom metrics to the particle system that you have already built in the disco demo.
These custom metrics allow you, as the developer, to monitor a multitude of aspects of the particle system at hand.

AmoebotSim metrics are broken up into two different classes: counts and measures.
Counts are things that increment up by a certain value (usually 1).
They count the number of times a certain event has happened.
Measures are essentially any other types of metrics, things that change differently than just going up by 1.
They measure a broader, more "global" aspect of the system.

In this tutorial, we will be creating three custom metrics for the disco demo system: the number of times particles bump into the boundary wall (a count), the percentage of the system that is red (a measure), and finally, the greatest distance between any pair of particles (a measure).

Setting up the Environment
^^^^^^^^^^^^^^^^^^^^^^^^^^

Create the alg/demo/metricsdemo.h and alg/demo/metricsdemo.cpp files.
Then, copy and paste the discodemo header/source files into the accompanying metricsdemo header/source file, because we will be using the exact, full code of the disco demo you have already completed, just adding metrics to it.
In your new metricsdemo files which are essentially copies of the discodemo files, change any instance where DiscoDemo is used to MetricsDemo (DiscoDemoSystem -> MetricsDemoSystem, DiscoDemoParticle -> MetricsDemoParticle, etc.).

Just to be sure, your metricsdemo files should now look like this:

metricsdemo.h: ::

  #ifndef AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H
  #define AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H

  #include <QString>

  #include "core/amoebotparticle.h"
  #include "core/amoebotsystem.h"

  class MetricsDemoParticle : public AmoebotParticle {

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
    MetricsDemoParticle(const Node head, const int globalTailDir,
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

   public:
    // Constructs a system of the specified number of MetricsDemoParticles enclosed
    // by a hexagonal ring of objects.
    MetricsDemoSystem(unsigned int numParticles = 30, int counterMax = 5);

  };

  #endif // AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H

metricsdemo.cpp: ::

  /* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  #include "alg/demo/metricsdemo.h"

  MetricsDemoParticle::MetricsDemoParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system,
                                       const int counterMax)
      : AmoebotParticle(head, globalTailDir, orientation, system),
        _counter(counterMax),
        _counterMax(counterMax) {
    _state = getRandColor();
  }

  void MetricsDemoParticle::activate() {
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

  int MetricsDemoParticle::headMarkColor() const {
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

  int MetricsDemoParticle::tailMarkColor() const {
    return headMarkColor();
  }

  QString MetricsDemoParticle::inspectionText() const {
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

  MetricsDemoParticle::State MetricsDemoParticle::getRandColor() const {
    // Randomly select an integer and return the corresponding state via casting.
    return static_cast<State>(randInt(0, 7));
  }

  MetricsDemoSystem::MetricsDemoSystem(unsigned int numParticles, int counterMax) {
    _counts.push_back(new Count("# Bumps Into Wall"));
    _measures.push_back(new PercentageRedMeasure("Percentage Red", 1, *this));
    _measures.push_back(new MaxDistanceMeasure("Max 2 Particle Dist", 1, *this));

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
        insert(new MetricsDemoParticle(node, -1, randDir(), *this, counterMax));
        occupied.insert(node);
      }
    }
  }

Finally, register your new metricsdemo algorithm by adding the necessary code in the following places: script/scriptinterface.h, script/scriptinterface.cpp, and ui/algorithm.cpp .
Revisit the DiscoDemo tutorial for how to register a new algorithm.
Because our MetricsDemo simulation is so similar to the DiscoDemo simulation, we will be using the same default parameters, parameter checking, system instantiation, etc. that is used in the "Registering the Algorithm" section of the DiscoDemo tutorial.

Now you are ready to begin creating your first custom metrics!

Counts - Number of Bumps Into Boundary Wall
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Counts show the number of times a certain event has happened up to that point in the system.
Before we create our custom count metric, let's take a look at the basic format of a count which is outlined in core/metric.h : ::

  class Count {
   public:
    // Constructs a new count initialized to zero.
    Count(const QString name);

    // Increments the value of this count by the number of events being recorded,
    // whose default is 1.
    void record(const unsigned int numEvents = 1);

    // Member variables. The count's name should be human-readable, as it is used
    // to represent this count in the GUI. The value of the count is what is
    // incremented. History records the count values over time, once per round.
    const QString _name;
    unsigned int _value;
    std::vector<int> _history;
  };

As the comments outline, when creating a new count with Count(), you must fill the name parameter with a string (this will show up in the GUI) and the count's value starts at zero.

The record() function is used to register each time the event you are looking at happens and to increase the value of the count accordingly.
It is placed in the activate() function of the particle wherever the occurrence of the event being looked at would be identified.
The one parameter that record() has is numEvents, which is set to 1 by default.
This parameter determines how much the count increments up by each time the record function is called.
So, by default, your count will count up as follows: 0, 1, 2, 3, 4... .
But if numEvents is set to 3, for example, your count will count up as follows: 0, 3, 6, 9, 12... .

Now that we understand the basic format of the Count class, let's create a custom count to monitor how many times particles have bumped into the boundary wall.

First, as is the case with any custom metric (count or measure), we need to add the new count to the function of the system.
In the metricsdemo.cpp file, we will add our new count to the top of MetricsDemoSystem::MetricsDemoSystem() {}, filling the name parameter of Count() with "# Bumps Into Wall" : ::

  MetricsDemoSystem::MetricsDemoSystem(unsigned int numParticles, int counterMax) {
    _counts.push_back(new Count("# Bumps Into Wall"));
    ...
    ...
    ...
  }

Next, we must place the record() function of our count to the activate() function of MetricsDemoParticle.
Our count needs to count up every time a particle bumps into the hexagonal boundary wall of the system, so record() must be placed where this event would happen.
The way AmoebotSim functions and the way the basic discodemo particles behave, particles never actually bump into the wall.
Instead, our # of "bumps" metric will be counting when a particle is unable to move because of the wall being in its way.

Before placing our record() function where the inability to expand because of a wall occurs, let's take a look at the pseudocode for this situation: ::

  if (P is contracted), then do:
    expandDir <- random direction in [0, 6)
    if (node in direction expandDir is empty), then do:   // P can expand
      expand towards expandDir
    else, do: // P cannot expand
      if (node in direction expandDir is occupied by an object), then do:   // P cannot expand because of an object (wall), as opposed to cannot expand because of another particle
        # bumps into wall <- # bumps into wall + 1    // record()
      end if
    end if
  else, do:   // P is expanded
    contract tail
  end if

Now let's put this into our metricsdemo.cpp file in the activate() function of MetricsDemoParticle: ::

  void MetricsDemoParticle::activate() {
    ...
    ...

    if (isContracted()) {
      int expandDir = randDir();
      if (canExpand(expandDir)) {
        expand(expandDir);
      } else {
        if (hasObjectAtLabel(expandDir)) {
          system.getCount("# Bumps Into Wall").record();
        }
      }
    } else {
      contractTail();
    }

    ...
  }

As you can see, we are just adding the else...if section (which is when the particle can't expand because of an object/wall node) that comes after the if(canExand) {} statement.

Congratulations! You have just created your first custom metric!

Run the simulator and select Demo: Metrics to see the simulation in action.
You should see your count ("# Bumps Into Wall") in the GUI next to the other default metrics.

Measures - Percentage of Red Particles
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Measures show more broad, "global" aspects of the particle system.
As opposed to a count that simply increments up when a certain event occurs, measures vary much more fluidly.
They are basically any metrics that do not simply count up by 1 each time an event occurs.
Examples of measures include the percentage of a system in a particular state (what we are going to fo here), the perimeter of the system, etc.

Before we created our custom measure metric, let's take a look at the basic format and creation of a measure which is outlined in core/metric.h : ::

  class Measure {
   public:
    // Constructs a new measure with a given name and calculation frequency.
    Measure(const QString name, const unsigned int freq);
    virtual ~Measure();

    // Implements the measurement from the "global" perspective of the
    // AmoebotSystem being measured. Examples: calculate the percentage of
    // particles in a particular state, calculate the perimeter of a system, etc.
    // This is a pure virtual function and must be overridden by child classes.
    virtual double calculate() const = 0;

    // Member variables. The measure's name should be human-readable, as it is
    // used to represent this measure in the GUI. Frequency determines how often
    // the measure is calculated in terms of # of rounds. History records the
    // measure values over time, once per round.
    const QString _name;
    const unsigned int _freq;
    std::vector<double> _history;
  };

When creating a new measure with Measure(), there are now two parameters: name (the GUI name of the metric, a string), and freq (the frequency in which the measure is calculated/updated, an integer representing the number of rounds between calculating the new value of the measure).

The calculate() function is used to calculate the value of the measure, with these calculations being done from a global perspective, a system view.
This is where most of your code for a measure will reside.

Now that we understand the basic format of the Measure class, let's create a measure to monitor the percentage of the particles in the system that are red.

With measures, unlike with counts, we need to add some code to our header file to set up our custom measure, "PercentageRedMeasure", which inherits from the Measur class we looked at in metric.h . ::

  class PercentageRedMeasure : public Measure {

  public:
    PercentageRedMeasure(const QString name, const unsigned int freq,
                         MetricsDemoSystem& system);

    double calculate() const final;

  protected:
    MetricsDemoSystem& _system;
  };

Next, in order for our custom measure to have access to the information of the particle system as well as of the individual particles (which are separate classes), we must add the following line of code:

  friend class PercentageRedMeasure;

This needs to be placed in the definition of the MetricsDemoSystem class; all measures must be a friend class of the particle system class.

Additionally, this line needs to be placed in the definition of the MetricsDemoParticle class.
Measures only need to be friend classes of the particle class in some cases, depending on what information the metric needs access to.
For our PercentageRedMeasure, as you will see shortly, we need to access the states of individual particles; therefore, the measure must be a friend class of the particle class.

Your header file, in total, should now look like this: ::

  #ifndef AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H
  #define AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H

  #include <QString>

  #include "core/amoebotparticle.h"
  #include "core/amoebotsystem.h"

  class MetricsDemoParticle : public AmoebotParticle {
    friend class PercentageRedMeasure;

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
    MetricsDemoParticle(const Node head, const int globalTailDir,
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
    friend class PercentageRedMeasure;

   public:
    // Constructs a system of the specified number of MetricsDemoParticles enclosed
    // by a hexagonal ring of objects.
    MetricsDemoSystem(unsigned int numParticles = 30, int counterMax = 5);

  };

  class PercentageRedMeasure : public Measure {

  public:
    PercentageRedMeasure(const QString name, const unsigned int freq,
                         MetricsDemoSystem& system);

    double calculate() const final;

  protected:
    MetricsDemoSystem& _system;
  };

  #endif // AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H

Next, moving on to the source file, we need to add our new PercentageRedMeasure to the measures of the simulation.
This should look similar to what we did when adding our count, with a few extra parameters and a few small differences.
Add the following line of code to the top of MetricsDemoSystem::MetricsDemoSystem() {} : (just above or below where you initialized the # Bumps count) ::

  _measures.push_back(new PercentageRedMeasure("Percentage Red", 1, *this));

Using our PercentageRedMeasure() which we just defined in the header file, the parameters are as follows:

- "Percentage Red" - name, the string for the measure's GUI name
- 1 - freq, the integer of how many rounds between the measure is re-calculated and updated
- *this - system, *this is use because we are already in MetricsDemoSystem::MetricsDemoSystem() {} which is the system we are looking at

Next, still in metricsdemo.cpp, we need to declare our custom measure class, PercentageRedMeasure : ::

  PercentageRedMeasure::PercentageRedMeasure(const QString name, const unsigned int freq,
                                             MetricsDemoSystem& system)
    : Measure(name, freq),
      _system(system) {}

The final step is to write the calculate() function of our PercentageRedMeasure, which returns the value of the measure.

First, here is the pseudocode of the calculate() function: ::

  numRed <- 0
  for (P in system.particles) :   // loop through all particles in the system
    if (P is red), then do :
      numRed <- numRed + 1
    end if
  end for
  return (numRed / system.size) * 100   // system.size equals the total number of particles in the system

Now let's put this into actual code: ::

  double PercentageRedMeasure::calculate() const {
    int numRed = 0;

    for (const auto& p : _system.particles) {
      auto metr_p = dynamic_cast<MetricsDemoParticle*>(p);
      if (metr_p->_state == MetricsDemoParticle::State::Red) {
        numRed++;
      }
    }

    return ( (double(numRed) / double(_system.size())) * 100);
  }

Everything put together, your metricsdemo.cpp should now look like this: ::

  /* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  #include "alg/demo/metricsdemo.h"

  MetricsDemoParticle::MetricsDemoParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system,
                                       const int counterMax)
      : AmoebotParticle(head, globalTailDir, orientation, system),
        _counter(counterMax),
        _counterMax(counterMax) {
    _state = getRandColor();
  }

  void MetricsDemoParticle::activate() {
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
      } else {
        if (hasObjectAtLabel(expandDir)) {
          system.getCount("# Bumps Into Wall").record();
        }
      }
    } else {  // isExpanded().
      contractTail();
    }
  }

  int MetricsDemoParticle::headMarkColor() const {
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

  int MetricsDemoParticle::tailMarkColor() const {
    return headMarkColor();
  }

  QString MetricsDemoParticle::inspectionText() const {
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

  MetricsDemoParticle::State MetricsDemoParticle::getRandColor() const {
    // Randomly select an integer and return the corresponding state via casting.
    return static_cast<State>(randInt(0, 7));
  }

  MetricsDemoSystem::MetricsDemoSystem(unsigned int numParticles, int counterMax) {
    _counts.push_back(new Count("# Bumps Into Wall"));
    _measures.push_back(new PercentageRedMeasure("Percentage Red", 1, *this));

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
        insert(new MetricsDemoParticle(node, -1, randDir(), *this, counterMax));
        occupied.insert(node);
      }
    }
  }

  PercentageRedMeasure::PercentageRedMeasure(const QString name, const unsigned int freq,
                                             MetricsDemoSystem& system)
    : Measure(name, freq),
      _system(system) {}

  double PercentageRedMeasure::calculate() const {
    int numRed = 0;

    for (const auto& p : _system.particles) {
      auto metr_p = dynamic_cast<MetricsDemoParticle*>(p);
      if (metr_p->_state == MetricsDemoParticle::State::Red) {
        numRed++;
      }
    }

    return ( (double(numRed) / double(_system.size())) * 100);
  }

Congratulations! You have just created your first measure metric!

Run the simulator and select Demo: Metrics to see the simulation in action.
You should see your measure ("Percentage Red") in the GUI next to the other metrics.

Measures Cont. - Greatest Distance Between Any Pair of Particles
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You have already learned how to create your own custom metrics, both a count and a measure.
We are going to create another measure to monitor the greatest distance between any pair of particles in the system; although we are not going to label it this way here, this maximum two-particle distance is essentially the diameter of the system.

Although we will be following the same format and setup as was outlined in the previous section, this measure will have a more complicated calculate() function, thus giving you more thorough practice with measures.
Additionally, we are calculating distances here, which is an extremely valuable and important aspect of a particle system.
You will likely need to calculate distance in your future custom metrics.

Following the same steps as before, let's begin by setting up our new measure class in the header file: ::

  class MaxDistanceMeasure : public Measure {

  public:
    MaxDistanceMeasure(const QString name, const unsigned int freq,
                         MetricsDemoSystem& system);

    double calculate() const final;

  protected:
    MetricsDemoSystem& _system;
  };

Next, we need to make our measure class a friend class of the particle system, adding the following line to the top of class MetricsDemoSystem : public AmoebotSystem {}, just as before (Note: for this measure we do not need explicit access to individual particles, so we do not need to make our measure a friend class of the MetricsDemoParticle class): ::

  friend class MaxDistanceMeasure;

The completed header file should now look like this: ::

  #ifndef AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H
  #define AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H

  #include <QString>

  #include "core/amoebotparticle.h"
  #include "core/amoebotsystem.h"

  class MetricsDemoParticle : public AmoebotParticle {
    friend class PercentageRedMeasure;

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
    MetricsDemoParticle(const Node head, const int globalTailDir,
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
    friend class PercentageRedMeasure;
    friend class MaxDistanceMeasure;

   public:
    // Constructs a system of the specified number of MetricsDemoParticles enclosed
    // by a hexagonal ring of objects.
    MetricsDemoSystem(unsigned int numParticles = 30, int counterMax = 5);

  };

  class PercentageRedMeasure : public Measure {

  public:
    PercentageRedMeasure(const QString name, const unsigned int freq,
                         MetricsDemoSystem& system);

    double calculate() const final;

  protected:
    MetricsDemoSystem& _system;
  };

  class MaxDistanceMeasure : public Measure {

  public:
    MaxDistanceMeasure(const QString name, const unsigned int freq,
                         MetricsDemoSystem& system);

    double calculate() const final;

  protected:
    MetricsDemoSystem& _system;
  };

  #endif // AMOEBOTSIM_ALG_DEMO_METRICSDEMO_H

Next, moving on to the source file, we need to add our new measure to the top of MetricsDemoSystem::MetricsDemoSystem() {}, using the same frequency and system parameters as before: ::

  _measures.push_back(new MaxDistanceMeasure("Max 2 Particle Dist", 1, *this));

Next, declare our new measure class: ::

  MaxDistanceMeasure::MaxDistanceMeasure(const QString name, const unsigned int freq,
                                             MetricsDemoSystem& system)
    : Measure(name, freq),
      _system(system) {}

Finally, we need to write the measure's calculate() function.

Before we look at the pseudocode, we must consider one very important thing about calculating distance in AmoebotSim.
The simulator using a triangular lattice, with the x and y coordinates being determined as follows:

Image Here

As a result, we need to convert the triangular lattice coordinates to rectangular/Cartesian coordinates before calculating the distance between two points: ::

  x_c = x + (y / 2)
  y_c = (sqrt(3) / 2) * y

The above formulas convert the lattice coordinates (x and y) to Cartesian coordinates (x_c and y_c).
Once converted to Cartesian coordinates, you can calculate distances between points using the typical distance formula. (Note: this method of converting lattice coordinates to cartesian coordinates and then using the standard distance formula returns the Euclidian distance between the nodes, not the distance that particles would have to move along the paths of the lattice)

Here is the psuedocode for the calculate() function of our MaxDistanceMeasure: ::

  maxDist <- 0

  for (P1 in system.particles) :    // loop through all particles in the system to find the first particle of the pair of particle
    x1_c = P1.x + (P1.y / 2)
    y1_c = (sqrt(3) / 2) * P1.y
    for (P2 in system.particles) :    // loop through all particles to get second particle of the pair
      x2_c = P2.x + (P2.y / 2)
      y2_c = (sqrt(3) / 2) * P2.y
      dist <- sqrt( ((x2_c - x1_c) * (x2_c - x1_c)) + ((y2_c - y1_c) * (y2_c - y1_c)) )   // distance formula
      if (dist > maxDist), then do :
        maxDist <- dist
      end if
    end for
  end for

  return maxDist

Now, the actual code to add to the source file: ::

  double MaxDistanceMeasure::calculate() const {
    double dist;
    double maxDist = 0.0;

    for (const auto& p1 : _system.particles) {
      double x1_c = p1->head.x + p1->head.y/2.0;
      double y1_c = sqrt(3.0)/2 * p1->head.y;
      for (const auto& p2 : _system.particles) {
        double x2_c = p2->head.x + p2->head.y/2.0;
        double y2_c = sqrt(3.0)/2 * p2->head.y;
        dist = sqrt( pow((x2_c - x1_c), 2) + pow((y2_c - y1_c), 2) );
        if (dist > maxDist) {
          maxDist = dist;
        }
      }
    }

    return maxDist;
  }

With everything, metricsdemo.cpp should now look like this: ::

  /* Copyright (C) 2019 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  #include "alg/demo/metricsdemo.h"

  MetricsDemoParticle::MetricsDemoParticle(const Node head, const int globalTailDir,
                                       const int orientation,
                                       AmoebotSystem& system,
                                       const int counterMax)
      : AmoebotParticle(head, globalTailDir, orientation, system),
        _counter(counterMax),
        _counterMax(counterMax) {
    _state = getRandColor();
  }

  void MetricsDemoParticle::activate() {
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
      } else {
        if (hasObjectAtLabel(expandDir)) {
          system.getCount("# Bumps Into Wall").record();
        }
      }
    } else {  // isExpanded().
      contractTail();
    }
  }

  int MetricsDemoParticle::headMarkColor() const {
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

  int MetricsDemoParticle::tailMarkColor() const {
    return headMarkColor();
  }

  QString MetricsDemoParticle::inspectionText() const {
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

  MetricsDemoParticle::State MetricsDemoParticle::getRandColor() const {
    // Randomly select an integer and return the corresponding state via casting.
    return static_cast<State>(randInt(0, 7));
  }

  MetricsDemoSystem::MetricsDemoSystem(unsigned int numParticles, int counterMax) {
    _counts.push_back(new Count("# Bumps Into Wall"));
    _measures.push_back(new PercentageRedMeasure("Percentage Red", 1, *this));
    _measures.push_back(new MaxDistanceMeasure("Max 2 Particle Dist", 1, *this));

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
        insert(new MetricsDemoParticle(node, -1, randDir(), *this, counterMax));
        occupied.insert(node);
      }
    }
  }

  PercentageRedMeasure::PercentageRedMeasure(const QString name, const unsigned int freq,
                                             MetricsDemoSystem& system)
    : Measure(name, freq),
      _system(system) {}

  double PercentageRedMeasure::calculate() const {
    int numRed = 0;

    for (const auto& p : _system.particles) {
      auto metr_p = dynamic_cast<MetricsDemoParticle*>(p);
      if (metr_p->_state == MetricsDemoParticle::State::Red) {
        numRed++;
      }
    }

    return ( (double(numRed) / double(_system.size())) * 100);
  }

  MaxDistanceMeasure::MaxDistanceMeasure(const QString name, const unsigned int freq,
                                             MetricsDemoSystem& system)
    : Measure(name, freq),
      _system(system) {}

  double MaxDistanceMeasure::calculate() const {
    double dist;
    double maxDist = 0.0;

    for (const auto& p1 : _system.particles) {
      double x1_c = p1->head.x + p1->head.y/2.0;
      double y1_c = sqrt(3.0)/2 * p1->head.y;
      for (const auto& p2 : _system.particles) {
        double x2_c = p2->head.x + p2->head.y/2.0;
        double y2_c = sqrt(3.0)/2 * p2->head.y;
        dist = sqrt( pow((x2_c - x1_c), 2) + pow((y2_c - y1_c), 2) );
        if (dist > maxDist) {
          maxDist = dist;
        }
      }
    }

    return maxDist;
  }

Run the simulation and you should now see your new measure, "Max 2 Particle Dist"!

Exporting Data
^^^^^^^^^^^^^^
