HabiCAT3D
Habitat Complexity Analysis Tool 3D
=====

# Initialize a new Git repository
git init

# Add the remote repository
git remote add origin https://github.com/Azzinoth/HabiCAT3D<br />
git fetch --all --prune<br />
git pull origin master<br />

# Initialize and update submodules
git submodule update --init --recursive

# Generate the build files using CMake
# CMake should be added to a PATH
# if not then use CMAKE GUI
cmake CMakeLists.txt
