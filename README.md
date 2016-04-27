# ColonyCounter

TODO:
* colonycounter lib in anderen ordner
    * ein kommandozeilen programm machen
    * selber compilieren
* lib muss dann fuer qt und command line compiliert werden und wird dann verlinkt dahin include "../lib/colonycounter.hpp"

ColonyCounter is currently in a very early stage of development. It is used to count bacterial colonies on petri dishes. The project is divided into a library that contains the code that handles to counting and processing of the images, the GUI versions which just adds a graphical interface on top of the library and a command line interface.

## Installation

ColonyCounter depends on [opencv and opencv_contrib] (http://docs.opencv.org/master/tutorial_table_of_content_introduction.html).
The GUI also depends on Qt.

The GUI version of the ColonyCounter links against the Qt Framework which is licenced under the LGPL licence.
