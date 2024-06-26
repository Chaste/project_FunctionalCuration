= Functional Curation Project =

This is a bolt-on project to Chaste accompanying the paper:

J. Cooper, G.R. Mirams, S.A. Niederer.
High throughput functional curation of cellular electrophysiology models.
Progress in Biophysics & Molecular Biology, 107, 11-20, (2011). 
doi: 10.1016/j.pbiomolbio.2011.06.003.

A pre-print is available at:
http://www.cs.ox.ac.uk/chaste/publications/2011-Cooper-Functional%20Curation.pdf

This code is maintained to be compatible with the latest Chaste trunk revision, as
swiftly as possible.  Occasional releases are made to correspond with Chaste releases,
and are made available from the main Chaste download page.

== Installation ==

This project requires the Chaste source tree to be installed in order to be usable.
Install the Chaste source following its installation instructions.  You will need
write access to the Chaste source tree in order to use this project.  Note that it
is advisable to install the source tree on local disk, rather than an NFS mount:
we have experienced occasional problems in the latter case, depending on Linux
distribution and version.  Compiling Chaste is also fairly disk intensive!  Note
also that you will require those Chaste dependencies marked as "cardiac only", and
that this project requires at least version 1.39 of the Boost libraries.

Having installed (and ideally tested!) the Chaste source tree, unpack this project
as <Chaste>/projects/FunctionalCuration.  It is crucial to match the folder name
and location, or the project will not work.

The python implementation of Functional Curation needs a few extra dependencies:
sudo apt-get install python-numpy python-scipy python-tables python-matplotlib cython

== Usage ==

Source code for the project is contained in the `src` folder, and tests of its
functionality in `tests`.  Annotated CellML files suitable for use with the framework
are in `cellml`.  Some additional interesting locations are:
 * `tests/protocols`  contains example protocols
 * `src/proto/library`  contains protocol libraries with functions available to all protocols
 * `src/proto/parsing/protocol.rnc`  is a schema for the protocol XML language

You can run the simulations from the paper using:
  scons b=GccOptNative ts=projects/FunctionalCuration/test/TestFunctionalCurationPaper.hpp

Output will appear in /tmp/$USER/testoutput/FunctionalCuration by default (unless the
environment variable CHASTE_TEST_OUTPUT is set to point elsewhere; it defaults to
/tmp/$USER/testoutput.  This location should probably also be on local disk).  The
test should pass (printing an 'OK' line towards the end) to show that the protocol
results generated are consistent with those in the paper.*

Note that some warnings will be printed at the end of the test output.  The following
are expected, for model/protocol combinations where we cannot run the protocol to
completion.  These were not included in the paper.
 * beeler_reuter_model_1977 under ICaL protocol: no (explicit) extracellular calcium
 * luo_rudy_1991 under ICaL protocol: no extracellular calcium
 * maleckar_model_2009 under ICaL protocol: CVODE fails to solve system with CV_CONV_FAILURE
 * nygren_atrial_model_1998 under S1S2 protocol: post-processing fails (irregular index)
   due to the model self-stimulating
 * nygren_atrial_model_1998 under ICaL protocol: CVODE fails to solve system with CV_CONV_FAILURE


There are also tests covering the lower-level functionality available for use by
protocols.  Run all the default tests with:
  scons b=GccOptNative projects/FunctionalCuration


To build an executable that can run a single protocol on a single model, do:
  scons cl=1 exe=1 projects/FunctionalCuration/apps

The executable will appear at projects/FunctionalCuration/apps/src/FunctionalCuration.
You'll need the environment variable LD_LIBRARY_PATH set up as described in the Chaste
documentation in order to run it, since it needs to find the Chaste libraries and their
dependencies.

== Contact details ==

We cannot guarantee support for this project, but will endeavour to respond to queries.
Contact information for the corresponding authors can be found via
http://www.cs.ox.ac.uk/chaste/theteam.html

== Footnotes ==

* The reference data included in this project do not all exactly match those used in the paper.
  For one model subsequent investigation has revealed the results in the paper to be slightly
  incorrect.  Tightening of tolerances and refinement of sampling intervals have also
  improved the quality of the results for several models, especially under the ICaL protocol,
  beyond the default comparison tolerance of 0.5%.  Another model (Faber-Rudy) shows extremely 
  high sensitivity to code generation settings in the peak transmembrane potential, which in 
  turn affects the APD90 calculation in the S1-S2 protocol.
  