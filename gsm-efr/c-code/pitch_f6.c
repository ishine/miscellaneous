/*************************************************************************
 *
 *  FUNCTION:   Pitch_fr6()
 *
 *  PURPOSE: Find the pitch period with 1/6 subsample resolution (closed loop).
 *
 *  DESCRIPTION:
 *        - find the normalized correlation between the target and filtered
 *          past excitation in the search range.
 *        - select the delay with maximum normalized correlation.
 *        - interpolate the normalized correlation at fractions -3/6 to 3/6
 *          with step 1/6 agsm_efr_round the chosen delay.
 *        - The fraction which gives the maximum interpolated value is chosen.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"
#include "codec.h"

 /* L_inter = Length for fractional interpolation = nb.coeff/2 */

#define L_inter 4

 /* Local functions */

void Norm_Corr (Word16 exc[], Word16 xn[], Word16 h[], Word16 L_subfr,
                Word16 t_min, Word16 t_max, Word16 corr_norm[]);

Word16 Pitch_fr6 (    /* (o)     : pitch period.                          */
    Word16 exc[],     /* (i)     : excitation buffer                      */
    Word16 xn[],      /* (i)     : target vector                          */
    Word16 h[],       /* (i)     : impulse response of synthesis and
                                    weighting filters                     */
    Word16 L_subfr,   /* (i)     : Length of subframe                     */
    Word16 t0_min,    /* (i)     : minimum value in the searched range.   */
    Word16 t0_max,    /* (i)     : maximum value in the searched range.   */
    Word16 i_subfr,   /* (i)     : indicator for first subframe.          */
    Word16 *pit_frac  /* (o)     : chosen fraction.                       */
)
{
    Word16 i;
    Word16 t_min, t_max;
    Word16 max, lag, frac;
    Word16 *corr;
    Word16 corr_int;
    Word16 corr_v[40];          /* Total length = t0_max-t0_min+1+2*L_inter */

    /* Find interval to compute normalized correlation */

    t_min = efr_sub (t0_min, L_inter);
    t_max = efr_add (t0_max, L_inter);

    corr = &corr_v[-t_min];                    move16 (); 

    /* Compute normalized correlation between target and filtered excitation */

    Norm_Corr (exc, xn, h, L_subfr, t_min, t_max, corr);

    /* Find integer pitch */

    max = corr[t0_min];                        move16 (); 
    lag = t0_min;                              move16 (); 

    for (i = t0_min + 1; i <= t0_max; i++)
    {
        test (); 
        if (efr_sub (corr[i], max) >= 0)
        {
            max = corr[i];                     move16 (); 
            lag = i;                           move16 (); 
        }
    }

    /* If first subframe and lag > 94 do not search fractional pitch */

    test (); test (); 
    if ((i_subfr == 0) && (efr_sub (lag, 94) > 0))
    {
        *pit_frac = 0;                         move16 (); 
        return (lag);
    }
    /* Test the fractions agsm_efr_round T0 and choose the one which maximizes   */
    /* the interpolated normalized correlation.                          */

    max = Interpol_6 (&corr[lag], -3);
    frac = -3;                                 move16 (); 

    for (i = -2; i <= 3; i++)
    {
        corr_int = Interpol_6 (&corr[lag], i); move16 (); 
        test (); 
        if (efr_sub (corr_int, max) > 0)
        {
            max = corr_int;                    move16 (); 
            frac = i;                          move16 (); 
        }
    }

    /* Limit the fraction value in the interval [-2,-1,0,1,2,3] */

    test (); 
    if (efr_sub (frac, -3) == 0)
    {
        frac = 3;                              move16 (); 
        lag = efr_sub (lag, 1);
    }
    *pit_frac = frac;                          move16 ();
    
    return (lag);
}

/*************************************************************************
 *
 *  FUNCTION:   Norm_Corr()
 *
 *  PURPOSE: Find the normalized correlation between the target vector
 *           and the filtered past excitation.
 *
 *  DESCRIPTION:
 *     The normalized correlation is given by the correlation between the
 *     target and filtered past excitation divided by the square root of
 *     the energy of filtered excitation.
 *                   corr[k] = <x[], y_k[]>/sqrt(y_k[],y_k[])
 *     where x[] is the target vector and y_k[] is the filtered past
 *     excitation at delay k.
 *
 *************************************************************************/

void 
Norm_Corr (Word16 exc[], Word16 xn[], Word16 h[], Word16 L_subfr,
           Word16 t_min, Word16 t_max, Word16 corr_norm[])
{
    Word16 i, j, k;
    Word16 corr_h, corr_l, norm_h, efr_norm_l;
    Word32 s;

    /* Usally dynamic allocation of (L_subfr) */
    Word16 excf[80];
    Word16 scaling, h_fac, *s_excf, scaled_excf[80];

    k = -t_min;                                move16 (); 

    /* compute the filtered excitation for the first delay t_min */

    Convolve (&exc[k], h, excf, L_subfr);

    /* scale "excf[]" to avoid overflow */

    for (j = 0; j < L_subfr; j++)
    {
        scaled_excf[j] = efr_shr (excf[j], 2);     move16 (); 
    }

    /* Compute 1/sqrt(energy of excf[]) */

    s = 0;                                     move32 (); 
    for (j = 0; j < L_subfr; j++)
    {
        s = efr_L_mac (s, excf[j], excf[j]);
    }
    test (); 
    if (efr_L_sub (s, 67108864L) <= 0)             /* if (s <= 2^26) */
    {
        s_excf = excf;                         move16 (); 
        h_fac = 15 - 12;                       move16 (); 
        scaling = 0;                           move16 (); 
    }
    else
    {
        /* "excf[]" is divided by 2 */
        s_excf = scaled_excf;                  move16 (); 
        h_fac = 15 - 12 - 2;                   move16 (); 
        scaling = 2;                           move16 (); 
    }

    /* loop for every possible period */

    for (i = t_min; i <= t_max; i++)
    {
        /* Compute 1/sqrt(energy of excf[]) */

        s = 0;                                 move32 (); 
        for (j = 0; j < L_subfr; j++)
        {
            s = efr_L_mac (s, s_excf[j], s_excf[j]);
        }

        s = Inv_sqrt (s);                      move16 (); 
        L_Extract (s, &norm_h, &efr_norm_l);

        /* Compute correlation between xn[] and excf[] */

        s = 0;                                  move32 (); 
        for (j = 0; j < L_subfr; j++)
        {
            s = efr_L_mac (s, xn[j], s_excf[j]);
        }
        L_Extract (s, &corr_h, &corr_l);

        /* Normalize correlation = correlation * (1/sqrt(energy)) */

        s = Mpy_32 (corr_h, corr_l, norm_h, efr_norm_l);

        corr_norm[i] = efr_extract_h (efr_L_shl (s, 16));
                                                move16 (); 

        /* modify the filtered excitation excf[] for the next iteration */

        test (); 
        if (efr_sub (i, t_max) != 0)
        {
            k--;
            for (j = L_subfr - 1; j > 0; j--)
            {
                s = efr_L_mult (exc[k], h[j]);
                s = efr_L_shl (s, h_fac);
                s_excf[j] = efr_add (efr_extract_h (s), s_excf[j - 1]); move16 (); 
            }
            s_excf[0] = efr_shr (exc[k], scaling);  move16 (); 
        }
    }
    return;
}
