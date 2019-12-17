Usage
=====

This usage guide details the controls and functionality of AmoebotSim's front-end user interface.


At A Glance
-----------

.. todo::
  Add an annotated figure of AmoebotSim's interface, similar to what matplotlib did in their `usage guide <https://matplotlib.org/tutorials/introductory/usage.html#sphx-glr-tutorials-introductory-usage-py>`_. Then follow up with various explanations.


.. _controls:

Controls
--------

Move the scene by left-clicking and dragging.
Zoom in and out with the scroll wheel.
Use the "Start" and "Stop" buttons to control the current simulation.
Use the slider to control the speed of the current simulation.

.. todo::
  Need to now also explain getting data.

Clicking on a particle while holding ``Ctrl`` on Windows (or ``Cmd`` on macOS) causes that particle to execute a single activation. Clicking on a particle while holding ``Alt`` prints its "inspection text", which contains various information about its state.

.. todo::
  Need to talk about algorithm instantiation.

The following keyboard shortcuts are also available.

.. csv-table::
  :header: "Windows", "macOS", "Description"
  :widths: auto

  ``Ctrl+E``, ``Cmd+E``, Start/stop the current simulation
  ``Ctrl+D``, ``Cmd+D``, Execute a single particle activation
  ``Ctrl+F``, ``Cmd+F``, Recenter the scene on the particle system
  ``Ctrl+B``, ``Cmd+B``, Hide/show UI elements (useful for presentations)

.. todo::
  Need to add a ``Ctrl+S`` shortcut to save data.


Command Line
------------

.. deprecated:: 0.1.0
  The script interface will likely no longer exist as a command line interface, but will still be available for scripting. So this section will likely need to move elsewhere or at least be presented differently.

AmoebotSim's command line can be opened by pressing ``Return``.
The following is a list of all recognized commands.

.. js:function:: log(msg, error)

  :param string msg: A message to log to the command line.
  :param boolean error: ``true`` if and only if this is an error message.

  Emits the message ``msg`` to the console, and can be denoted as an error message by setting ``error`` to ``true``.

.. todo::
  There would of course be more things exposed by ``ScriptInterface``, so we'll need to potentially add everything in there to an API area.
