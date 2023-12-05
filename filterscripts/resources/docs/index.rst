.. _filterscripts:

=============
filterscripts
=============

* Maintainer: Erik Blaufuss (blaufuss @ umd.edu)

.. toctree::
   :maxdepth: 3

   release_notes


Overview
--------

This is the OLD level 1 (L1) and level 2 (L2) processing project

It has been replaced by online_filterscripts and offline_filterscripts projects with the 2023 Physics run and beyond.

L1 : Pole processing
  Any processing that occurs at pole to select the events to be sent north.

L2 : North processing
  Any global (standard) processing that gets run on all data before being
  released to analyzers.
  
For simulation, L1 and L2 are run back-to-back to mimic normal data,
with a few changes to remove compression and space saving.


Usage
-----

Users wanting to do their own L2 processing should use the following
directions:

.. toctree::
   :maxdepth: 2
   
   ic86_2015
   ic86_2014
   ic86_2013
   ic86_2012
   ic86_2011


TFT Planning
------------

The :wiki:`Trigger Filter Transmission Board <Trigger_Filter_Transmission_Board>`
is in charge of changes to the detector as an interface between
analysis groups and operations. Changes to triggers or filters
go through annual review.

See the wiki pages:

| 2018 : https://drive.google.com/open?id=1a5LRRn8faCpTF0DVJtqk0XvQlQ-oF7ka
| 2016 : :wiki:`TFT_2016_Season_Planning`
| 2015 : :wiki:`TFT_2015_Season_Planning`
| 2014 : :wiki:`TFT_2014_Season_Planning`
| 2013 : :wiki:`TFT_2013_Season_Planning`
| 2012 : :wiki:`TFT_2012_Season_Planning`
| 2011 : :wiki:`TFT_2011_Season_Planning`

C++ Documentation
-----------------

:ref:`Generated doxygen for this project <filterscripts-cpp>`

