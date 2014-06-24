# Welcome to AmoebotSim, a graphical simulator for the Amoebot model. #

The following instructions should get you started working with AmoebotSim; please read them carefully.

###Setup###

1. Download and install the latest version of Qt [1], __version 5.3 or newer is required__. In my experience, the online installer works best. On windows, please make sure to use the MinGW platform (there are checkboxes under _Qt 5.x.x_ and under _Tools_) and not MSVC. In general, the additional platforms (Android, iOS, ...) are not required and can be left out to decrease the size of the installation.

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
- A command-line can be opened by pressing the return-key. The available commands are the methods of the _ScriptInterface_ class in _script/scriptinterface.h_.
- The following keyboard shortcuts are available (the shortcuts are given for Windows; on OS X replace _Ctrl_ with _Cmd_):
    - __Ctrl + B__: Hide / Show buttons in the bottom-right corner (nice for presentations).
    - __Ctrl + E__: Start / Stop the simulation.
    - __Ctrl + D__: Execute a single round of the simulation.

[1] http://qt-project.org/downloads

[2] http://bitbucket.org/gmyr/amoebotsim

[3] http://www.sourcetreeapp.com
