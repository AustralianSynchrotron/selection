# $File: //ASP/tec/epics/selection/trunk/selectionSup/src/Makefile $
# $Revision: #1 $
# $DateTime: 2023/01/10 23:12:38 $
# Last checked in by: $Author: starritt $
#
TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#========================================

# Make EPICS 7 friendly
#
USR_CFLAGS += -DUSE_TYPED_RSET

#========================================
# build a support library
#
LIBRARY_IOC += selection

DBD += selection.dbd

# specify all source files to be compiled and added to the library
#
selection_SRCS += selection.c

selection_LIBS += $(EPICS_BASE_IOC_LIBS)

#========================================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
