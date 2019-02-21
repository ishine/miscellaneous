/*************************************************************************
 *
 *  FUNCTION:  G_pitch
 *
 *  PURPOSE:  Compute the pitch (adaptive codebook) gain. Result in Q12
 *
 *  DESCRIPTION:
 *      The adaptive codebook gain is given by
 *
 *              g = <x[], y[]> / <y[], y[]>
 *
 *      where x[] is the target vector, y[] is the filtered adaptive
 *      codevector, and <> denotes dot product.
 *      The gain is limited to the range [0,1.2]
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "sig_proc.h"

Word16 G_pitch (        /* (o)   : Gain of pitch lag saturated to 1.2      */
    Word16 xn[],        /* (i)   : Pitch target.                           */
    Word16 y1[],        /* (i)   : Filtered adaptive codebook.             */
    Word16 L_subfr      /*       : Length of subframe.                     */
)
{
    Word16 i;
    Word16 xy, yy, exp_xy, exp_yy, gain;
    Word32 s;

    Word16 scaled_y1[80];       /* Usually dynamic allocation of (L_subfr) */

    /* divide by 2 "y1[]" to avoid overflow */

    for (i = 0; i < L_subfr; i++)
    {
        scaled_y1[i] = efr_shr (y1[i], 2); move16 (); 
    }

    /* Compute scalar product <y1[],y1[]> */

    s = 0L;                            move32 (); /* Avoid case of all zeros */
    for (i = 0; i < L_subfr; i++)
    {
        s = efr_L_mac (s, y1[i], y1[i]);
    }
    test (); 
    if (efr_L_sub (s, MAX_32) != 0L)       /* Test for overflow */
    {
        s = efr_L_add (s, 1L);             /* Avoid case of all zeros */
        exp_yy = efr_norm_l (s);
        yy = gsm_efr_round (efr_L_shl (s, exp_yy));
    }
    else
    {
        s = 1L;                        move32 (); /* Avoid case of all zeros */
        for (i = 0; i < L_subfr; i++)
        {
            s = efr_L_mac (s, scaled_y1[i], scaled_y1[i]);
        }
        exp_yy = efr_norm_l (s);
        yy = gsm_efr_round (efr_L_shl (s, exp_yy));
        exp_yy = efr_sub (exp_yy, 4);
    }

    /* Compute scalar product <xn[],y1[]> */

    efr_Overflow = 0;                      move16 (); 
    s = 1L;                            move32 (); /* Avoid case of all zeros */
    for (i = 0; i < L_subfr; i++)
    {
        efr_Carry = 0;                     move16 ();
        s = efr_L_macNs (s, xn[i], y1[i]);

        test ();
        if (efr_Overflow != 0)
        {
	    break;
        }
    }
    test (); 
    if (efr_Overflow == 0)
    {
        exp_xy = efr_norm_l (s);
        xy = gsm_efr_round (efr_L_shl (s, exp_xy));
    }
    else
    {
        s = 1L;                        move32 (); /* Avoid case of all zeros */
        for (i = 0; i < L_subfr; i++)
        {
            s = efr_L_mac (s, xn[i], scaled_y1[i]);
        }
        exp_xy = efr_norm_l (s);
        xy = gsm_efr_round (efr_L_shl (s, exp_xy));
        exp_xy = efr_sub (exp_xy, 2);
    }

    /* If (xy < 4) gain = 0 */

    i = efr_sub (xy, 4);

    test (); 
    if (i < 0)
        return ((Word16) 0);

    /* compute gain = xy/yy */

    xy = efr_shr (xy, 1);                  /* Be sure xy < yy */
    gain = efr_div_s (xy, yy);

    i = efr_add (exp_xy, 3 - 1);           /* Denormalization of division */
    i = efr_sub (i, exp_yy);

    gain = efr_shr (gain, i);

    /* if(gain >1.2) gain = 1.2 */

    test (); 
    if (efr_sub (gain, 4915) > 0)
    {
        gain = 4915;                   move16 (); 
    }
    return (gain);
}
