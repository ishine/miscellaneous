/***************************************************************************
 *
 *   File Name: vad.c
 *
 *   Purpose:   Contains all functions for voice activity detection, as
 *              described in the high level specification of VAD.
 *
 *     Below is a listing of all the functions appearing in the file.
 *     The functions are arranged according to their purpose.  Under
 *     each heading, the ordering is hierarchical.
 *
 *     Resetting of static variables of VAD:
 *       reset_vad()
 *
 *     Main routine of VAD (called by the speech encoder):
 *       vad_computation()
 *       Adaptive filtering and energy computation:
 *         energy_computation()
 *       Averaging of autocorrelation function values:
 *         acf_averaging()
 *       Computation of predictor values:
 *         predictor_values()
 *           schur_recursion()
 *           step_up()
 *           compute_rav1()
 *       Spectral comparison:
 *         spectral_comparison()
 *       Information tone detection:
 *         tone_detection()
 *           step_up()
 *       Threshold adaptation:
 *         threshold_adaptation()
 *       VAD decision:
 *         vad_decision()
 *       VAD hangover addition:
 *         vad_hangover()
 *
 *     Periodicity detection routine (called by the speech encoder):
 *       periodicity_detection()
 *
 **************************************************************************/

#include "typedef.h"
#include "cnst.h"
#include "basic_op.h"
#include "oper_32b.h"
#include "count.h"
#include "vad.h"

/* Constants of VAD hangover addition */

#define HANGCONST 10
#define BURSTCONST 3

/* Constant of spectral comparison */

#define STAT_THRESH 3670L       /* 0.056 */

/* Constants of periodicity detection */

#define LTHRESH 2
#define NTHRESH 4

/* Pseudo floating point representations of constants
   for threshold adaptation */

#define M_PTH    32500          /*** 130000.0 ***/
#define E_PTH    17
#define M_PLEV   21667          /*** 346666.7 ***/
#define E_PLEV   19
#define M_MARGIN 16927          /*** 69333340 ***/
#define E_MARGIN 27

#define FAC 17203               /* 2.1 */

/* Constants of tone detection */

#define FREQTH 3189
#define PREDTH 1464

/* Static variables of VAD */

static Word16 rvad[9], scal_rvad;
static Pfloat thvad;
static Word32 L_sacf[27];
static Word32 L_sav0[36];
static Word16 pt_sacf, pt_sav0;
static Word32 L_lastdm;
static Word16 adaptcount;
static Word16 burstcount, hangcount;
static Word16 oldlagcount, veryoldlagcount, oldlag;

Word16 ptch;

/*************************************************************************
 *
 *   FUNCTION NAME: vad_reset
 *
 *   PURPOSE:  Resets the static variables of the VAD to their
 *             initial values
 *
 *************************************************************************/

