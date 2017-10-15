Cloud Point README
==================


Introduction:
-------------

This small poject got me my first job interview outside of academia.
I'm posting it here as is, with minimum modification necessary to respect the confidentiality of the hiring company.

The objective of this project is, given data from a single video frame, to identify players in a basketball game, compute
their positions on the field and classify them into players belonging to two teams (TeamA and TeamB) and the Referees.


General Code Description:
-------------------------

This code has been developed and tested on a machine running ubuntu 16.04,
but should work on any linux distribution with gcc instaled.

### Getting the code:

This code is available on GitHub at: <br>
<a href="https://github.com/remizaidan/PointCloud">
https://github.com/remizaidan/PointCloud
</a>

To download from the command-line: <br>
> git clone https://github.com/remizaidan/PointCloud

### Dependencies:
- ROOT Data Analysis Framework (version 6.08). Documentation and download are avaiable here: \n
<a href="https://root.cern.ch">https://root.cern.ch</a>


### Doxygen documentation:
<a href="https://remizaidan.github.io/PointCloud">
https://remizaidan.github.io/PointCloud
</a>

### Code structure:

The code is structured into the following sub-directories:
- <b>include</b> Contains all header files.
- <b>src</b> Contains all source files that are to be compiled into static libraries.
- <b>utils</b> Contains all source files that are to be compiled into executables.
- <b>doc</b> Contains documentation files.
- <b>share</b> Contains miscallaneous files such as inputs to the code.

After comiling, the following directories may be created:
- <b>bin</b> Contains all executables.
- <b>lib</b> Contains project libraries (shared or static).
- <b>.objs</b> Contains static libraries corresponding to source files in the src folder.
- <b>.deps</b> Contains dependency files.

After running, the folowing directories may be created:
- <b>outputs</b> Contains training xml files

### Running the code:

Compile:
> make -j<N>

Run:
> ./bin/pointCloud.exe [options]

For the list of available options see @ref CloudPoints#parseCommandLine or run:
> ./bin/pointCloud.exe -h


### Other compiling options:

Compile into a shared library:
> make -j<N> shared

Compile into a static library:
> make -j<N> static

Generate local doxygen documentation:
> make doc
