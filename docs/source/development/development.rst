Development
===========

This development guide gives background on Git workflow, code review, and coding best practices for developers 💻 contributing new features and algorithms to AmoebotSim.


Repository Architecture
-----------------------

Our recommended repository architecture involves three distinct repositories (remotes); in this guide, we will refer to them as **public**, **fork**, and **private**.

The **public** repository is `SOPSLab/AmoebotSim <https://github.com/SOPSLab/AmoebotSim>`_, the canonical version of AmoebotSim maintained by the SOPS Lab.
The SOPS Lab developers use **public** for developing new simulator features, collecting issues from users, and maintaining the documentation.
This repository is publicly visible and readable, but only SOPS Lab developers have push access.

The **fork** repository is your `fork <https://help.github.com/en/github/getting-started-with-github/fork-a-repo>`_ of AmoebotSim, e.g., *janedoe/AmoebotSim*.
You and your team would use **fork** primarily as a place to open `pull requests <https://help.github.com/en/github/collaborating-with-issues-and-pull-requests/about-pull-requests>`_ (PRs) for changes that you want reviewed by a SOPS Lab developer and merged into **public**.
This repository would also be publicly visible and readable, but only you and your `collaborators <https://help.github.com/en/github/setting-up-and-managing-your-github-user-account/inviting-collaborators-to-a-personal-repository>`_ would have push access.

Finally, the **private** repository is a private `duplicate <https://help.github.com/en/github/creating-cloning-and-archiving-repositories/duplicating-a-repository>`_ of **public**, e.g., *janedoe/AmoebotSim-Internal*.
This is where you and your team would do actual development work, either in helping us create new features for AmoebotSim or in working on your own amoebot model algorithms.
Importantly, you would set this up as a `private repository <https://help.github.com/en/github/administering-a-repository/setting-repository-visibility>`_ (i.e., only visible to you and your collaborators) so you can safely experiment with new or unpublished ideas.

.. note::
  You may not need **private** in your architecture if (1) you don't need to keep your algorithms private, or (2) you only plan to contribute to feature development, which can be done in the open.
  In these cases, you can simply do all your development work in **fork**.


Git Workflow
------------

A Git workflow and branching model guides how development work is organized, reviewed, and ultimately released for public use.
If you're new to contributing to projects on GitHub, you should read Aaron Meurer's `tutorial <https://www.asmeurer.com/git-workflow/>`_ on the standard Git workflow.
This will give you the basic concept of branching, PRs, and merging.
Note that there are some minor differences,

We envision your workflow as follows:


1. Identify Your Contribution
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you want to contribute new code, `open an issue <https://github.com/SOPSLab/AmoebotSim/issues/new/choose>`_ first to tell us what you want to do.
Each issue you open should be a small, logical collection of changes (e.g., one bug fix, one new algorithm, etc.).
Feel free to open multiple issues if you have several ideas!

If you want to contribute code for one of our `open issues <https://github.com/SOPSLab/AmoebotSim/issues>`_, feel free to choose any labeled `help-wanted <https://github.com/SOPSLab/AmoebotSim/labels/help-wanted>`_.
Please leave a comment on the issue you've chosen so we know you want to work on it.


2. Do Your Development Work
^^^^^^^^^^^^^^^^^^^^^^^^^^^

We use Vincent Driessen's `Git branching model <https://nvie.com/posts/a-successful-git-branching-model/>`_, so all feature branches branch from and merge back to the ``dev`` branch.
Create a new feature branch for your issue off of ``dev`` in your **private** repository (or in **fork**, if you don't need the privacy).
Give it a descriptive name, like ``bugfix-alg-inputs`` or ``docs-api-update``.
Then, write your code as usual, committing your changes to your feature branch.

Please follow our style guides for :ref:`C++ <cpp-style>` and :ref:`Git commit messages <git-commit-style>` as closely as possible.
These style guides help make your code readable and understandable, which will help in getting your changes accepted quickly.
Your code doesn't need to be perfect, but we will be asking you to fix any issues we find during code review.


3. Code Review
^^^^^^^^^^^^^^

When you're ready to have your changes reviewed, push your feature branch to **fork** (if it wasn't already there) and open a PR from your feature branch on **fork** to ``dev`` on **public**.
Please include a clear, detailed description of what the PR does, including an `issue reference <https://help.github.com/en/github/managing-your-work-on-github/closing-issues-using-keywords#about-issue-references>`_ so we can quickly associate your PR with the original issue it addresses.