void vad_reset ()
{
    Word16 i;

    /* Initialize rvad variables */
    rvad[0] = 0x6000;
    for (i = 1; i < 9; i++)
    {
        rvad[i] = 0;
    }
    scal_rvad = 7;

    /* Initialize threshold level */
    thvad.e = 20;               /*** exponent ***/
    thvad.m = 27083;            /*** mantissa ***/

    /* Initialize ACF averaging variables */
    for (i = 0; i < 27; i++)
    {
        L_sacf[i] = 0L;
    }
    for (i = 0; i < 36; i++)
    {
        L_sav0[i] = 0L;
    }
    pt_sacf = 0;
    pt_sav0 = 0;

    /* Initialize spectral comparison variable */
    L_lastdm = 0L;

    /* Initialize threshold adaptation variable */
    adaptcount = 0;

    /* Initialize VAD hangover addition variables */
    burstcount = 0;
    hangcount = -1;

    /* Initialize periodicity detection variables */
    oldlagcount = 0;
    veryoldlagcount = 0;
    oldlag = 18;

    ptch = 1;

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  vad_computation
 *
 *     PURPOSE:   Returns a decision as to whether the current frame being
 *                processed by the speech encoder contains speech or not.
 *
 *     INPUTS:    r_h[0..8]     autocorrelation of input signal frame (msb)
 *                r_l[0..8]     autocorrelation of input signal frame (lsb)
 *                scal_acf      scaling factor for the autocorrelations
 *                rc[0..3]      speech encoder reflection coefficients
 *                ptch          flag to indicate a periodic signal component
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: vad decision
 *
 ***************************************************************************/

Word16 vad_computation (
    Word16 r_h[],
    Word16 r_l[],
    Word16 scal_acf,
    Word16 rc[],
    Word16 ptch
)
{
    Word32 L_av0[9], L_av1[9];
    Word16 vad, vvad, rav1[9], scal_rav1, stat, tone;
    Pfloat acf0, pvad;

    energy_computation (r_h, scal_acf, rvad, scal_rvad, &acf0, &pvad);
    acf_averaging (r_h, r_l, scal_acf, L_av0, L_av1);
    predictor_values (L_av1, rav1, &scal_rav1);
    stat = spectral_comparison (rav1, scal_rav1, L_av0);        move16 (); 
    tone_detection (rc, &tone);
    threshold_adaptation (stat, ptch, tone, rav1, scal_rav1, pvad, acf0,
                          rvad, &scal_rvad, &thvad);
    vvad = vad_decision (pvad, thvad);                          move16 (); 
    vad = vad_hangover (vvad);                                  move16 (); 

    return vad;
}

/****************************************************************************
 *
 *     FUNCTION:  energy_computation
 *
 *     PURPOSE:   Computes the input and residual energies of the adaptive
 *                filter in a floating point representation.
 *
 *     INPUTS:    r_h[0..8]      autocorrelation of input signal frame (msb)
 *                scal_acf       scaling factor for the autocorrelations
 *                rvad[0..8]     autocorrelated adaptive filter coefficients
 *                scal_rvad      scaling factor for rvad[]
 *
 *     OUTPUTS:   *acf0          signal frame energy (mantissa+exponent)
 *                *pvad          filtered signal energy (mantissa+exponent)
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void energy_computation (
    Word16 r_h[],
    Word16 scal_acf,
    Word16 rvad[],
    Word16 scal_rvad,
    Pfloat * acf0,
    Pfloat * pvad
)
{
    Word16 i, temp, norm_prod;
    Word32 L_temp;

    /* r[0] is always greater than zero (no need to test for r[0] == 0) */

    /* Computation of acf0 (exponent and mantissa) */

    acf0->e = efr_sub (32, scal_acf);       move16 (); 
    acf0->m = r_h[0] & 0x7ff8;          move16 (); logic16 (); 

    /* Computation of pvad (exponent and mantissa) */

    pvad->e = efr_add (acf0->e, 14);        move16 (); 
    pvad->e = efr_sub (pvad->e, scal_rvad); move16 (); 

    L_temp = 0L;                        move32 (); 

    for (i = 1; i <= 8; i++)
    {
        temp = efr_shr (r_h[i], 3);
        L_temp = efr_L_mac (L_temp, temp, rvad[i]);
    }

    temp = efr_shr (r_h[0], 3);
    L_temp = efr_L_add (L_temp, efr_L_shr (efr_L_mult (temp, rvad[0]), 1));

    test (); 
    if (L_temp <= 0L)
    {
        L_temp = 1L;                    move32 (); 
    }
    norm_prod = efr_norm_l (L_temp);
    pvad->e = efr_sub (pvad->e, norm_prod); move16 (); 
    pvad->m = efr_extract_h (efr_L_shl (L_temp, norm_prod));
                                        move16 (); 

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  acf_averaging
 *
 *     PURPOSE:   Computes the arrays L_av0[0..8] and L_av1[0..8].
 *
 *     INPUTS:    r_h[0..8]     autocorrelation of input signal frame (msb)
 *                r_l[0..8]     autocorrelation of input signal frame (lsb)
 *                scal_acf      scaling factor for the autocorrelations
 *
 *     OUTPUTS:   L_av0[0..8]   ACF averaged over last four frames
 *                L_av1[0..8]   ACF averaged over previous four frames
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void acf_averaging (
    Word16 r_h[],
    Word16 r_l[],
    Word16 scal_acf,
    Word32 L_av0[],
    Word32 L_av1[]
)
{
    Word32 L_temp;
    Word16 scale;
    Word16 i;

    scale = efr_add (9, scal_acf);

    for (i = 0; i <= 8; i++)
    {
        L_temp = efr_L_shr (L_Comp (r_h[i], r_l[i]), scale);
        L_av0[i] = efr_L_add (L_sacf[i], L_temp);           move32 (); 
        L_av0[i] = efr_L_add (L_sacf[i + 9], L_av0[i]);     move32 (); 
        L_av0[i] = efr_L_add (L_sacf[i + 18], L_av0[i]);    move32 (); 
        L_sacf[pt_sacf + i] = L_temp;                   move32 (); 
        L_av1[i] = L_sav0[pt_sav0 + i];                 move32 (); 
        L_sav0[pt_sav0 + i] = L_av0[i];                 move32 (); 
    }

    /* Update the array pointers */

    test (); 
    if (efr_sub (pt_sacf, 18) == 0)
    {
        pt_sacf = 0;                                    move16 (); 
    }
    else
    {
        pt_sacf = efr_add (pt_sacf, 9);
    }

    test (); 
    if (efr_sub (pt_sav0, 27) == 0)
    {
        pt_sav0 = 0;                                    move16 (); 
    }
    else
    {
        pt_sav0 = efr_add (pt_sav0, 9);
    }

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  predictor_values
 *
 *     PURPOSE:   Computes the array rav[0..8] needed for the spectral
 *                comparison and the threshold adaptation.
 *
 *     INPUTS:    L_av1[0..8]   ACF averaged over previous four frames
 *
 *     OUTPUTS:   rav1[0..8]    ACF obtained from L_av1
 *                *scal_rav1    rav1[] scaling factor
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void predictor_values (
    Word32 L_av1[],
    Word16 rav1[],
    Word16 *scal_rav1
)
{
    Word16 vpar[8], aav1[9];

    schur_recursion (L_av1, vpar);
    step_up (8, vpar, aav1);
    compute_rav1 (aav1, rav1, scal_rav1);

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  schur_recursion
 *
 *     PURPOSE:   Uses the Schur recursion to compute adaptive filter
 *                reflection coefficients from an autorrelation function.
 *
 *     INPUTS:    L_av1[0..8]    autocorrelation function
 *
 *     OUTPUTS:   vpar[0..7]     reflection coefficients
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void schur_recursion (
    Word32 L_av1[],
    Word16 vpar[]
)
{
    Word16 acf[9], pp[9], kk[9], temp;
    Word16 i, k, m, n;

    /*** Schur recursion with 16-bit arithmetic ***/

    test (); move32 (); 
    if (L_av1[0] == 0)
    {
        for (i = 0; i < 8; i++)
        {
            vpar[i] = 0;                                move16 (); 
        }
        return;
    }
    temp = efr_norm_l (L_av1[0]);

    for (k = 0; k <= 8; k++)
    {
        acf[k] = efr_extract_h (efr_L_shl (L_av1[k], temp));    move16 (); 
    }

    /*** Initialize arrays pp[..] and kk[..] for the recursion: ***/

    for (i = 1; i <= 7; i++)
    {
        kk[9 - i] = acf[i];                             move16 (); 
    }

    for (i = 0; i <= 8; i++)
    {
        pp[i] = acf[i];                                 move16 (); 
    }

    /*** Compute Parcor coefficients: ***/

    for (n = 0; n < 8; n++)
    {
        test (); 
        if ((pp[0] == 0) ||
            (efr_sub (pp[0], efr_abs_s (pp[1])) < 0))
        {
            for (i = n; i < 8; i++)
            {
                vpar[i] = 0;                            move16 (); 
            }
            return;
        }
        vpar[n] = efr_div_s (efr_abs_s (pp[1]), pp[0]);         move16 (); 

        test ();                                        move16 (); 
        if (pp[1] > 0)
        {
            vpar[n] = efr_negate (vpar[n]);                 move16 (); 
        }
        test (); 
        if (efr_sub (n, 7) == 0)
        {
            return;
        }
        /*** Schur recursion: ***/

        pp[0] = efr_add (pp[0], efr_mult_r (pp[1], vpar[n]));   move16 (); 

        for (m = 1; m <= 7 - n; m++)
        {
            pp[m] = efr_add (pp[1 + m], efr_mult_r (kk[9 - m], vpar[n]));
                                                        move16 (); 
            kk[9 - m] = efr_add (kk[9 - m], efr_mult_r (pp[1 + m], vpar[n]));
                                                        move16 (); 
        }
    }

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  step_up
 *
 *     PURPOSE:   Computes the transversal filter coefficients from the
 *                reflection coefficients.
 *
 *     INPUTS:    np               filter order (2..8)
 *                vpar[0..np-1]    reflection coefficients
 *
 *     OUTPUTS:   aav1[0..np]      transversal filter coefficients
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void step_up (
    Word16 np,
    Word16 vpar[],
    Word16 aav1[]
)
{
    Word32 L_coef[9], L_work[9];
    Word16 temp;
    Word16 i, m;

    /*** Initialization of the step-up recursion ***/

    L_coef[0] = 0x20000000L;    move32 (); 
    L_coef[1] = efr_L_shl (efr_L_deposit_l (vpar[0]), 14);              move32 (); 

    /*** Loop on the LPC analysis order: ***/

    for (m = 2; m <= np; m++)
    {
        for (i = 1; i < m; i++)
        {
            temp = efr_extract_h (L_coef[m - i]);
            L_work[i] = efr_L_mac (L_coef[i], vpar[m - 1], temp);   move32 (); 
        }

        for (i = 1; i < m; i++)
        {
            L_coef[i] = L_work[i];                              move32 (); 
        }

        L_coef[m] = efr_L_shl (efr_L_deposit_l (vpar[m - 1]), 14);      move32 (); 
    }

    /*** Keep the aav1[0..np] in 15 bits ***/

    for (i = 0; i <= np; i++)
    {
        aav1[i] = efr_extract_h (efr_L_shr (L_coef[i], 3));             move32 (); 
    }

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  compute_rav1
 *
 *     PURPOSE:   Computes the autocorrelation function of the adaptive
 *                filter coefficients.
 *
 *     INPUTS:    aav1[0..8]     adaptive filter coefficients
 *
 *     OUTPUTS:   rav1[0..8]     ACF of aav1
 *                *scal_rav1     rav1[] scaling factor
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void compute_rav1 (
    Word16 aav1[],
    Word16 rav1[],
    Word16 *scal_rav1
)
{
    Word32 L_work[9];
    Word16 i, k;

    /*** Computation of the rav1[0..8] ***/

    for (i = 0; i <= 8; i++)
    {
        L_work[i] = 0L;                                           move32 (); 

        for (k = 0; k <= 8 - i; k++)
        {
            L_work[i] = efr_L_mac (L_work[i], aav1[k], aav1[k + i]);  move32 (); 
        }
    }

    test (); move32 (); 
    if (L_work[0] == 0L)
    {
        *scal_rav1 = 0;                                           move16 (); 
    }
    else
    {
        *scal_rav1 = efr_norm_l (L_work[0]);
    }

    for (i = 0; i <= 8; i++)
    {
        rav1[i] = efr_extract_h (efr_L_shl (L_work[i], *scal_rav1));      move16 (); 
    }

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  spectral_comparison
 *
 *     PURPOSE:   Computes the stat flag needed for the threshold
 *                adaptation decision.
 *
 *     INPUTS:    rav1[0..8]      ACF obtained from L_av1
 *                *scal_rav1      rav1[] scaling factor
 *                L_av0[0..8]     ACF averaged over last four frames
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: flag to indicate spectral stationarity
 *
 ***************************************************************************/

Word16 spectral_comparison (
    Word16 rav1[],
    Word16 scal_rav1,
    Word32 L_av0[]
)
{
    Word32 L_dm, L_sump, L_temp;
    Word16 stat, sav0[9], shift, divshift, temp;
    Word16 i;

    /*** Re-normalize L_av0[0..8] ***/

    test (); move32 (); 
    if (L_av0[0] == 0L)
    {
        for (i = 0; i <= 8; i++)
        {
            sav0[i] = 0x0fff;   /* 4095 */                      move16 (); 
        }
    }
    else
    {
        shift = efr_sub (efr_norm_l (L_av0[0]), 3);
        for (i = 0; i <= 8; i++)
        {
            sav0[i] = efr_extract_h (efr_L_shl (L_av0[i], shift));      move16 (); 
        }
    }

    /*** Compute partial sum of dm ***/

    L_sump = 0L;                                                move32 (); 
    for (i = 1; i <= 8; i++)
    {
        L_sump = efr_L_mac (L_sump, rav1[i], sav0[i]);
    }

    /*** Compute the division of the partial sum by sav0[0] ***/

    test (); 
    if (L_sump < 0L)
    {
        L_temp = efr_L_negate (L_sump);
    }
    else
    {
        L_temp = L_sump;                                        move32 (); 
    }

    test (); 
    if (L_temp == 0L)
    {
        L_dm = 0L;                                              move32 (); 
        shift = 0;                                              move16 (); 
    }
    else
    {
        sav0[0] = efr_shl (sav0[0], 3);                             move16 (); 
        shift = efr_norm_l (L_temp);
        temp = efr_extract_h (efr_L_shl (L_temp, shift));

        test (); 
        if (efr_sub (sav0[0], temp) >= 0)
        {
            divshift = 0;                                       move16 (); 
            temp = efr_div_s (temp, sav0[0]);
        }
        else
        {
            divshift = 1;                                       move16 (); 
            temp = efr_sub (temp, sav0[0]);
            temp = efr_div_s (temp, sav0[0]);
        }

        test (); 
        if (efr_sub (divshift, 1) == 0)
        {
            L_dm = 0x8000L;                                     move32 (); 
        }
        else
        {
            L_dm = 0L;                                          move32 (); 
        }

        L_dm = efr_L_shl (efr_L_add (L_dm, efr_L_deposit_l (temp)), 1);

        test (); 
        if (L_sump < 0L)
        {
            L_dm = efr_L_negate (L_dm);
        }
    }

    /*** Re-normalization and final computation of L_dm ***/

    L_dm = efr_L_shl (L_dm, 14);
    L_dm = efr_L_shr (L_dm, shift);
    L_dm = efr_L_add (L_dm, efr_L_shl (efr_L_deposit_l (rav1[0]), 11));
    L_dm = efr_L_shr (L_dm, scal_rav1);

    /*** Compute the difference and save L_dm ***/

    L_temp = efr_L_sub (L_dm, L_lastdm);
    L_lastdm = L_dm;                                            move32 (); 

    test (); 
    if (L_temp < 0L)
    {
        L_temp = efr_L_negate (L_temp);
    }
    /*** Evaluation of the stat flag ***/

    L_temp = efr_L_sub (L_temp, STAT_THRESH);

    test (); 
    if (L_temp < 0L)
    {
        stat = 1;                                               move16 (); 
    }
    else
    {
        stat = 0;                                               move16 (); 
    }

    return stat;
}

/****************************************************************************
 *
 *     FUNCTION:  threshold_adaptation
 *
 *     PURPOSE:   Evaluates the secondary VAD decision.  If speech is not
 *                present then the noise model rvad and adaptive threshold
 *                thvad are updated.
 *
 *     INPUTS:    stat          flag to indicate spectral stationarity
 *                ptch          flag to indicate a periodic signal component
 *                tone          flag to indicate a tone signal component
 *                rav1[0..8]    ACF obtained from L_av1
 *                scal_rav1     rav1[] scaling factor
 *                pvad          filtered signal energy (mantissa+exponent)
 *                acf0          signal frame energy (mantissa+exponent)
 *
 *     OUTPUTS:   rvad[0..8]    autocorrelated adaptive filter coefficients
 *                *scal_rvad    rvad[] scaling factor
 *                *thvad        decision threshold (mantissa+exponent)
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void threshold_adaptation (
    Word16 stat,
    Word16 ptch,
    Word16 tone,
    Word16 rav1[],
    Word16 scal_rav1,
    Pfloat pvad,
    Pfloat acf0,
    Word16 rvad[],
    Word16 *scal_rvad,
    Pfloat * thvad
)
{
    Word16 comp, comp2;
    Word32 L_temp;
    Word16 temp;
    Pfloat p_temp;
    Word16 i;

    comp = 0;                                           move16 (); 

    /*** Test if acf0 < pth; if yes set thvad to plev ***/

    test (); 
    if (efr_sub (acf0.e, E_PTH) < 0)
    {
        comp = 1;                                       move16 (); 
    }
    test (); test (); 
    if ((efr_sub (acf0.e, E_PTH) == 0) && (efr_sub (acf0.m, M_PTH) < 0))
    {
        comp = 1;                                       move16 (); 
    }
    test (); 
    if (efr_sub (comp, 1) == 0)
    {
        thvad->e = E_PLEV;                              move16 (); 
        thvad->m = M_PLEV;                              move16 (); 

        return;
    }
    /*** Test if an adaption is required ***/

    test (); 
    if (efr_sub (ptch, 1) == 0)
    {
        comp = 1;                                       move16 (); 
    }
    test (); 
    if (stat == 0)
    {
        comp = 1;                                       move16 (); 
    }
    test (); 
    if (efr_sub (tone, 1) == 0)
    {
        comp = 1;                                       move16 (); 
    }
    test (); 
    if (efr_sub (comp, 1) == 0)
    {
        adaptcount = 0;                                 move16 (); 
        return;
    }
    /*** Increment adaptcount ***/

    adaptcount = efr_add (adaptcount, 1);
    test (); 
    if (efr_sub (adaptcount, 8) <= 0)
    {
        return;
    }
    /*** computation of thvad-(thvad/dec) ***/

    thvad->m = efr_sub (thvad->m, efr_shr (thvad->m, 5));       move16 (); 

    test (); 
    if (efr_sub (thvad->m, 0x4000) < 0)
    {
        thvad->m = efr_shl (thvad->m, 1);                   move16 (); 
        thvad->e = efr_sub (thvad->e, 1);                   move16 (); 
    }
    /*** computation of pvad*fac ***/

    L_temp = efr_L_mult (pvad.m, FAC);
    L_temp = efr_L_shr (L_temp, 15);
    p_temp.e = efr_add (pvad.e, 1);                         move16 (); 

    test (); 
    if (L_temp > 0x7fffL)
    {
        L_temp = efr_L_shr (L_temp, 1);
        p_temp.e = efr_add (p_temp.e, 1);                   move16 (); 
    }
    p_temp.m = efr_extract_l (L_temp);                      move16 (); 

    /*** test if thvad < pvad*fac ***/

    test (); 
    if (efr_sub (thvad->e, p_temp.e) < 0)
    {
        comp = 1;                                       move16 (); 
    }
    test (); test (); 
    if ((efr_sub (thvad->e, p_temp.e) == 0) &&
        (efr_sub (thvad->m, p_temp.m) < 0))
    {
        comp = 1;                                       move16 (); 
    }
    /*** compute minimum(thvad+(thvad/inc), pvad*fac) when comp = 1 ***/

    test (); 
    if (efr_sub (comp, 1) == 0)
    {
        /*** compute thvad + (thvad/inc) ***/

        L_temp = efr_L_add (efr_L_deposit_l (thvad->m),
                        efr_L_deposit_l (efr_shr (thvad->m, 4)));

        test (); 
        if (efr_L_sub (L_temp, 0x7fffL) > 0)
        {
            thvad->m = efr_extract_l (efr_L_shr (L_temp, 1));   move16 (); 
            thvad->e = efr_add (thvad->e, 1);               move16 (); 
        }
        else
        {
            thvad->m = efr_extract_l (L_temp);              move16 (); 
        }

        comp2 = 0;                                      move16 (); 

        test (); 
        if (efr_sub (p_temp.e, thvad->e) < 0)
        {
            comp2 = 1;                                  move16 (); 
        }
        test (); test (); 
        if ((efr_sub (p_temp.e, thvad->e) == 0) &&
            (efr_sub (p_temp.m, thvad->m) < 0))
        {
            comp2 = 1;                                  move16 (); 
        }
        test (); 
        if (efr_sub (comp2, 1) == 0)
        {
            thvad->e = p_temp.e;move16 (); 
            thvad->m = p_temp.m;move16 (); 
        }
    }
    /*** compute pvad + margin ***/

    test (); 
    if (efr_sub (pvad.e, E_MARGIN) == 0)
    {
        L_temp = efr_L_add (efr_L_deposit_l (pvad.m), efr_L_deposit_l (M_MARGIN));
        p_temp.m = efr_extract_l (efr_L_shr (L_temp, 1));       move16 (); 
        p_temp.e = efr_add (pvad.e, 1);     move16 (); 
    }
    else
    {
        test (); 
        if (efr_sub (pvad.e, E_MARGIN) > 0)
        {
            temp = efr_sub (pvad.e, E_MARGIN);
            temp = efr_shr (M_MARGIN, temp);
            L_temp = efr_L_add (efr_L_deposit_l (pvad.m), efr_L_deposit_l (temp));

            test (); 
            if (efr_L_sub (L_temp, 0x7fffL) > 0)
            {
                p_temp.e = efr_add (pvad.e, 1);             move16 (); 
                p_temp.m = efr_extract_l (efr_L_shr (L_temp, 1));
                                                        move16 (); 
            }
            else
            {
                p_temp.e = pvad.e;                      move16 (); 
                p_temp.m = efr_extract_l (L_temp);          move16 (); 
            }
        }
        else
        {
            temp = efr_sub (E_MARGIN, pvad.e);
            temp = efr_shr (pvad.m, temp);
            L_temp = efr_L_add (efr_L_deposit_l (M_MARGIN), efr_L_deposit_l (temp));

            test (); 
            if (efr_L_sub (L_temp, 0x7fffL) > 0)
            {
                p_temp.e = efr_add (E_MARGIN, 1);           move16 (); 
                p_temp.m = efr_extract_l (efr_L_shr (L_temp, 1));
                                                        move16 (); 
            }
            else
            {
                p_temp.e = E_MARGIN;                    move16 (); 
                p_temp.m = efr_extract_l (L_temp);          move16 (); 
            }
        }
    }

    /*** Test if thvad > pvad + margin ***/

    comp = 0;                                           move16 (); 

    test (); 
    if (efr_sub (thvad->e, p_temp.e) > 0)
    {
        comp = 1;                                       move16 (); 
    }
    test (); test (); 
    if ((efr_sub (thvad->e, p_temp.e) == 0) &&
        (efr_sub (thvad->m, p_temp.m) > 0))
    {
        comp = 1;                                       move16 (); 
    }
    test (); 
    if (efr_sub (comp, 1) == 0)
    {
        thvad->e = p_temp.e;                            move16 (); 
        thvad->m = p_temp.m;                            move16 (); 
    }
    /*** Normalise and retain rvad[0..8] in memory ***/

    *scal_rvad = scal_rav1;                             move16 (); 

    for (i = 0; i <= 8; i++)
    {
        rvad[i] = rav1[i];                              move16 (); 
    }

    /*** Set adaptcount to adp + 1 ***/

    adaptcount = 9;                                     move16 (); 

    return;
}

/****************************************************************************
 *
 *     FUNCTION:  tone_detection
 *
 *     PURPOSE:   Computes the tone flag needed for the threshold
 *                adaptation decision.
 *
 *     INPUTS:    rc[0..3]    reflection coefficients calculated in the
 *                            speech encoder short term predictor
 *
 *     OUTPUTS:   *tone       flag to indicate a periodic signal component
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void tone_detection (
    Word16 rc[],
    Word16 *tone
)
{
    Word32 L_num, L_den, L_temp;
    Word16 temp, prederr, a[3];
    Word16 i;

    *tone = 0;                  move16 (); 

    /*** Calculate filter coefficients ***/

    step_up (2, rc, a);

    /*** Calculate ( a[1] * a[1] ) ***/

    temp = efr_shl (a[1], 3);
    L_den = efr_L_mult (temp, temp);

    /*** Calculate ( 4*a[2] - a[1]*a[1] ) ***/

    L_temp = efr_L_shl (efr_L_deposit_h (a[2]), 3);
    L_num = efr_L_sub (L_temp, L_den);

    /*** Check if pole frequency is less than 385 Hz ***/

    test (); 
    if (L_num <= 0)
    {
        return;
    }
    test (); move16 (); 
    if (a[1] < 0)
    {
        temp = efr_extract_h (L_den);
        L_den = efr_L_mult (temp, FREQTH);

        L_temp = efr_L_sub (L_num, L_den);

        test (); 
        if (L_temp < 0)
        {
            return;
        }
    }
    /*** Calculate normalised prediction error ***/

    prederr = 0x7fff;           move16 (); 

    for (i = 0; i < 4; i++)
    {
        temp = efr_mult (rc[i], rc[i]);
        temp = efr_sub (0x7fff, temp);
        prederr = efr_mult (prederr, temp);
    }

    /*** Test if prediction error is smaller than threshold ***/

    temp = efr_sub (prederr, PREDTH);

    test (); 
    if (temp < 0)
    {
        *tone = 1;              move16 (); 
    }
    return;
}

/****************************************************************************
 *
 *     FUNCTION:  vad_decision
 *
 *     PURPOSE:   Computes the VAD decision based on the comparison of the
 *                floating point representations of pvad and thvad.
 *
 *     INPUTS:    pvad          filtered signal energy (mantissa+exponent)
 *                thvad         decision threshold (mantissa+exponent)
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: vad decision before hangover is added
 *
 ***************************************************************************/

Word16 vad_decision (
    Pfloat pvad,
    Pfloat thvad
)
{
    Word16 vvad;

    test (); test (); test (); 
    if (efr_sub (pvad.e, thvad.e) > 0)
    {
        vvad = 1;               move16 (); 
    }
    else if ((efr_sub (pvad.e, thvad.e) == 0) &&
               (efr_sub (pvad.m, thvad.m) > 0))
    {
        vvad = 1;               move16 (); 
    }
    else
    {
        vvad = 0;               move16 (); 
    }

    return vvad;
}

/****************************************************************************
 *
 *     FUNCTION:  vad_hangover
 *
 *     PURPOSE:   Computes the final VAD decision for the current frame
 *                being processed.
 *
 *     INPUTS:    vvad           vad decision before hangover is added
 *
 *     OUTPUTS:   none
 *
 *     RETURN VALUE: vad decision after hangover is added
 *
 ***************************************************************************/

Word16 vad_hangover (
    Word16 vvad
)
{
    test (); 
    if (efr_sub (vvad, 1) == 0)
    {
        burstcount = efr_add (burstcount, 1);
    }
    else
    {
        burstcount = 0;         move16 (); 
    }

    test (); 
    if (efr_sub (burstcount, BURSTCONST) >= 0)
    {
        hangcount = HANGCONST;  move16 (); 
        burstcount = BURSTCONST;move16 (); 
    }
    test (); 
    if (hangcount >= 0)
    {
        hangcount = efr_sub (hangcount, 1);
        return 1;               /* vad = 1 */
    }
    return vvad;                /* vad = vvad */
}

/****************************************************************************
 *
 *     FUNCTION:  periodicity_update
 *
 *     PURPOSE:   Computes the ptch flag needed for the threshold
 *                adaptation decision for the next frame.
 *
 *     INPUTS:    lags[0..1]       speech encoder long term predictor lags
 *
 *     OUTPUTS:   *ptch            Boolean voiced / unvoiced decision
 *
 *     RETURN VALUE: none
 *
 ***************************************************************************/

void periodicity_update (
    Word16 lags[],
    Word16 *ptch
)
{
    Word16 minlag, maxlag, lagcount, temp;
    Word16 i;

    /*** Run loop for the two halves in the frame ***/

    lagcount = 0;               move16 (); 

    for (i = 0; i <= 1; i++)
    {
        /*** Search the maximum and minimum of consecutive lags ***/

        test (); 
        if (efr_sub (oldlag, lags[i]) > 0)
        {
            minlag = lags[i];   move16 (); 
            maxlag = oldlag;    move16 (); 
        }
        else
        {
            minlag = oldlag;    move16 (); 
            maxlag = lags[i];   move16 (); 
        }

        temp = efr_sub (maxlag, minlag);

        test (); 
        if (efr_sub (temp, LTHRESH) < 0)
        {
            lagcount = efr_add (lagcount, 1);
        }
        /*** Save the current LTP lag ***/

        oldlag = lags[i];       move16 (); 
    }

    /*** Update the veryoldlagcount and oldlagcount ***/

    veryoldlagcount = oldlagcount;
                                move16 (); 
    oldlagcount = lagcount;     move16 (); 

    /*** Make ptch decision ready for next frame ***/

    temp = efr_add (oldlagcount, veryoldlagcount);

    test (); 
    if (efr_sub (temp, NTHRESH) >= 0)
    {
        *ptch = 1;              move16 (); 
    }
    else
    {
        *ptch = 0;              move16 (); 
    }

    return;
}
