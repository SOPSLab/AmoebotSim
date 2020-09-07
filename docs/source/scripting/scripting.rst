Scripting
=========

This scripting reference is for researchers 🧪 and developers 💻 learning how to write custom JavaScript experiments for AmoebotSim.

.. todo::
  Add an introduction to scripting in AmoebotSim.


.. _script-api:

Scripting API
-------------

The following is a list of all recognized commands.

.. js:function:: log(msg, error)

  :param string msg: A message to log to AmoebotSim's interface.
  :param boolean error: ``true`` if and only if this is an error message.

  Emits the message ``msg`` to the console, and can be denoted as an error message by setting ``error`` to ``true``.

.. js:function:: runScript(scriptFilePath)

  :param string scriptFilePath: The file path of a JavaScript script to be run by the simulator's engine.

  Loads a JavaScript script from the provided filepath ``scriptFilePath`` and executes it.

.. js:function:: writeToFile(filePath, text)

  :param string filePath: The path of a file to write text to.
  :param string text: A piece of text to write to a specified file.

  Appends the specified ``text`` to a file at the given location ``filePath``.

.. js:function:: step()

  Executes a single particle activation.

.. js:function:: setStepDuration(ms)

  :param int ms: The number of milliseconds (positive integer) between individual particle activations.

  Sets the simulator's delay between particle activations to the given value ``ms``.

.. js:function:: runUntilTermination()

  Runs the current algorithm instance until its ``hasTerminated`` function returns true.

.. js:function:: getNumParticles()

  Returns the number of particles in the system in the given instance.

.. js:function:: getNumObjects()

  Returns the number of objects in the system in the given instance.

.. js:function:: exportMetrics()

  Writes the metrics (all metrics' historical data) to JSON as ``your_build_directory/metrics/metrics_<secs_since_epoch>.json``.

.. js:function:: getMetrics(name, mode)

  :param string name: The name of a metric.
  :param string mode: ``"v"`` to return the metric's current value and ``"h"`` to return the metric's history.

  For a metric with specified ``name``, returns either its current value (``mode = "v"``) or historical data (``mode = "h"``).

.. js:function:: setWindowSize(width, height)

  :param int width: The width in pixels; 800 by default.
  :param int height: The height in pixels; 600 by default.

  Sets the size of the application window to the specified ``width`` and ``height``.

.. js:function:: focusOn(x, y)

  :param int x: An *x*-coordinate on the triangular lattice.
  :param int y: A *y*-coordinate on the triangular lattice.

  Sets the window's center of focus to the given (``x``, ``y``) node.
  Zoom level is unaffected.

.. js:function:: setZoom(zoom)

  :param float zoom: A value defining the level/amount of zoom.

  Sets the zoom level of the window to the given value ``zoom``.

.. js:function:: saveScreenshot(filePath)

  :param string filePath: A filepath of the image to be captured; ``your_build_directory/amoebotsim_<secs_since_epoch>.png`` by default.

  Saves the current window as a .png in the specified location ``filePath``.

.. js:function:: filmSimulation(filePath, stepLimit)

  :param string filePath: A filepath of the images to be captured.
  :param int stepLimit: A maximum number of steps that will be captured by the screenshots.

  Saves a series of screenshots to the specified location ``filePath``, up to the specified number of steps ``stepLimit``.
