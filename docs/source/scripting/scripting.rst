Scripting
=========

This scripting reference is for researchers 🧪 and developers 💻 learning how to write custom JavaScript experiments for AmoebotSim.

Instead of simply using the user interface controls to run a single algorithm instance, AmoebotSim also allows for scripts to be run, enabling more complex and custom control of the simulator.
Applications and benefits of scripting include: running large numbers of algorithm instances automatically and consecutively, adjusting algorithm parameters more fluidly, using metrics data in different ways, lowering runtime due to graphics being streamlined, etc.


Writing Scripts
---------------

Writing custom JavaScript experiments for AmoebotSim follows the same exact syntax and process as writing any other standard JavaScript file.
The only difference is the added set of custom commands specific to AmoebotSim (listed below in the :ref:`JavaScript API <script-api>`), which can all be easily added to your script.

Here is an example of a simple JavaScript experiment:

.. code-block:: javascript

  for (var run = 0; run < 25; run++) {
    shapeformation(100, 0.2, "h");
    runUntilTermination();
    writeToFile('shapeformation_data.txt', getMetric("# Rounds") + '\n');
  }

In the above script, AmoebotSim will run 25 instances of the shape formation algorithm (with given parameters), writing the value of the "# Rounds" metric at the end of each individual algorithm instance to a text file.

It is clear how the simple scripting above can be expanded to carry out much more complex experiments.

.. warning::
  The JavaScript file must be saved in a location that is within the application's executable/build directory.
  Otherwise, AmoebotSim's JavaScript engine will not be able to locate or execute the script.


Running/Executing Scripts
-------------------------

Once you have written your JavaScript experiment and saved it in an accepted location, press the *Run Script* button in the sidebar of the user interface (see the :ref:`usage guide <usage-at-a-glance>`) to run your script.
This will open a file manager window where you will be prompted to find and select the desired JavaScript file.
Immediately after the file is selected and accepted by the dialog, the script will begin to run.
Notably, graphics (moving particles, etc.) are not updated or displayed by AmoebotSim while a script is being executed.
When the script execution completes, graphics will return, showing the new state of the particle system, and the following message will be logged to the simulator: ``Ran script: path_to_file/your_script.js``.

The following animation illustrates the process of loading and running a script in AmoebotSim:

.. image:: graphics/scriptinganimation.gif

.. note::
  As seen in the video demonstration, the simulator window may emit a message reading *"Not Responding"* while the script is in the process of executing.
  Ignore this error message; it is simply acknowledging that graphics are not currently being updating.


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

.. js:function:: getMetric(name, history)

  :param string name: The name of a metric.
  :param boolean history: ``true`` to return the metric's history or ``false`` to return the metric's current value; ``false`` by default.

  For a metric with specified ``name``, returns either its current value (``history = false``) or historical data (``history = true``).

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
