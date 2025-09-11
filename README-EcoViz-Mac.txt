MAC INSTALLATION:
-----------------

Install HomeBrew if you haven't already (https://brew.sh).

Intel processors:

with M Processors:
brew install gcc-11 

Create a symbolic link from gcc-11 and g++-11, or whichever version you have just installed, to gcc and g++ to override the default use of clang

Something like:
cd /opt/homebrew/bin/
ln -s gcc-11 gcc
ln -s g++-11 g++

brew install cmake qt5 mesa-glu freeglut glew libomp

You might also need some of the following:
automake pkg-config libglm-dev qtbase5-dev libqt5charts5-dev libboost-all-dev libglew-dev qtcreator sqlite3 sqlitebrowser libsqlite3-dev libxcb-cursor-dev

Compiling and Executing
-----------------------

There is a build script that you can run from the ecoviz subdirectory called buildecovizapple.sh. 

The executable is ./viz/ecoviz. The system must be run from the build directory because there are some relative paths.

To run the visualizer with test data execute: ./viz/ecoviz -prefix ecoviz ../../data/test2

This will read an an elevation map (ecoviz.elv) and the set of simulation files (ecoviz0.pdb, ecoviz1.pdb...)  The default simply reads 
in a set of these that is hardcoded at present). Rather than using these text files as input, you should generate binary file equivalents and copy
these to test2.  

NOTE: you can set different elevation/terrain files and simulation files for the left and right windows by using -lprefix and -rprefix to set
different  base file names for the left and right windows. If you use -prefix then the same files are used for both the left and right displays. 

Generating binary input files from text files (preferred):

There is an executable file - ecosimtobin - in the 'tools' sub-directory which can be run to translate text versions of elevation files (.elv extension) to binary (.elvb extension). The program can also translate the cohort simulation files (.pdb format) to their binary equivalents (.pdbb). 
The binary files load faster and take up less space on disk. For the cohort files, they should have a common basename and an integer sequence number (starting at 0) e.g. ecoviz0.pdb, ecoviz1.pdb.  To see how to invoke the converter, run it with no command line arguments. 


GUI
---
The visualizer is divided horizontally into two scenes (these happen at the moment to be identical but that need not necessarily be the case). Vertically there are four panels:
1) The transect view at the top, which is initially grey because no transect has been selected. Once a transect has been selected (see below) the right mouse button can be used to pan and the middle scroll wheel to zoom in and out.
2) Next, a 3D scene view. This can be rotated by holding down the right mouse button and moving the cursor. Zooming is with the mouse wheel. Double click with the right mouse button to change the focal point on the terrain. In order to select a transect click using <ctrl><leftmoustbutton> on two points of the terrain. These determine the start and end point of the transect, which will now appear in the transect window. To widen and narrow the transect use <ctl><mousewheel>. 
3) Below this is the timeline media bar with controls for backtracking a time-step, play/pause, and advancing a time-step.
4) The final window shows a graph with a bar that corresponds to the current timestep. There are no direct interactions possible in this window at the moment.
5) The inset mini-map can be used to select a region from the larger domain. The current selection can be translated by left clicking and dragging the selection region. A new region can be defined by right clicking and dragging. The new selection are applied once the mouse button is released. 

The are also two view panels accessible from the view menu: the first controls the terrain viewing and the second controls the display of plants, such as selecting which species are currently displayed. I suggest ignoring these for the moment, except perhaps to experiment with the smoothing level in the plant view.


