#ifdef __cplusplus
extern "C" {
#endif
/* Global counter variable for calculation of complexity weight */

typedef struct {
    Word32 efr_add;        /* Complexity Weight of 1 */
    Word32 efr_sub;
    Word32 efr_abs_s;
    Word32 efr_shl;
    Word32 efr_shr;
    Word32 efr_extract_h;
    Word32 efr_extract_l;
    Word32 efr_mult;
    Word32 efr_L_mult;
    Word32 efr_negate;
    Word32 gsm_efr_round;
    Word32 efr_L_mac;
    Word32 efr_L_msu;
    Word32 efr_L_macNs;
    Word32 efr_L_msuNs;
    Word32 efr_L_add;      /* Complexity Weight of 2 */
    Word32 efr_L_sub;
    Word32 efr_L_add_c;
    Word32 efr_L_sub_c;
    Word32 efr_L_negate;
    Word32 efr_L_shl;
    Word32 efr_L_shr;
    Word32 efr_mult_r;
    Word32 efr_shr_r;
    Word32 shift_r;
    Word32 efr_mac_r;
    Word32 efr_msu_r;
    Word32 efr_L_deposit_h;
    Word32 efr_L_deposit_l;
    Word32 efr_L_shr_r;    /* Complexity Weight of 3 */
    Word32 L_shift_r;
    Word32 efr_L_abs;
    Word32 efr_L_sat;      /* Complexity Weight of 4 */
    Word32 efr_norm_s;     /* Complexity Weight of 15 */
    Word32 efr_div_s;      /* Complexity Weight of 18 */
    Word32 efr_norm_l;     /* Complexity Weight of 30 */
    Word32 DataMove16; /* Complexity Weight of 1 */
    Word32 DataMove32; /* Complexity Weight of 2 */
    Word32 Logic16;    /* Complexity Weight of 1 */
    Word32 Logic32;    /* Complexity Weight of 2 */
    Word32 Test;       /* Complexity Weight of 2 */
}
BASIC_OP;

Word32 TotalWeightedOperation (void);
Word32 DeltaWeightedOperation (void);

void Init_WMOPS_counter (void);
void Reset_WMOPS_counter (void);
void WMOPS_output (Word16 dtx_mode);
Word32 fwc (void);

void move16 (void);
void move32 (void);
void logic16 (void);
void logic32 (void);
void test (void);
#ifdef __cplusplus
}
#endif