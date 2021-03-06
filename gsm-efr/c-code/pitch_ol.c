/*************************************************************************
 *
 *  FUNCTION:  Pitch_ol
 *
 *  PURPOSE: Compute the open loop pitch lag.
 *
 *  DESCRIPTION:
 *      The open-loop pitch lag is determined based on the perceptually
 *      weighted speech signal. This is done in the following steps:
 *        - find three maxima of the correlation <sw[n],sw[n-T]> in the
 *          follwing three ranges of T : [18,35], [36,71], and [72, 143]
 *        - divide each maximum by <sw[n-t], sw[n-t]> where t is the delay at
 *          that maximum correlation.
 *        - select the delay of maximum normalized correlation (among the
 *          three candidates) while favoring the lower delay ranges.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"

#define THRESHOLD 27853

/* local function */

static Word16 Lag_max (   /* output: lag found                              */
    Word16 scal_sig[],    /* input : scaled signal                          */
    Word16 scal_fac,      /* input : scaled signal factor                   */
    Word16 L_frame,       /* input : length of frame to compute pitch       */
    Word16 lag_max,       /* input : maximum lag                            */
    Word16 lag_min,       /* input : minimum lag                            */
    Word16 *cor_max);     /* output: normalized correlation of selected lag */

Word16 Pitch_ol (      /* output: open loop pitch lag                        */
    Word16 signal[],   /* input : signal used to compute the open loop pitch */
                       /*     signal[-pit_max] to signal[-1] should be known */
    Word16 pit_min,    /* input : minimum pitch lag                          */
    Word16 pit_max,    /* input : maximum pitch lag                          */
    Word16 L_frame     /* input : length of frame to compute pitch           */
)
{
    Word16 i, j;
    Word16 max1, max2, max3;
    Word16 p_max1, p_max2, p_max3;
    Word32 t0;

    /* Scaled signal                                                */
    /* Can be allocated with memory allocation of(pit_max+L_frame)  */

    Word16 scaled_signal[512];
    Word16 *scal_sig, scal_fac;

    scal_sig = &scaled_signal[pit_max]; move16 (); 

    t0 = 0L;                            move32 (); 
    for (i = -pit_max; i < L_frame; i++)
    {
        t0 = efr_L_mac (t0, signal[i], signal[i]);
    }
    /*--------------------------------------------------------*
     * Scaling of input signal.                               *
     *                                                        *
     *   if efr_Overflow        -> scal_sig[i] = signal[i]>>2     *
     *   else if t0 < 1^22  -> scal_sig[i] = signal[i]<<2     *
     *   else               -> scal_sig[i] = signal[i]        *
     *--------------------------------------------------------*/

    /*--------------------------------------------------------*
     *  Verification for risk of overflow.                    *
     *--------------------------------------------------------*/

    test (); test (); 
    if (efr_L_sub (t0, MAX_32) == 0L)               /* Test for overflow */
    {
        for (i = -pit_max; i < L_frame; i++)
        {
            scal_sig[i] = efr_shr (signal[i], 3);   move16 (); 
        }
        scal_fac = 3;                           move16 (); 
    }
    else if (efr_L_sub (t0, (Word32) 1048576L) < (Word32) 0)
        /* if (t0 < 2^20) */
    {
        for (i = -pit_max; i < L_frame; i++)
        {
            scal_sig[i] = efr_shl (signal[i], 3);   move16 (); 
        }
        scal_fac = -3;                          move16 (); 
    }
    else
    {
        for (i = -pit_max; i < L_frame; i++)
        {
            scal_sig[i] = signal[i];            move16 (); 
        }
        scal_fac = 0;                           move16 (); 
    }

    /*--------------------------------------------------------------------*
     *  The pitch lag search is divided in three sections.                *
     *  Each section cannot have a pitch multiple.                        *
     *  We find a maximum for each section.                               *
     *  We compare the maximum of each section by favoring small lags.    *
     *                                                                    *
     *  First section:  lag delay = pit_max     downto 4*pit_min          *
     *  Second section: lag delay = 4*pit_min-1 downto 2*pit_min          *
     *  Third section:  lag delay = 2*pit_min-1 downto pit_min            *
     *-------------------------------------------------------------------*/
    
    j = efr_shl (pit_min, 2);
    p_max1 = Lag_max (scal_sig, scal_fac, L_frame, pit_max, j, &max1);

    i = efr_sub (j, 1);
    j = efr_shl (pit_min, 1);
    p_max2 = Lag_max (scal_sig, scal_fac, L_frame, i, j, &max2);

    i = efr_sub (j, 1);
    p_max3 = Lag_max (scal_sig, scal_fac, L_frame, i, pit_min, &max3);

    /*--------------------------------------------------------------------*
     * Compare the 3 sections maximum, and favor small lag.               *
     *-------------------------------------------------------------------*/
    
    test (); 
    if (efr_sub (efr_mult (max1, THRESHOLD), max2) < 0)
    {
        max1 = max2;                       move16 (); 
        p_max1 = p_max2;                   move16 (); 
    }
    test (); 
    if (efr_sub (efr_mult (max1, THRESHOLD), max3) < 0)
    {
        p_max1 = p_max3;                   move16 (); 
    }
    return (p_max1);
}

