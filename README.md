<img src="https://dalihub.github.io/images/DaliLogo320x200.png">

# Table of Contents

   * [Build Instructions](#build-instructions)
      * [Building for Ubuntu desktop](#1-building-for-ubuntu-desktop)
         * [Minimum Requirements](#minimum-requirements)
         * [Building the Repository](#building-the-repository)
   * [Running the tests](#running-the-tests)
   * [Creating a visual test](#creating-a-visual-test)

# Build Instructions

## Building for Ubuntu desktop

### Requirements

 - Ubuntu 20.04 or later
 - GCC version 6
 - OpenCV version 2.4.9 or above
 - Xvfb
 - Magick++
 - Environment created using dali_env script in dali-core repository
 - This environment should have the DALi libraries installed

DALi requires a compiler supporting C++11 features.
Ubuntu 16.04 is the first version to offer this by default (GCC v5.4.0).

GCC version 6 is recommended since it has fixes for issues in version 5
e.g. it avoids spurious 'defined but not used' warnings in header files.

OpenCV version 2.4.9 is the default version installed in Ubuntu 16.04

### Building the Repository

To build the repository:

         $ ./build.sh

For debug builds:

         $ ./build.sh --debug

# Running the tests

To run all the tests:

         $ ./execute.sh

This will run each test on it's own X server.

# Running individual tests

The tests are installed into dali-env, and can be run directly.
In this case, the test is run on the desktop's X server.

# Creating a visual test

 - Make a directory in the "visual-tests" directory. Only one visual test will be created per directory.
 - The executable installed will have a ".test" appended to it, e.g. a "my-first-visual-test" directory produces "my-first-visual-test.test".
 - Add all source files for the required visual test in this directory.
 - No changes are required to the make system as long as the above is followed, your visual test will be automatically built & installed.
