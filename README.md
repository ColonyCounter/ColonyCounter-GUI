# ColonyCounter

ColonyCounter is currently in a very early stage of development. It is used to count bacterial colonies on petri dishes. The project is divided into a library that contains the code that handles to counting and processing of the images, the GUI versions which just adds a graphical interface on top of the library.

# Open-Source

The software is published under the [Apache License 2.0](https://github.com/ColonyCounter/ColonyCounter/blob/master/LICENSE).

The GUI version of the ColonyCounter links against the Qt Framework which is licenced under the LGPL licence.

The project was inspired by the [CELLCOUNTER from Nghia Ho](http://nghiaho.com/?page_id=1011).

This is currently a hobby project. We do not have a professional IT background, so we are happy about every advice to improve the software.

# Installation

* Download an install [opencv](https://github.com/Itseez/opencv) and [opencv_contrib](https://github.com/Itseez/opencv_contrib)
    * [OpenCV installation tutorial](docs.opencv.org/3.1.0/df/d65/tutorial_table_of_content_introduction.html)
* Download and install [Qt Open Source](https://www.qt.io/download-open-source/) and [Qt Creator](https://www.qt.io/ide/)
    * On Linux you can use the official repositories of your OS to install it
* Download [ColonyCounter-GUI](https://github.com/ColonyCounter)
* Open the *.pro* file with Qt Creator and build it.
    * To use the cascade classifier you need to copy the data folder inside the build directory

# Roadmap

1. A
    * Train the cascade classifier
    * Improve splitting process
    * Find out optimal parameters

2. B
    * Make pyrMeanShift optional
    * Create a wiki
        * Explain different values that can be set in the settings
        * Explain the installation
        * Add site with known bugs and more detailed roadmap
    * Remove bugs:
        * Fix mouse arrow offset
    * Speed improvements
    * Split the ColonyCounter
        * library
        * GUI
        * cmd line tool

3. C
    * Create cmd line tool
    * Save/Load settings to/from file
