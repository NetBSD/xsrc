; $Xorg: cr16.s,v 1.3 2000/08/17 19:48:22 cpqbld Exp $
        .SPACE  $TEXT$
        .SUBSPA $CODE$
        .export cr16
        .PROC
        .CALLINFO
        .ENTRY
cr16
        bv      (%rp)
	mfctl	16,%ret0
	.EXIT
        .PROCEND
