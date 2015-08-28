# Welcome to AmoebotSim, a graphical simulator for the Amoebot model. #

The following instructions should get you started working with AmoebotSim; please read them carefully.

###Setup###

1. Download and install the latest version of Qt [1], __version 5.3 or newer is required__. In my experience, the online installer works best. On windows, please make sure to use the MinGW platform (there are checkboxes under _Qt 5.x.x_ and under _Tools_) and not MSVC. In general, the additional platforms (Android, iOS, ...) are not required and can be left out to decrease the size of the installation. When installing Qt 5.5 make sure you also install the qt script module (there's a checkbox under Qt 5.5). 

2. Clone this repository using Git. The address of the repository depends on your BitBucket username and can be found in the top right corner of the project site on BitBucket [2]. If you want to use a GUI-Application for Git and you are on Windows or OS X , I can recommend Source Tree [3].

3. In the cloned repository, simply double-click the file _AmoebotSim.pss_ to open the project in Qt Creator, the IDE distributed with Qt.

4. Choose your build directories.

5. Run AmoebotSim by pressing the green _play_-button in the bottom left corner.

###Development###

- It is advisable to enable building using multiple threads. This can be achieved in Qt Creator by adding the parameter _"-j"_ in _Projects_ in the bar on the left, then _Build Steps_, _Make_, _Make arguments_. However, note that building with this option might considerably increase memory usage.

- Also, it is advisable to disable QML debugging as leaving this enabled might pose a security risk. It can be disabled with a checkbox under _Projects_, _Build Steps_, _qmake_.

- If you are new to developing algorithms in AmoebotSim, the best way to start is looking at the example algorithm in the "alg" directory. Changes on the remaining code should not be required.

###User Interface###

- Move the visualization by holding the left mouse button and moving the mouse, zoom by using the scroll-wheel.
- Use the buttons in the bottom-right corner to control the simulation.
- The following keyboard shortcuts are available (the shortcuts are given for Windows; on OS X replace _Ctrl_ with _Cmd_):
    - __Ctrl + B__: Hide / Show buttons in the bottom-right corner (nice for presentations).
    - __Ctrl + E__: Start / Stop the simulation.
    - __Ctrl + D__: Execute a single round of the simulation.
    - __Ctrl + F__: Focus on center of mass.
- Clicking on a particle while holding Ctrl (Cmd) results in that particle executing a single turn, if possible.
- Clicking on an unoccupied place while holding Shift results in adding a new particle on this place. You can also hold the left mousebutton and "draw" particles along the moved cursor.
  When setting the connectivity check via __setCheckConnectivity(true)__ you can decide whether the new particle must be connected or not. By default the check is activated and new particles must be connected.
- A command-line can be opened by pressing the return-key. The available commands are the methods of the _ScriptInterface_ class in _script/scriptinterface.h_.
    - __round()__: Executes one round in the simulation
    - __runScript(path)__: Executes the scriptfile found in __path__. It must be written in Javascript.
    - __writeToFile(path, text)__: Writes the given __text__ to the file stated in __path__.
    - __saveScreenshot(filepath)__: Saves a screenshot of the current window in __filepath__. If __filepath__ is not provided a screenshot will be generated in the build folder.
    - __isValid()__: Returns true when the algorithm is valid, otherwise false.
    - __isDisconnected()__: Returns true when the algorithm is disconnected, otherwise false.
    - __isTerminated()__: Returns true when the algorithm is terminated, otherwise false.
    - __isDeadlocked()__: Returns true when the algorithm is deadlocked, otherwise false.
    - __getNumParticles()__: Returns the amount of particles in the simulation.
    - __getNumNonStaticParticles()__: Returns the amount of non static particles in the simulation.
    - __getNumMovements()__: Returns the amount of already made movements by the particles in the simulation.
    - __getNumRounds()__: Returns the amount of finished rounds in the simulation.
    - __setCheckConnectivity(check)__: When __check__ is true newly created particles must be connected, otherwise they don't need to. Default value is true.
    - __setRoundDuration(ms)__: Sets the duration for a round to __ms__ milliseconds.
    - __moveCameraTo(nodeX, nodeY)__: Moves center of the camera to the __(nodeX, nodeY)__ coordinate in the node coordinate system.
    - __setZoom(factor)__: Sets the zoomfactor. The value ranges between 4 and 128.
    - __setResolution(width, height)__: Sets the resolution of the simulator. 
    - __setFullscreen()__: Runs the simulator in fullscreen mode.
    - __setWindowed()__: Runs the simulator in windowed mode. 

[1] http://qt-project.org/downloads

[2] http://bitbucket.org/gmyr/amoebotsim

[3] http://www.sourcetreeapp.com
