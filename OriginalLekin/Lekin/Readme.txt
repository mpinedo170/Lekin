LEKIN (June 1998)
====================

LEKIN is an academic scheduling system.  Its main purpose is to introduce
the students to scheduling theory and applications.  However, the system's
extensibility allows (and encourages) to use it as an algorithm development
tool.

LEKIN supports 6 basic environments: single m/c, parallel m/c, flow shop,
flexible flow shop, job shop, and flexible job shop.

Release version notes
========================

This is a fully functional version, with a complete online manual.  However,
a few minor bugs may have escaped our extensive testing.  Please contact the
developers (see below) if you find any.

Sample data
==============
The system includes several sets of sample files.  Their names are
self-explanatory.  To load a sample,
   (0. Start the program.)
    1. Select "Open an existing framework" in the Main Menu.
    2. Locate the desired .mch file in the file selection dialog box.  The
       appropriate Job and Sequence files will be opened automatically.

System operation
===================

To generate a new schedule,
   (0. Load sample files.)
    1. Select "Schedule" menu.
    2a. Select either a sequencing rule in the "Rule" submenu...
    2b. ...or an advanced heuristic in the "Heuristic" submenu.
    3. Activate the Sequence or Gantt Chart window to check the result.

To manually modify a schedule,
   (0. Generate a schedule.)
    1. Activate the Gantt Chart window.
    2. Each box on the Gantt Chart represents the operation under
       processing.  Double-click on a box to see the operation
       details.
    3. Drag the operation and Drop it on the new location to modify the
       sequence.  The schedule will be recomputed.
Note: Use toolbar buttons to rescale the Gantt Chart.

To compare various schedules,
    1. Generate several schedules.
    2. Select Tools->Objectives to generate the graph.
    3. When you click on a schedule name on this graph, the clicked schedule
       becomes "current".  The Sequence and the Gantt Chart windows always
       show the "current" schedule.
    4. If you are not satisfied with the looks of the graph,
      4a. double-click on a legend entry corresponding to one of the schedules;
      4b. change its color and name if necessary;
      4c. press Ok;
      4d. if necessary, repeat steps 4a through 4c for other schedules;
      4e. uncheck the schedules you don't want to see on the graph/chart.

To get further help,
    1a. Press F1, or 
    1b. Click the "Help" (the question-mark) button on the main toolbar.

Program support
==================
With questions / comments / bug reports / compliments, you are always welcome to
contact:

- the superviser of the project -- Michael Pinedo (mpinedo@stern.nyu.edu)
- the chief designer/programmer -- Andrew Feldman (abfeld@optonline.net)

                                               Copyright 1998.
