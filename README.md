# Welcome to AmoebotSim, a graphical simulator for the Amoebot model. #

###Getting Started###

1. Download and install the latest version of Qt [1], version 5.3 or newer is required. In my experience, the online installer works best. On windows, please make sure to use the MinGW platform (there are checkboxes under "Qt 5.x.x" and under "Tools") and not MSVC. In general, the additional platforms (Android, iOS, ...) are not required and can be left out to decrease the size of the installation.

2. Clone this repository using Git. The address of the repository can be found in the top right corner of this page. If you want to use a GUI-Application for Git and you are on Windows or OS X , I can recommend Source Tree [2].

3. In the cloned repository, simply double-click the file "AmoebotSim.pss" to open the project in Qt Creator, the IDE distributed with Qt.

4. Choose your build directories.

5. Run AmoebotSim by pressing the green "play"-button in the bottom left corner.

###Additional Hints###

- If you are new to developing algorithms, the best way to start is looking at the example algorithm in the "alg" directory. This simple algorithm is extensively commented (in contrast to the remaining code). In principle, developing new algorithms only requires creating a new subclass of the Algorithm class and making it available in the application by introducing it in the script interface in the "script" directory. Changes on the remaining code should not be required.

- It is advisable to enable building using multiple threads. This can be achieved in Qt Creator by adding the parameter "-j" in "Projects" in the bar on the left, then "Build Steps", "Make", "Make arguments".

- Also, it is advisable to disable QML debugging as leaving this enabled might pose a security risk. It can be disabled with a checkbox under "Projects", "Build Steps", "qmake".

If you have any questions, please contact Robert Gmyr [3].


[1] http://qt-project.org/downloads

[2] http://www.sourcetreeapp.com

[3] gmyr@mail.upb.de
