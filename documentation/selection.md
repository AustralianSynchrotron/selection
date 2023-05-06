Selection Component

# Introduction

This EPICS module is an aSub record support library that can provide either a
selection or a reverse selection capability.
These two functionalities are described below.

This document assumes the reader is familiar with using EPICS in general, and
the aSub record in particular, which is described in the
[EPICS record reference manual
](https://epics.anl.gov/base/R7-0/4-docs/aSubRecord.html).

The selection module is located in perforce at: __//ASP/tec/epics/selection/trunk__

As of 9th January 2023, the latest release is:  __selection/release_1-1-1__

# Nomenclature

The aSub record has 21 sets of input/output fields A, B, C, ... U.

Within this document, INPX is used to generically refer to any of the data
input fields, i.e.: INPB, INPC, ... INPU.

Likewise FTX is used to generically refer to any of the 20 data type fields:
FTB, FTC, ... FTU.

And so on and so forth.

The "A" fields are used for control, not data.

# Selection

## General

In selection mode, the aSub record can select up-to 20 values, one from each of
up-to 20 arbitrarily large input arrays.
The selection is based on the INPA index value.
While the index value may be specified as a constant input, it would
normally be dynamic and provided by a record or some other PV.
The output values may be held in the VALX fields, however the output values
would typically be written to set point PVs, e.g. to the VAL field of a
number motor records.

While input values would normally be arrays, scalars input are allowed, and
if index is 0, the "selection" becomes a means to copy upto up-to 20 values
from one set of PVs to another set of PVs.

Valid index values are >= 0 and < max_index, where max_index the minimum of
(NOX / NOVX) for each X. If the index is out of bounds, no copy/selection is performed at all.

__Note:__ there is an exception to the above rule to cater for when an
input/output set of the aSub record's fields are undefined.
In this case the default values are:

    FTX = FTVX = DOUBLE and
    NOX = NOVX = 1

In this case, the max_index rule check does not apply fir this particualr X.
However if the index value is 0, the scalar double INPX value __is__ still
copied to VALX.
This is a coding quirk that has become a bonus copy/select on zero feature.

## Comparison with other records

To a certain extent, in selection mode the aSub record behaves like up-to 20
sel records, save that the number of inputs can be arbitrarily large as opposed
to being limited to 12 values;
however the values must be in an array PV (such as a waveform, an aai record or
a concat record) as opposed to a set of individual scalar PVs - see notes 1
and 2 below.

If the output values are array values of size N, then N values are copied from
the input array to the output array.
This is some what like the behaviour of up-to 20 subArray records,
however the INDX equivalent can be defined by a PV as opposed to being a
static or fixed value.

Unlike the subArray and sel records, this module allows up-to 20 selections
to be made in parallel; and each selection may be any of the different field
types supported by the aSub record, e.g.: DOUBLE, FLOAT, LONG, STRING etc.

## Status

The record's value after processing provides status.
The value is the inclusive-or of the following:

    - 0  - okay - data copied
    - 1  - underflow, input index < 0 - no data copied
    - 2  - overflow, input index >= NOX - no data copied
    - 4  - type mis-match - FTX != FTVX - no data copied

## Field usage

| Field | Type      | Description        |
| ----- | --------- | ------------------ |
| INAM  | STRING    | Use "selectionInit"  |
| SNAM  | STRING    | Use "selectionProc"  |
| INPA  | LONG      | Input index value - must be >= 0 |
| INPB  | Any       | Input from which value(s) is/are selected |
| FTB   | menuFtype | The type of INPB |
| NOB   | ULONG     | The number of elements in INPB, must be >= 1. |
| OUTB  | Any       | Holds the output value(s). Typically PP qualified. |
| FTVB  | menuFtype | The type of OUTB. Must be the same as FTB |
| NOVB  | ULONG     | The number of output values. Often one. |
| INPC  | Any       | Input from which value(s) is/are selected |
| FTC   | menuFtype | The type of INPC |
| NOC   | ULONG     | The number of elements in INPC, must be >= 2. |
| OUTC  | Any       | Holds the output value(s). Typically PP qualified. |
| FTVC  | menuFtype | The type of OUTC. Must be the same as FTC |
| NOVC  | ULONG     | The number of output values. Often one. |
| ...   | ...       | So on and so forth for D, E, F, ?.R, S,T |
| INPU  | Any       | Input from which value(s) is/are selected |
| FTU   | menuFtype | The type of INPU |
| NOU   | ULONG     | The number of elements in INPU, must be >= 2 |
| OUTU  | Any       | Holds the output value(s). Typically PP qualified. |
| FTVU  | menuFtype | The type of OUTU. Must be the same as FTU |
| NOVU  | LONG      | The number of output values. Often one. |

For the INPX/OUTX pair to be used, FTX == FTVX.

### Scalar Output

Scalar output can be considered as arrays of length 1.
The index value, from INPA, is used as a zero-based index into the input arrays.

### Array Output

Example, suppose

INPB is as below, and NOVB = 2

| 0 | 1 | 2 | 3 | 4 | 5 |
| - | - | - | - | - | - |
| a | b | c | d | e | f |

The the allowed index values (INPA) are 0, 1 and 2 and the data is copied to
VALB for each index value is show below.

| INPA | VALB[0] | VALB[1] |
|:---: | :---:   | :---:   |
| 0    | a       | b       |
| 1    | c       | d       |
| 2    | e       | f       |

However if NOVB = 3, the allowed index values are 0 or 1, and data is copied
to VALB for each index value is show below.

| INPA | VALB[0] | VALB[1] | VALB[2] |
|:---: | :---:   | :---:   | :---:   |
| 0    | a       | b       | c       |
| 1    | d       | e       | f       |

<br>

## Notes

Note 1: the [concat record](https://github.com/AustralianSynchrotron/concat-record) can be used to concatenate up-to 100 scalar PV values into an array record,
and if more than 100 values are required, two levels of concat record could
concatenate up-to 10,000 values and so on.

Note 2: the EPICS Qt framework widgets such as QELabel, QESimpleShape,
QELineEdit, QENumericEdit are capable of displaying and/or updating a
single element of an array PV by setting the arrayIndex property and where
required the arrayAction property values.
This mitigates the need to create individual scalar records.

# Reverse Selection

## General

This provides the reverse of the selection describe above.
It is limited to 7 reverse look ups per record.

Given a value, it will search an array of values to find the first value
that matches (if any).
This yields index value, i.e.  0 .. array length - 1, if a match found,
otherwise, the output value is -1.
For numeric values, a dead-band/tolerance value is used to allow a little
bit of wiggle room.
For string values, this feature is not applicable.

## Field usage

| Field | Field Type  | Description |
| ----- | ----------- | ----------- |
| SNAM  | STRING    | Use "reverseSelectionProc" |
| INPA  | Any       | Input value |
| FTA   | menuFtype | This can be any supported type |
| NOBA  | ULONG     | Set to 1 |
| INPB  | Any       | Look up array which will be searched |
| FTB   | menuFtype | The type of INPB, must be the same as FTA |
| NOB   | ULONG     | The number of elements in INPB, must be >= 2. |
| INPC  | DOUBLE    | Input deadband/tolerance value - must be >= 0.0 |
| FTC   | menuFtype | Set to DOUBLE - igmored when FTA/FTB is STRING |
| NOBC  | ULONG     | Set to 1 |
| OUTA  | LONG      | Matching index value, or -1 |
| FTVA  | menuFtype | Must be LONG |
| NOVA  | ULONG     | Output size: 1 |

The triplet D, E and F mirror A, B and C.<br>
The triplet G, H and I mirror A, B and C.<br>
The triplet J, K and L mirror A, B and C.<br>
The triplet M, N and O mirror A, B and C.<br>
The triplet P, Q and R mirror A, B and C.<br>
The triplet S, T and U mirror A, B and C.<br>


#  Including Selection into an EPICS IOC

For an IOC using the easy build, in the build_instructions file add:

    comp=/tec/epics/selection/trunk/selectionSup

For a bundle build IOC, p4 sync and build the selection component
(note selection is not currently in the pre-built bundle).
Within the IOC's configure/RELEASE file, add:

    SELECTION=/beamline/perforce/tec/epics/selection/trunk

In either case, add the following to the EPICS IOC's make file
(<top>/IOC_Name_App/src/Makefile):

    IOC_Name_DBD  += selection.dbd
    IOC_Name_LIBS += selection

# Examples

Within the selectionTestApp are example databases files and example EPICS Qt
ui files.

<font size="-1">Last updated: Sun Jan  1 15:12:54 AEDT 2023</font>
<br>