Your PR will be approved by two SOPS Lab developers: a primary reviewer responsible for the correctness, efficiency, and style of your code, and a secondary reviewer responsible for cross-platform validation.
We may leave comments on your PR for you to address before we're ready to incorporate your changes into ``dev``.

Once approved, your PR will be accepted, the associated issues will be closed, and your changes will be merged into ``dev``.
Your changes will be included in the next release.


Workflow for SOPS Lab Developers
--------------------------------

SOPS Lab developers follow the same workflow as all other developers, with a few extra responsibilities.


Shepherding Issues
^^^^^^^^^^^^^^^^^^

New issues have to be sorted into one of three categories:

- **Work**: These are bug fixes, feature requests, and algorithm additions that we decide should be implemented in AmoebotSim. These issues (and *only* these issues) should be `added to our project board <https://help.github.com/en/github/managing-your-work-on-github/adding-issues-and-pull-requests-to-a-project-board#adding-issues-and-pull-requests-to-a-project-board-from-the-sidebar>`_. They should also be given appropriate labels, and can optionally be labeled `"help-wanted" <https://github.com/SOPSLab/AmoebotSim/labels/help-wanted>`_ to indicate that contributions are welcome.
- **No-go**: These are bug fixes, feature requests, and algorithm additions that we decide should *not* be implemented in AmoebotSim. These issues should be labeled `"no-go" <https://github.com/SOPSLab/AmoebotSim/labels/no-go>`_ and closed.
- **Question**: These are questions that need to be answered. If the question hasn't been answered before, feel free to answer in a comment. Otherwise, label the question as `"duplicate" <https://github.com/SOPSLab/AmoebotSim/labels/duplicate>`_, refer the asker to the `previous question <https://github.com/SOPSLab/AmoebotSim/issues?utf8=%E2%9C%93&q=is%3Aissue+label%3Aquestion>`_ or the :ref:`FAQs <faqs>` where their question has already been answered, and close the issue.


Prioritizing Work
^^^^^^^^^^^^^^^^^

Issues added to our `project board <https://github.com/SOPSLab/AmoebotSim/projects/2?fullscreen=true>`_ automatically appear in the *Backlog* column.
When a SOPS Lab developer is ready to work on an issue, they should `assign themselves <https://help.github.com/en/github/managing-your-work-on-github/assigning-issues-and-pull-requests-to-other-github-users>`_ to the issue and move the corresponding card on the project board to *Current Sprint*.
In this workflow, *Current Sprint* represents features and bug fixes currently being developed in house.

.. note::
  SOPS Lab developers should remove the `"help-wanted" <https://github.com/SOPSLab/AmoebotSim/labels/help-wanted>`_ label on issues moved to *Current Sprint* so that it's clear the issue is already being worked on.


Code Review
^^^^^^^^^^^

Just like new issues, new pull requests to ``dev`` need to be shepherded.
If a PR is associated with an issue that we've already decided to implement, then it should be given labels, assigned reviewers (see below), and `added to our project board <https://help.github.com/en/github/managing-your-work-on-github/adding-issues-and-pull-requests-to-a-project-board#adding-issues-and-pull-requests-to-a-project-board-from-the-sidebar>`_.
These will automatically be added to the *Staged* column.

All PRs to ``dev`` must have two reviewers, one on Windows and the other on macOS.
Both reviewers must check:

- Do all visual components (particle system, sidebar, buttons, etc.) render correctly?
- Do all :ref:`controls <controls>` work correctly?
- Do at least three algorithms (especially any added/updated by this PR) instantiate and run successfully?

The primary reviewer additionally checks correctness, efficiency, and style of the code.

Once both reviewers approve the PR, it will automatically move to the *Approved* column.
Then, once it's merged, it along with any issues it resolves (according to its `issue references <https://help.github.com/en/github/managing-your-work-on-github/closing-issues-using-keywords#about-issue-references>`_) will be closed and moved to the *Done* column.


