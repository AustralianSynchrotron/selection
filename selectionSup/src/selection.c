/* $File: //ASP/tec/epics/selection/trunk/selectionSup/src/selection.c $
 * $Revision: #1 $
 * $DateTime: 2023/01/10 23:12:38 $
 * Last checked in by: $Author: starritt $
 *
 * Description:
 *
 * This aSubRecord module provides the means to do selection or reverse selection.
 * Reverse selection is described at approx line 60.
 * Add notes - comparison to positioner selection.
 *
 * Selection
 *
 * When SNAM is set selectionProc, the aSub record will extract upto 20 values,
 * each from one of upto 20 input arrays, based on the input index value.
 * While the index value may be constant, it will typically be a PV.
 * Valid index values are >= 0 and < the length of the corresponding input array.
 *
 * To a certain extent, it behaves like upto 20 sel records, save that the number
 * of inputs can be arbitarily large as opposed to being limited to 12; however
 * the values must be in an array PV as opposed to a number of individual scalar
 * PVs.
 *
 * If the output values are themselves array values, say of size N, then N values
 * are copied from the input array to the output array. This is some what like
 * the behaviour of upto 20 subArray records, however the INDX equivilent can be
 * defined by a PV as opposed to being a static or fixed value.
 *
 * Note: the concat record may be used to cancantinate upto 100 PV values
 * into an array record, and if more than 100 values, two levels of concat record
 * could concatinate upto 10000 values and so on.
 *
 * Unlike the subArray and sel records, this module allows upto 20 selections
 * to be made in parallel; and each selection may be any of the different field
 * types supported by the aSub record, e.g.: DOUBLE, FLOAT, LONG, STRING etc.
 *
 * Field usage:
 * SNAM: "selectionProc"
 * INPA: LONG  - input index value - must be >= 0
 * INPB: array - input data B - this can be any supported type,
 * INPC: array - input data C - this can be any supported type
 *  ,,    ,,      ,,    ,, ,,    ,,   ,, ,,  ,,    ,,      ,,
 * INPU: array - input data U - this can be any supported type
 *
 * OUTB: - scalar/array - selection output B value(s) - must be the same as type as INPB
 * OUTC: - scalar/array - selection output C value(s) - must be the same as type as INPC
 *  ,,      ,,    ,,         ,,      ,,   ,,   ,,        ,,  ,,  ,,  ,,  ,,  ,,  ,,  ,,
 * OUTU: - scalar/array - selection output U value(s) - must be the same as type as INPU
 *
 * For the INPX/OUTX pair to be used, NOX must be > 1 and FTX == FTVX.
 * 
 * The record VALue after processing is the inclusive-or of the following:
 * 0  - okay
 * 1  - underflow       - index < 0
 * 2  - overflow        - index >= NOX
 * 4  - type mis-match  - NOX > 1 and FTX != FTVX
 *
 *
 * Reverse Selection
 * This is still under development.
 * Given a value, it will search an array of values to find the first value that
 * matches (if any). This yields index value, i.e.  0 .. array length - 1, if
 * a match found, otherwise, the output value is -1.
 * For numeric values, a dead-band/tolerance value is used to allow a little bit
 * of wiggle room. For string values, this feature is not applicable.
 *
 * Field usage:
 * SNAM: "reverseSelectionProc"
 * INPA: any type - input value (scalar) - this can be any supported type
 * INPB: array    - look up array - this must be same type as A and >= 2.
 * INPC: DOUBLE   - input deadband/tolerance value - must be >= 0.0
 * OUTA: LONG     - matching index value, or -1
 *
 * The triplet D, E and F mirror A, B and C.
 * The triplet G, H and I mirror A, B and C.
 * The triplet J, K and L mirror A, B and C.
 * The triplet M, N and O mirror A, B and C.
 * The triplet P, Q and R mirror A, B and C.
 * The triplet S, T and U mirror A, B and C.
 *
 * Note: whereas slection may to upto 20 selections in parallel, reverese
 * selection can only to up to 7 reverese selections in parallel.
 *
 *
 * Copyright (c) 2022-2023 Australian Synchrotron
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * Licence as published by the Free Software Foundation; either
 * version 2.1 of the Licence, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public Licence for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * Licence along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Original author: Andrew Starritt
 * Maintained by:   Andrew Starritt
 *
 * Contact details:
 * as-open-source@ansto.gov.au
 * 800 Blackburn Road, Clayton, Victoria 3168, Australia.
 */

#include <stdio.h>
#include <string.h>

#include <dbAccessDefs.h>
#include <epicsExport.h>
#include <epicsTypes.h>
#include <errlog.h>
#include <menuFtype.h>
#include <registryFunction.h>
#include <aSubRecord.h>


// Common macro functions.
//
#define ABS(a)              ((a) >= 0  ? (a) : -(a))
#define MAX(a, b)           ((a) >= (b) ? (a) : (b))
#define MIN(a, b)           ((a) <= (b) ? (a) : (b))
#define LIMIT(x,low,high)   (MAX(low, MIN(x, high)))


//------------------------------------------------------------------------------
//  Record functions
//------------------------------------------------------------------------------
//
static long selectionInit (aSubRecord * prec)
{
   // Apart from the friendly output, this function is essentially
   // just a place holder.
   //
   prec->dpvt = NULL;
   printf ("+++selection init: %s\n", prec->name);
   return 0;
}

