# Ecoviz - Ecosystem Visualization Application

Ecoviz is a C++ application developed using the Qt6 framework for visualizing ecosystems. This README provides detailed instructions on setting up the development environment on both Windows and Ubuntu, configuring the necessary dependencies, and running the application. It also includes comprehensive steps for rendering scenes using Mitsuba with Python.

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

To set up Ecoviz on Ubuntu, follow these steps:

1. **Install Required Packages**:
   ```bash
   sudo apt-get install cmake automake pkg-config libglm-dev qtbase5-dev libqt5charts5-dev libboost-all-dev libglew-dev qtcreator sqlite3 sqlitebrowser libsqlite3-dev libxcb-cursor-dev
   ```

2. **Install a Specific Version of g++ (if required)**:
   ```bash
   sudo add-apt-repository ppa:ubuntu-toolchain-r/test
   sudo apt install gcc-9 g++-9
   ```

### Windows Installation

1. **Install Qt6**:
   - Download Qt6 from the [official Qt website](https://www.qt.io/download).
   - Ensure the **QtCharts** module is selected during installation.
   - Set the `QT6DIR` environment variable:
     1. Open the **Start Menu**, search for "Environment Variables".
     2. Click on "Edit the system environment variables".
     3. Under "System variables", click "New..." and set the name as `QT6DIR` and the value as your Qt6 installation path (e.g., `C:\Qt\6.5.0\msvc2019_64`).

2. **Set Up Visual Studio Project**:
   - Open the Ecoviz `.sln` file in **Visual Studio 2022**.
   - Install the following NuGet packages:
     - `boost`
     - `boost_serialization-vc143`
     - `glm`
   - Configure the project properties:
     1. Right-click on the Ecoviz project in the Solution Explorer and select **Properties**.
     2. Under **Configuration Properties > Debugging**:
        - Set **Command Arguments** to `..\..\data\test`.
        - Set **Working Directory** to `$(ProjectDir)\out` for both **Debug** and **Release** configurations.

3. **Run the `copydll.bat` Script**:
   - Go to the `script` directory.
   - Run the `copydll.bat` script to copy necessary DLLs to the output directory.

## Building and Running Ecoviz

### Ubuntu

1. **Compile the Application**:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

2. **Run the Application**:
   ```bash
   ./viz/ecoviz -prefix ecoviz ../../data/test2
   ```

### Windows

1. **Build the Solution**:
   - Select the desired build configuration (Debug/Release).
   - Build the solution using **Ctrl + Shift + B**.

2. **Run the Application**:
   - Run the application by pressing **F5** or selecting **Debug > Start Debugging**.

## Rendering with Mitsuba

To render 3D scenes from a JSON file using Mitsuba:

1. **Install Python Dependencies**:
   - Ensure Python 3.8 or newer is installed:
     ```bash
     python --version
     ```
   - Install dependencies from `requirements.txt`:
     ```bash
     pip install -r requirements.txt
     ```
   - The `requirements.txt` should include:
     ```
     mitsuba
     numpy
     matplotlib
     ```

2. **Run the `render_scene.py` Script**:
   ```bash
   python render_scene.py path_to_json_file
   ```

   - Replace `path_to_json_file` with your JSON file path.

## Project Structure

- **`script/copydll.bat`**: Copies necessary DLLs to the output directory.
- **`data/`**: Input data and configuration files.
- **`out/`**: Working directory for the application.
- **`Ecoviz.sln`**: Visual Studio solution file.
- **`render_scene.py`**: Python script for rendering scenes using Mitsuba.
- **`requirements.txt`**: Python dependencies for rendering.

## Description

Ecoviz is an advanced ecosystem visualization application designed for interactive exploration of complex ecological data. Developed with the Qt6 framework, it offers rich graphical interfaces and supports 3D visualization through Mitsuba.
