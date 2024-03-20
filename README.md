# HabiCAT3D (Habitat Complexity Analysis Tool 3D)

- **Figure 1:** App icon?

- **Figure 2:** Heatmap of rugosity values on a complex 3D model.(Landscape type?)

## Overview
This open-source software implements novel algorithms for generating multi-scale complexity metrics maps(like rugosity, fractal dimension and others) for complex 3D habitat models.
It extends traditional complexity metrics calculations to fully capture the three-dimensional complexity of real-world environments.
Designed for ecologists and researchers, this tool facilitates the analysis, export and visualization of 3D habitat models, particularly coral reefs, forests, and other complex structures.

## Features
- **Multi-Scale Maps:** Generates detailed maps over varying scales to capture the intricate details of complex models.
- **Optimized Reference Planes:** Introduces a rugosity-minimizing technique for accurate reference plane selection, improving calculation accuracy.
- **Fractal Dimension Calculations:** Extends fractal dimension metrics for 3D models, providing a deeper insight into structural complexity.
- **Visualization Tools:** Includes heatmap visualization and interactive histograms to explore and analyze rugosity and fractal dimension distributions.

## How To Get
You can download compiled version from .... (some realease)
Or compile it on your own.

## Compilation:
```bash
# Initialize a new Git repository
git init

# Add the remote repository
git remote add origin https://github.com/Azzinoth/HabiCAT3D

git fetch --all --prune

# Pull the contents of the remote repository
git pull origin master

# Initialize and update submodules
git submodule update --init --recursive

# Generate the build files using CMake
# CMake should be added to a PATH
# if not then use CMAKE GUI
cmake CMakeLists.txt
```

## How To Use
You can use GUI or command-line arguments....

## License
license.
