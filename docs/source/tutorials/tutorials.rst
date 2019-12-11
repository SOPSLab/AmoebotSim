Tutorials
=========

These tutorials are for researchers and developers wanting to implement distributed algorithms for programmable matter in AmoebotSim.
As a prerequisite, make sure you've followed the instructions in the :doc:`Installation Guide </install/install>` so that everything builds and runs correctly.
Additionally, you should be comfortable with how the `amoebot model <?>`_ is defined.

All tutorials can be found in the ``alg/demo/`` directory of AmoebotSim so you can build and run them yourself.
Note that the coding style in these tutorials follows our :ref:`C++ Style Guide <?>` which we will not be re-explaining here.

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

* ``script/`` contains the classes used for interpreting and executing commands issued to AmoebotSim via its `JavaScript API <?>`_.

* ``ui/`` contains the classes used for front end GUI rendering and visualization.

.. todo::
    Add links.

If you're a researcher or developer wanting to implement new distributed algorithms in AmoebotSim, these are the classes you should get familiar with while reading these tutorials:

* ``Node`` (in ``core/node.h``) describes the ``(x,y)`` nodes of the triangular lattice.

* ``Object`` (in ``core/object.h``) describes a single node of a static object.

* ``Particle`` (in ``core/particle.*``) describes the most basic form of a particle, essentially reducing this entity to the node(s) it occupies.

* ``LocalParticle`` (in ``core/localparticle.*``) is a child class of ``Particle`` that adds functions for observing and using local information. In particular, these functions focus on obscuring global directions from a particle so that everything it sees is from its own local perspective. There are also functions for "cheating" and accessing some level of global information, if desired; however, these are meant only for convenience and are not compliant with the assumptions of the amoebot model.

* ``AmoebotParticle`` (in ``core/amoebotparticle.*``) is a child class of ``LocalParticle`` that adds functions for particle activations, particle movements, and token passing. All particles running new algorithms inherit from this class.

* ``AmoebotSystem`` (in ``core/amoebotsystem.*``) is a glorified container of ``AmoebotParticles`` that keeps track of the particle system's size, position, and progress. All particle systems running new algorithms inherit from this class.

.. todo::
    Circle back later to check if there's a level of class-linking that would be beneficial to include in this documentation.


Your First Algorithm: DiscoDemo
-------------------------------
