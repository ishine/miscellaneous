/*************************************************************************
 *
 *   FUNCTION:  Levinson()
 *
 *   PURPOSE:  Levinson-Durbin algorithm in double precision. To compute the
 *             LP filter parameters from the speech autocorrelations.
 *
 *   DESCRIPTION:
 *       R[i]    autocorrelations.
 *       A[i]    filter coefficients.
 *       K       reflection coefficients.
 *       Alpha   prediction gain.
 *
 *       Initialisation:
 *               A[0] = 1
 *               K    = -R[1]/R[0]
 *               A[1] = K
 *               Alpha = R[0] * (1-K**2]
 *
 *       Do for  i = 2 to M
 *
 *            S =  SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i]
 *
 *            K = -S / Alpha
 *
 *            An[j] = A[j] + K*A[i-j]   for j=1 to i-1
 *                                      where   An[i] = new A[i]
 *            An[i]=K
 *
 *            Alpha=Alpha * (1-K**2)
 *
 *       END
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"

/* Lpc order == 10 */

#define M 10

/* Last A(z) for case of unstable filter */

Word16 old_A[M + 1];

void Levinson (
    Word16 Rh[],    /* (i)     : Rh[m+1] Vector of autocorrelations (msb) */
    Word16 Rl[],    /* (i)     : Rl[m+1] Vector of autocorrelations (lsb) */
    Word16 A[],     /* (o)     : A[m]    LPC coefficients  (m = 10)       */
    Word16 rc[]     /* (o)     : rc[4]   First 4 reflection coefficients  */

)
{
    Word16 i, j;
    Word16 hi, lo;
    Word16 Kh, Kl;                /* reflexion coefficient; hi and lo      */
    Word16 alp_h, alp_l, alp_exp; /* Prediction gain; hi lo and exponent   */
    Word16 Ah[M + 1], Al[M + 1];  /* LPC coef. in double prec.             */
    Word16 Anh[M + 1], Anl[M + 1];/* LPC coef.for next iteration in double
                                     prec. */
    Word32 t0, t1, t2;            /* temporary variable                    */

    /* K = A[1] = -R[1] / R[0] */

    t1 = L_Comp (Rh[1], Rl[1]);
    t2 = efr_L_abs (t1);                    /* abs R[1]         */
    t0 = Div_32 (t2, Rh[0], Rl[0]);     /* R[1]/R[0]        */
    test (); 
    if (t1 > 0)
        t0 = efr_L_negate (t0);             /* -R[1]/R[0]       */
    L_Extract (t0, &Kh, &Kl);           /* K in DPF         */

    rc[0] = gsm_efr_round (t0);                 move16 (); 

    t0 = efr_L_shr (t0, 4);                 /* A[1] in          */
    L_Extract (t0, &Ah[1], &Al[1]);     /* A[1] in DPF      */

    /*  Alpha = R[0] * (1-K**2) */

    t0 = Mpy_32 (Kh, Kl, Kh, Kl);       /* K*K             */
    t0 = efr_L_abs (t0);                    /* Some case <0 !! */
    t0 = efr_L_sub ((Word32) 0x7fffffffL, t0); /* 1 - K*K        */
    L_Extract (t0, &hi, &lo);           /* DPF format      */
    t0 = Mpy_32 (Rh[0], Rl[0], hi, lo); /* Alpha in        */

    /* Normalize Alpha */

    alp_exp = efr_norm_l (t0);
    t0 = efr_L_shl (t0, alp_exp);
    L_Extract (t0, &alp_h, &alp_l);     /* DPF format    */

    /*--------------------------------------*
     * ITERATIONS  I=2 to M                 *
     *--------------------------------------*/

    for (i = 2; i <= M; i++)
    {
        /* t0 = SUM ( R[j]*A[i-j] ,j=1,i-1 ) +  R[i] */

        t0 = 0;                         move32 (); 
        for (j = 1; j < i; j++)
        {
            t0 = efr_L_add (t0, Mpy_32 (Rh[j], Rl[j], Ah[i - j], Al[i - j]));
        }
        t0 = efr_L_shl (t0, 4);

        t1 = L_Comp (Rh[i], Rl[i]);
        t0 = efr_L_add (t0, t1);            /* efr_add R[i]        */

        /* K = -t0 / Alpha */

        t1 = efr_L_abs (t0);
        t2 = Div_32 (t1, alp_h, alp_l); /* abs(t0)/Alpha              */
        test (); 
        if (t0 > 0)
            t2 = efr_L_negate (t2);         /* K =-t0/Alpha                */
        t2 = efr_L_shl (t2, alp_exp);       /* denormalize; compare to Alpha */
        L_Extract (t2, &Kh, &Kl);       /* K in DPF                      */

        test (); 
        if (efr_sub (i, 5) < 0)
        {
            rc[i - 1] = gsm_efr_round (t2);     move16 (); 
        }
        /* Test for unstable filter. If unstable keep old A(z) */

        test (); 
        if (efr_sub (efr_abs_s (Kh), 32750) > 0)
        {
            for (j = 0; j <= M; j++)
            {
                A[j] = old_A[j];        move16 (); 
            }

            for (j = 0; j < 4; j++)
            {
                rc[j] = 0;              move16 (); 
            }

            return;
        }
        /*------------------------------------------*
         *  Compute new LPC coeff. -> An[i]         *
         *  An[j]= A[j] + K*A[i-j]     , j=1 to i-1 *
         *  An[i]= K                                *
         *------------------------------------------*/

        for (j = 1; j < i; j++)
        {
            t0 = Mpy_32 (Kh, Kl, Ah[i - j], Al[i - j]);
            t0 = efr_L_mac (t0, Ah[j], 32767);
            t0 = efr_L_mac (t0, Al[j], 1);
            L_Extract (t0, &Anh[j], &Anl[j]);
        }
        t2 = efr_L_shr (t2, 4);
        L_Extract (t2, &Anh[i], &Anl[i]);

        /*  Alpha = Alpha * (1-K**2) */

        t0 = Mpy_32 (Kh, Kl, Kh, Kl);           /* K*K             */
        t0 = efr_L_abs (t0);                        /* Some case <0 !! */
        t0 = efr_L_sub ((Word32) 0x7fffffffL, t0);  /* 1 - K*K        */
        L_Extract (t0, &hi, &lo);               /* DPF format      */
        t0 = Mpy_32 (alp_h, alp_l, hi, lo);

        /* Normalize Alpha */

        j = efr_norm_l (t0);
        t0 = efr_L_shl (t0, j);
        L_Extract (t0, &alp_h, &alp_l);         /* DPF format    */
        alp_exp = efr_add (alp_exp, j);             /* Add normalization to
                                                   alp_exp */

        /* A[j] = An[j] */

        for (j = 1; j <= i; j++)
        {
            Ah[j] = Anh[j];                     move16 (); 
            Al[j] = Anl[j];                     move16 (); 
        }
    }

    A[0] = 4096;                                move16 (); 
    for (i = 1; i <= M; i++)
    {
        t0 = L_Comp (Ah[i], Al[i]);
        old_A[i] = A[i] = gsm_efr_round (efr_L_shl (t0, 1));move16 (); move16 (); 
    }

    return;
}
