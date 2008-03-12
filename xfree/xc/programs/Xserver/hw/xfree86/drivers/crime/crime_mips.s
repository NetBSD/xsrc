/* */
#include <mips/asm.h>

LEAF(mips3_sd)
        dsll    a2, a2, 32                      # high word in a2
        dsll    a3, a3, 32                      # low word in a3
        dsrl    a3, a3, 32
        or      a1, a2, a3
        sd      a1, 0(a0)
        j      ra
        nop
END(mips3_sd)
