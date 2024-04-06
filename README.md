<div align="center">
	<img src="https://github.com/Azzinoth/HabiCAT3D/blob/media/HabiCAT3D_Logo.png" width=256 />
	<h1> HabiCAT 3D <br>Habitat Complexity Analysis Tool 3D </h1>
	<img src="https://github.com/Azzinoth/HabiCAT3D/actions/workflows/Build.yml/badge.svg" alt="Build Status">
	<img src="https://github.com/Azzinoth/HabiCAT3D/blob/media/Coral_With_Rugosity.png"/>
</div>

HabiCAT 3D is an open-source software that implements novel algorithms for generating multi-scale complexity metrics maps(like rugosity, fractal dimension, vector dispersion and others) for complex 3D habitat models.
It extends traditional complexity metrics calculations to fully capture the three-dimensional complexity of real-world environments.
Designed for ecologists and researchers, this tool facilitates the analysis, export and visualization of 3D habitat models, particularly coral reefs, and other complex structures.

## Features
- **Multi-Scale Maps:** Generates detailed maps over varying scales to capture the intricate details of complex models.
- **Complexity Metrics Available to Calculate:**
	- **Rugosity:**
		- **Average Normal(default):** The simplest and fastest way to calculate rugosity, but for more acurate results, we highly recommend using "Min Rugosity".
		- **Optimized Reference Planes(Min Rugosity):** A rugosity-minimizing technique for accurate reference plane selection, improving calculation accuracy.
		- **Unique Projected Area:** To correctly calculate rugosity on very complex models, users can opt to use this option. Currently, it is very slow, but we plan to improve its performance in the future.
	- **Fractal Dimension.**
	- **Vector Dispersion.**
	- **Triangle Area.**
	- **Triangle Density.**
	
- **Layers:** Our layer system allows users to compare results obtained using different metrics and settings. Each analysis run creates a new layer, which can be accessed through tabs at the top of the screen. Users can visually compare layers or generate heatmaps to highlight the differences between them.

![](https://github.com/Azzinoth/HabiCAT3D/blob/media/Layer_Tabs.png)

- **.RUG file format:** Users can save and load entire workspace using a custom (.RUG) format. RUG files save all layers alongside the model, decreasing model load times and eliminating the need to re-run calculations.

- **Histograms:** Interactive histograms provide deeper insight into the distribution of complexity metrics across the model. At a glance, it is possible to quickly evaluate the uniformity or concentration of complexity in a model.
	- **Histogram selection:** Histograms can be queried by mouse to calculate how much surface area falls within a complexity range. Regions corresponding to selected ranges are highlighted to aid correlating the histogram to the model:
<div align="center">
  <table>
    <tr>
      <td><img src="https://github.com/Azzinoth/HabiCAT3D/blob/media/Histograms.png" width=512></td>
      <td><img src="https://github.com/Azzinoth/HabiCAT3D/blob/media/Histogram_Selection.png" width=512></td>
    </tr>
  </table>
</div>

- **CLI Scripting:** The application supports a command-line interface (CLI) and script execution without a graphical user interface (GUI), allowing for streamlined integration into existing researchers' workflows.

- **Export Options:**
	- **Selection tool (Only in GUI Mode):** The user can query the layer value of a single triangle or the layer values within a specified radius of a point. If the 'Export to File' option is activated, then the values will be automatically saved to a text file.
	- **Screenshot (Only in GUI Mode):** The 'Take Screenshot' button generates a screenshot without the GUI and with a larger legend, making it suitable for use in figures.
	- **Whole Layer as Image:** Layer values will be projected onto a plane and converted to a `PNG` or `GeoTIFF` (color or `32-bit float` with raw values) image. The `32-bit float` raw export is essential for easy use of the application's calculation results in other applications.
		- **Cumulative Suboption:** In complex 3D models, multiple triangles may project to the same pixel when creating a 2D image. The cumulative option resolves this by accumulating the complexity metric along the projection axis, creating an "X-ray" effect that preserves information about high-complexity areas. This option is useful for models with overhangs and caves.
## Quick Start Guides

[Quick start guide for GUI mode.](https://github.com/Azzinoth/HabiCAT3D/tree/media/Quick%20Start%20Guide/Quick%20start%20guide(GUI).md)

[Quick start guide for CLI mode.](https://github.com/Azzinoth/HabiCAT3D/tree/media/Quick%20Start%20Guide/Quick%20start%20guide(CLI).md)

## How To Get
- **Download ready-to-use application:**
- **Download source code and compile:** Look for instructions below.

## How to compile
The compilation process was tested with Windows 10 (and 11) and Visual Studio 2022. To compile, you would need Git, CMake, and Visual Studio.
```bash
# Initialize a new Git repository
git init

# Add the remote repository
git remote add origin https://github.com/Azzinoth/HabiCAT3D

# Pull the contents of the remote repository
git pull origin master

# Initialize and update submodules
git submodule update --init --recursive

# Generate the build files using CMake
# CMake should be added to a PATH
# if not then use CMAKE GUI
cmake CMakeLists.txt
```
After running these commands, you should have a Visual Studio project that is ready to be compiled.

## Third Party Licenses

This project uses the following third-party libraries:

1) **Focal Engine**: This library is licensed under MIT License. The full license text can be found at [Focal Engine's GitHub repository](https://github.com/Azzinoth/FocalEngine/blob/master/LICENSE.md)
2) **CGAL**: Some parts of CGAL are available under the LGPL, whereas other parts are under the GPL. The full license text can be found at [CGAL's GitHub repository](https://github.com/CGAL/cgal/blob/master/Installation/LICENSE) or [CGAL's webpage](https://www.cgal.org/license.html).
3) **boost**: This library is licensed under Boost Software License. The full license text can be found at [boost's GitHub repository](https://github.com/boostorg/boost/blob/master/LICENSE_1_0.txt)
4) **Eigen**: This library is part of CGAL.
5) **GDAL**: This library is licensed under MIT License. The full license text can be found at [GDAL's GitHub repository](https://github.com/OSGeo/gdal?tab=License-1-ov-file#readme)
