/*************************************************************************
 *
 *  FUNCTION:  agc
 *
 *  PURPOSE: Scales the postfilter output on a subframe basis by automatic
 *           control of the subframe gain.
 *
 *  DESCRIPTION:
 *   sig_out[n] = sig_out[n] * gain[n];
 *   where gain[n] is the gain at the nth sample given by
 *     gain[n] = agc_fac * gain[n-1] + (1 - agc_fac) g_in/g_out
 *   g_in/g_out is the square root of the ratio of energy at the input
 *   and output of the postfilter.
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"
#include "sig_proc.h"
#include "cnst.h"

Word16 past_gain;               /* initial value of past_gain = 1.0  */

void agc (
    Word16 *sig_in,             /* (i)     : postfilter input signal  */
    Word16 *sig_out,            /* (i/o)   : postfilter output signal */
    Word16 agc_fac,             /* (i)     : AGC factor               */
    Word16 l_trm                /* (i)     : subframe size            */
) {
    Word16 i, exp;
    Word16 gain_in, gain_out, g0, gain;
    Word32 s;

    Word16 temp;

    /* calculate gain_out with exponent */

    temp = efr_shr (sig_out[0], 2);
    s = efr_L_mult (temp, temp);

    for (i = 1; i < l_trm; i++) {
        temp = efr_shr (sig_out[i], 2);
        s = efr_L_mac (s, temp, temp);
    }

    test ();
    if (s == 0) {
        past_gain = 0;          move16 ();
        return;
    }
    exp = efr_sub (efr_norm_l (s), 1);
    gain_out = gsm_efr_round (efr_L_shl (s, exp));

    /* calculate gain_in with exponent */

    temp = efr_shr (sig_in[0], 2);
    s = efr_L_mult (temp, temp);

    for (i = 1; i < l_trm; i++) {
        temp = efr_shr (sig_in[i], 2);
        s = efr_L_mac (s, temp, temp);
    }

    test ();
    if (s == 0) {
        g0 = 0;                 move16 ();
    } else {
        i = efr_norm_l (s);
        gain_in = gsm_efr_round (efr_L_shl (s, i));
        exp = efr_sub (exp, i);

        /*---------------------------------------------------*
         *  g0 = (1-agc_fac) * sqrt(gain_in/gain_out);       *
         *---------------------------------------------------*/

        s = efr_L_deposit_l (efr_div_s (gain_out, gain_in));
        s = efr_L_shl (s, 7);       /* s = gain_out / gain_in */
        s = efr_L_shr (s, exp);     /* efr_add exponent */

        s = Inv_sqrt (s);
        i = gsm_efr_round (efr_L_shl (s, 9));

        /* g0 = i * (1-agc_fac) */
        g0 = efr_mult (i, efr_sub (32767, agc_fac));
    }

    /* compute gain[n] = agc_fac * gain[n-1]
                        + (1-agc_fac) * sqrt(gain_in/gain_out) */
    /* sig_out[n] = gain[n] * sig_out[n]                        */

    gain = past_gain;           move16 ();

    for (i = 0; i < l_trm; i++) {
        gain = efr_mult (gain, agc_fac);
        gain = efr_add (gain, g0);
        sig_out[i] = efr_extract_h (efr_L_shl (efr_L_mult (sig_out[i], gain), 3));
        move16 ();
    }

    past_gain = gain;           move16 ();

    return;
}

void agc2 (
    Word16 *sig_in,        /* (i)     : postfilter input signal  */
    Word16 *sig_out,       /* (i/o)   : postfilter output signal */
    Word16 l_trm           /* (i)     : subframe size            */
) {
    Word16 i, exp;
    Word16 gain_in, gain_out, g0;
    Word32 s;

    Word16 temp;

    /* calculate gain_out with exponent */

    temp = efr_shr (sig_out[0], 2);
    s = efr_L_mult (temp, temp);
    for (i = 1; i < l_trm; i++) {
        temp = efr_shr (sig_out[i], 2);
        s = efr_L_mac (s, temp, temp);
    }

    test ();
    if (s == 0) {
        return;
    }
    exp = efr_sub (efr_norm_l (s), 1);
    gain_out = gsm_efr_round (efr_L_shl (s, exp));

    /* calculate gain_in with exponent */

    temp = efr_shr (sig_in[0], 2);
    s = efr_L_mult (temp, temp);
    for (i = 1; i < l_trm; i++) {
        temp = efr_shr (sig_in[i], 2);
        s = efr_L_mac (s, temp, temp);
    }

    test ();
    if (s == 0) {
        g0 = 0;                 move16 ();
    } else {
        i = efr_norm_l (s);
        gain_in = gsm_efr_round (efr_L_shl (s, i));
        exp = efr_sub (exp, i);

        /*---------------------------------------------------*
         *  g0 = sqrt(gain_in/gain_out);                     *
         *---------------------------------------------------*/

        s = efr_L_deposit_l (efr_div_s (gain_out, gain_in));
        s = efr_L_shl (s, 7);       /* s = gain_out / gain_in */
        s = efr_L_shr (s, exp);     /* efr_add exponent */

        s = Inv_sqrt (s);
        g0 = gsm_efr_round (efr_L_shl (s, 9));
    }

    /* sig_out(n) = gain(n) sig_out(n) */

    for (i = 0; i < l_trm; i++) {
        sig_out[i] = efr_extract_h (efr_L_shl (efr_L_mult (sig_out[i], g0), 3));
        move16 ();
    }

    return;
}
