/*************************************************************************
 *
 *  FUNCTION:  Residu
 *
 *  PURPOSE:  Computes the LP residual.
 *
 *  DESCRIPTION:
 *     The LP residual is computed by filtering the input speech through
 *     the LP inverse filter A(z).
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

/* m = LPC order == 10 */
#define m 10

void Residu (
    Word16 a[], /* (i)     : prediction coefficients                      */
    Word16 x[], /* (i)     : speech signal                                */
    Word16 y[], /* (o)     : residual signal                              */
    Word16 lg   /* (i)     : size of filtering                            */
)
{
    Word16 i, j;
    Word32 s;

    for (i = 0; i < lg; i++)
    {
        s = efr_L_mult (x[i], a[0]);
        for (j = 1; j <= m; j++)
        {
            s = efr_L_mac (s, a[j], x[i - j]);
        }
        s = efr_L_shl (s, 3);
        y[i] = gsm_efr_round (s);       move16 (); 
    }
    return;
}
