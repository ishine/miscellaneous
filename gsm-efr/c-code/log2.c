/*************************************************************************
 *
 *   FUNCTION:   Log2()
 *
 *   PURPOSE:   Computes log2(L_x),  where   L_x is positive.
 *              If L_x is negative or zero, the result is 0.
 *
 *   DESCRIPTION:
 *        The function Log2(L_x) is approximated by a table and linear
 *        interpolation. The following steps are used to compute Log2(L_x)
 *
 *           1- Normalization of L_x.
 *           2- exponent = 30-exponent
 *           3- i = bit25-b31 of L_x;  32<=i<=63  (because of normalization).
 *           4- a = bit10-b24
 *           5- i -=32
 *           6- fraction = table[i]<<16 - (table[i] - table[i+1]) * a * 2
 *
 *************************************************************************/

#include "typedef.h"
#include "basic_op.h"
#include "count.h"

#include "log2.tab"     /* Table for Log2() */

void Log2 (
    Word32 L_x,         /* (i) : input value                                 */
    Word16 *exponent,   /* (o) : Integer part of Log2.   (range: 0<=val<=30) */
    Word16 *fraction    /* (o) : Fractional part of Log2. (range: 0<=val<1) */
)
{
    Word16 exp, i, a, tmp;
    Word32 L_y;

    test (); 
    if (L_x <= (Word32) 0)
    {
        *exponent = 0;          move16 (); 
        *fraction = 0;          move16 (); 
        return;
    }
    exp = efr_norm_l (L_x);
    L_x = efr_L_shl (L_x, exp);     /* L_x is normalized */

    *exponent = efr_sub (30, exp);  move16 (); 

    L_x = efr_L_shr (L_x, 9);
    i = efr_extract_h (L_x);        /* Extract b25-b31 */
    L_x = efr_L_shr (L_x, 1);
    a = efr_extract_l (L_x);        /* Extract b10-b24 of fraction */
    a = a & (Word16) 0x7fff;    logic16 (); 

    i = efr_sub (i, 32);

    L_y = efr_L_deposit_h (table[i]);       /* table[i] << 16        */
    tmp = efr_sub (table[i], table[i + 1]); /* table[i] - table[i+1] */
    L_y = efr_L_msu (L_y, tmp, a);  /* L_y -= tmp*a*2        */

    *fraction = efr_extract_h (L_y);move16 (); 

    return;
}
