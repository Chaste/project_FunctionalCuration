# Example CMake build configuration file for a Chaste user project.
# See also https://chaste.cs.ox.ac.uk/trac/wiki/ChasteGuides/CmakeBuildGuide for full documentation.

# XSD < 4.0.0 generates code with std::auto_ptr.  This is deprecated from C++11 (in favour of new standard smart
# pointers). This code turns off the deprecation warnings so that Chaste compiles.
# TODO: Once we drop Chaste support for XSD 3.3, this should be removed in favour of changing auto_ptr to unique_ptr
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# You can set which Chaste components (or other projects) your project depends on by editing the
# find_package() call for Chaste.  E.g.
#   find_package(Chaste COMPONENTS cell_based)
# for a project just using the cell_based component (and its dependencies), or
#   find_package(Chaste COMPONENTS heart lung)
# for a project combining heart & lung simulations.
#
# Note that the order in which components are specified does not matter.

# Functional curation requires the heart code from Chaste
find_package(Chaste COMPONENTS heart)

# Alternatively, to specify a Chaste installation directory use a line like that below.
# This is needed if your project is not contained in the projects folder within a Chaste source tree.
#find_package(Chaste COMPONENTS heart PATHS /path/to/chaste-install NO_DEFAULT_PATH)

# Change the project name in the line below to match the folder this file is in,
# i.e. the name of your project.
chaste_do_project(FunctionalCuration)
