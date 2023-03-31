# $File: //ASP/tec/epics/selection/trunk/selectionTestApp/src/Makefile $
# $Revision: #1 $
# $DateTime: 2023/01/10 23:12:38 $
# Last checked in by: $Author: starritt $
#

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = selectionTest
# selectionTest.dbd will be created and installed
DBD += selectionTest.dbd

# selectionTest.dbd will be made up from these files:
selectionTest_DBD += base.dbd

# Include dbd files from all support applications, and
# add all the support libraries needed by this IOC
#
# Autosave related dbd/libraries.
#
selectionTest_DBD  += asSupport.dbd
selectionTest_LIBS += autosave

# Concat record
#
selectionTest_DBD  += concatRecord.dbd
selectionTest_LIBS += concatRecord

# Selection library
#
selectionTest_DBD  += selection.dbd
selectionTest_LIBS += selection


# selectionTest_registerRecordDeviceDriver.cpp derives from selectionTest.dbd
selectionTest_SRCS += selectionTest_registerRecordDeviceDriver.cpp

# Build the main IOC entry point on workstation OSs.
selectionTest_SRCS_DEFAULT += selectionTestMain.cpp
selectionTest_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#selectionTest_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
selectionTest_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
