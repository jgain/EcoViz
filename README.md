# Ecoviz - Ecosystem Visualization Application

Ecoviz is a C++ application developed using the Qt6 framework for visualizing ecosystems. This README provides detailed instructions on setting up the development environment on both Windows and Ubuntu, configuring the necessary dependencies, and running the application. Additionally, it includes comprehensive steps for rendering scenes using Mitsuba with Python.

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Installation](#installation)
   - [Ubuntu Installation](#ubuntu-installation)
   - [Windows Installation](#windows-installation)
3. [Building and Running Ecoviz](#building-and-running-ecoviz)
   - [Ubuntu](#ubuntu)
   - [Windows](#windows)
4. [Rendering with Mitsuba](#rendering-with-mitsuba)
5. [Project Structure](#project-structure)
6. [Description](#description)

## Prerequisites

Before you can run Ecoviz, ensure that you have the following tools and libraries installed:

- **Qt6 (including QtCharts)**: Required for the user interface and charts.
- **Boost Libraries**: Specifically, the `boost` and `boost_serialization-vc143` (for Visual Studio 2022) libraries.
- **GLM**: A header-only C++ library for graphics software.
- **SQLite3**: Required for database management.
- **Mitsuba**: Required for rendering scenes.
- **Python >= 3.8**: Required for running the Mitsuba rendering script.

## Installation

### Ubuntu Installation

Follow these steps to set up Ecoviz on Ubuntu:

1. **Install Required Packages**:
   - Open a terminal and run the following command to install the necessary packages:
     ```sh
     sudo apt-get install cmake automake pkg-config libglm-dev qtbase5-dev libqt5charts5-dev libboost-all-dev libglew-dev qtcreator sqlite3 sqlitebrowser libsqlite3-dev libxcb-cursor-dev
     ```

2. **Install a Specific Version of g++ (if required)**:
   - To install `g++-9` or another specific version, run:
     ```sh
     sudo add-apt-repository ppa:ubuntu-toolchain-r/test
     sudo apt install gcc-9 g++-9
     ```
   - Ensure that the correct version is set as default if multiple versions are installed.

### Windows Installation

1. **Install Qt6**:
   - Download the installer from the [official Qt website](https://www.qt.io/download) and follow the installation steps.
   - During installation, ensure that you include the **QtCharts** module.
   - After installation, you need to set the `QT6DIR` environment variable:
     1. Open the **Start Menu** and search for "Environment Variables".
     2. Click on "Edit the system environment variables".
     3. In the System Properties window, click on "Environment Variables...".
     4. Under "System variables", click "New..." and enter `QT6DIR` as the variable name.
     5. Set the variable value to the path of your Qt6 installation (e.g., `C:\Qt\6.5.0\msvc2019_64`).

2. **Set Up the Visual Studio Project**:
   - Open the Ecoviz Visual Studio solution (`.sln`) using **Visual Studio 2022**.
   - Open the **NuGet Package Manager** and install the following packages:
     - `boost`
     - `boost_serialization-vc143` (specifically for Visual Studio 2022)
     - `glm`
   - Set up the project properties:
     1. Right-click on the Ecoviz project in the Solution Explorer and select **Properties**.
     2. Go to **Configuration Properties > Debugging**.
     3. Set **Command Arguments** to `..\..\data\test2`.
     4. Set **Working Directory** to `$(ProjectDir)\out` for both **Debug** and **Release** configurations.

3. **Run the `copydll.bat` Script**:
   - Navigate to the `script` folder in the project directory.
   - Double-click the `copydll.bat` script or run it from the command prompt. This script will copy the necessary DLL files to the output directory where the application will run.

## Building and Running Ecoviz

### Ubuntu

1. **Compile the Application**:
   - If a build script is provided, you can use it. Otherwise, follow these steps:
     1. Create a directory for building the project:
        ```sh
        mkdir build
        cd build
        ```
     2. Run `cmake` to configure the build system:
        ```sh
        cmake ..
        ```
     3. Build the project:
        ```sh
        make
        ```

2. **Run the Application**:
   - After the build completes, the executable should be available in the `build/viz/` directory.
   - To run the application with test data, use:
     ```sh
     ./viz/ecoviz -prefix ecoviz ../../data/test2
     ```

### Windows

1. **Build the Solution**:
   - Open the Ecoviz solution in Visual Studio.
   - Ensure that the desired build configuration is selected (either **Debug** or **Release**).
   - Build the solution by selecting **Build > Build Solution** from the menu, or by pressing `Ctrl + Shift + B`.

2. **Run the Application**:
   - Once the build is successful, run the application by pressing **F5** or by selecting **Debug > Start Debugging** from the menu.

## Rendering with Mitsuba

To render 3D scenes from a JSON file using Mitsuba, follow these steps:

1. **Install Python Dependencies**:
   - Ensure that Python 3.8 or newer is installed on your system. You can check your Python version by running:
     ```sh
     python --version
     ```
   - Install the necessary Python packages using `pip`. Create a `requirements.txt` file with the following content:
     ```
     mitsuba
     numpy
     matplotlib
     ```
   - Run the following command to install the packages:
     ```sh
     pip install -r requirements.txt
     ```

2. **Run the `render_scene.py` Script**:
   - Use the `render_scene.py` script to render a scene from a JSON file. The JSON file should be generated by the **Export** feature in the Ecoviz application.
   - Run the script from the command line as follows:
     ```sh
     python render_scene.py path_to_json_file
     ```
   - Replace `path_to_json_file` with the path to the actual JSON file you want to render.

3. **Example Usage**:
   - To render a scene defined in `scene.json`, the command would be:
     ```sh
     python render_scene.py scene.json
     ```

## Project Structure

- **`script/copydll.bat`**: Script for copying necessary DLLs to the output directory.
- **`data/`**: Contains input data and configuration files used by the application.
- **`out/`**: The working directory where the application will execute.
- **`Ecoviz.sln`**: The Visual Studio solution file for the project.
- **`render_scene.py`**: The Python script for rendering scenes using Mitsuba.
- **`requirements.txt`**: List of Python dependencies for rendering.

## Description

Ecoviz is an advanced ecosystem visualization application designed to provide an interactive and immersive experience for exploring complex ecological data. Developed using the Qt6 framework, Ecoviz offers rich graphical interfaces and supports the generation of high-quality 3D visualizations through integration with the Mitsuba renderer.

