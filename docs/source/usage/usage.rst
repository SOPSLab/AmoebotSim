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

Interact with individual particles by using the following button+click interactions.

.. csv-table::
  :header: "Windows", "macOS", "The selected particle..."
  :widths: auto

  ``Ctrl`` + ``left click``, ``Cmd`` + ``left click``, ...executes a single activation
  ``Alt`` + ``left click``, ``Option`` + ``left click``, ...shows its inspection text

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

AmoebotSim's command line can be opened by pressing ``Return``.
The following is a list of all recognized commands.

.. todo::
  The script interface will likely no longer exist as a command line interface, but will still be available for scripting. So this section will likely need to move elsewhere or at least be presented differently.
  There would of course be more things exposed by ``ScriptInterface``, so we'll need to potentially add everything in there to an API area.

.. js:function:: log(msg, error)

  :param string msg: A message to log to the command line.
  :param boolean error: ``true`` if and only if this is an error message.

  Emits the message ``msg`` to the console, and can be denoted as an error message by setting ``error`` to ``true``.