Managing Releases
^^^^^^^^^^^^^^^^^

When ``dev`` reflects the desired state of a new release, then (following Vincent Driessen's `branching model <https://nvie.com/posts/a-successful-git-branching-model/#release-branches>`_), the version number is incremented according to `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_ and a new branch ``release-[major].[minor].[patch]`` is branched off of ``dev``.
This release branch acts as a holding place to do final testing and incrementing any version numbers appearing in the source code.

Add a new column to the project board titled *Release (v.[major].[minor].[patch])*, and move all cards corresponding to completed issues and PRs included in this release from *Done* to this column.
Update AmoebotSim's :ref:`Changelog <changelog>` by summarizing the cards in this release column, adding to the changelog in reverse chronological order (newest release first).

When the release branch is ready, merge it into ``master`` and tag the resulting merge commit with the ``[major].[minor].[patch]`` version number.
The release branch should also be merged back into ``dev``.
After these merges, the release branch can be deleted.

The last step is to deploy AmoebotSim as Windows and macOS applications from this new tag on ``master``.
Instructions for Qt deployment can be found `here <https://doc.qt.io/qt-5/windows-deployment.html>`_ for Windows and `here <https://doc.qt.io/qt-5/macos-deployment.html>`_ for macOS.
Compress the resulting deployments as ``amoebotsim-windows.zip`` and ``amoebotsim-macos.zip`` for Windows and macOS, respectively, and upload both to a new `GitHub release <https://help.github.com/en/github/administering-a-repository/creating-releases>`_ associated with tag ``[major].[minor].[patch]`` in our repository.


Style Guides
------------


.. _cpp-style:

C++ Source Code
^^^^^^^^^^^^^^^

To the best of our academic ability, we try to adhere to the `Google C++ Style Guide <https://google.github.io/styleguide/cppguide.html>`_.
Here, we highlight some cherry-picked favorites to get you started.
Use existing code as a guide; feel free to :ref:`contact us <contact-us>` with any questions related to particular applications of style rules to AmoebotSim.

We aim to achieve two major goals with our style:

#. Code should be correct, clean, concise, and well documented. Code should be written with the understanding that people other than the person writing it need to read and understand it.
#. Code should compile without warnings (as much as possible) in order to avoid common coding mistakes, security concerns, etc.


General Aesthetics
""""""""""""""""""

The Google C++ Style Guide has many formatting rules meant to optimize code readability and portability; we emphasize two:

#. There should be at most 80 characters per line to make it easy to view and ``diff`` files side-by-side on reasonable screen sizes.
#. Indentations should be 2 spaces (instead of 1 tab or 4 spaces).

These can be set as default in Qt by navigating to "Tools > Options..." on Windows or "Qt Creator > Preferences" on macOS and making the following changes:

- Under "Text Editor > Display > Text Wrapping", make sure "Display right margin at column" is checked and set to 80.
- Under "Text Editor > Behavior > Tabs and Indentation", the tab policy should be "Spaces Only", the tab and indent size should both be 2, and "Align continuation lines:" should be "With Spaces".
- Under "C++ > Code Style", make a copy of "Qt [built-in]" and then "Edit..." this new code style. Under "General > Tabs and Indentation", set the variables as in the above step.


Defining Classes
""""""""""""""""

Classes in AmoebotSim should always be written in header/source file pairs.
Suppose you're developing a ``ExampleParticle`` class.
The associated files are ``alg/exampleparticle.h`` and ``alg/exampleparticle.cpp``.
The following code snippet shows the important elements of your header file.

.. code-block:: c++

  /* Copyright (C) 2020 Joshua J. Daymude, Robert Gmyr, and Kristian Hinnenthal.
   * The full GNU GPLv3 can be found in the LICENSE file, and the full copyright
   * notice can be found at the top of main/main.cpp. */

  // This is exampleparticle.h. After the copyright notice, the first thing that
  // goes in this file is a descriptive comment that explains what this class is
  // about. For example, you might say "Defines the particle system and composing
  // particles for the Example Algorithm, as defined in 'A Paper about Example
  // Amoebot Algorithms' [link to paper]." You might also include some notes, like
  // "This simulation differs from the algorithm in the paper in such and such
  // way."

  #ifndef AMOEBOTSIM_ALG_EXAMPLEPARTICLE_H_
  #define AMOEBOTSIM_ALG_EXAMPLEPARTICLE_H_

  #include <cmath>
  #include <vector>

  #include <QDebug>

  #include "core/amoebotparticle.h"

  class ExampleParticle : public AmoebotParticle {
   public:
    // (Almost) every function declaration should have a comment describing what
    // it does. Note that this does not need to describe *how* the function
    // achieves its goals, only what its goals actually are. Functions with
    // obvious goals (e.g., getters and setters) can be left uncommented.
    ExampleParticle(...);

    // "Executes one particle activation."
    virtual void activate();

    // ...
  };

  #endif  // AMOEBOTSIM_ALG_EXAMPLEPARTICLE_H_

A few takeaways from the above example:

- Every header file starts with a `class comment <https://google.github.io/styleguide/cppguide.html#Class_Comments>`_ that describes what the class is/does, and provides any relevant publications and references if applicable.

- Every header file uses `#define guards <https://google.github.io/styleguide/cppguide.html#The__define_Guard>`_ of the form ``<PROJECT>_<PATH>_<FILE>_H_``.

- Any ``#includes`` are grouped in order of standard C/C++ libraries, then any Qt libraries, and finally any AmoebotSim-specific headers. Each subgroup should appear in alphabetical order.

- Every non-obvious function declaration is preceded by a `function comment <https://google.github.io/styleguide/cppguide.html#Function_Comments>`_ describing *what* it does, but not *how* it does it. Accompanying comments in the function's definition (in the ``.cpp`` file) can describe tricky parts about *how* it does what it does.


.. _git-commit-style:

Git Commit Messages
^^^^^^^^^^^^^^^^^^^

Writing clear, concise, and descriptive commit messages is a key component of tracking a project's changelog.
Commit messages should be one or more lines, with the first being a subject line that summarizes the rest.
Each line (except possibly the subject line) should begin with ``+``, ``~``, ``-``, or ``*`` as follows:

.. csv-table::
  :header: "Start", "Meaning", "Example"
  :widths: auto

  ``+``, Addition, New features or documentation
  ``~``, Update/Change, Changes were made in a bug fix or optimization
  ``-``, Removal, Cleaning up or deprecating old code
  ``*``, Git Operation, Merge or rebase

Each line following the subject line should describe an individual change that was made.
For example, this is a good standalone commit message:

.. code-block:: text

  Updated functionality and fixed bugs for FuturisticParticles
  + Added functionality for FuturisticParticles to shoot lasers
  ~ Updated FuturisticParticle constructor to take laser color as input
  ~ Updated documentation that incorrectly stated FuturisticParticles hate Star Trek
  - Removed code that allowed FuturisticParticles to teleport
  ~ Fixed a bug where a FuturisticParticle couldn't communicate with its fifth neighbor
  - Deleted obsolete comments

In general, the change list should be ordered from most to least important.
Note that each change is described as a complete thought (though perhaps not a complete sentence).
If you tend to commit more frequently, consider something like the following:

.. code-block:: text

  Commit: [23ad5e1]
  ~ Fixed the bug where FuturisticParticles couldn't communicate with their fifth neighbors
  - Deleted some obsolete comments
  ~ Still investigating the teleportation bug...

  Commit: [bbda445]
  + Added a laser attribute to FuturisticParticle and updated the constructor accordingly
  - Deleted more obsolete comments
  - Deleted the bad code causing FuturisticParticles to teleport

  Commit: [c22f4f0]
  - Deleted all obsolete comments
  ~ Updated FuturisticParticle documentation regarding Star Trek

Don't write commit messages like:

.. code-block:: text

  Edited future particles

Or:

.. code-block:: text

  lasers, teleport fix

Or, for a bad multi-commit example:

.. code-block:: text

  Commit: [23ad5e1]
  working on FuturisticParticle

  Commit: [bbda445]
  more changes
  + lasers
  ~ Star Trek

  Commit: [c22f4f0]
  finished updates

These messages don't really tell the reader anything, which makes them hard to navigate later when hunting for a specific change.