/*************************************************************************
 *
 *  FUNCTION:  Lag_max
 *
 *  PURPOSE: Find the lag that has maximum correlation of scal_sig[] in a
 *           given delay range.
 *
 *  DESCRIPTION:
 *      The correlation is given by
 *           cor[t] = <scal_sig[n],scal_sig[n-t]>,  t=lag_min,...,lag_max
 *      The functions outputs the maximum correlation after normalization
 *      and the corresponding lag.
 *
 *************************************************************************/

static Word16 Lag_max ( /* output: lag found                               */
    Word16 scal_sig[],  /* input : scaled signal.                          */
    Word16 scal_fac,    /* input : scaled signal factor.                   */
    Word16 L_frame,     /* input : length of frame to compute pitch        */
    Word16 lag_max,     /* input : maximum lag                             */
    Word16 lag_min,     /* input : minimum lag                             */
    Word16 *cor_max)    /* output: normalized correlation of selected lag  */
{
    Word16 i, j;
    Word16 *p, *p1;
    Word32 max, t0;
    Word16 max_h, max_l, ener_h, ener_l;
    Word16 p_max;

    max = MIN_32;               move32 (); 

    for (i = lag_max; i >= lag_min; i--)
    {
        p = scal_sig;           move16 (); 
        p1 = &scal_sig[-i];     move16 (); 
        t0 = 0;                 move32 (); 

        for (j = 0; j < L_frame; j++, p++, p1++)
        {
            t0 = efr_L_mac (t0, *p, *p1);
        }
        test (); 
        if (efr_L_sub (t0, max) >= 0)
        {
            max = t0;           move32 (); 
            p_max = i;          move16 (); 
        }
    }

    /* compute energy */

    t0 = 0;                     move32 (); 
    p = &scal_sig[-p_max];      move16 (); 
    for (i = 0; i < L_frame; i++, p++)
    {
        t0 = efr_L_mac (t0, *p, *p);
    }
    /* 1/sqrt(energy) */

    t0 = Inv_sqrt (t0);
    t0 = efr_L_shl (t0, 1);

    /* max = max/sqrt(energy)  */

    L_Extract (max, &max_h, &max_l);
    L_Extract (t0, &ener_h, &ener_l);

    t0 = Mpy_32 (max_h, max_l, ener_h, ener_l);
    t0 = efr_L_shr (t0, scal_fac);

    *cor_max = efr_extract_h (efr_L_shl (t0, 15));      move16 (); /* divide by 2 */

    return (p_max);
}
