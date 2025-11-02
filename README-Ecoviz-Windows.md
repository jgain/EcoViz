# Ecoviz - Ecosystem Visualization Application

Ecoviz is a C++ application developed using the Qt6 framework for visualizing ecosystems. This guide provides instructions on how to set up the development environment on Windows, configure the necessary dependencies, and run the application using Visual Studio 2022.

## Prerequisites

Before you can run Ecoviz, ensure that you have the following tools and libraries installed:

- **Visual Studio 2022**: Required for building the project.
- **Qt6 (including QtCharts)**: The application is built using Qt6. Make sure Qt6 is installed and properly configured.
- **Boost Libraries**: Specifically, the `boost` and `boost_serialization` libraries are needed.
- **GLM**: A header-only C++ library for graphics software.

## Setting Up the Development Environment

### 1. Install Qt6

1. **Download and Install Qt6**:
   - Visit the [official Qt website](https://www.qt.io/download) to download the Qt6 installer.
   - Run the installer and follow the on-screen instructions.
     - **Note**: Ensure that you install the `msvc` (**not** `mingw`) version of Qt6, which can be checked under **Customize > Qt 6.X.X** during installation.
   
2. **Include QtCharts Module**:
   - During the installation process, ensure that you select the **QtCharts** module to be installed. This module is essential for rendering charts and visualizations within Ecoviz.

3. **Set the `QT6DIR` Environment Variable**:
   - After installation, you need to set an environment variable to let Visual Studio know where Qt6 is installed.
   - **Steps to Set `QT6DIR`**:
     1. Open the **Start Menu** and search for "Environment Variables".
     2. Click on **"Edit the system environment variables"**.
     3. In the **System Properties** window, click on **"Environment Variables..."**.
     4. Under **"System variables"**, click **"New..."**.
     5. Enter `QT6DIR` as the **Variable name**.
     6. Enter the path to your Qt6 installation directory (e.g., `C:\Qt\6.5.0\msvc2019_64`) as the **Variable value**.
     7. Click **"OK"** to save.

### 2. Set Up the Visual Studio Project

1. **Open the Solution File**:
   - Launch **Visual Studio 2022**.
   - Open the Ecoviz solution file (`.sln`) by navigating to **File > Open > Project/Solution** and selecting the `.sln` file.

2. **Install Required NuGet Packages**:
   - **Boost**:
     - Right-click on the solution in the **Solution Explorer**.
     - Select **"Manage NuGet Packages for Solution..."**.
     - In the **Browse** tab, search for `boost` and install version `v1.85.0` for the Ecoviz project.
   
   - **Boost.Serialization**:
     - In the **NuGet Package Manager**, search for `boost_serialization-vc143` (specific to Visual Studio 2022) and install it for the Ecoviz project.
   
   - **GLM**:
     - Similarly, search for `glm` and install it for the Ecoviz project.
   
   - **Steps Summary**:
     1. Right-click on the **solution** and select **"Manage NuGet Packages for Solution..."**.
     2. For each package (`boost`, `boost_serialization-vc143`, `glm`), search and install them for the Ecoviz project.

### 3. Configure Project Properties

1. **Access Project Properties**:
   - In the **Solution Explorer**, right-click on the **Ecoviz project** and select **"Properties"**.

2. **Set Debugging Parameters**:
   - Navigate to **Configuration Properties > Debugging**.
   
   - **For Both Debug and Release Configurations**:
     - **Command Arguments**: Set to `..\..\data\test`
     - **Working Directory**: Set to `$(ProjectDir)\out`
   
   - **Steps**:
     1. At the top of the Properties window, ensure you are configuring both **Debug** and **Release** configurations.
     2. Under **Configuration Properties > Debugging**:
        - Enter `..\..\data\test` in the **Command Arguments** field.
        - Enter `$(ProjectDir)\out` in the **Working Directory** field.
     3. Repeat for each configuration if necessary.

### 4. Run the `copydll.bat` Script

1. **Locate the Script**:
   - Navigate to the `script` folder within the Ecoviz project directory.

2. **Execute the Script**:
   - **Steps**:
     1. Open **Command Prompt**.
     2. Change the directory to the `script` folder using:
        ```sh
        cd path\to\Ecoviz\script
        ```
     3. Run the batch script by typing:
        ```sh
        copydll.bat
        ```
   - This script copies the necessary DLLs to the output directory, ensuring that all required dynamic libraries are available when running the application.

## Building and Running Ecoviz

After completing the setup, follow these steps to build and run Ecoviz:

1. **Select Build Configuration**:
   - In Visual Studio, choose either **Debug** or **Release** from the configuration dropdown in the toolbar, depending on your needs.

2. **Build the Solution**:
   - Click on **Build > Build Solution** or press `Ctrl + Shift + B`.
   - Ensure that the build completes successfully without errors.

3. **Run the Application**:
   - Press **F5** or go to **Debug > Start Debugging**.
   - The Ecoviz application should launch, using the specified command arguments and working directory.

## Project Structure

- **`script/copydll.bat`**: A script to copy necessary DLLs to the output directory.
- **`data/`**: Contains input data and configuration files for the application.
- **`out/`**: The working directory where the application will run.
- **`Ecoviz.sln`**: The Visual Studio solution file.
- **Other Project Files**: Such as `.vcxproj` files and resource files.

## Description

Ecoviz is an ecosystem visualization application that allows users to visualize and interact with various ecosystem parameters. It leverages the Qt6 framework for a rich user interface and employs advanced rendering techniques to provide an immersive visualization experience. Users can explore different ecosystem scenarios, analyze data, and gain insights through intuitive visual representations.


