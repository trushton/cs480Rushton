A simple example of matrix use in OpenGL
========================================

Program will ask for vertex and fragment shader filenames, for this assignment the corresponding files are vertex.txt and fragment.txt

Input Controls
--------------

*bring up menu options using right click*

*press 'r' or left click to reverse rotation*

*press 't' or middle click to reverse translation*

*press 'q' or select 'Quit' from menu to quit*

*screen stretching is functional*


Building This Example
---------------------

*This example requires GLM*
*On ubuntu it can be installed with this command*

>$ sudo apt-get install libglm-dev

*On a Mac you can install GLM with this command(using homebrew)*
>$ brew install glm

To build this example just 

>$ cd build
>$ make

*If you are using a Mac you will need to edit the makefile in the build directory*

The excutable will be put in bin

Additional Notes For OSX Users
------------------------------

Ensure that the latest version of the Developer Tools is installed.