//------------------------------------------------------------------------------
// x is one of prec->a, prec->b etc.
// ftx is one of prec->fta, prec->ftb etc. and so on and so forth.
//
static void selectField (long* status,  const epicsUInt32 index,
                         const void* x, const epicsEnum16 ftx,  const epicsUInt32 nox,
                         void* valx,    const epicsEnum16 ftvx, const epicsUInt32 novx)
{
   if (novx > 0) {
      if (ftx == ftvx) {
         int number = nox / novx;
//       printf ("index %2d  number %d\n", index, number);
         if (index < number) {
            const size_t itemSize = dbValueSize (ftx);
            const size_t copySize = novx * itemSize;
            const epicsUInt32 source = index * copySize;
            const epicsUInt8* inpx = (epicsUInt8*) x;
            memcpy (valx, &inpx[source], copySize);

         } else {
            // Only set if/when any values is not a default.
            // We need do not set the status for unused inputs
            //
            if ((ftx != menuFtypeDOUBLE) || (nox != 1) || (novx != 1)) {
               *status |= 2;   // not enough input data
            }
         }
      } else {
         *status |= 4;   // type mis match
      }
   }   // else no output required.
}

//------------------------------------------------------------------------------
//
static long selectionProc (aSubRecord* prec)
{
   const int index = *(epicsInt32*) prec->a;
   
   // The index must be >= 0 for aSubRecord to do anything.
   //
   if (index < 0) {
       return 1;    // under flow
   }

   long status = 0;

   // j = 1 corresponds to B, j = 20 corresponds to U.
   //
   int j;
   for (j = 1; j <= 20; j++) {
      // Recall &prec->fta is implicitly of type epicsEnum16*  etc. etc.
      //
      selectField (&status, index,
                   (&prec->a)[j],    (&prec->fta)[j],  (&prec->noa)[j],
                   (&prec->vala)[j], (&prec->ftva)[j], (&prec->nova)[j] );
   }

// printf ("status %ld\n\n", status);

   return status;
}

//------------------------------------------------------------------------------
//
static long reverseSelectionProc (aSubRecord* prec)
{

// All numeric check are done using epicsFloat64/double
//
#define SEARCH(kind, valuePtr, testValuePtr, number, deadBand) {               \
   const epicsFloat64 val = (epicsFloat64) ((kind*)valuePtr)[0];               \
   index = -1;                                                                 \
   int j;                                                                      \
   for (j = 0; j < number; j++) {                                              \
      const epicsFloat64 test = (epicsFloat64) ((kind*)testValuePtr)[j];       \
      const epicsFloat64 deviation = ABS (val - test);                         \
      if (deviation <= deadBand) {                                             \
         index = j;                                                            \
         break;                                                                \
      }                                                                        \
   }                                                                           \
}


   long status = 0;   
   int p;
   for (p = 0; p < 21; p += 3) {
      const menuFtype ftvx  = (&prec->ftva)[p];
      const menuFtype ftx  = (&prec->fta)[p];
      const menuFtype fty  = (&prec->fta)[p+1];
      const epicsUInt32 number = (&prec->noa)[p+1];

      // To be used the value and the test values must be of the same type
      // and the the number of test values must be >= 2.
      // The output type must be LONG.
      //
      if (ftvx != menuFtypeLONG) continue;
      if (ftx != fty) continue;
      if (number < 2) continue;

      int index = -1;     // until we know otherwise

      void* valuePtr = (&prec->a)[p];
      void* testValues = (&prec->a)[p+1];
      void* deadBandPtr = (&prec->a)[p+2];
      const epicsFloat64 deadBand =  *(epicsFloat64*) deadBandPtr;

      switch (ftx) {

         case menuFtypeSTRING:
            {
               epicsOldString* val = &((epicsOldString*)valuePtr)[0];
               index = -1;
               int j;
               for (j = 0; j < number; j++) {
                  epicsOldString* test = &((epicsOldString*)testValues)[j];
                  if (strncmp (*val, *test, 40) == 0) {
                     index = j;
                     break;
                  }
               }
            }
            break;

         case menuFtypeCHAR:
            SEARCH (epicsInt8, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeUCHAR:
            SEARCH (epicsUInt8, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeSHORT:
            SEARCH (epicsInt16, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeUSHORT:
            SEARCH (epicsUInt16, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeLONG:
            SEARCH (epicsInt32, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeULONG:
            SEARCH (epicsUInt32, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeINT64:
            SEARCH (epicsInt64, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeUINT64:
            SEARCH (epicsUInt64, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeFLOAT:
            SEARCH (epicsFloat32, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeDOUBLE:
            SEARCH (epicsFloat64, valuePtr, testValues, number, deadBand);
            break;

         case menuFtypeENUM:
            SEARCH (epicsEnum16, valuePtr, testValues, number, deadBand);
            break;

         default:
            status |= 1;
            index = -1;
            break;
      }

      epicsInt32* outptr = (epicsInt32*) (&prec->vala)[p];
      *outptr = index;
   }

   return status;

#undef SEARCH
}

//------------------------------------------------------------------------------
//
epicsRegisterFunction (selectionInit);
epicsRegisterFunction (selectionProc);
epicsRegisterFunction (reverseSelectionProc);

/* end */
