#!../../bin/linux-x86_64/selectionTest

# $File: //ASP/tec/epics/selection/trunk/iocBoot/iocselectionTest/st.cmd $
# $Revision: #1 $
# $DateTime: 2023/01/10 23:12:38 $
# Last checked in by: $Author: starritt $
#

#- You may have to change selectionTest to something else
#- everywhere it appears in this file

< envPaths
 
cd "${TOP}"
 
## Register all support components
dbLoadDatabase "dbd/selectionTest.dbd"
selectionTest_registerRecordDeviceDriver pdbbase

## Autosave set-up
#
< ${AUTOSAVESETUP}/crapi/save_restore.cmd

## Restore autosave values
#
set_pass0_restoreFile ("example_selection.sav", "PREFIX=TEST")
set_pass1_restoreFile ("example_selection.sav", "PREFIX=TEST")

## Load record instances
#
dbLoadRecords("db/example_selection.template", "PREFIX=TEST, NELM=24")
 
cd "${TOP}/iocBoot/${IOC}"
iocInit
  
dbl
  
## Autosave monitor set-up.
#
create_monitor_set ("example_selection.req", 30, "PREFIX=TEST")

# end
