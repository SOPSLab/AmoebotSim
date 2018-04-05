# Welcome to AmoebotSim, a graphical simulator for the Amoebot model. #

The following instructions should get you started working with AmoebotSim; please read them carefully.

###Setup###

1. Download and install the latest version of Qt [1], __version 5.3 or newer is required__. In my experience, the online installer works best. On windows, please make sure to use the MinGW platform (there are checkboxes under _Qt 5.x.x_ and under _Tools_) and not MSVC. In general, the additional platforms (Android, iOS, ...) are not required and can be left out to decrease the size of the installation. When installing Qt 5.5 make sure you also install the qt script module (there's a checkbox under Qt 5.5).

2. Clone this repository using Git. The address of the repository depends on your BitBucket username and can be found in the top right corner of the project site on BitBucket [2]. If you want to use a GUI-Application for Git and you are on Windows or OS X , I can recommend Source Tree [3].

3. In the cloned repository, simply double-click the file _AmoebotSim.pss_ to open the project in Qt Creator, the IDE distributed with Qt.

4. Choose your build directories.

5. Run AmoebotSim by pressing the green _play_-button in the bottom left corner.

###Development###

- It is advisable to enable building using multiple threads. This can be achieved in Qt Creator by adding the parameter _"-j"_  to _Projects_ (left side-bar) > _Build Steps_ > _Make_ > _Make arguments_. Note that this might considerably increase memory usage.

- Leaving QML debugging enabled might pose a security risk. It can be disabled by unchecking the checkbox under _Projects_ > _Build Steps_ > _qmake_ > _Details_.

- If you are new to developing algorithms in AmoebotSim, _line.cpp_ in the "alg" directory serves as a self-contained example. For an example which includes token passing, see _tokendemo.cpp_.

###User Interface###

- Move the visualization by holding the left mouse button and moving the mouse, zoom by using the scroll-wheel.
- Use the buttons in the bottom-right corner to control the simulation.
- The following keyboard shortcuts are available (the shortcuts are given for Windows; on OS X replace _Ctrl_ with _Cmd_):
    - __Ctrl + B__: Hide / Show buttons in the bottom-right corner (nice for presentations).
    - __Ctrl + E__: Start / Stop the simulation.
    - __Ctrl + D__: Execute a single round of the simulation.
    - __Ctrl + F__: Focus on center of mass.
- Clicking on a particle while holding Ctrl (Cmd) results in that particle executing a single turn, if possible.
- Clicking on a particle while holding Alt prints its __inspectionText()__ results, if it is implemented. This provides easier runtime debugging.
- A command-line can be opened by pressing the return-key. The available commands are the methods of the _ScriptInterface_ class in _script/scriptinterface.h_.
    - __log(msg, error)__: Emits the message __msg__ to the console, and can be denoted as an error message by setting __error__ to true.
    - __runScript(path)__: Executes the script found at __path__. It must be written in Javascript.
    - __writeToFile(path, text)__: Writes the given __text__ to the file at __path__.
    - __round()__: Executes one round of the simulation (see __Ctrl + D__ above).
    - __runUntilTermination()__: Runs the given simulation until the system's __hasTerminated()__ function signals true.
    - __getNumParticles()__: Returns the number of particles in the system.
    - __getNumMovements()__: Returns the current total number of expansions and contractions executed by the particles in the simulation.
    - __getNumRounds()__: Returns the current number of asynchronous rounds completed in the simulation.
    - __setRoundDuration(ms)__: Sets the duration of a round of simulation to __ms__ milliseconds. Setting __ms__ to 0 will push the simulation as fast as possible.
    - __focusOn(x, y)__: Centers the camera on the coordinate __(x,y)__.
    - __setZoom(factor)__: Sets the zoom to __factor__, which must be between 4 and 128.
    - __saveScreenshot(path)__: Saves a screenshot of the current window to __path__. If __path__ is not provided, a screenshot will be generated in the build folder.
    - __filmSimulation(path, limit)__: Runs up to __limit__ iterations of the current simulation and saves a screenshot to __path__ after each step.

###Video capturing###

AmoebotSim provides indirect video export by capturing a series of frames which can be stitched together into a video. To do so, first start AmoebotSim and set up your algorithm. The frame series can be created in one of two ways: by a native command, or by script. The native command is __filmsimulation__, described above. To use a customizable script, copy the following script to a .js file and save it somewhere (e.g., `C:/script.js`).

```
    function pad(number, length)
    {
        var str = '' + number;
        while (str.length < length) {
            str = '0' + str;
        }

        return str;
    }

    i = 0
    while(isValid())
    {
        saveScreenshot(pad(i, 6) + ".png")
        round()
        i = i + 1
    }
```

Run the script using the __runScript__ command in AmoebotSim's command line (e.g. `runScript("C:/script.js")`). This will take a screenshot of the simulator window at each round and advance the algorithm until it finishes. All screenshots will be saved in the build folder. You can then create a video out of these pictures.

One method is using the ffmpeg library. When installed properly you only need to open a console, go to the directory with the screenshots and execute the following command:
```
ffmpeg -framerate 90 -pattern_type glob -i '*.png' -c:v libx264 -pix_fmt yuv420p -r 30 out.mov
```

Another method is using virtualdub [4]. Just open the application, drag the first screenshot in and then select all the other screenshots. While holding Ctrl (Cmd) drag all the other screenshots also into the application.
Under __Video__ you select the framerate and adjust the compression settings. Afterwards you can export the file via File -> Save as AVI.

[1] http://qt-project.org/downloads

[2] http://bitbucket.org/gmyr/amoebotsim

[3] http://www.sourcetreeapp.com

[4] http://www.virtualdub.org
