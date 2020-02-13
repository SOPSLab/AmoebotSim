Installation Guide
==================

Regardless of whether you're studying the amoebot model for a class project, conducting world-class programmable matter research, or contributing to the ongoing development of this simulator, there's a version of AmoebotSim for you.
Note that the installation details of third-party software may change without our knowledge.
Feel free to :ref:`contact us <contact-us>` if you encounter any issues.


.. _install-explorer:

For Explorers 🔎
----------------

If you want a standalone, front-end version of AmoebotSim without having to deal with its source code, you can download the latest stable release `here <https://github.com/SOPSLab/AmoebotSim/releases>`_.


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


Installing Sphinx
^^^^^^^^^^^^^^^^^

These docs are written in reStructuredText and built using `Sphinx <https://www.sphinx-doc.org>`_.
If you want to edit these docs and preview them on your own machine, follow these instructions:

#. Follow the `Sphinx installation instructions <https://www.sphinx-doc.org/en/master/usage/installation.html>`_ for your platform.
#. Use ``pip`` to install the ReadTheDocs theme: ``pip install sphinx_rtd_theme``.
#. On the command line, navigate to the ``docs/`` directory of AmoebotSim.
#. Run the ``make html`` command to build the docs.
#. Preview the docs by opening ``docs/build/html/index.html`` in your browser of choice.


Installing Qt
^^^^^^^^^^^^^

AmoebotSim is built in Qt to ensure compatibility across operating systems.
Please follow these instructions carefully, as many small deviations can cause issues with building and running the project.

#. Navigate to Qt's `download page <https://www.qt.io/download>`_ and choose the "Go open source" option under "Downloads for open source users".
#. Scroll to the bottom of the page and select "Download the Qt Online Installer".
#. Download the correct version of the installer for your operating system.
#. Run the installer. Due to recent `Qt offering changes <https://www.qt.io/blog/qt-offering-changes-2020>`_, you will have to create a Qt account.
#. On the "Select Components" screen, locate the ``Qt`` tab and choose the components listed below. Note that you do not need any other components; these can be unselected to dramatically decrease the application size and installation time.
#. Agree to the licenses and install.

Under the subtab for the latest Qt version, e.g., ``Qt > Qt 5.14.1``, select the following Qt components:

* The latest prebuilt components for your platform. On macOS, this is ``macOS``; on Windows, this is the latest MinGW toolchain, e.g., ``MinGW 7.3.0 64-bit``. Note that on Windows there may be 32-bit and 64-bit versions to choose from; you should choose the latest version for your architecture.
* The latest Qt source components, ``Sources``.
* The latest scripting engine, ``Qt Script (Deprecated)``.


Configuring, Building, and Running AmoebotSim
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

With the repository cloned and Qt installed, the only thing that's left to do is configure the project settings in Qt.

#. Open AmoebotSim in Qt Creator by opening ``AmoebotSim.pro`` in the repository directory.
#. Select "Projects" in the left sidebar, and in the next-left sidebar that appears, choose "Build" under "Build & Run" (this may already be selected).
#. At the top of the page next to "Edit build configuration", choose "Debug" from the first drop-down menu.
#. For "General > Build Directory", choose a directory *outside* the repository directory housing the AmoebotSim source code (otherwise, you will need to add the build directory to your ``.gitignore``). Repeat this step for the "Profile" and "Release" configurations, targeting different build directories for each.
#. In the bottom-left of Qt Creator, set the configuration back to "Debug" (best for development) and click the green arrow to build and run. AmoebotSim should appear.
