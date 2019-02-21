/***********************************************************************
 *
 *   This file contains functions for the automatic complexity calculation
 *
*************************************************************************/

#include <stdio.h>
#include "typedef.h"
#include "count.h"

/* Global counter variable for calculation of complexity weight */

BASIC_OP counter;

const BASIC_OP op_weight = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 4, 15, 18, 30, 1, 2, 1, 2, 2
};

/* local variable */

#define NbFuncMax  1024

static Word16 funcid, nbframe;
static Word32 glob_wc, wc[NbFuncMax];
static float total_wmops;

static Word32 LastWOper;

Word32 TotalWeightedOperation () {
  Word16 i;
  Word32 tot, *ptr, *ptr2;

  tot = 0;
  ptr = (Word32 *) &counter;
  ptr2 = (Word32 *) &op_weight;
  for (i = 0; i < (sizeof (counter) / sizeof (Word32)); i++)

  {
    tot += ((*ptr++) * (*ptr2++));
  }

  return ((Word32) tot);
}

Word32 DeltaWeightedOperation () {
  Word32 NewWOper, delta;

  NewWOper = TotalWeightedOperation ();
  delta = NewWOper - LastWOper;
  LastWOper = NewWOper;
  return (delta);
}

void move16 (void) {
  counter.DataMove16++;
}

void move32 (void) {
  counter.DataMove32++;
}

void test (void) {
  counter.Test++;
}

void logic16 (void) {
  counter.Logic16++;
}

void logic32 (void) {
  counter.Logic32++;
}

void Init_WMOPS_counter (void) {
  Word16 i;

  /* reset function weight operation counter variable */

  for (i = 0; i < NbFuncMax; i++)
    wc[i] = (Word32) 0;
  glob_wc = 0;
  nbframe = 0;
  total_wmops = 0.0;

}

void Reset_WMOPS_counter (void) {
  Word16 i;
  Word32 *ptr;

  ptr = (Word32 *) &counter;
  for (i = 0; i < (sizeof (counter) / sizeof (Word32)); i++)

  {
    *ptr++ = 0;
  }
  LastWOper = 0;

  funcid = 0;                 /* new frame, set function id to zero */
}

Word32 fwc (void) {                    /* function worst case */
  Word32 tot;

  tot = DeltaWeightedOperation ();
  if (tot > wc[funcid])
    wc[funcid] = tot;

  funcid++;

  return (tot);
}

void WMOPS_output (Word16 dtx_mode) {
  Word16 i;
  Word32 tot, tot_wc;

  tot = TotalWeightedOperation ();
  if (tot > glob_wc)
    glob_wc = tot;

  fprintf (stderr, "WMOPS=%.2f", ((float) tot) * 0.00005);

  nbframe++;
  total_wmops += ((float) tot) * 0.00005;
  fprintf (stderr, "  Average=%.2f", total_wmops / (float) nbframe);

  fprintf (stderr, "  WorstCase=%.2f", ((float) glob_wc) * 0.00005);

  /* Worst worst case printed only when not in DTX mode */
  if (dtx_mode == 0) {
    tot_wc = 0L;
    for (i = 0; i < funcid; i++)
      tot_wc += wc[i];
    fprintf (stderr, "  WorstWC=%.2f", ((float) tot_wc) * 0.00005);
  }
  fprintf (stderr, "\n");
}
