#ifdef __cplusplus
extern "C"{
#endif
#if defined(__BORLANDC__)
#include <dos.h>
extern unsigned _stklen = 32000U;

#endif

void proc_head (char *mes);
#ifdef __cplusplus
}
#endif