Installation Guide
==================

Regardless of whether you're studying the amoebot model for a class project, conducting world-class programmable matter research, or contributing to the ongoing development of this simulator, there's a version of AmoebotSim for you.
Note that the installation details of third-party software may change without our knowledge.
Feel free to :ref:`contact us <contact-us>` if you encounter any issues.


.. _install-explorer:

For Explorers 🔎
----------------

If you want a standalone, front-end version of AmoebotSim without having to deal with its source code, you can download the latest stable release `here <todo>`_.

.. todo::
  Add link to releases page. Add any extra instructions needed once that's in place.


.. _install-researchdev:

For Researchers 🧪 and Developers 💻
-------------------------------------

If you want to modify AmoebotSim's source code, either to experiment with your own algorithms or integrate your changes with our repository, follow these instructions.


Accessing the AmoebotSim Repository
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

`AmoebotSim <https://github.com/SOPSLab/AmoebotSim>`_ is hosted as a public repository on GitHub.
However, only our SOPS Lab development team has push access, meaning that you won't be able to push your code changes directly.
For those familiar with this paradigm, feel free to fork the AmoebotSim repository, clone your forked repository onto your machine, and go on to the next section.

For those who'd like a little more help or are new to Git, here's a walkthrough.
Our team has experimented with a variety of visual Git clients over the years to simplify our workflows.
We recommend using `GitKraken <https://www.gitkraken.com/>`_, though you're more than welcome to use another client that better suits your tastes (e.g., `GitHub Desktop <https://desktop.github.com/>`_ or `SourceTree <https://www.sourcetreeapp.com/>`_) or stick to Git's command line interface.
Doesn't matter to us :)
This walkthrough will assume you're using GitKraken.

#. If you don't have a GitHub account, `sign up <https://github.com/>`_ for one.
#. `Fork our repository <https://help.github.com/en/github/getting-started-with-github/fork-a-repo>`_ so you can work on your own copy.
#. Download and install `GitKraken <https://www.gitkraken.com/>`_, and log in with your GitHub account.
#. Clone *your forked copy* of the AmoebotSim repository. In more detail:

  * Open GitKraken and navigate to "File > Clone Repo".
  * In the sidebar of the overlay that opens, choose "GitHub.com" and connect your GitHub account if not already done.
  * Choose ``AmoebotSim`` under your username as the "Repository to clone".
  * For "Where to clone to", specify the directory on your machine where you'd like AmoebotSim's source code to live. You may also optionally change the "Full Path".
  * Select "Clone the repo!" to finish the process. You may be prompted to enter your GitHub login credentials.

AmoebotSim's source code should now be in the directory you specified.


Installing Qt
^^^^^^^^^^^^^

AmoebotSim is built in Qt to ensure compatibility across operating systems.
Please follow these instructions carefully, as many small deviations can cause issues with building and running the project.

#. Navigate to Qt's `download page <https://www.qt.io/download>`_ and choose the "Go open source" option under "Downloads for open source users".
#. Scroll to the bottom of the page and select "Download the Qt Online Installer".
#. Download the correct version of the installer for your operating system.
#. Run the installer. You can skip the step asking for your Qt login if you don't have one.
#. On the "Select Components" screen, locate the ``Qt`` tab (you can ignore ``Preview``) and choose the components listed below. Note that you do not need any other components; these can be unselected to dramatically decrease the application size and installation time.
#. Agree to the licenses and install.

You should select the following Qt components:

.. todo::
  Need to verify all of the above instructions and then actually do the installer with some version that works.


Configuring, Building, and Running AmoebotSim
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With the repository cloned and Qt installed, the only thing that's left to do is configure the project settings in Qt.

#. Open AmoebotSim in Qt Creator by opening ``AmoebotSim.pro`` in the repository directory.
#. Select "Projects" in the left sidebar, and in the next-left sidebar that appears, choose "Build" under "Build & Run" (this may already be selected).
#. At the top of the page next to "Edit build configuration", choose "Debug" from the first drop-down menu.
#. For "General > Build Directory", choose a directory *outside* the repository directory housing the AmoebotSim source code (otherwise, you will need to add the build directory to your ``.gitignore``).
#. Select "Build Steps > qmake > Enable QML debugging and profiling".
#. Set "Build Steps > make > Parallel jobs" to the number of cores on your machine.
    .. note::
      This will considerably increase memory usage during compilation.
#. Repeat Steps 3, 4, and 6 for the "Profile" and "Release" configurations, but do not enable QML debugging and profiling.
#. If you are using Windows, select "Run" under "Build & Run" in the second-left sidebar. Under "Run Environment", look for an environment variable called ``QT_OPENGL``. If this variable exists, make sure its value is ``desktop``.
    .. todo::
      Check if this is still an issue with the updated Qt versions.
#. In the bottom-left of Qt Creator, set the configuration back to "Debug" (best for development) and click the green arrow to build and run. AmoebotSim should appear.

.. todo::
  Add an updated screenshot of AmoebotSim once we settle on the public release version.
