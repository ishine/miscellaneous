/*___________________________________________________________________________
 |                                                                           |
 | Basic arithmetic operators.                                               |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |   Include-Files                                                           |
 |___________________________________________________________________________|
*/

#include <stdio.h>
#include <stdlib.h>
#include "typedef.h"
#include "basic_op.h"

#if (WMOPS)
#include "count.h"
extern BASIC_OP counter;

#endif

/*___________________________________________________________________________
 |                                                                           |
 |   Local Functions                                                         |
 |___________________________________________________________________________|
*/
Word16 saturate (Word32 L_var1);

/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
Flag efr_Overflow = 0;
Flag efr_Carry = 0;

/*___________________________________________________________________________
 |                                                                           |
 |   Functions                                                               |
 |___________________________________________________________________________|
*/

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : saturate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Limit the 32 bit input to the range of a 16 bit word.                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16
saturate (Word32 L_var1) {
    Word16 var_out;

    if (L_var1 > 0X00007fffL) {
        efr_Overflow = 1;
        var_out = MAX_16;
    } else if (L_var1 < (Word32) 0xffff8000L) {
        efr_Overflow = 1;
        var_out = MIN_16;
    } else {
        efr_Overflow = 0;
        var_out = efr_extract_l (L_var1);
#if (WMOPS)
        counter.efr_extract_l--;
#endif
    }

    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_add                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the addition (var1+var2) with overflow control and saturation;|
 |    the 16 bit result is set at +32767 when overflow occurs or at -32768   |
 |    when underflow occurs.                                                 |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_add (Word16 var1, Word16 var2) {
    Word16 var_out;
    Word32 L_sum;

    L_sum = (Word32) var1 + var2;
    var_out = saturate (L_sum);
#if (WMOPS)
    counter.efr_add++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_sub                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the subtraction (var1+var2) with overflow control and satu-   |
 |    ration; the 16 bit result is set at +32767 when overflow occurs or at  |
 |    -32768 when underflow occurs.                                          |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_sub (Word16 var1, Word16 var2) {
    Word16 var_out;
    Word32 L_diff;

    L_diff = (Word32) var1 - var2;
    var_out = saturate (L_diff);
#if (WMOPS)
    counter.efr_sub++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_abs_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of var1; efr_abs_s(-32768) = 32767.                         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_abs_s (Word16 var1) {
    Word16 var_out;

    if (var1 == (Word16) 0X8000) {
        var_out = MAX_16;
    } else {
        if (var1 < 0) {
            var_out = -var1;
        } else {
            var_out = var1;
        }
    }
#if (WMOPS)
    counter.efr_abs_s++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_shl                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 left var2 positions.Zero fill|
 |   the var2 LSB of the result. If var2 is negative, arithmetically shift   |
 |   var1 right by -var2 with sign extension. Saturate the result in case of |
 |   underflows or overflows.                                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_shl (Word16 var1, Word16 var2) {
    Word16 var_out;
    Word32 result;

    if (var2 < 0) {
        var_out = efr_shr (var1, -var2);
#if (WMOPS)
        counter.efr_shr--;
#endif
    } else {
        result = (Word32) var1 * ((Word32) 1 << var2);

        if ((var2 > 15 && var1 != 0) || (result != (Word32) ((Word16) result))) {
            efr_Overflow = 1;
            var_out = (var1 > 0) ? MAX_16 : MIN_16;
        } else {
            var_out = efr_extract_l (result);
#if (WMOPS)
            counter.efr_extract_l--;
#endif
        }
    }
#if (WMOPS)
    counter.efr_shl++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_shr                                                     |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 16 bit input var1 right var2 positions with    |
 |   sign extension. If var2 is negative, arithmetically shift var1 left by  |
 |   -var2 with sign extension. Saturate the result in case of underflows or |
 |   overflows.                                                              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_shr (Word16 var1, Word16 var2) {
    Word16 var_out;

    if (var2 < 0) {
        var_out = efr_shl (var1, -var2);
#if (WMOPS)
        counter.efr_shl--;
#endif
    } else {
        if (var2 >= 15) {
            var_out = (var1 < 0) ? -1 : 0;
        } else {
            if (var1 < 0) {
                var_out = ~((~var1) >> var2);
            } else {
                var_out = var1 >> var2;
            }
        }
    }

#if (WMOPS)
    counter.efr_shr++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_mult                                                    |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Performs the multiplication of var1 by var2 and gives a 16 bit result  |
 |    which is scaled i.e.:                                                  |
 |             efr_mult(var1,var2) = efr_extract_l(efr_L_shr((var1 times var2),15)) and  |
 |             efr_mult(-32768,-32768) = 32767.                                  |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_mult (Word16 var1, Word16 var2) {
    Word16 var_out;
    Word32 L_product;

    L_product = (Word32) var1 * (Word32) var2;

    L_product = (L_product & (Word32) 0xffff8000L) >> 15;

    if (L_product & (Word32) 0x00010000L)
        L_product = L_product | (Word32) 0xffff0000L;

    var_out = saturate (L_product);
#if (WMOPS)
    counter.efr_mult++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_mult                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   efr_L_mult is the 32 bit result of the multiplication of var1 times var2    |
 |   with one shift left i.e.:                                               |
 |        efr_L_mult(var1,var2) = efr_L_shl((var1 times var2),1) and                   |
 |        efr_L_mult(-32768,-32768) = 2147483647.                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_mult (Word16 var1, Word16 var2) {
    Word32 L_var_out;

    L_var_out = (Word32) var1 * (Word32) var2;

    if (L_var_out != (Word32) 0x40000000L) {
        L_var_out *= 2;
    } else {
        efr_Overflow = 1;
        L_var_out = MAX_32;
    }

#if (WMOPS)
    counter.efr_L_mult++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_negate                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate var1 with saturation, saturate in the case where input is -32768:|
 |                efr_negate(var1) = efr_sub(0,var1).                                |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_negate (Word16 var1) {
    Word16 var_out;

    var_out = (var1 == MIN_16) ? MAX_16 : -var1;
#if (WMOPS)
    counter.efr_negate++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_extract_h                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 MSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_extract_h (Word32 L_var1) {
    Word16 var_out;

    var_out = (Word16) (L_var1 >> 16);
#if (WMOPS)
    counter.efr_extract_h++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_extract_l                                               |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Return the 16 LSB of L_var1.                                            |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_extract_l (Word32 L_var1) {
    Word16 var_out;

    var_out = (Word16) L_var1;
#if (WMOPS)
    counter.efr_extract_l++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : gsm_efr_round                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Round the lower 16 bits of the 32 bit input number into the MS 16 bits  |
 |   with saturation. Shift the resulting bits right by 16 and return the 16 |
 |   bit number:                                                             |
 |               gsm_efr_round(L_var1) = efr_extract_h(efr_L_add(L_var1,32768))              |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32 ) whose value falls in the |
 |             range : 0x8000 0000 <= L_var1 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 gsm_efr_round (Word32 L_var1) {
    Word16 var_out;
    Word32 L_gsm_efr_rounded;

    L_gsm_efr_rounded = efr_L_add (L_var1, (Word32) 0x00008000L);
#if (WMOPS)
    counter.efr_L_add--;
#endif
    var_out = efr_extract_h (L_gsm_efr_rounded);
#if (WMOPS)
    counter.efr_extract_h--;
    counter.gsm_efr_round++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_mac                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation, return a 32 bit result:               |
 |        efr_L_mac(L_var3,var1,var2) = efr_L_add(L_var3,efr_L_mult(var1,var2)).         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_mac (Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;
    Word32 L_product;

    L_product = efr_L_mult (var1, var2);
#if (WMOPS)
    counter.efr_L_mult--;
#endif
    L_var_out = efr_L_add (L_var3, L_product);
#if (WMOPS)
    counter.efr_L_add--;
    counter.efr_L_mac++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_msu                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation, return a 32 bit result:           |
 |        efr_L_msu(L_var3,var1,var2) = efr_L_sub(L_var3,efr_L_mult(var1,var2)).         |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_msu (Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;
    Word32 L_product;

    L_product = efr_L_mult (var1, var2);
#if (WMOPS)
    counter.efr_L_mult--;
#endif
    L_var_out = efr_L_sub (L_var3, L_product);
#if (WMOPS)
    counter.efr_L_sub--;
    counter.efr_L_msu++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_macNs                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 without saturation, return a 32 bit result. Generate   |
 |   carry and overflow values :                                             |
 |        efr_L_macNs(L_var3,var1,var2) = efr_L_add_c(L_var3,efr_L_mult(var1,var2)).     |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the efr_Carry flag has to be cleared or set before using     |
 |    operators which take into account its value.                           |
 |___________________________________________________________________________|
*/

Word32 efr_L_macNs (Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;

    L_var_out = efr_L_mult (var1, var2);
#if (WMOPS)
    counter.efr_L_mult--;
#endif
    L_var_out = efr_L_add_c (L_var3, L_var_out);
#if (WMOPS)
    counter.efr_L_add_c--;
    counter.efr_L_macNs++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_msuNs                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result from L_var3 without saturation, return a 32 bit result. Ge-  |
 |   nerate carry and overflow values :                                      |
 |        efr_L_msuNs(L_var3,var1,var2) = efr_L_sub_c(L_var3,efr_L_mult(var1,var2)).     |
 |                                                                           |
 |   Complexity weight : 1                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the efr_Carry flag has to be cleared or set before using     |
 |    operators which take into account its value.                           |
 |___________________________________________________________________________|
*/

Word32 efr_L_msuNs (Word32 L_var3, Word16 var1, Word16 var2) {
    Word32 L_var_out;

    L_var_out = efr_L_mult (var1, var2);
#if (WMOPS)
    counter.efr_L_mult--;
#endif
    L_var_out = efr_L_sub_c (L_var3, L_var_out);
#if (WMOPS)
    counter.efr_L_sub_c--;
    counter.efr_L_msuNs++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_add                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits addition of the two 32 bits variables (L_var1+L_var2) with      |
 |   overflow control and saturation; the result is set at +2147483647 when  |
 |   overflow occurs or at -2147483648 when underflow occurs.                |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_add (Word32 L_var1, Word32 L_var2) {
    Word32 L_var_out;

    L_var_out = L_var1 + L_var2;

    if (((L_var1 ^ L_var2) & MIN_32) == 0) {
        if ((L_var_out ^ L_var1) & MIN_32) {
            L_var_out = (L_var1 < 0) ? MIN_32 : MAX_32;
            efr_Overflow = 1;
        }
    }
#if (WMOPS)
    counter.efr_L_add++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_sub                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   32 bits subtraction of the two 32 bits variables (L_var1-L_var2) with   |
 |   overflow control and saturation; the result is set at +2147483647 when  |
 |   overflow occurs or at -2147483648 when underflow occurs.                |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_sub (Word32 L_var1, Word32 L_var2) {
    Word32 L_var_out;

    L_var_out = L_var1 - L_var2;

    if (((L_var1 ^ L_var2) & MIN_32) != 0) {
        if ((L_var_out ^ L_var1) & MIN_32) {
            L_var_out = (L_var1 < 0L) ? MIN_32 : MAX_32;
            efr_Overflow = 1;
        }
    }
#if (WMOPS)
    counter.efr_L_sub++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_add_c                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Performs 32 bits addition of the two 32 bits variables (L_var1+L_var2+C)|
 |   with carry. No saturation. Generate carry and efr_Overflow values. The car- |
 |   ry and overflow values are binary variables which can be tested and as- |
 |   signed values.                                                          |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the efr_Carry flag has to be cleared or set before using     |
 |    operators which take into account its value.                           |
 |___________________________________________________________________________|
*/
Word32 efr_L_add_c (Word32 L_var1, Word32 L_var2) {
    Word32 L_var_out;
    Word32 L_test;
    Flag carry_int = 0;

    L_var_out = L_var1 + L_var2 + efr_Carry;

    L_test = L_var1 + L_var2;

    if ((L_var1 > 0) && (L_var2 > 0) && (L_test < 0)) {
        efr_Overflow = 1;
        carry_int = 0;
    } else {
        if ((L_var1 < 0) && (L_var2 < 0)) {
            if (L_test >= 0) {
                efr_Overflow = 1;
                carry_int = 1;
            } else {
                efr_Overflow = 0;
                carry_int = 1;
            }
        } else {
            if (((L_var1 ^ L_var2) < 0) && (L_test >= 0)) {
                efr_Overflow = 0;
                carry_int = 1;
            } else {
                efr_Overflow = 0;
                carry_int = 0;
            }
        }
    }

    if (efr_Carry) {
        if (L_test == MAX_32) {
            efr_Overflow = 1;
            efr_Carry = carry_int;
        } else {
            if (L_test == (Word32) 0xFFFFFFFFL) {
                efr_Carry = 1;
            } else {
                efr_Carry = carry_int;
            }
        }
    } else {
        efr_Carry = carry_int;
    }

#if (WMOPS)
    counter.efr_L_add_c++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_sub_c                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Performs 32 bits subtraction of the two 32 bits variables with carry    |
 |   (borrow) : L_var1-L_var2-C. No saturation. Generate carry and efr_Overflow  |
 |   values. The carry and overflow values are binary variables which can    |
 |   be tested and assigned values.                                          |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    L_var2   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |                                                                           |
 |   Caution :                                                               |
 |                                                                           |
 |    In some cases the efr_Carry flag has to be cleared or set before using     |
 |    operators which take into account its value.                           |
 |___________________________________________________________________________|
*/

Word32 efr_L_sub_c (Word32 L_var1, Word32 L_var2) {
    Word32 L_var_out;
    Word32 L_test;
    Flag carry_int = 0;

    if (efr_Carry) {
        efr_Carry = 0;
        if (L_var2 != MIN_32) {
            L_var_out = efr_L_add_c (L_var1, -L_var2);
#if (WMOPS)
            counter.efr_L_add_c--;
#endif
        } else {
            L_var_out = L_var1 - L_var2;
            if (L_var1 > 0L) {
                efr_Overflow = 1;
                efr_Carry = 0;
            }
        }
    } else {
        L_var_out = L_var1 - L_var2 - (Word32) 0X00000001L;
        L_test = L_var1 - L_var2;

        if ((L_test < 0) && (L_var1 > 0) && (L_var2 < 0)) {
            efr_Overflow = 1;
            carry_int = 0;
        } else if ((L_test > 0) && (L_var1 < 0) && (L_var2 > 0)) {
            efr_Overflow = 1;
            carry_int = 1;
        } else if ((L_test > 0) && ((L_var1 ^ L_var2) > 0)) {
            efr_Overflow = 0;
            carry_int = 1;
        }
        if (L_test == MIN_32) {
            efr_Overflow = 1;
            efr_Carry = carry_int;
        } else {
            efr_Carry = carry_int;
        }
    }

#if (WMOPS)
    counter.efr_L_sub_c++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_negate                                                |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Negate the 32 bit variable L_var1 with saturation; saturate in the case |
 |   where input is -2147483648 (0x8000 0000).                               |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_negate (Word32 L_var1) {
    Word32 L_var_out;

    L_var_out = (L_var1 == MIN_32) ? MAX_32 : -L_var1;
#if (WMOPS)
    counter.efr_L_negate++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_mult_r                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as efr_mult with gsm_efr_rounding, i.e.:                                       |
 |     efr_mult_r(var1,var2) = efr_extract_l(efr_L_shr(((var1 * var2) + 16384),15)) and  |
 |     efr_mult_r(-32768,-32768) = 32767.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_mult_r (Word16 var1, Word16 var2) {
    Word16 var_out;
    Word32 L_product_arr;

    L_product_arr = (Word32) var1 * (Word32) var2;      /* product */
    L_product_arr += (Word32) 0x00004000L;      /* gsm_efr_round */
    L_product_arr &= (Word32) 0xffff8000L;
    L_product_arr >>= 15;       /* shift */

    if (L_product_arr & (Word32) 0x00010000L) { /* sign extend when necessary */
        L_product_arr |= (Word32) 0xffff0000L;
    }
    var_out = saturate (L_product_arr);
#if (WMOPS)
    counter.efr_mult_r++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_shl                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 left var2 positions. Zero  |
 |   fill the var2 LSB of the result. If var2 is negative, arithmetically    |
 |   shift L_var1 right by -var2 with sign extension. Saturate the result in |
 |   case of underflows or overflows.                                        |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_shl (Word32 L_var1, Word16 var2) {
    Word32 L_var_out;

    if (var2 <= 0) {
        L_var_out = efr_L_shr (L_var1, -var2);
#if (WMOPS)
        counter.efr_L_shr--;
#endif
    } else {
        for (; var2 > 0; var2--) {
            if (L_var1 > (Word32) 0X3fffffffL) {
                efr_Overflow = 1;
                L_var_out = MAX_32;
                break;
            } else {
                if (L_var1 < (Word32) 0xc0000000L) {
                    efr_Overflow = 1;
                    L_var_out = MIN_32;
                    break;
                }
            }
            L_var1 *= 2;
            L_var_out = L_var1;
        }
    }
#if (WMOPS)
    counter.efr_L_shl++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_shr                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Arithmetically shift the 32 bit input L_var1 right var2 positions with  |
 |   sign extension. If var2 is negative, arithmetically shift L_var1 left   |
 |   by -var2 and zero fill the -var2 LSB of the result. Saturate the result |
 |   in case of underflows or overflows.                                     |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var_out <= 0x7fff ffff.              |
 |___________________________________________________________________________|
*/

Word32 efr_L_shr (Word32 L_var1, Word16 var2) {
    Word32 L_var_out;

    if (var2 < 0) {
        L_var_out = efr_L_shl (L_var1, -var2);
#if (WMOPS)
        counter.efr_L_shl--;
#endif
    } else {
        if (var2 >= 31) {
            L_var_out = (L_var1 < 0L) ? -1 : 0;
        } else {
            if (L_var1 < 0) {
                L_var_out = ~((~L_var1) >> var2);
            } else {
                L_var_out = L_var1 >> var2;
            }
        }
    }
#if (WMOPS)
    counter.efr_L_shr++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_shr_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as efr_shr(var1,var2) but with gsm_efr_rounding. Saturate the result in case of|
 |   underflows or overflows :                                               |
 |    - If var2 is greater than zero :                                       |
 |          if (efr_sub(efr_shl(efr_shr(var1,var2),1),efr_shr(var1,efr_sub(var2,1))))            |
 |          is equal to zero                                                 |
 |                     then                                                  |
 |                     efr_shr_r(var1,var2) = efr_shr(var1,var2)                     |
 |                     else                                                  |
 |                     efr_shr_r(var1,var2) = efr_add(efr_shr(var1,var2),1)              |
 |    - If var2 is less than or equal to zero :                              |
 |                     efr_shr_r(var1,var2) = efr_shr(var1,var2).                    |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word16 efr_shr_r (Word16 var1, Word16 var2) {
    Word16 var_out;

    if (var2 > 15) {
        var_out = 0;
    } else {
        var_out = efr_shr (var1, var2);
#if (WMOPS)
        counter.efr_shr--;
#endif

        if (var2 > 0) {
            if ((var1 & ((Word16) 1 << (var2 - 1))) != 0) {
                var_out++;
            }
        }
    }
#if (WMOPS)
    counter.efr_shr_r++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_mac_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Add the 32 bit    |
 |   result to L_var3 with saturation. Round the LS 16 bits of the result    |
 |   into the MS 16 bits with saturation and shift the result right by 16.   |
 |   Return a 16 bit result.                                                 |
 |            efr_mac_r(L_var3,var1,var2) = gsm_efr_round(efr_L_mac(L_var3,var1,var2))       |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.              |
 |___________________________________________________________________________|
*/

Word16 efr_mac_r (Word32 L_var3, Word16 var1, Word16 var2) {
    Word16 var_out;

    L_var3 = efr_L_mac (L_var3, var1, var2);
#if (WMOPS)
    counter.efr_L_mac--;
#endif
    L_var3 = efr_L_add (L_var3, (Word32) 0x00008000L);
#if (WMOPS)
    counter.efr_L_add--;
#endif
    var_out = efr_extract_h (L_var3);
#if (WMOPS)
    counter.efr_extract_h--;
    counter.efr_mac_r++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_msu_r                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Multiply var1 by var2 and shift the result left by 1. Subtract the 32   |
 |   bit result to L_var3 with saturation. Round the LS 16 bits of the res-  |
 |   ult into the MS 16 bits with saturation and shift the result right by   |
 |   16. Return a 16 bit result.                                             |
 |            efr_msu_r(L_var3,var1,var2) = gsm_efr_round(efr_L_msu(L_var3,var1,var2))       |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var3   32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= L_var3 <= 0x7fff ffff.                 |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 8000 <= L_var_out <= 0x0000 7fff.              |
 |___________________________________________________________________________|
*/

Word16 efr_msu_r (Word32 L_var3, Word16 var1, Word16 var2) {
    Word16 var_out;

    L_var3 = efr_L_msu (L_var3, var1, var2);
#if (WMOPS)
    counter.efr_L_msu--;
#endif
    L_var3 = efr_L_add (L_var3, (Word32) 0x00008000L);
#if (WMOPS)
    counter.efr_L_add--;
#endif
    var_out = efr_extract_h (L_var3);
#if (WMOPS)
    counter.efr_extract_h--;
    counter.efr_msu_r++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_deposit_h                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 MS bits of the 32 bit output. The   |
 |   16 LS bits of the output are zeroed.                                    |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff 0000.                |
 |___________________________________________________________________________|
*/

Word32 efr_L_deposit_h (Word16 var1) {
    Word32 L_var_out;

    L_var_out = (Word32) var1 << 16;
#if (WMOPS)
    counter.efr_L_deposit_h++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_deposit_l                                             |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Deposit the 16 bit var1 into the 16 LS bits of the 32 bit output. The   |
 |   16 MS bits of the output are sign extended.                             |
 |                                                                           |
 |   Complexity weight : 2                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0xFFFF 8000 <= var_out <= 0x0000 7fff.                |
 |___________________________________________________________________________|
*/

Word32 efr_L_deposit_l (Word16 var1) {
    Word32 L_var_out;

    L_var_out = (Word32) var1;
#if (WMOPS)
    counter.efr_L_deposit_l++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_shr_r                                                 |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Same as efr_L_shr(L_var1,var2) but with gsm_efr_rounding. Saturate the result in    |
 |   case of underflows or overflows :                                       |
 |    - If var2 is greater than zero :                                       |
 |          if (efr_L_sub(efr_L_shl(efr_L_shr(L_var1,var2),1),efr_L_shr(L_var1,efr_sub(var2,1))))|
 |          is equal to zero                                                 |
 |                     then                                                  |
 |                     efr_L_shr_r(L_var1,var2) = efr_L_shr(L_var1,var2)             |
 |                     else                                                  |
 |                     efr_L_shr_r(L_var1,var2) = efr_L_add(efr_L_shr(L_var1,var2),1)    |
 |    - If var2 is less than or equal to zero :                              |
 |                     efr_L_shr_r(L_var1,var2) = efr_L_shr(L_var1,var2).            |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/

Word32 efr_L_shr_r (Word32 L_var1, Word16 var2) {
    Word32 L_var_out;

    if (var2 > 31) {
        L_var_out = 0;
    } else {
        L_var_out = efr_L_shr (L_var1, var2);
#if (WMOPS)
        counter.efr_L_shr--;
#endif
        if (var2 > 0) {
            if ((L_var1 & ((Word32) 1 << (var2 - 1))) != 0) {
                L_var_out++;
            }
        }
    }
#if (WMOPS)
    counter.efr_L_shr_r++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_abs                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    Absolute value of L_var1; Saturate in case where the input is          |
 |                                                               -214783648  |
 |                                                                           |
 |   Complexity weight : 3                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x0000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/

Word32 efr_L_abs (Word32 L_var1) {
    Word32 L_var_out;

    if (L_var1 == MIN_32) {
        L_var_out = MAX_32;
    } else {
        if (L_var1 < 0) {
            L_var_out = -L_var1;
        } else {
            L_var_out = L_var1;
        }
    }

#if (WMOPS)
    counter.efr_L_abs++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_L_sat                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |    32 bit L_var1 is set to 2147483647 if an overflow occured or to        |
 |    -2147483648 if an underflow occured on the most recent efr_L_add_c,        |
 |    efr_L_sub_c, efr_L_macNs or efr_L_msuNs operations. The carry and overflow values  |
 |    are binary values which can be tested and assigned values.             |
 |                                                                           |
 |   Complexity weight : 4                                                   |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    L_var_out                                                              |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var_out <= 0x7fff ffff.                |
 |___________________________________________________________________________|
*/

Word32 efr_L_sat (Word32 L_var1) {
    Word32 L_var_out;

    L_var_out = L_var1;

    if (efr_Overflow) {

        if (efr_Carry) {
            L_var_out = MIN_32;
        } else {
            L_var_out = MAX_32;
        }

        efr_Carry = 0;
        efr_Overflow = 0;
    }
#if (WMOPS)
    counter.efr_L_sat++;
#endif
    return (L_var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_norm_s                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shift needed to normalize the 16 bit varia- |
 |   ble var1 for positive values on the interval with minimum of 16384 and  |
 |   maximum of 32767, and for negative values on the interval with minimum  |
 |   of -32768 and maximum of -16384; in order to normalize the result, the  |
 |   following operation must be done :                                      |
 |                    norm_var1 = efr_shl(var1,efr_norm_s(var1)).                    |
 |                                                                           |
 |   Complexity weight : 15                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0xffff 8000 <= var1 <= 0x0000 7fff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 000f.                |
 |___________________________________________________________________________|
*/

Word16 efr_norm_s (Word16 var1) {
    Word16 var_out;

    if (var1 == 0) {
        var_out = 0;
    } else {
        if (var1 == (Word16) 0xffff) {
            var_out = 15;
        } else {
            if (var1 < 0) {
                var1 = ~var1;
            }
            for (var_out = 0; var1 < 0x4000; var_out++) {
                var1 <<= 1;
            }
        }
    }

#if (WMOPS)
    counter.efr_norm_s++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_div_s                                                   |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces a result which is the fractional integer division of var1  by  |
 |   var2; var1 and var2 must be positive and var2 must be greater or equal  |
 |   to var1; the result is positive (leading bit equal to 0) and truncated  |
 |   to 16 bits.                                                             |
 |   If var1 = var2 then div(var1,var2) = 32767.                             |
 |                                                                           |
 |   Complexity weight : 18                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    var1                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var1 <= var2 and var2 != 0.            |
 |                                                                           |
 |    var2                                                                   |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : var1 <= var2 <= 0x0000 7fff and var2 != 0.            |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 7fff.                |
 |             It's a Q15 value (point between b15 and b14).                 |
 |___________________________________________________________________________|
*/

Word16 efr_div_s (Word16 var1, Word16 var2) {
    Word16 var_out = 0;
    Word16 iteration;
    Word32 L_num;
    Word32 L_denom;

    if ((var1 > var2) || (var1 < 0) || (var2 < 0)) {
        printf ("Division Error var1=%d  var2=%d\n", var1, var2);
        exit (0);
    }
    if (var2 == 0) {
        printf ("Division by 0, Fatal error \n");
        exit (0);
    }
    if (var1 == 0) {
        var_out = 0;
    } else {
        if (var1 == var2) {
            var_out = MAX_16;
        } else {
            L_num = efr_L_deposit_l (var1);
#if (WMOPS)
            counter.efr_L_deposit_l--;
#endif
            L_denom = efr_L_deposit_l (var2);
#if (WMOPS)
            counter.efr_L_deposit_l--;
#endif

            for (iteration = 0; iteration < 15; iteration++) {
                var_out <<= 1;
                L_num <<= 1;

                if (L_num >= L_denom) {
                    L_num = efr_L_sub (L_num, L_denom);
#if (WMOPS)
                    counter.efr_L_sub--;
#endif
                    var_out = efr_add (var_out, 1);
#if (WMOPS)
                    counter.efr_add--;
#endif
                }
            }
        }
    }

#if (WMOPS)
    counter.efr_div_s++;
#endif
    return (var_out);
}

/*___________________________________________________________________________
 |                                                                           |
 |   Function Name : efr_norm_l                                                  |
 |                                                                           |
 |   Purpose :                                                               |
 |                                                                           |
 |   Produces the number of left shifts needed to normalize the 32 bit varia-|
 |   ble L_var1 for positive values on the interval with minimum of          |
 |   1073741824 and maximum of 2147483647, and for negative values on the in-|
 |   terval with minimum of -2147483648 and maximum of -1073741824; in order |
 |   to normalize the result, the following operation must be done :         |
 |                   norm_L_var1 = efr_L_shl(L_var1,efr_norm_l(L_var1)).             |
 |                                                                           |
 |   Complexity weight : 30                                                  |
 |                                                                           |
 |   Inputs :                                                                |
 |                                                                           |
 |    L_var1                                                                 |
 |             32 bit long signed integer (Word32) whose value falls in the  |
 |             range : 0x8000 0000 <= var1 <= 0x7fff ffff.                   |
 |                                                                           |
 |   Outputs :                                                               |
 |                                                                           |
 |    none                                                                   |
 |                                                                           |
 |   Return Value :                                                          |
 |                                                                           |
 |    var_out                                                                |
 |             16 bit short signed integer (Word16) whose value falls in the |
 |             range : 0x0000 0000 <= var_out <= 0x0000 001f.                |
 |___________________________________________________________________________|
*/

Word16 efr_norm_l (Word32 L_var1) {
    Word16 var_out;

    if (L_var1 == 0) {
        var_out = 0;
    } else {
        if (L_var1 == (Word32) 0xffffffffL) {
            var_out = 31;
        } else {
            if (L_var1 < 0) {
                L_var1 = ~L_var1;
            }
            for (var_out = 0; L_var1 < (Word32) 0x40000000L; var_out++) {
                L_var1 <<= 1;
            }
        }
    }

#if (WMOPS)
    counter.efr_norm_l++;
#endif
    return (var_out);
}
