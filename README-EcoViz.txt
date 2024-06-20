UBUNTU INSTALLATION:
--------------------
These are requirements for Ubuntu. In each case, I've listed where to get the software 
for Ubuntu 18.10. For newer versions of Ubuntu you might not need all the PPAs.

CMake 2.8.7+, make, automake 1.9+, pkg-config, QT5.7+, Boost1.49+, SQLite3

sudo apt-get install cmake automake pkg-config libglm-dev qtbase5-dev libqt5charts5-dev libboost-all-dev libglew-dev qtcreator sqlite3 sqlitebrowser libsqlite3-dev

To install a particular version of g++ (e.g. g++8):

sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt install gcc-8 g++-8
then make sure to set the correct version in the build script (buildecoviz.sh)

NOTE: the latest version of g++ should work. 

Compiling and Executing
-----------------------

There is a build script that you can run from the ecoviz subdirectory called buildecoviz.sh. 

Alternatively, once all the requirements are running, create a subdirectory called build (or
anything starting with build - you can have multiple build directories), switch
into it, and run

cmake <options> ..

Some useful options
-DCMAKE_CXX_COMPILER=g++-4.8          (to force a specific compiler)
-DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebInfo  (to select build type, only choose one option here)
-DCMAKE_CXX_FLAGS_DEBUG="-gdwarf-3"   (helps older versions of GDB interpret debug symbols)
-DCMAKE_CXX_FLAGS_RELWITHDEBINFO="-O3 -DNDEBUG -g -gdwarf-3" (similar)
-DCMAKE_CXX_FLAGS="-fPIC" (if the compilation complains about position independent code)

Then run make to build. cmake options are sticky. 

The executable is ./viz/ecoviz. The system must be run from the build directory because there are some relative paths.

To run the visualizer with test data execute: ./viz/ecoviz -prefix ecoviz_ ../../data/test2

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


