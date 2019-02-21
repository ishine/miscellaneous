/*___________________________________________________________________________
 |                                                                           |
 |   Constants and Globals                                                   |
 |___________________________________________________________________________|
*/
#include "typedef.h"
#ifndef BASIC_OP_H
#define BASIC_OP_H

#ifdef __cplusplus
extern "C" {
#endif

extern Flag efr_Overflow;
extern Flag efr_Carry;

#define MAX_32 (Word32)0x7fffffffL
#define MIN_32 (Word32)0x80000000L

#define MAX_16 (Word16)0x7fff
#define MIN_16 (Word16)0x8000

/*___________________________________________________________________________
 |                                                                           |
 |   Prototypes for basic arithmetic operators                               |
 |___________________________________________________________________________|
*/

Word16 efr_add (Word16 var1, Word16 var2);    /* Short efr_add,           1   */
Word16 efr_sub (Word16 var1, Word16 var2);    /* Short efr_sub,           1   */
Word16 efr_abs_s (Word16 var1);               /* Short abs,           1   */
Word16 efr_shl (Word16 var1, Word16 var2);    /* Short shift left,    1   */
Word16 efr_shr (Word16 var1, Word16 var2);    /* Short shift right,   1   */
Word16 efr_mult (Word16 var1, Word16 var2);   /* Short efr_mult,          1   */
Word32 efr_L_mult (Word16 var1, Word16 var2); /* Long efr_mult,           1   */
Word16 efr_negate (Word16 var1);              /* Short efr_negate,        1   */
Word16 efr_extract_h (Word32 L_var1);         /* Extract high,        1   */
Word16 efr_extract_l (Word32 L_var1);         /* Extract low,         1   */
Word16 gsm_efr_round (Word32 L_var1);             /* Round,               1   */
Word32 efr_L_mac (Word32 L_var3, Word16 var1, Word16 var2);   /* Mac,  1  */
Word32 efr_L_msu (Word32 L_var3, Word16 var1, Word16 var2);   /* Msu,  1  */
Word32 efr_L_macNs (Word32 L_var3, Word16 var1, Word16 var2); /* Mac without
                                                             sat, 1   */
Word32 efr_L_msuNs (Word32 L_var3, Word16 var1, Word16 var2); /* Msu without
                                                             sat, 1   */
Word32 efr_L_add (Word32 L_var1, Word32 L_var2);    /* Long efr_add,        2 */
Word32 efr_L_sub (Word32 L_var1, Word32 L_var2);    /* Long efr_sub,        2 */
Word32 efr_L_add_c (Word32 L_var1, Word32 L_var2);  /* Long efr_add with c, 2 */
Word32 efr_L_sub_c (Word32 L_var1, Word32 L_var2);  /* Long efr_sub with c, 2 */
Word32 efr_L_negate (Word32 L_var1);                /* Long efr_negate,     2 */
Word16 efr_mult_r (Word16 var1, Word16 var2);       /* Mult with gsm_efr_round, 2 */
Word32 efr_L_shl (Word32 L_var1, Word16 var2);      /* Long shift left, 2 */
Word32 efr_L_shr (Word32 L_var1, Word16 var2);      /* Long shift right, 2*/
Word16 efr_shr_r (Word16 var1, Word16 var2);        /* Shift right with
                                                   gsm_efr_round, 2           */
Word16 efr_mac_r (Word32 L_var3, Word16 var1, Word16 var2); /* Mac with
                                                           gsm_efr_rounding,2 */
Word16 efr_msu_r (Word32 L_var3, Word16 var1, Word16 var2); /* Msu with
                                                           gsm_efr_rounding,2 */
Word32 efr_L_deposit_h (Word16 var1);        /* 16 bit var1 -> MSB,     2 */
Word32 efr_L_deposit_l (Word16 var1);        /* 16 bit var1 -> LSB,     2 */

Word32 efr_L_shr_r (Word32 L_var1, Word16 var2); /* Long shift right with
                                                gsm_efr_round,  3             */
Word32 efr_L_abs (Word32 L_var1);            /* Long abs,              3  */
Word32 efr_L_sat (Word32 L_var1);            /* Long saturation,       4  */
Word16 efr_norm_s (Word16 var1);             /* Short norm,           15  */
Word16 efr_div_s (Word16 var1, Word16 var2); /* Short division,       18  */
Word16 efr_norm_l (Word32 L_var1);           /* Long norm,            30  */
#ifdef __cplusplus
}
#endif
#endif