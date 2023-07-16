#include "util/macros.h"
#include "disassemble.h"
#define _BITS(bits, pos, width) (((bits) >> (pos)) & ((1 << (width)) - 1))
static void
bi_disasm_fma_arshift_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    fputs("*ARSHIFT.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0x8 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_arshift_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    fputs("*ARSHIFT.v2i16", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0x8 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_arshift_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    fputs("*ARSHIFT.v2i16", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0x8 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_arshift_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    fputs("*ARSHIFT.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0x8 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_arshift_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    fputs("*ARSHIFT.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0x8 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_arshift_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    static const char *result_word_table[] = {
        "", ".w1"
    };

    const char *result_word = result_word_table[_BITS(bits, 11, 1)];

    fputs("*ARSHIFT_DOUBLE.i32", fp);
    fputs(result_word, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_atom_c_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".reserved", ".reserved", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 9, 4)];

    fputs("*ATOM_C.i32", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".aaddu", ".aadds", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 9, 4)];

    fputs("*ATOM_C.i64", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c1_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".ainc", ".adec", ".aumax1", ".asmax1", ".aor1", ".reserved", ".reserved", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 6, 3)];

    fputs("*ATOM_C1.i32", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c1_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".ainc", ".adec", ".aumax1", ".asmax1", ".aor1", ".reserved", ".reserved", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 6, 3)];

    fputs("*ATOM_C1.i64", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c1_return_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".ainc", ".adec", ".aumax1", ".asmax1", ".aor1", ".reserved", ".reserved", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 6, 3)];

    fputs("*ATOM_C1_RETURN.i32", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c1_return_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".ainc", ".adec", ".aumax1", ".asmax1", ".aor1", ".reserved", ".reserved", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 6, 3)];

    fputs("*ATOM_C1_RETURN.i64", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c_return_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".reserved", ".reserved", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 9, 4)];

    fputs("*ATOM_C_RETURN.i32", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_c_return_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".aaddu", ".aadds", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 9, 4)];

    fputs("*ATOM_C_RETURN.i64", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xf3 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_post_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".reserved", ".reserved", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 6, 4)];

    fputs("*ATOM_POST.i32", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_post_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".aaddu", ".aadds", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 6, 4)];

    fputs("*ATOM_POST.i64", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_atom_pre_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *atom_opc_table[] = {
        ".aaddu", ".aadds", ".aadd", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".asmin", ".asmax", ".aumin", ".aumax", ".aand", ".aor", ".axor", ".reserved"
    };

    const char *atom_opc = atom_opc_table[_BITS(bits, 9, 4)];

    fputs("*ATOM_PRE.i64", fp);
    fputs(atom_opc, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_bitrev_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*BITREV.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_clz_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *mask_table[] = {
        "", ".mask"
    };

    const char *mask = mask_table[_BITS(bits, 3, 1)];

    fputs("*CLZ.u32", fp);
    fputs(mask, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_clz_v2u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *mask_table[] = {
        "", ".mask"
    };

    const char *mask = mask_table[_BITS(bits, 3, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("*CLZ.v2u16", fp);
    fputs(mask, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(swz0, fp);
}

static void
bi_disasm_fma_clz_v4u8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *mask_table[] = {
        "", ".mask"
    };

    const char *mask = mask_table[_BITS(bits, 3, 1)];

    fputs("*CLZ.v4u8", fp);
    fputs(mask, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_csel_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".eq", ".gt", ".ge", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 2) << 0)];
    fputs("*CSEL.f32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".reserved", ".reserved", ".eq", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 3) << 0)];
    fputs("*CSEL.i32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 1) << 0)];
    fputs("*CSEL.s32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 1) << 0)];
    fputs("*CSEL.u32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".eq", ".gt", ".ge", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 2) << 0)];
    fputs("*CSEL.v2f16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".reserved", ".reserved", ".eq", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 3) << 0)];
    fputs("*CSEL.v2i16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_v2s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 1) << 0)];
    fputs("*CSEL.v2s16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_csel_v2u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 1) << 0)];
    fputs("*CSEL.v2u16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_cubeface1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *neg2_table[] = {
        "", ".neg"
    };
    const char *neg2 = neg2_table[(_BITS(bits, 9, 1) << 0)];
    fputs("*CUBEFACE1", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(neg1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
}

static void
bi_disasm_fma_dtsel_imm(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *table_table[] = {
        ".attribute_1", ".attribute_2", "", ".flat"
    };

    const char *table = table_table[_BITS(bits, 3, 2)];

    fputs("*DTSEL_IMM", fp);
    fputs(table, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_f16_to_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 3, 1)];

    fputs("*F16_TO_F32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
}

static void
bi_disasm_fma_fadd_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen1_table[] = {
        "", ".h0", ".h1", ".h0", ".h1", ".h1", "", ""
    };
    const char *widen1 = widen1_table[(_BITS(bits, 9, 3) << 0)];
    static const char *widen0_table[] = {
        "", "", "", ".h0", ".h0", ".h1", ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 9, 3) << 0)];
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 12, 1)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 13, 2)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 15, 2)];

    fputs("*FADD.f32", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(widen1, fp);
}

static void
bi_disasm_fma_fadd_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *abs0_0[] = {
        "", ".abs"
    };
    static const char *abs0_1[] = {
        ".abs", ".abs"
    };
    const char *abs0 = ordering ? abs0_1[(_BITS(bits, 6, 1) << 0)] : abs0_0[(_BITS(bits, 6, 1) << 0)];
    static const char *abs1_0[] = {
        "", ""
    };
    static const char *abs1_1[] = {
        "", ".abs"
    };
    const char *abs1 = ordering ? abs1_1[(_BITS(bits, 6, 1) << 0)] : abs1_0[(_BITS(bits, 6, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 13, 2)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 15, 2)];

    fputs("*FADD.v2f16", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(swz1, fp);
}

static void
bi_disasm_fma_fadd_lscale_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 8, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 9, 1)];

    fputs("*FADD_LSCALE.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(abs1, fp);
    fputs(neg1, fp);
}

static void
bi_disasm_fma_fcmp_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen1_table[] = {
        "", ".h0", ".h1", ".h0", ".h1", ".h1", "", ""
    };
    const char *widen1 = widen1_table[(_BITS(bits, 9, 3) << 0)];
    static const char *widen0_table[] = {
        "", "", "", ".h0", ".h0", ".h1", ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 9, 3) << 0)];
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 12, 1)];

    static const char *cmpf_table[] = {
        ".eq", ".gt", ".ge", ".ne", ".lt", ".le", ".gtlt", ".total"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 13, 3)];

    static const char *result_type_table[] = {
        "", ".f1", ".m1", ".reserved"
    };

    const char *result_type = result_type_table[_BITS(bits, 16, 2)];

    fputs("*FCMP.f32", fp);
    fputs(cmpf, fp);
    fputs(result_type, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(widen1, fp);
    fputs(abs1, fp);
    fputs(neg1, fp);
}

static void
bi_disasm_fma_fcmp_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *abs0_0[] = {
        "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".reserved"
    };
    static const char *abs0_1[] = {
        ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".abs", ".reserved", ".reserved"
    };
    const char *abs0 = ordering ? abs0_1[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 13, 3) << 1)] : abs0_0[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 13, 3) << 1)];
    static const char *cmpf_0[] = {
        ".eq", ".eq", ".gt", ".gt", ".ge", ".ge", ".ne", ".ne", ".lt", ".lt", ".le", ".le", ".gtlt", ".gtlt", ".total", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".eq", ".eq", ".gt", ".gt", ".ge", ".ge", ".ne", ".ne", ".lt", ".lt", ".le", ".le", ".gtlt", ".gtlt", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 13, 3) << 1)] : cmpf_0[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 13, 3) << 1)];
    static const char *abs1_0[] = {
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ".reserved"
    };
    static const char *abs1_1[] = {
        "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".abs", "", ".abs", ".reserved", ".reserved"
    };
    const char *abs1 = ordering ? abs1_1[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 13, 3) << 1)] : abs1_0[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 13, 3) << 1)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *result_type_table[] = {
        "", ".f1", ".m1", ".reserved"
    };

    const char *result_type = result_type_table[_BITS(bits, 16, 2)];

    fputs("*FCMP.v2f16", fp);
    fputs(cmpf, fp);
    fputs(result_type, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(swz1, fp);
}

static void
bi_disasm_fma_flshift_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    fputs("*FLSHIFT_DOUBLE.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_fma_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", "", "", "", "", "", "", "", ".neg", ".neg", ".neg", ".neg", ".neg", ".neg", ".neg", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 9, 3) << 0) | (_BITS(bits, 17, 1) << 3)];
    static const char *neg0_table[] = {
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
    };
    const char *neg0 = neg0_table[(_BITS(bits, 9, 3) << 0) | (_BITS(bits, 17, 1) << 3)];
    static const char *widen1_table[] = {
        "", ".h0", ".h1", ".h0", ".h1", ".h1", "", "", "", ".h0", ".h1", ".h0", ".h1", ".h1", "", ""
    };
    const char *widen1 = widen1_table[(_BITS(bits, 9, 3) << 0) | (_BITS(bits, 17, 1) << 3)];
    static const char *widen0_table[] = {
        "", "", "", ".h0", ".h0", ".h1", ".h0", ".h1", "", "", "", ".h0", ".h0", ".h1", ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 9, 3) << 0) | (_BITS(bits, 17, 1) << 3)];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 12, 1)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 13, 2)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 15, 2)];

    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 19, 1)];

    static const char *neg2_table[] = {
        "", ".neg"
    };

    const char *neg2 = neg2_table[_BITS(bits, 18, 1)];

    static const char *abs2_table[] = {
        "", ".abs"
    };

    const char *abs2 = abs2_table[_BITS(bits, 20, 1)];

    fputs("*FMA.f32", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(widen1, fp);
    fputs(neg1, fp);
    fputs(abs1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
    fputs(abs2, fp);
}

static void
bi_disasm_fma_fma_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 17, 1) << 0)];
    static const char *neg0_table[] = {
        "", ""
    };
    const char *neg0 = neg0_table[(_BITS(bits, 17, 1) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 13, 2)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 15, 2)];

    static const char *neg2_table[] = {
        "", ".neg"
    };

    const char *neg2 = neg2_table[_BITS(bits, 18, 1)];

    static const char *swz2_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz2 = swz2_table[_BITS(bits, 19, 2)];

    fputs("*FMA.v2f16", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(neg1, fp);
    fputs(swz1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
    fputs(swz2, fp);
}

static void
bi_disasm_fma_fma_rscale_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ".neg", "", ".neg", "", ".neg", "", ".neg", "", ".neg", "", ".neg", "", ".neg", "", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *neg0_table[] = {
        "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
    };
    const char *neg0 = neg0_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *round_table[] = {
        "", "", "", "", "", "", "", "", "", "", ".rtz", ".rtz", "", "", "", ""
    };
    const char *round = round_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *special_table[] = {
        "", "", "", "", "", "", "", "", ".n", ".n", ".n", ".n", ".scale16", ".scale16", ".left", ".left"
    };
    const char *special = special_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *clamp_table[] = {
        "", "", ".clamp_0_inf", ".clamp_0_inf", ".clamp_m1_1", ".clamp_m1_1", ".clamp_0_1", ".clamp_0_1", "", "", "", "", "", "", "", ""
    };
    const char *clamp = clamp_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 15, 1)];

    static const char *neg2_table[] = {
        "", ".neg"
    };

    const char *neg2 = neg2_table[_BITS(bits, 17, 1)];

    fputs("*FMA_RSCALE.f32", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(special, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(neg1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_fma_rscale_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ".neg", "", ".neg", "", ".neg", "", ".neg", "", ".neg", "", ".neg", ".reserved", ".reserved", "", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *neg0_table[] = {
        "", "", "", "", "", "", "", "", "", "", "", "", ".reserved", ".reserved", "", ""
    };
    const char *neg0 = neg0_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *round_table[] = {
        "", "", "", "", "", "", "", "", "", "", ".rtz", ".rtz", ".reserved", ".reserved", "", ""
    };
    const char *round = round_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *special_table[] = {
        "", "", "", "", "", "", "", "", ".n", ".n", ".n", ".n", ".reserved", ".reserved", ".left", ".left"
    };
    const char *special = special_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *clamp_table[] = {
        "", "", ".clamp_0_inf", ".clamp_0_inf", ".clamp_m1_1", ".clamp_m1_1", ".clamp_0_1", ".clamp_0_1", "", "", "", "", ".reserved", ".reserved", "", ""
    };
    const char *clamp = clamp_table[(_BITS(bits, 16, 1) << 0) | (_BITS(bits, 12, 3) << 1)];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 15, 1)];

    static const char *neg2_table[] = {
        "", ".neg"
    };

    const char *neg2 = neg2_table[_BITS(bits, 17, 1)];

    fputs("*FMA_RSCALE.v2f16", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(special, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(neg1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_fmul_cslice(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 6, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    fputs("*FMUL_CSLICE", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_fmul_slice_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*FMUL_SLICE.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_frexpe_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 8, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("*FREXPE.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_fma_frexpe_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("*FREXPE.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_fma_frexpe_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("*FREXPE.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_fma_frexpe_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("*FREXPE.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_fma_frexpm_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 7, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("*FREXPM.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_fma_frexpm_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    fputs("*FREXPM.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_fma_frexpm_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 7, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("*FREXPM.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(swz0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_fma_frexpm_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    fputs("*FREXPM.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(swz0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_fma_fround_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 9, 2) << 0)];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("*FROUND.f32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_fma_fround_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("*FROUND.f32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_fma_fround_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 9, 2) << 0)];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("*FROUND.v2f16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_fma_fround_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("*FROUND.v2f16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_fma_frshift_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    fputs("*FRSHIFT_DOUBLE.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_iaddc_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*IADDC.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_idp_v4i8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *sign0_table[] = {
        ".zext", ".sext"
    };

    const char *sign0 = sign0_table[_BITS(bits, 9, 1)];

    static const char *sign1_table[] = {
        ".zext", ".sext"
    };

    const char *sign1 = sign1_table[_BITS(bits, 10, 1)];

    fputs("*IDP.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(sign0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(sign1, fp);
}

static void
bi_disasm_fma_imul_i32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *extend_table[] = {
        ""
    };
    const char *extend = extend_table[0];
    static const char *widen1_table[] = {
        ""
    };
    const char *widen1 = widen1_table[0];
    fputs("*IMUL.i32", fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(widen1, fp);
}

static void
bi_disasm_fma_imul_i32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *extend_table[] = {
        ".zext", ".zext", ".sext", ".sext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *widen1_table[] = {
        ".h0", ".h1", ".h0", ".h1"
    };
    const char *widen1 = widen1_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    fputs("*IMUL.i32", fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(widen1, fp);
}

static void
bi_disasm_fma_imul_i32_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *extend_table[] = {
        ".zext", ".zext", ".zext", ".zext", ".sext", ".sext", ".sext", ".sext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 2) << 0) | (_BITS(bits, 11, 1) << 2)];
    static const char *widen1_table[] = {
        ".b0", ".b1", ".b2", ".b3", ".b0", ".b1", ".b2", ".b3"
    };
    const char *widen1 = widen1_table[(_BITS(bits, 9, 2) << 0) | (_BITS(bits, 11, 1) << 2)];
    fputs("*IMUL.i32", fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(widen1, fp);
}

static void
bi_disasm_fma_imul_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    fputs("*IMUL.v2i16", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(swz1, fp);
}

static void
bi_disasm_fma_imul_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *replicate0_table[] = {
        ""
    };
    const char *replicate0 = replicate0_table[0];
    static const char *replicate1_table[] = {
        ""
    };
    const char *replicate1 = replicate1_table[0];
    fputs("*IMUL.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(replicate0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(replicate1, fp);
}

static void
bi_disasm_fma_imul_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *replicate0_table[] = {
        "", "", "", ""
    };
    const char *replicate0 = replicate0_table[(_BITS(bits, 9, 2) << 0)];
    static const char *replicate1_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *replicate1 = replicate1_table[(_BITS(bits, 9, 2) << 0)];
    fputs("*IMUL.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(replicate0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(replicate1, fp);
}

static void
bi_disasm_fma_imuld(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *threads_table[] = {
        ".even", ""
    };

    const char *threads = threads_table[_BITS(bits, 6, 1)];

    fputs("*IMULD", fp);
    fputs(threads, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0x33 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0x33 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_isubb_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*ISUBB.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_jump_ex(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *test_mode_table[] = {
        "", ".nz"
    };

    const char *test_mode = test_mode_table[_BITS(bits, 9, 1)];

    static const char *stack_mode_table[] = {
        ".return", ".call", "", ".replace"
    };

    const char *stack_mode = stack_mode_table[_BITS(bits, 10, 2)];

    fputs("*JUMP_EX", fp);
    fputs(test_mode, fp);
    fputs(stack_mode, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_fma_lrot_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    static const char *result_word_table[] = {
        "", ".w1"
    };

    const char *result_word = result_word_table[_BITS(bits, 11, 1)];

    fputs("*LROT_DOUBLE.i32", fp);
    fputs(result_word, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_lshift_and_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_AND.i32", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_lshift_and_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_AND.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_and_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_AND.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_and_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_AND.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_and_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_AND.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    static const char *result_word_table[] = {
        "", ".w1"
    };

    const char *result_word = result_word_table[_BITS(bits, 11, 1)];

    fputs("*LSHIFT_DOUBLE.i32", fp);
    fputs(result_word, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_lshift_or_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_OR.i32", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_lshift_or_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_OR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_or_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_OR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_or_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_OR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_or_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*LSHIFT_OR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_xor_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*LSHIFT_XOR.i32", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_lshift_xor_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*LSHIFT_XOR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_xor_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*LSHIFT_XOR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_xor_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*LSHIFT_XOR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_lshift_xor_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*LSHIFT_XOR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_mkvec_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 6, 1)];

    static const char *lane1_table[] = {
        "", ".h1"
    };

    const char *lane1 = lane1_table[_BITS(bits, 7, 1)];

    fputs("*MKVEC.v2i16", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
}

static void
bi_disasm_fma_mkvec_v4i8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b2"
    };

    const char *lane0 = lane0_table[_BITS(bits, 12, 1)];

    static const char *lane1_table[] = {
        "", ".b2"
    };

    const char *lane1 = lane1_table[_BITS(bits, 13, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 14, 1)];

    static const char *lane3_table[] = {
        "", ".b2"
    };

    const char *lane3 = lane3_table[_BITS(bits, 15, 1)];

    fputs("*MKVEC.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
    fputs(lane3, fp);
}

static void
bi_disasm_fma_mov_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*MOV.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_nop(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*NOP", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
}

static void
bi_disasm_fma_popcount_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*POPCOUNT.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_quiet_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*QUIET.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_quiet_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("*QUIET.v2f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(swz0, fp);
}

static void
bi_disasm_fma_rrot_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    static const char *result_word_table[] = {
        "", ".w1"
    };

    const char *result_word = result_word_table[_BITS(bits, 11, 1)];

    fputs("*RROT_DOUBLE.i32", fp);
    fputs(result_word, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_rshift_and_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_AND.i32", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_rshift_and_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_AND.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_and_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_AND.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_and_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_AND.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_and_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    static const char *not1_table[] = {
        "", ".not"
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        ".not", ""
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_AND.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *bytes2_table[] = {
        "", ".bytes2"
    };

    const char *bytes2 = bytes2_table[_BITS(bits, 9, 1)];

    static const char *lane2_table[] = {
        "", ".b2"
    };

    const char *lane2 = lane2_table[_BITS(bits, 10, 1)];

    static const char *result_word_table[] = {
        "", ".w1"
    };

    const char *result_word = result_word_table[_BITS(bits, 11, 1)];

    fputs("*RSHIFT_DOUBLE.i32", fp);
    fputs(result_word, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(bytes2, fp);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_rshift_or_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_OR.i32", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_rshift_or_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_OR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_or_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_OR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_or_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_OR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_or_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    static const char *not1_table[] = {
        ".not", ""
    };

    const char *not1 = not1_table[_BITS(bits, 14, 1)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 15, 1)];

    fputs("*RSHIFT_OR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(not1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_xor_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane2_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane2 = lane2_table[_BITS(bits, 9, 2)];

    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*RSHIFT_XOR.i32", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lane2, fp);
}

static void
bi_disasm_fma_rshift_xor_v2i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b00", ".b11", ".b22", ".b33"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*RSHIFT_XOR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_xor_v2i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".reserved", ".b01", ".b23", ""
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*RSHIFT_XOR.v2i16", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_xor_v4i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes2 = lanes2_table[(_BITS(bits, 9, 2) << 0)];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*RSHIFT_XOR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_rshift_xor_v4i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes2_table[] = {
        ""
    };
    const char *lanes2 = lanes2_table[0];
    static const char *not_result_table[] = {
        "", ".not"
    };

    const char *not_result = not_result_table[_BITS(bits, 13, 1)];

    fputs("*RSHIFT_XOR.v4i8", fp);
    fputs(not_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(lanes2, fp);
}

static void
bi_disasm_fma_s16_to_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 1)];

    fputs("*S16_TO_S32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
}

static void
bi_disasm_fma_s8_to_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 2)];

    fputs("*S8_TO_S32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
}

static void
bi_disasm_fma_seg_add(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", ".reserved", ".wls", ".reserved", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 3, 3)];

    static const char *preserve_null_table[] = {
        "", ".preserve_null"
    };

    const char *preserve_null = preserve_null_table[_BITS(bits, 7, 1)];

    fputs("*SEG_ADD", fp);
    fputs(seg, fp);
    fputs(preserve_null, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_seg_sub(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", ".reserved", ".wls", ".reserved", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 3, 3)];

    static const char *preserve_null_table[] = {
        "", ".preserve_null"
    };

    const char *preserve_null = preserve_null_table[_BITS(bits, 7, 1)];

    fputs("*SEG_SUB", fp);
    fputs(seg, fp);
    fputs(preserve_null, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_fma_shaddxl_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("*SHADDXL.i64", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", shift:%u", _BITS(bits, 6, 3));
}

static void
bi_disasm_fma_shaddxl_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane1_table[] = {
        ".h0", ".h1", "", ".reserved"
    };

    const char *lane1 = lane1_table[_BITS(bits, 9, 2)];

    fputs("*SHADDXL.s32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
    fprintf(fp, ", shift:%u", _BITS(bits, 6, 3));
}

static void
bi_disasm_fma_shaddxl_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane1_table[] = {
        ".h0", ".h1", "", ".reserved"
    };

    const char *lane1 = lane1_table[_BITS(bits, 9, 2)];

    fputs("*SHADDXL.u32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
    fprintf(fp, ", shift:%u", _BITS(bits, 6, 3));
}

static void
bi_disasm_fma_u16_to_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 1)];

    fputs("*U16_TO_U32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
}

static void
bi_disasm_fma_u8_to_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 2)];

    fputs("*U8_TO_U32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
}

static void
bi_disasm_fma_v2f32_to_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", "", ".neg", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *abs0_table[] = {
        "", ".abs", "", ".abs"
    };
    const char *abs0 = abs0_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *neg0_table[] = {
        "", "", ".neg", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *abs1_table[] = {
        "", ".abs", "", ".abs"
    };
    const char *abs1 = abs1_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 8, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz", ".rtna", ".reserved", ".reserved", ".reserved"
    };

    const char *round = round_table[_BITS(bits, 10, 3)];

    fputs("*V2F32_TO_V2F16", fp);
    fputs(clamp, fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(abs1, fp);
    fputs(neg1, fp);
}

static void
bi_disasm_fma_vn_asst1_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *h_table[] = {
        "", ".h"
    };

    const char *h = h_table[_BITS(bits, 9, 1)];

    static const char *l_table[] = {
        "", ".l"
    };

    const char *l = l_table[_BITS(bits, 10, 1)];

    static const char *neg2_table[] = {
        "", ".neg"
    };

    const char *neg2 = neg2_table[_BITS(bits, 11, 1)];

    fputs("*VN_ASST1.f16", fp);
    fputs(h, fp);
    fputs(l, fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
}

static void
bi_disasm_fma_vn_asst1_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg2_table[] = {
        "", ".neg"
    };

    const char *neg2 = neg2_table[_BITS(bits, 12, 1)];

    fputs("*VN_ASST1.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_fma(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, true);
    if (!(0xfb & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, true);
    fputs(neg2, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 9, 3), *srcs, branch_offset, consts, true);
}

static void
bi_disasm_add_acmpstore_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        "", ".wls"
    };

    const char *seg = seg_table[_BITS(bits, 9, 1)];

    fputs("+ACMPSTORE.i32", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_acmpstore_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        "", ".wls"
    };

    const char *seg = seg_table[_BITS(bits, 9, 1)];

    fputs("+ACMPSTORE.i64", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_acmpxchg_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        "", ".wls"
    };

    const char *seg = seg_table[_BITS(bits, 9, 1)];

    fputs("+ACMPXCHG.i32", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_acmpxchg_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        "", ".wls"
    };

    const char *seg = seg_table[_BITS(bits, 9, 1)];

    fputs("+ACMPXCHG.i64", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_atest(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen1_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen1 = widen1_table[_BITS(bits, 6, 2)];

    fputs("+ATEST", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(widen1, fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_atom_cx(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+ATOM_CX", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_axchg_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        "", ".wls"
    };

    const char *seg = seg_table[_BITS(bits, 9, 1)];

    fputs("+AXCHG.i32", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_axchg_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        "", ".wls"
    };

    const char *seg = seg_table[_BITS(bits, 9, 1)];

    fputs("+AXCHG.i64", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_barrier(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+BARRIER", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
}

static void
bi_disasm_add_blend(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+BLEND", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_branch_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".eq", ".eq", ".ne", ".eq", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".gt", ".ge", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".lt", ".lt", ".le", ".lt", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".ne", ".ne", ".eq", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".ge", ".ge", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".le", ".le", ".lt", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.f16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".ne", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".eq", ".reserved", ".reserved", ".reserved", ".reserved", ".eq", ".eq", ".reserved", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".gt", ".reserved", ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".lt", ".lt", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".ne", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".reserved", ".reserved", ".reserved", ".reserved", ".eq", ".eq", ".reserved", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".gt", ".reserved", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".lt", ".lt", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", "", "", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.f32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".eq", ".eq", ".ne", ".eq", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".ne", ".ne", ".eq", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.i16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".eq", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.i32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".lt", ".lt", ".reserved", ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".le", ".reserved", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".ge", ".reserved", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".gt", ".reserved", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".h0", ".h1", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".h0", ".h1", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.s16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.s32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".reserved", ".reserved", ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".reserved", ".lt", ".lt", ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".le", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".ge", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".gt", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        ".reserved", ".h0", ".h1", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        ".reserved", ".h0", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h0", ".h1", ".h1", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.u16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *cmpf_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *cmpf_1[] = {
        ".lt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ge", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = ordering ? cmpf_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : cmpf_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen1_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen1_1[] = {
        "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = ordering ? widen1_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen1_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_0[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    static const char *widen0_1[] = {
        "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = ordering ? widen0_1[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)] : widen0_0[(_BITS(bits, 12, 3) << 0) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCH.u32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchc_i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        ".reserved", ".h1", "", ".reserved"
    };
    const char *lane0 = lane0_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 3, 1) << 1)];
    static const char *combine_table[] = {
        ".any", ".all"
    };

    const char *combine = combine_table[_BITS(bits, 10, 1)];

    fputs("+BRANCHC.i16", fp);
    fputs(combine, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchc_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *combine_table[] = {
        ".any", ".all"
    };

    const char *combine = combine_table[_BITS(bits, 10, 1)];

    fputs("+BRANCHC.i32", fp);
    fputs(combine, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".ne", ".reserved", ".reserved", ".eq", ".eq", ".reserved", ".reserved", ".ge", ".ge", ".reserved", ".reserved", ".gt", ".gt", ".reserved", ".reserved", ".le", ".le", ".reserved", ".reserved", ".lt", ".lt", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 3, 1) << 2) | (_BITS(bits, 9, 3) << 3)];
    static const char *widen0_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 3, 1) << 2) | (_BITS(bits, 9, 3) << 3)];
    fputs("+BRANCHZ.f16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".ne", ".eq", ".ge", ".gt", ".le", ".lt"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 9, 3) << 1)];
    fputs("+BRANCHZ.f32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".ne", ".ne", ".reserved", ".reserved", ".eq", ".eq", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 3, 1) << 2)];
    static const char *widen0_table[] = {
        ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 3, 1) << 2)];
    fputs("+BRANCHZ.i16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".ne", ".eq"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 3, 1) << 0)];
    fputs("+BRANCHZ.i32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".lt", ".lt", ".reserved", ".reserved", ".le", ".le", ".reserved", ".reserved", ".ge", ".ge", ".reserved", ".reserved", ".gt", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 9, 3) << 2)];
    static const char *widen0_table[] = {
        ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 9, 3) << 2)];
    fputs("+BRANCHZ.s16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".lt", ".le", ".ge", ".gt", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 9, 3) << 0)];
    fputs("+BRANCHZ.s32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".reserved", ".lt", ".lt", ".reserved", ".reserved", ".le", ".le", ".reserved", ".reserved", ".ge", ".ge", ".reserved", ".reserved", ".gt", ".gt", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 9, 3) << 2)];
    static const char *widen0_table[] = {
        ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".h1", ".h0", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 4, 2) << 0) | (_BITS(bits, 9, 3) << 2)];
    fputs("+BRANCHZ.u16", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branchz_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".lt", ".le", ".ge", ".gt", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 9, 3) << 0)];
    fputs("+BRANCHZ.u32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_diverg(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+BRANCH_DIVERG", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_lowbits_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+BRANCH_LOWBITS.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_branch_no_diverg(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+BRANCH_NO_DIVERG", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_clper_v6_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+CLPER_V6.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0x7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_clper_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_op_table[] = {
        "", ".xor", ".accumulate", ".shift"
    };

    const char *lane_op = lane_op_table[_BITS(bits, 6, 2)];

    static const char *subgroup_table[] = {
        ".subgroup2", ".subgroup4", ".subgroup8", ".reserved"
    };

    const char *subgroup = subgroup_table[_BITS(bits, 8, 2)];

    static const char *inactive_result_table[] = {
        ".zero", ".umax", ".i1", ".v2i1", ".smin", ".smax", ".v2smin", ".v2smax", ".v4smin", ".v4smax", ".f1", ".v2f1", ".infn", ".inf", ".v2infn", ".v2inf"
    };

    const char *inactive_result = inactive_result_table[_BITS(bits, 10, 4)];

    fputs("+CLPER.i32", fp);
    fputs(lane_op, fp);
    fputs(subgroup, fp);
    fputs(inactive_result, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0x7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_cubeface2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+CUBEFACE2", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_cube_ssel(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 9, 1) << 0)];
    fputs("+CUBE_SSEL", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(neg1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_cube_tsel(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 9, 1) << 0)];
    fputs("+CUBE_TSEL", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(neg1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_discard_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".eq", ".ne", ".lt", ".le", ".eq", ".ne", ".lt", ".le", ".eq", ".ne", ".lt", ".le", ".eq", ".ne", ".lt", ".le", ".eq", ".ne", ".lt", ".le", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 6, 2) << 0) | (_BITS(bits, 8, 3) << 2)];
    static const char *widen1_table[] = {
        ".h0", ".h0", ".h0", ".h0", ".h0", ".h0", ".h0", ".h0", ".h1", ".h1", ".h1", ".h1", ".h1", ".h1", ".h1", ".h1", "", "", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen1 = widen1_table[(_BITS(bits, 6, 2) << 0) | (_BITS(bits, 8, 3) << 2)];
    static const char *widen0_table[] = {
        ".h0", ".h0", ".h0", ".h0", ".h1", ".h1", ".h1", ".h1", ".h0", ".h0", ".h0", ".h0", ".h1", ".h1", ".h1", ".h1", "", "", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 6, 2) << 0) | (_BITS(bits, 8, 3) << 2)];
    fputs("+DISCARD.f32", fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
}

static void
bi_disasm_add_doorbell(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+DOORBELL", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_eureka(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+EUREKA", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_f16_to_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 3, 1)];

    fputs("+F16_TO_F32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_f16_to_s32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 7, 1)];

    fputs("+F16_TO_S32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_f16_to_s32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 5, 1)];

    fputs("+F16_TO_S32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_f16_to_u32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 7, 1)];

    fputs("+F16_TO_U32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_f16_to_u32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 5, 1)];

    fputs("+F16_TO_U32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_f32_to_s32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    fputs("+F32_TO_S32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_f32_to_s32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    fputs("+F32_TO_S32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_f32_to_u32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    fputs("+F32_TO_U32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_f32_to_u32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    fputs("+F32_TO_U32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_fadd_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz", "", ".rtp", ".rtn", ".rtz", "", ".rtp", ".rtn", ".rtz", "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 13, 2) << 0) | (_BITS(bits, 9, 2) << 2)];
    static const char *widen1_table[] = {
        "", "", "", "", ".h0", ".h0", ".h0", ".h0", ".h1", ".h1", ".h1", ".h1", ".h0", ".h0", ".h0", ".h0"
    };
    const char *widen1 = widen1_table[(_BITS(bits, 13, 2) << 0) | (_BITS(bits, 9, 2) << 2)];
    static const char *widen0_table[] = {
        "", "", "", "", "", "", "", "", "", "", "", "", ".h0", ".h0", ".h0", ".h0"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 13, 2) << 0) | (_BITS(bits, 9, 2) << 2)];
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 11, 2)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 15, 1)];

    fputs("+FADD.f32", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(widen1, fp);
}

static void
bi_disasm_add_fadd_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs0_table[] = {
        ""
    };
    const char *abs0 = abs0_table[0];
    static const char *round_table[] = {
        ".rto"
    };
    const char *round = round_table[0];
    static const char *clamp_table[] = {
        ""
    };
    const char *clamp = clamp_table[0];
    static const char *widen1_table[] = {
        ""
    };
    const char *widen1 = widen1_table[0];
    static const char *neg1_table[] = {
        ""
    };
    const char *neg1 = neg1_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *abs1_table[] = {
        ""
    };
    const char *abs1 = abs1_table[0];
    static const char *widen0_table[] = {
        ""
    };
    const char *widen0 = widen0_table[0];
    fputs("+FADD.f32", fp);
    fputs(round, fp);
    fputs(clamp, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(widen1, fp);
}

static void
bi_disasm_add_fadd_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 13, 2)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 15, 1)];

    fputs("+FADD.v2f16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(swz1, fp);
}

static void
bi_disasm_add_fadd_rscale_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".reserved", "", ".rtna", "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 9, 3) << 0)];
    static const char *special_table[] = {
        "", ".reserved", "", ".n", ".n", ".n", ".n", ".n"
    };
    const char *special = special_table[(_BITS(bits, 9, 3) << 0)];
    static const char *clamp_table[] = {
        "", ".reserved", ".clamp_0_1", "", "", "", "", ""
    };
    const char *clamp = clamp_table[(_BITS(bits, 9, 3) << 0)];
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 12, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 13, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 14, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 16, 1)];

    fputs("+FADD_RSCALE.f32", fp);
    fputs(clamp, fp);
    fputs(special, fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_fatan_assist_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane1_table[] = {
        "", ".h1"
    };

    const char *lane1 = lane1_table[_BITS(bits, 6, 1)];

    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 7, 1)];

    fputs("+FATAN_ASSIST.f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
}

static void
bi_disasm_add_fatan_assist_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FATAN_ASSIST.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_fatan_table_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane1_table[] = {
        "", ".h1"
    };

    const char *lane1 = lane1_table[_BITS(bits, 6, 1)];

    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 7, 1)];

    fputs("+FATAN_TABLE.f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
}

static void
bi_disasm_add_fatan_table_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FATAN_TABLE.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_fcmp_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", "", "", "", "", "", "", ""
    };
    const char *neg1 = neg1_table[(_BITS(bits, 9, 2) << 0) | (_BITS(bits, 13, 1) << 2)];
    static const char *neg0_table[] = {
        "", "", "", "", ".neg", ".neg", ".neg", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 9, 2) << 0) | (_BITS(bits, 13, 1) << 2)];
    static const char *widen1_table[] = {
        "", ".h0", ".h1", ".h0", "", ".h0", ".h1", ".h0"
    };
    const char *widen1 = widen1_table[(_BITS(bits, 9, 2) << 0) | (_BITS(bits, 13, 1) << 2)];
    static const char *widen0_table[] = {
        "", "", "", ".h0", "", "", "", ".h0"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 9, 2) << 0) | (_BITS(bits, 13, 1) << 2)];
    static const char *cmpf_table[] = {
        ".eq", ".gt", ".ge", ".ne", ".lt", ".le", ".gtlt", ".total"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 3)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 11, 1)];

    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 12, 1)];

    static const char *result_type_table[] = {
        "", ".f1", ".m1", ".reserved"
    };

    const char *result_type = result_type_table[_BITS(bits, 14, 2)];

    fputs("+FCMP.f32", fp);
    fputs(cmpf, fp);
    fputs(result_type, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(widen1, fp);
    fputs(neg1, fp);
    fputs(abs1, fp);
}

static void
bi_disasm_add_fcmp_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", ""
    };
    const char *neg1 = neg1_table[(_BITS(bits, 13, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 13, 1) << 0)];
    static const char *cmpf_table[] = {
        ".eq", ".gt", ".ge", ".ne", ".lt", ".le", ".gtlt", ".total"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 3)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *result_type_table[] = {
        "", ".f1", ".m1", ".reserved"
    };

    const char *result_type = result_type_table[_BITS(bits, 14, 2)];

    fputs("+FCMP.v2f16", fp);
    fputs(cmpf, fp);
    fputs(result_type, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(neg1, fp);
    fputs(swz1, fp);
}

static void
bi_disasm_add_fcos_table_u6(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *offset_table[] = {
        "", ".offset"
    };

    const char *offset = offset_table[_BITS(bits, 4, 1)];

    fputs("+FCOS_TABLE.u6", fp);
    fputs(offset, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_fexp_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FEXP.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_fexp_table_u4(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *adj_table[] = {
        "", ".small", ".low", ".reserved"
    };

    const char *adj = adj_table[_BITS(bits, 3, 2)];

    fputs("+FEXP_TABLE.u4", fp);
    fputs(adj, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_flogd_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FLOGD.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_flog_table_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *precision_table[] = {
        ""
    };
    const char *precision = precision_table[0];
    static const char *mode_table[] = {
        ".red"
    };
    const char *mode = mode_table[0];
    static const char *widen0_table[] = {
        ""
    };
    const char *widen0 = widen0_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FLOG_TABLE.f32", fp);
    fputs(mode, fp);
    fputs(precision, fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_flog_table_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *precision_table[] = {
        "", ""
    };
    const char *precision = precision_table[(_BITS(bits, 7, 1) << 0)];
    static const char *mode_table[] = {
        ".red", ".red"
    };
    const char *mode = mode_table[(_BITS(bits, 7, 1) << 0)];
    static const char *widen0_table[] = {
        ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 7, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FLOG_TABLE.f32", fp);
    fputs(mode, fp);
    fputs(precision, fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_flog_table_f32_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *divzero_table[] = {
        "", ""
    };
    const char *divzero = divzero_table[(_BITS(bits, 5, 1) << 0)];
    static const char *mode_table[] = {
        ".base2", ".natural"
    };
    const char *mode = mode_table[(_BITS(bits, 5, 1) << 0)];
    static const char *precision_table[] = {
        "", ""
    };
    const char *precision = precision_table[(_BITS(bits, 5, 1) << 0)];
    static const char *widen0_table[] = {
        "", ""
    };
    const char *widen0 = widen0_table[(_BITS(bits, 5, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    fputs("+FLOG_TABLE.f32", fp);
    fputs(mode, fp);
    fputs(precision, fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_flog_table_f32_3(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *divzero_table[] = {
        "", "", "", ""
    };
    const char *divzero = divzero_table[(_BITS(bits, 5, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *mode_table[] = {
        ".base2", ".natural", ".base2", ".natural"
    };
    const char *mode = mode_table[(_BITS(bits, 5, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *precision_table[] = {
        "", "", "", ""
    };
    const char *precision = precision_table[(_BITS(bits, 5, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *widen0_table[] = {
        ".h0", ".h0", ".h1", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 5, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    fputs("+FLOG_TABLE.f32", fp);
    fputs(mode, fp);
    fputs(precision, fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_flog_table_f32_4(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs0_table[] = {
        "", "", "", ""
    };
    const char *abs0 = abs0_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 4, 1) << 1)];
    static const char *divzero_table[] = {
        "", "", "", ""
    };
    const char *divzero = divzero_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 4, 1) << 1)];
    static const char *mode_table[] = {
        ".natural", ".base2", ".natural", ".base2"
    };
    const char *mode = mode_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 4, 1) << 1)];
    static const char *neg0_table[] = {
        "", "", "", ""
    };
    const char *neg0 = neg0_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 4, 1) << 1)];
    static const char *precision_table[] = {
        ".high", ".high", ".low", ".low"
    };
    const char *precision = precision_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 4, 1) << 1)];
    static const char *widen0_table[] = {
        "", "", "", ""
    };
    const char *widen0 = widen0_table[(_BITS(bits, 3, 1) << 0) | (_BITS(bits, 4, 1) << 1)];
    fputs("+FLOG_TABLE.f32", fp);
    fputs(mode, fp);
    fputs(precision, fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_fmax_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 11, 2)];

    static const char *sem_table[] = {
        "", ".nan_propagate", ".c", ".inverse_c"
    };

    const char *sem = sem_table[_BITS(bits, 13, 2)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 15, 1)];

    fputs("+FMAX.f32", fp);
    fputs(clamp, fp);
    fputs(sem, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
}

static void
bi_disasm_add_fmax_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *abs0_0[] = {
        "", ".abs"
    };
    static const char *abs0_1[] = {
        ".abs", ".abs"
    };
    const char *abs0 = ordering ? abs0_1[(_BITS(bits, 6, 1) << 0)] : abs0_0[(_BITS(bits, 6, 1) << 0)];
    static const char *abs1_0[] = {
        "", ""
    };
    static const char *abs1_1[] = {
        "", ".abs"
    };
    const char *abs1 = ordering ? abs1_1[(_BITS(bits, 6, 1) << 0)] : abs1_0[(_BITS(bits, 6, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *sem_table[] = {
        "", ".nan_propagate", ".c", ".inverse_c"
    };

    const char *sem = sem_table[_BITS(bits, 13, 2)];

    fputs("+FMAX.v2f16", fp);
    fputs(sem, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(swz1, fp);
}

static void
bi_disasm_add_fmin_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs1_table[] = {
        "", ".abs"
    };

    const char *abs1 = abs1_table[_BITS(bits, 6, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 11, 2)];

    static const char *sem_table[] = {
        "", ".nan_propagate", ".c", ".inverse_c"
    };

    const char *sem = sem_table[_BITS(bits, 13, 2)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 15, 1)];

    fputs("+FMIN.f32", fp);
    fputs(clamp, fp);
    fputs(sem, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
}

static void
bi_disasm_add_fmin_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    bool ordering = (_BITS(bits, 0, 3) > _BITS(bits, 3, 3));
    static const char *abs0_0[] = {
        "", ".abs"
    };
    static const char *abs0_1[] = {
        ".abs", ".abs"
    };
    const char *abs0 = ordering ? abs0_1[(_BITS(bits, 6, 1) << 0)] : abs0_0[(_BITS(bits, 6, 1) << 0)];
    static const char *abs1_0[] = {
        "", ""
    };
    static const char *abs1_1[] = {
        "", ".abs"
    };
    const char *abs1 = ordering ? abs1_1[(_BITS(bits, 6, 1) << 0)] : abs1_0[(_BITS(bits, 6, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    static const char *neg1_table[] = {
        "", ".neg"
    };

    const char *neg1 = neg1_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 9, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 11, 2)];

    static const char *sem_table[] = {
        "", ".nan_propagate", ".c", ".inverse_c"
    };

    const char *sem = sem_table[_BITS(bits, 13, 2)];

    fputs("+FMIN.v2f16", fp);
    fputs(sem, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
    fputs(swz1, fp);
}

static void
bi_disasm_add_fpclass_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 3, 1)];

    fputs("+FPCLASS.f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
}

static void
bi_disasm_add_fpclass_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FPCLASS.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_fpow_sc_apply(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FPOW_SC_APPLY", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_fpow_sc_det_f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane1_table[] = {
        ".h0", ".h1", ".h0", ".h1"
    };
    const char *lane1 = lane1_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 8, 1) << 1)];
    static const char *func_table[] = {
        ".pow", ".pow", ".powr", ".powr"
    };
    const char *func = func_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 8, 1) << 1)];
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 7, 1)];

    fputs("+FPOW_SC_DET.f16", fp);
    fputs(func, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
}

static void
bi_disasm_add_fpow_sc_det_f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane1_table[] = {
        "", ""
    };
    const char *lane1 = lane1_table[(_BITS(bits, 8, 1) << 0)];
    static const char *func_table[] = {
        ".pown", ".rootn"
    };
    const char *func = func_table[(_BITS(bits, 8, 1) << 0)];
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 7, 1)];

    fputs("+FPOW_SC_DET.f16", fp);
    fputs(func, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
    fputs(lane1, fp);
}

static void
bi_disasm_add_fpow_sc_det_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *func_table[] = {
        ".pow", ".powr", ".pown", ".rootn"
    };

    const char *func = func_table[_BITS(bits, 7, 2)];

    fputs("+FPOW_SC_DET.f32", fp);
    fputs(func, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 3, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_frcbrt_approx_a_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ""
    };
    const char *widen0 = widen0_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FRCBRT_APPROX_A.f32", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frcbrt_approx_a_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 7, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FRCBRT_APPROX_A.f32", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frcbrt_approx_b_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FRCBRT_APPROX_B.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_frcbrt_approx_c_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+FRCBRT_APPROX_C.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_frcp_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 8, 1)];

    fputs("+FRCP.f16", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(lane0, fp);
}

static void
bi_disasm_add_frcp_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        "", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 6, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    fputs("+FRCP.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frcp_approx_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ""
    };
    const char *widen0 = widen0_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FRCP_APPROX.f32", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frcp_approx_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 7, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FRCP_APPROX.f32", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frexpe_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 8, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("+FREXPE.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_add_frexpe_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("+FREXPE.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_add_frexpe_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("+FREXPE.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_add_frexpe_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("+FREXPE.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_add_frexpm_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 7, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("+FREXPM.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_add_frexpm_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    fputs("+FREXPM.f32", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_add_frexpm_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ""
    };
    const char *log = log_table[0];
    static const char *neg0_table[] = {
        ""
    };
    const char *neg0 = neg0_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *sqrt_table[] = {
        "", ".sqrt"
    };

    const char *sqrt = sqrt_table[_BITS(bits, 7, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("+FREXPM.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(swz0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_add_frexpm_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *log_table[] = {
        ".log"
    };
    const char *log = log_table[0];
    static const char *sqrt_table[] = {
        ""
    };
    const char *sqrt = sqrt_table[0];
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 6, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 7, 1)];

    fputs("+FREXPM.v2f16", fp);
    fputs(sqrt, fp);
    fputs(log, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(swz0, fp);
    fputs(neg0, fp);
}

static void
bi_disasm_add_fround_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 9, 2)];

    fputs("+FROUND.f32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(widen0, fp);
}

static void
bi_disasm_add_fround_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 7, 1)];

    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 8, 1)];

    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };

    const char *round = round_table[_BITS(bits, 9, 2)];

    fputs("+FROUND.v2f16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(swz0, fp);
}

static void
bi_disasm_add_frsq_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 8, 1)];

    fputs("+FRSQ.f16", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
    fputs(lane0, fp);
}

static void
bi_disasm_add_frsq_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        "", ".reserved"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 6, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    fputs("+FRSQ.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frsq_approx_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ""
    };
    const char *widen0 = widen0_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FRSQ_APPROX.f32", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_frsq_approx_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ".h0", ".h1"
    };
    const char *widen0 = widen0_table[(_BITS(bits, 7, 1) << 0)];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    static const char *abs0_table[] = {
        "", ".abs"
    };

    const char *abs0 = abs0_table[_BITS(bits, 4, 1)];

    static const char *divzero_table[] = {
        "", ".divzero"
    };

    const char *divzero = divzero_table[_BITS(bits, 5, 1)];

    fputs("+FRSQ_APPROX.f32", fp);
    fputs(divzero, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
    fputs(widen0, fp);
    fputs(neg0, fp);
    fputs(abs0, fp);
}

static void
bi_disasm_add_fsincos_offset_u6(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *scale_table[] = {
        "", ".scale"
    };

    const char *scale = scale_table[_BITS(bits, 3, 1)];

    fputs("+FSINCOS_OFFSET.u6", fp);
    fputs(scale, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_fsin_table_u6(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *offset_table[] = {
        "", ".offset"
    };

    const char *offset = offset_table[_BITS(bits, 4, 1)];

    fputs("+FSIN_TABLE.u6", fp);
    fputs(offset, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 0, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_hadd_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp"
    };

    const char *round = round_table[_BITS(bits, 12, 1)];

    fputs("+HADD.s32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_hadd_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp"
    };

    const char *round = round_table[_BITS(bits, 12, 1)];

    fputs("+HADD.u32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_hadd_v2s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp"
    };

    const char *round = round_table[_BITS(bits, 12, 1)];

    static const char *swap1_table[] = {
        "", ".h10"
    };

    const char *swap1 = swap1_table[_BITS(bits, 9, 1)];

    static const char *swap0_table[] = {
        "", ".h10"
    };

    const char *swap0 = swap0_table[_BITS(bits, 10, 1)];

    fputs("+HADD.v2s16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swap0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(swap1, fp);
}

static void
bi_disasm_add_hadd_v2u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp"
    };

    const char *round = round_table[_BITS(bits, 12, 1)];

    static const char *swap1_table[] = {
        "", ".h10"
    };

    const char *swap1 = swap1_table[_BITS(bits, 9, 1)];

    static const char *swap0_table[] = {
        "", ".h10"
    };

    const char *swap0 = swap0_table[_BITS(bits, 10, 1)];

    fputs("+HADD.v2u16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swap0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(swap1, fp);
}

static void
bi_disasm_add_hadd_v4s8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp"
    };

    const char *round = round_table[_BITS(bits, 12, 1)];

    fputs("+HADD.v4s8", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_hadd_v4u8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp"
    };

    const char *round = round_table[_BITS(bits, 12, 1)];

    fputs("+HADD.v4u8", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_iabs_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+IABS.s32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_iabs_v2s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+IABS.v2s16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_iabs_v4s8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+IABS.v4s8", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_iadd_s32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ""
    };
    const char *lanes1 = lanes1_table[0];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.s32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_s32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".h0", ".h1"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.s32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_s32_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b0", ".b1", ".b2", ".b3"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 2) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.s32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_u32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ""
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.u32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_u32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".h0", ".reserved", ".h1"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.u32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_u32_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b0", ".reserved", ".b1", ".reserved", ".b2", ".reserved", ".b3"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 2) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.u32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v2s16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        "", ".h10", "", ".h10"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *lanes0_table[] = {
        "", "", ".h10", ".h10"
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v2s16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v2s16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".h00", ".h11"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *lanes0_table[] = {
        "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v2s16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v2s16_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b01", ".b23"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *lanes0_table[] = {
        "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v2s16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v2u16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", "", ".reserved", ".h10", ".reserved", "", ".reserved", ".h10"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1) | (_BITS(bits, 10, 1) << 2)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", "", ".reserved", ".h10", ".reserved", ".h10"
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1) | (_BITS(bits, 10, 1) << 2)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v2u16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v2u16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".h00", ".reserved", ".h11"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v2u16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v2u16_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b01", ".reserved", ".b23"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v2u16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v4s8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ""
    };
    const char *lanes1 = lanes1_table[0];
    static const char *lanes0_table[] = {
        ""
    };
    const char *lanes0 = lanes0_table[0];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v4s8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v4s8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 2) << 0)];
    static const char *lanes0_table[] = {
        "", "", "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 2) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v4s8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v4s8_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b0101", ".b2323"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *lanes0_table[] = {
        "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v4s8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v4u8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ""
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0)];
    static const char *lanes0_table[] = {
        ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v4u8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v4u8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b0000", ".reserved", ".b1111", ".reserved", ".b2222", ".reserved", ".b3333"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 2) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", "", ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 2) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v4u8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_iadd_v4u8_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b0101", ".reserved", ".b2323"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+IADD.v4u8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_icmp_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    static const char *cmpf_table[] = {
        ".eq", ".ne"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 1)];

    fputs("+ICMP.i32", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmp_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 6, 1) << 0)];
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    fputs("+ICMP.s32", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmp_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 6, 1) << 0)];
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    fputs("+ICMP.u32", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmp_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 8, 2)];

    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    static const char *cmpf_table[] = {
        ".eq", ".ne"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 11, 1)];

    fputs("+ICMP.v2i16", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(swz1, fp);
}

static void
bi_disasm_add_icmp_v2s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 1) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 8, 2)];

    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    fputs("+ICMP.v2s16", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(swz1, fp);
}

static void
bi_disasm_add_icmp_v2u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 12, 1) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    static const char *swz1_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz1 = swz1_table[_BITS(bits, 8, 2)];

    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    fputs("+ICMP.v2u16", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(swz1, fp);
}

static void
bi_disasm_add_icmp_v4i8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    static const char *cmpf_table[] = {
        ".eq", ".ne"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 1)];

    fputs("+ICMP.v4i8", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmp_v4s8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 6, 1) << 0)];
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    fputs("+ICMP.v4s8", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmp_v4u8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };
    const char *cmpf = cmpf_table[(_BITS(bits, 6, 1) << 0)];
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    fputs("+ICMP.v4u8", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmpf_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+ICMPF.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmpi_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    static const char *cmpf_table[] = {
        ".eq", ".ne"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 1)];

    fputs("+ICMPI.i32", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmpi_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 1)];

    fputs("+ICMPI.s32", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmpi_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *result_type_table[] = {
        "", ".m1"
    };

    const char *result_type = result_type_table[_BITS(bits, 10, 1)];

    static const char *cmpf_table[] = {
        ".gt", ".ge"
    };

    const char *cmpf = cmpf_table[_BITS(bits, 6, 1)];

    fputs("+ICMPI.u32", fp);
    fputs(result_type, fp);
    fputs(cmpf, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_icmpm_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+ICMPM.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_ilogb_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("+ILOGB.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
}

static void
bi_disasm_add_ilogb_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("+ILOGB.v2f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_imov_fma(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *threads_table[] = {
        ".even", ""
    };

    const char *threads = threads_table[_BITS(bits, 3, 1)];

    fputs("+IMOV_FMA", fp);
    fputs(threads, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
}

static void
bi_disasm_add_isub_s32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ""
    };
    const char *lanes1 = lanes1_table[0];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.s32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_s32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".h0", ".h1"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.s32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_s32_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b0", ".b1", ".b2", ".b3"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 2) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.s32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_u32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ""
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.u32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_u32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".h0", ".reserved", ".h1"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.u32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_u32_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b0", ".reserved", ".b1", ".reserved", ".b2", ".reserved", ".b3"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 2) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.u32", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v2s16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        "", ".h10", "", ".h10"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *lanes0_table[] = {
        "", "", ".h10", ".h10"
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v2s16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v2s16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".h00", ".h11"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *lanes0_table[] = {
        "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v2s16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v2s16_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b01", ".b23"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *lanes0_table[] = {
        "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v2s16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v2u16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", "", ".reserved", ".h10", ".reserved", "", ".reserved", ".h10"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1) | (_BITS(bits, 10, 1) << 2)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", "", ".reserved", ".h10", ".reserved", ".h10"
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1) | (_BITS(bits, 10, 1) << 2)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v2u16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v2u16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".h00", ".reserved", ".h11"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v2u16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v2u16_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b01", ".reserved", ".b23"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v2u16", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v4s8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ""
    };
    const char *lanes1 = lanes1_table[0];
    static const char *lanes0_table[] = {
        ""
    };
    const char *lanes0 = lanes0_table[0];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v4s8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v4s8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 2) << 0)];
    static const char *lanes0_table[] = {
        "", "", "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 2) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v4s8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v4s8_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".b0101", ".b2323"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 9, 1) << 0)];
    static const char *lanes0_table[] = {
        "", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 9, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v4s8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v4u8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ""
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0)];
    static const char *lanes0_table[] = {
        ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v4u8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v4u8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b0000", ".reserved", ".b1111", ".reserved", ".b2222", ".reserved", ".b3333"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 2) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", "", ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 2) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v4u8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_isub_v4u8_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lanes1_table[] = {
        ".reserved", ".b0101", ".reserved", ".b2323"
    };
    const char *lanes1 = lanes1_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *lanes0_table[] = {
        ".reserved", "", ".reserved", ""
    };
    const char *lanes0 = lanes0_table[(_BITS(bits, 7, 1) << 0) | (_BITS(bits, 9, 1) << 1)];
    static const char *saturate_table[] = {
        "", ".sat"
    };

    const char *saturate = saturate_table[_BITS(bits, 8, 1)];

    fputs("+ISUB.v4u8", fp);
    fputs(saturate, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lanes0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lanes1, fp);
}

static void
bi_disasm_add_jump(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+JUMP", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
}

static void
bi_disasm_add_kaboom(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+KABOOM", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_ldexp_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz", ".rtna", ".reserved", ".inf", ".inf0"
    };

    const char *round = round_table[_BITS(bits, 6, 3)];

    fputs("+LDEXP.f32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_ldexp_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz", ".rtna", ".reserved", ".inf", ".inf0"
    };

    const char *round = round_table[_BITS(bits, 6, 3)];

    fputs("+LDEXP.v2f16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_ld_attr_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f16", ".f32", ".s32", ".u32", ".s16", ".u16", ".f64", ".i64"
    };
    const char *register_format = register_format_table[(_BITS(bits, 13, 3) << 0)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 11, 2)];

    fputs("+LD_ATTR", fp);
    fputs(register_format, fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_attr_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 11, 2)];

    fputs("+LD_ATTR", fp);
    fputs(register_format, fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_attr_imm_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f16", ".f32", ".s32", ".u32", ".s16", ".u16", ".f64", ".i64"
    };
    const char *register_format = register_format_table[(_BITS(bits, 13, 3) << 0)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 11, 2)];

    fputs("+LD_ATTR_IMM", fp);
    fputs(register_format, fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", attribute_index:%u", _BITS(bits, 6, 4));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_attr_imm_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 11, 2)];

    fputs("+LD_ATTR_IMM", fp);
    fputs(register_format, fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", attribute_index:%u", _BITS(bits, 6, 4));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_attr_tex_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f16", ".f32", ".s32", ".u32", ".s16", ".u16", ".f64", ".i64"
    };
    const char *register_format = register_format_table[(_BITS(bits, 13, 3) << 0)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 11, 2)];

    fputs("+LD_ATTR_TEX", fp);
    fputs(register_format, fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_attr_tex_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 11, 2)];

    fputs("+LD_ATTR_TEX", fp);
    fputs(register_format, fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_cvt(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 9, 2)];

    fputs("+LD_CVT", fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_gclk_u64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *source_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".system_timestamp", ".cycle_counter"
    };

    const char *source = source_table[_BITS(bits, 0, 3)];

    fputs("+LD_GCLK.u64", fp);
    fputs(source, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_tile(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 9, 2)];

    fputs("+LD_TILE", fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".store", ".store", ".store", ".store", ".store", ".store", ".store", ".store", ".retrieve", ".retrieve", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".conditional", ".conditional", ".conditional", ".conditional", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 19, 1) << 0) | (_BITS(bits, 10, 4) << 1)];
    static const char *sample_table[] = {
        ".center", ".center", ".centroid", ".centroid", ".sample", ".sample", ".explicit", ".explicit", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".center", ".center", ".centroid", ".centroid", ".center", ".center", ".centroid", ".centroid", ".sample", ".sample", ".explicit", ".explicit", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 19, 1) << 0) | (_BITS(bits, 10, 4) << 1)];
    static const char *register_format_table[] = {
        ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *register_format = register_format_table[(_BITS(bits, 19, 1) << 0) | (_BITS(bits, 10, 4) << 1)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    fputs("+LD_VAR", fp);
    fputs(vecsize, fp);
    fputs(update, fp);
    fputs(register_format, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".store", ".store", ".store", ".store", ".retrieve", ".reserved", ".reserved", ".reserved", ".conditional", ".conditional", ".clobber", ".clobber", ".clobber", ".clobber", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 10, 4) << 0)];
    static const char *sample_table[] = {
        ".center", ".centroid", ".sample", ".explicit", "", ".reserved", ".reserved", ".reserved", ".center", ".centroid", ".center", ".centroid", ".sample", ".explicit", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 10, 4) << 0)];
    static const char *register_format_table[] = {
        ".auto", ".auto", ".auto", ".auto", ".auto", ".reserved", ".reserved", ".reserved", ".auto", ".auto", ".auto", ".auto", ".auto", ".auto", ".reserved", ".reserved"
    };
    const char *register_format = register_format_table[(_BITS(bits, 10, 4) << 0)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    fputs("+LD_VAR", fp);
    fputs(vecsize, fp);
    fputs(update, fp);
    fputs(register_format, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_flat_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f32", ".u32", ".f16", ".s32"
    };
    const char *register_format = register_format_table[(_BITS(bits, 10, 1) << 0) | (_BITS(bits, 19, 1) << 1)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    static const char *function_table[] = {
        ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".and", ".or"
    };

    const char *function = function_table[_BITS(bits, 0, 3)];

    fputs("+LD_VAR_FLAT", fp);
    fputs(vecsize, fp);
    fputs(register_format, fp);
    fputs(function, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_flat_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    static const char *function_table[] = {
        ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".and", ".or"
    };

    const char *function = function_table[_BITS(bits, 0, 3)];

    fputs("+LD_VAR_FLAT", fp);
    fputs(vecsize, fp);
    fputs(register_format, fp);
    fputs(function, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_flat_imm_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f32", ".u32", ".f16", ".s32"
    };
    const char *register_format = register_format_table[(_BITS(bits, 10, 1) << 0) | (_BITS(bits, 19, 1) << 1)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    static const char *function_table[] = {
        ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".and", ".or"
    };

    const char *function = function_table[_BITS(bits, 0, 3)];

    fputs("+LD_VAR_FLAT_IMM", fp);
    fputs(vecsize, fp);
    fputs(register_format, fp);
    fputs(function, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fprintf(fp, ", index:%u", _BITS(bits, 3, 5));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_flat_imm_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    static const char *function_table[] = {
        ".reserved", ".reserved", ".reserved", "", ".reserved", ".reserved", ".and", ".or"
    };

    const char *function = function_table[_BITS(bits, 0, 3)];

    fputs("+LD_VAR_FLAT_IMM", fp);
    fputs(vecsize, fp);
    fputs(register_format, fp);
    fputs(function, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fprintf(fp, ", index:%u", _BITS(bits, 3, 5));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_imm_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".store", ".store", ".store", ".store", ".store", ".store", ".store", ".store", ".retrieve", ".retrieve", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".conditional", ".conditional", ".conditional", ".conditional", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".clobber", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 19, 1) << 0) | (_BITS(bits, 10, 4) << 1)];
    static const char *sample_table[] = {
        ".center", ".center", ".centroid", ".centroid", ".sample", ".sample", ".explicit", ".explicit", "", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".center", ".center", ".centroid", ".centroid", ".center", ".center", ".centroid", ".centroid", ".sample", ".sample", ".explicit", ".explicit", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 19, 1) << 0) | (_BITS(bits, 10, 4) << 1)];
    static const char *register_format_table[] = {
        ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".f32", ".f16", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *register_format = register_format_table[(_BITS(bits, 19, 1) << 0) | (_BITS(bits, 10, 4) << 1)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    fputs("+LD_VAR_IMM", fp);
    fputs(vecsize, fp);
    fputs(update, fp);
    fputs(register_format, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", index:%u", _BITS(bits, 3, 5));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_imm_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".store", ".store", ".store", ".store", ".retrieve", ".reserved", ".reserved", ".reserved", ".conditional", ".conditional", ".clobber", ".clobber", ".clobber", ".clobber", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 10, 4) << 0)];
    static const char *sample_table[] = {
        ".center", ".centroid", ".sample", ".explicit", "", ".reserved", ".reserved", ".reserved", ".center", ".centroid", ".center", ".centroid", ".sample", ".explicit", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 10, 4) << 0)];
    static const char *register_format_table[] = {
        ".auto", ".auto", ".auto", ".auto", ".auto", ".reserved", ".reserved", ".reserved", ".auto", ".auto", ".auto", ".auto", ".auto", ".auto", ".reserved", ".reserved"
    };
    const char *register_format = register_format_table[(_BITS(bits, 10, 4) << 0)];
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 8, 2)];

    fputs("+LD_VAR_IMM", fp);
    fputs(vecsize, fp);
    fputs(update, fp);
    fputs(register_format, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", index:%u", _BITS(bits, 3, 5));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_special_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".clobber", ".reserved", ".clobber", ".clobber", ".clobber", ".reserved", ".clobber", ".clobber", ".clobber", ".reserved", ".clobber", ".clobber", ".clobber", ".reserved", ".clobber", ".clobber", ".clobber", ".reserved", ".clobber", ".clobber", ".clobber", ".reserved", ".clobber", ".clobber", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 19, 1) << 2) | (_BITS(bits, 10, 4) << 3)];
    static const char *register_format_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".f32", ".reserved", ".f32", ".f32", ".f16", ".reserved", ".f16", ".f16", ".f32", ".reserved", ".f32", ".f32", ".f16", ".reserved", ".f16", ".f16", ".f32", ".reserved", ".f32", ".f32", ".f16", ".reserved", ".f16", ".f16", ".f32", ".reserved", ".f32", ".reserved", ".f16", ".reserved", ".f16", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *register_format = register_format_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 19, 1) << 2) | (_BITS(bits, 10, 4) << 3)];
    static const char *sample_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".center", ".reserved", ".center", ".center", ".center", ".reserved", ".center", ".center", ".centroid", ".reserved", ".centroid", ".centroid", ".centroid", ".reserved", ".centroid", ".centroid", ".sample", ".reserved", ".sample", ".sample", ".sample", ".reserved", ".sample", ".sample", ".explicit", ".reserved", ".explicit", ".reserved", ".explicit", ".reserved", ".explicit", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 19, 1) << 2) | (_BITS(bits, 10, 4) << 3)];
    static const char *varying_name_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".point", ".reserved", ".frag_w", ".frag_z", ".point", ".reserved", ".frag_w", ".frag_z", ".point", ".reserved", ".frag_w", ".frag_z", ".point", ".reserved", ".frag_w", ".frag_z", ".point", ".reserved", ".frag_w", ".frag_z", ".point", ".reserved", ".frag_w", ".frag_z", ".point", ".reserved", ".frag_w", ".reserved", ".point", ".reserved", ".frag_w", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *varying_name = varying_name_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 19, 1) << 2) | (_BITS(bits, 10, 4) << 3)];
    static const char *vecsize_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".v2", ".reserved", "", "", ".v2", ".reserved", "", "", ".v2", ".reserved", "", "", ".v2", ".reserved", "", "", ".v2", ".reserved", "", "", ".v2", ".reserved", "", "", ".v2", ".reserved", "", ".reserved", ".v2", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *vecsize = vecsize_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 19, 1) << 2) | (_BITS(bits, 10, 4) << 3)];
    fputs("+LD_VAR_SPECIAL", fp);
    fputs(varying_name, fp);
    fputs(vecsize, fp);
    fputs(update, fp);
    fputs(register_format, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_ld_var_special_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".clobber", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 10, 4) << 2)];
    static const char *register_format_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".auto", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *register_format = register_format_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 10, 4) << 2)];
    static const char *sample_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".center", ".reserved", ".center", ".reserved", ".centroid", ".reserved", ".centroid", ".reserved", ".sample", ".reserved", ".sample", ".reserved", ".explicit", ".reserved", ".explicit", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 10, 4) << 2)];
    static const char *varying_name_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".point", ".reserved", ".frag_w", ".reserved", ".point", ".reserved", ".frag_w", ".reserved", ".point", ".reserved", ".frag_w", ".reserved", ".point", ".reserved", ".frag_w", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *varying_name = varying_name_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 10, 4) << 2)];
    static const char *vecsize_table[] = {
        ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".v2", ".reserved", "", ".reserved", ".v2", ".reserved", "", ".reserved", ".v2", ".reserved", "", ".reserved", ".v2", ".reserved", "", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved", ".reserved"
    };
    const char *vecsize = vecsize_table[(_BITS(bits, 3, 2) << 0) | (_BITS(bits, 10, 4) << 2)];
    fputs("+LD_VAR_SPECIAL", fp);
    fputs(varying_name, fp);
    fputs(vecsize, fp);
    fputs(update, fp);
    fputs(register_format, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_attr_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f16", ".f32", ".s32", ".u32", ".s16", ".u16", ".f64", ".i64"
    };
    const char *register_format = register_format_table[(_BITS(bits, 11, 3) << 0)];
    fputs("+LEA_ATTR", fp);
    fputs(register_format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_attr_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    fputs("+LEA_ATTR", fp);
    fputs(register_format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_attr_imm_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f16", ".f32", ".s32", ".u32", ".s16", ".u16", ".f64", ".i64"
    };
    const char *register_format = register_format_table[(_BITS(bits, 11, 3) << 0)];
    fputs("+LEA_ATTR_IMM", fp);
    fputs(register_format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", attribute_index:%u", _BITS(bits, 6, 4));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_attr_imm_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    fputs("+LEA_ATTR_IMM", fp);
    fputs(register_format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", attribute_index:%u", _BITS(bits, 6, 4));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_attr_tex_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".f16", ".f32", ".s32", ".u32", ".s16", ".u16", ".f64", ".i64"
    };
    const char *register_format = register_format_table[(_BITS(bits, 11, 3) << 0)];
    fputs("+LEA_ATTR_TEX", fp);
    fputs(register_format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_attr_tex_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *register_format_table[] = {
        ".auto"
    };
    const char *register_format = register_format_table[0];
    fputs("+LEA_ATTR_TEX", fp);
    fputs(register_format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_tex(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *format_table[] = {
        ".u16", ".u32"
    };

    const char *format = format_table[_BITS(bits, 11, 1)];

    fputs("+LEA_TEX", fp);
    fputs(format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_lea_tex_imm(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *format_table[] = {
        ".u16", ".u32"
    };

    const char *format = format_table[_BITS(bits, 11, 1)];

    fputs("+LEA_TEX_IMM", fp);
    fputs(format, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", texture_index:%u", _BITS(bits, 6, 5));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i128(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i128", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        "", ".h1"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0)];
    static const char *extend_table[] = {
        "", ""
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i16", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ".w0", ".w0"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0)];
    static const char *extend_table[] = {
        ".sext", ".zext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i16", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i16_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ".d0", ".d0"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0)];
    static const char *extend_table[] = {
        ".sext", ".zext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i16", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i24(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i24", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ""
    };
    const char *lane_dest = lane_dest_table[0];
    static const char *extend_table[] = {
        ""
    };
    const char *extend = extend_table[0];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i32", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ".d0", ".d0"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0)];
    static const char *extend_table[] = {
        ".sext", ".zext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i32", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i48(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i48", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i64", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i8_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        "", ".b1", ".b2", ".b3"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 2) << 0)];
    static const char *extend_table[] = {
        "", "", "", ""
    };
    const char *extend = extend_table[(_BITS(bits, 9, 2) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i8", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i8_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ".h0", ".h0", ".h1", ".h1"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *extend_table[] = {
        ".sext", ".zext", ".sext", ".zext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0) | (_BITS(bits, 10, 1) << 1)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i8", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i8_2(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ".w0", ".w0"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0)];
    static const char *extend_table[] = {
        ".sext", ".zext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i8", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i8_3(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane_dest_table[] = {
        ".d0", ".d0"
    };
    const char *lane_dest = lane_dest_table[(_BITS(bits, 9, 1) << 0)];
    static const char *extend_table[] = {
        ".sext", ".zext"
    };
    const char *extend = extend_table[(_BITS(bits, 9, 1) << 0)];
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i8", fp);
    fputs(seg, fp);
    fputs(lane_dest, fp);
    fputs(extend, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_load_i96(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".ubo", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+LOAD.i96", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_logb_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *widen0_table[] = {
        ".reserved", "", ".h0", ".h1"
    };

    const char *widen0 = widen0_table[_BITS(bits, 3, 2)];

    fputs("+LOGB.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(widen0, fp);
}

static void
bi_disasm_add_logb_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 2)];

    fputs("+LOGB.v2f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_mkvec_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 6, 1)];

    static const char *lane1_table[] = {
        "", ".h1"
    };

    const char *lane1 = lane1_table[_BITS(bits, 7, 1)];

    fputs("+MKVEC.v2i16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(lane1, fp);
}

static void
bi_disasm_add_mov_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+MOV.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_mux_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *mux_table[] = {
        ".neg", "", ".fp_zero", ".bit"
    };

    const char *mux = mux_table[_BITS(bits, 9, 2)];

    fputs("+MUX.i32", fp);
    fputs(mux, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_mux_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *mux_table[] = {
        ".neg", "", ".fp_zero", ".bit"
    };

    const char *mux = mux_table[_BITS(bits, 9, 2)];

    static const char *swap2_table[] = {
        "", ".h10"
    };

    const char *swap2 = swap2_table[_BITS(bits, 11, 1)];

    static const char *swap1_table[] = {
        "", ".h10"
    };

    const char *swap1 = swap1_table[_BITS(bits, 12, 1)];

    static const char *swap0_table[] = {
        "", ".h10"
    };

    const char *swap0 = swap0_table[_BITS(bits, 13, 1)];

    fputs("+MUX.v2i16", fp);
    fputs(mux, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swap0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(swap1, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fputs(swap2, fp);
}

static void
bi_disasm_add_mux_v4i8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *mux_table[] = {
        ".neg", ""
    };

    const char *mux = mux_table[_BITS(bits, 9, 1)];

    fputs("+MUX.v4i8", fp);
    fputs(mux, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_nop(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+NOP", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
}

static void
bi_disasm_add_quiet_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+QUIET.f32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_quiet_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+QUIET.v2f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_s16_to_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 1)];

    fputs("+S16_TO_F32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_s16_to_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 1)];

    fputs("+S16_TO_S32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_s32_to_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    fputs("+S32_TO_F32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_s32_to_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    fputs("+S32_TO_F32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_s8_to_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 2)];

    fputs("+S8_TO_F32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_s8_to_s32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 2)];

    fputs("+S8_TO_S32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_seg_add(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", ".reserved", ".wls", ".reserved", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 3, 3)];

    static const char *preserve_null_table[] = {
        "", ".preserve_null"
    };

    const char *preserve_null = preserve_null_table[_BITS(bits, 7, 1)];

    fputs("+SEG_ADD", fp);
    fputs(seg, fp);
    fputs(preserve_null, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_seg_sub(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", ".reserved", ".wls", ".reserved", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 3, 3)];

    static const char *preserve_null_table[] = {
        "", ".preserve_null"
    };

    const char *preserve_null = preserve_null_table[_BITS(bits, 7, 1)];

    fputs("+SEG_SUB", fp);
    fputs(seg, fp);
    fputs(preserve_null, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_shaddxh_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+SHADDXH.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_shift_double_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("+SHIFT_DOUBLE.i32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_store_i128(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i128", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i16", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i24(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i24", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i32", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i48(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i48", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i64(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i64", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i8", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_store_i96(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *seg_table[] = {
        ".reserved", "", ".wls", ".stream", ".reserved", ".reserved", ".reserved", ".tl"
    };

    const char *seg = seg_table[_BITS(bits, 6, 3)];

    fputs("+STORE.i96", fp);
    fputs(seg, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_st_cvt(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 9, 2)];

    fputs("+ST_CVT", fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_st_tile(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *vecsize_table[] = {
        "", ".v2", ".v3", ".v4"
    };

    const char *vecsize = vecsize_table[_BITS(bits, 9, 2)];

    fputs("+ST_TILE", fp);
    fputs(vecsize, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_swz_v2i16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".h00", ".h10", ".reserved", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+SWZ.v2i16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_swz_v4i8(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".b0000", ".b1111", ".b2222", ".b3333", ".b0011", ".b2233", ".b1032", ".b3210"
    };

    const char *swz0 = swz0_table[_BITS(bits, 3, 3)];

    fputs("+SWZ.v4i8", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_texc(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 9, 1)];

    fputs("+TEXC", fp);
    fputs(skip, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    if (!(0xf7 & (1 << _BITS(bits, 6, 3)))) fputs("(INVALID)", fp);
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_texs_2d_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 9, 1)];

    static const char *lod_mode_table[] = {
        ".computed_lod", ""
    };

    const char *lod_mode = lod_mode_table[_BITS(bits, 13, 1)];

    fputs("+TEXS_2D.f16", fp);
    fputs(skip, fp);
    fputs(lod_mode, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", texture_index:%u", _BITS(bits, 6, 3));
    fprintf(fp, ", sampler_index:%u", _BITS(bits, 10, 3));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_texs_2d_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 9, 1)];

    static const char *lod_mode_table[] = {
        ".computed_lod", ""
    };

    const char *lod_mode = lod_mode_table[_BITS(bits, 13, 1)];

    fputs("+TEXS_2D.f32", fp);
    fputs(skip, fp);
    fputs(lod_mode, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", texture_index:%u", _BITS(bits, 6, 3));
    fprintf(fp, ", sampler_index:%u", _BITS(bits, 10, 3));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_texs_cube_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 9, 1)];

    fputs("+TEXS_CUBE.f16", fp);
    fputs(skip, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", sampler_index:%u", _BITS(bits, 10, 2));
    fprintf(fp, ", texture_index:%u", _BITS(bits, 12, 2));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_texs_cube_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 9, 1)];

    fputs("+TEXS_CUBE.f32", fp);
    fputs(skip, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", sampler_index:%u", _BITS(bits, 10, 2));
    fprintf(fp, ", texture_index:%u", _BITS(bits, 12, 2));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_u16_to_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 1)];

    fputs("+U16_TO_F32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_u16_to_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".h1"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 1)];

    fputs("+U16_TO_U32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_u32_to_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    fputs("+U32_TO_F32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_u32_to_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    fputs("+U32_TO_F32", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
}

static void
bi_disasm_add_u8_to_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 2)];

    fputs("+U8_TO_F32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_u8_to_u32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *lane0_table[] = {
        "", ".b1", ".b2", ".b3"
    };

    const char *lane0 = lane0_table[_BITS(bits, 4, 2)];

    fputs("+U8_TO_U32", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(lane0, fp);
}

static void
bi_disasm_add_v2f16_to_v2s16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    fputs("+V2F16_TO_V2S16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2f16_to_v2s16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+V2F16_TO_V2S16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2f16_to_v2u16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    fputs("+V2F16_TO_V2U16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2f16_to_v2u16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+V2F16_TO_V2U16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2f32_to_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg1_table[] = {
        "", "", ".neg", ".neg"
    };
    const char *neg1 = neg1_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *abs0_table[] = {
        "", ".abs", "", ".abs"
    };
    const char *abs0 = abs0_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *neg0_table[] = {
        "", "", ".neg", ".neg"
    };
    const char *neg0 = neg0_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *abs1_table[] = {
        "", ".abs", "", ".abs"
    };
    const char *abs1 = abs1_table[(_BITS(bits, 6, 1) << 0) | (_BITS(bits, 7, 1) << 1)];
    static const char *clamp_table[] = {
        "", ".clamp_0_inf", ".clamp_m1_1", ".clamp_0_1"
    };

    const char *clamp = clamp_table[_BITS(bits, 8, 2)];

    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz", ".rtna", ".reserved", ".reserved", ".reserved"
    };

    const char *round = round_table[_BITS(bits, 10, 3)];

    fputs("+V2F32_TO_V2F16", fp);
    fputs(clamp, fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(abs0, fp);
    fputs(neg0, fp);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(abs1, fp);
    fputs(neg1, fp);
}

static void
bi_disasm_add_v2s16_to_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    fputs("+V2S16_TO_V2F16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2s16_to_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+V2S16_TO_V2F16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2s8_to_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".b00", ".b10", ".b20", ".b30", "", ".b11", ".b21", ".b31", ".b02", ".b12", ".b22", ".b32", ".b03", ".b13", ".b23", ".b33"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 4)];

    fputs("+V2S8_TO_V2F16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2s8_to_v2s16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".b00", ".b10", ".b20", ".b30", "", ".b11", ".b21", ".b31", ".b02", ".b12", ".b22", ".b32", ".b03", ".b13", ".b23", ".b33"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 4)];

    fputs("+V2S8_TO_V2S16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2u16_to_v2f16_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        "", ".rtp", ".rtn", ".rtz"
    };
    const char *round = round_table[(_BITS(bits, 4, 2) << 0)];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 6, 2)];

    fputs("+V2U16_TO_V2F16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2u16_to_v2f16_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *round_table[] = {
        ".rtna"
    };
    const char *round = round_table[0];
    static const char *swz0_table[] = {
        ".h00", ".h10", "", ".h11"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 2)];

    fputs("+V2U16_TO_V2F16", fp);
    fputs(round, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2u8_to_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".b00", ".b10", ".b20", ".b30", "", ".b11", ".b21", ".b31", ".b02", ".b12", ".b22", ".b32", ".b03", ".b13", ".b23", ".b33"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 4)];

    fputs("+V2U8_TO_V2F16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_v2u8_to_v2u16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *swz0_table[] = {
        ".b00", ".b10", ".b20", ".b30", "", ".b11", ".b21", ".b31", ".b02", ".b12", ".b22", ".b32", ".b03", ".b13", ".b23", ".b33"
    };

    const char *swz0 = swz0_table[_BITS(bits, 4, 4)];

    fputs("+V2U8_TO_V2U16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(swz0, fp);
}

static void
bi_disasm_add_var_tex_f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".store", ".retrieve", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 5, 2) << 0)];
    static const char *sample_table[] = {
        ".center", "", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 5, 2) << 0)];
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 7, 1)];

    static const char *lod_mode_table[] = {
        ".computed_lod", ""
    };

    const char *lod_mode = lod_mode_table[_BITS(bits, 9, 1)];

    fputs("+VAR_TEX.f16", fp);
    fputs(update, fp);
    fputs(skip, fp);
    fputs(lod_mode, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fprintf(fp, ", varying_index:%u", _BITS(bits, 0, 3));
    fprintf(fp, ", texture_index:%u", _BITS(bits, 3, 2));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_var_tex_f32(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *update_table[] = {
        ".store", ".retrieve", ".reserved", ".reserved"
    };
    const char *update = update_table[(_BITS(bits, 5, 2) << 0)];
    static const char *sample_table[] = {
        ".center", "", ".reserved", ".reserved"
    };
    const char *sample = sample_table[(_BITS(bits, 5, 2) << 0)];
    static const char *skip_table[] = {
        "", ".skip"
    };

    const char *skip = skip_table[_BITS(bits, 7, 1)];

    static const char *lod_mode_table[] = {
        ".computed_lod", ""
    };

    const char *lod_mode = lod_mode_table[_BITS(bits, 9, 1)];

    fputs("+VAR_TEX.f32", fp);
    fputs(update, fp);
    fputs(skip, fp);
    fputs(lod_mode, fp);
    fputs(sample, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fprintf(fp, ", varying_index:%u", _BITS(bits, 0, 3));
    fprintf(fp, ", texture_index:%u", _BITS(bits, 3, 2));
    fprintf(fp, ", @r%u", staging_register);
}

static void
bi_disasm_add_vn_asst2_f32_0(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *scale_table[] = {
        ""
    };
    const char *scale = scale_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    fputs("+VN_ASST2.f32", fp);
    fputs(scale, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
}

static void
bi_disasm_add_vn_asst2_f32_1(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *scale_table[] = {
        ".scale"
    };
    const char *scale = scale_table[0];
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 4, 1)];

    fputs("+VN_ASST2.f32", fp);
    fputs(scale, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
}

static void
bi_disasm_add_vn_asst2_v2f16(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *neg0_table[] = {
        "", ".neg"
    };

    const char *neg0 = neg0_table[_BITS(bits, 3, 1)];

    fputs("+VN_ASST2.v2f16", fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(neg0, fp);
}

static void
bi_disasm_add_wmask(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *subgroup_table[] = {
        ".subgroup2", ".subgroup4", ".subgroup8", ".reserved"
    };

    const char *subgroup = subgroup_table[_BITS(bits, 4, 2)];

    fputs("+WMASK", fp);
    fputs(subgroup, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", fill:%u", _BITS(bits, 3, 1));
}

static void
bi_disasm_add_zs_emit(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    static const char *z_table[] = {
        ".reserved", "", ".z", ".z"
    };
    const char *z = z_table[(_BITS(bits, 9, 2) << 0)];
    static const char *stencil_table[] = {
        ".reserved", ".stencil", "", ".stencil"
    };
    const char *stencil = stencil_table[(_BITS(bits, 9, 2) << 0)];
    fputs("+ZS_EMIT", fp);
    fputs(stencil, fp);
    fputs(z, fp);
    fputs(" ", fp);
    bi_disasm_dest_add(fp, next_regs, last);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 0, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 3, 3), *srcs, branch_offset, consts, false);
    fputs(", ", fp);
    dump_src(fp, _BITS(bits, 6, 3), *srcs, branch_offset, consts, false);
    fprintf(fp, ", @r%u", staging_register);
}

void
bi_disasm_fma(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("    ", fp);

    if (unlikely(((bits & 0x7fffff) == 0x701963)))
        bi_disasm_fma_nop(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff8) == 0x701fc0)))
        bi_disasm_fma_bitrev_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff8) == 0x701968)))
        bi_disasm_fma_mov_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff8) == 0x73c6d8)))
        bi_disasm_fma_popcount_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff8) == 0x701970)))
        bi_disasm_fma_quiet_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff0) == 0x701fd0)))
        bi_disasm_fma_clz_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff0) == 0x701f90)))
        bi_disasm_fma_clz_v4u8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffff0) == 0x700d10)))
        bi_disasm_fma_f16_to_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffe8) == 0x700cc0)))
        bi_disasm_fma_s16_to_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffe8) == 0x700cc8)))
        bi_disasm_fma_u16_to_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffe0) == 0x70f3e0)))
        bi_disasm_fma_dtsel_imm(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffe0) == 0x701e20)))
        bi_disasm_fma_frexpe_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffe0) == 0x701e00)))
        bi_disasm_fma_frexpe_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc8) == 0x701900)))
        bi_disasm_fma_quiet_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc8) == 0x700b40)))
        bi_disasm_fma_s8_to_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc8) == 0x700b48)))
        bi_disasm_fma_u8_to_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc0) == 0x701ec0)))
        bi_disasm_fma_clz_v2u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc0) == 0x70cb40)))
        bi_disasm_fma_fmul_slice_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc0) == 0x73c0c0)))
        bi_disasm_fma_imul_i32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fffc0) == 0x73e0c0)))
        bi_disasm_fma_imul_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff80) == 0x70f100)))
        bi_disasm_fma_imuld(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff40) == 0x701500)))
        bi_disasm_fma_seg_add(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff40) == 0x701540)))
        bi_disasm_fma_seg_sub(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff20) == 0x701b20)))
        bi_disasm_fma_frexpm_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff20) == 0x701a20)))
        bi_disasm_fma_frexpm_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff20) == 0x701b00)))
        bi_disasm_fma_frexpm_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff20) == 0x701a00)))
        bi_disasm_fma_frexpm_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fff00) == 0x70f000)))
        bi_disasm_fma_mkvec_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffea0) == 0x701c20)))
        bi_disasm_fma_frexpe_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffea0) == 0x701c00)))
        bi_disasm_fma_frexpe_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe60) == 0x707620)))
        bi_disasm_fma_fround_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe60) == 0x707600)))
        bi_disasm_fma_fround_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe38) == 0x335818)))
        bi_disasm_fma_arshift_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x2f5e00)))
        bi_disasm_fma_atom_c1_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x2f1e00)))
        bi_disasm_fma_atom_c1_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x2f7e00)))
        bi_disasm_fma_atom_c1_return_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x2f3e00)))
        bi_disasm_fma_atom_c1_return_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x70d000)))
        bi_disasm_fma_fmul_cslice(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x27fc00)))
        bi_disasm_fma_iaddc_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x27fe00)))
        bi_disasm_fma_isubb_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffe00) == 0x70e600)))
        bi_disasm_fma_shaddxl_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffc00) == 0x6ee400)))
        bi_disasm_fma_atom_post_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffc00) == 0x6ee000)))
        bi_disasm_fma_atom_post_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffc00) == 0x706800)))
        bi_disasm_fma_cubeface1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ffc00) == 0x70f400)))
        bi_disasm_fma_fadd_lscale_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff9c0) == 0x73e8c0)))
        bi_disasm_fma_idp_v4i8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff9c0) == 0x73c8c0)))
        bi_disasm_fma_imul_i32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff9c0) == 0x7380c0)))
        bi_disasm_fma_imul_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff860) == 0x70c020)))
        bi_disasm_fma_fround_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff860) == 0x70c000)))
        bi_disasm_fma_fround_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff838) == 0x335018)))
        bi_disasm_fma_arshift_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff838) == 0x334818)))
        bi_disasm_fma_arshift_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff838) == 0x335818)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_arshift_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff838) == 0x334018)))
        bi_disasm_fma_arshift_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff800) == 0x33f800)))
        bi_disasm_fma_flshift_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff800) == 0x33f000)))
        bi_disasm_fma_frshift_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff800) == 0x70e800)))
        bi_disasm_fma_shaddxl_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff800) == 0x70e000)))
        bi_disasm_fma_shaddxl_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff1c0) == 0x73b0c0)))
        bi_disasm_fma_imul_i32_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x33e000)))
        bi_disasm_fma_arshift_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x2eb000)))
        bi_disasm_fma_jump_ex(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x33b000)))
        bi_disasm_fma_lrot_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x33c000)))
        bi_disasm_fma_lshift_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x33a000)))
        bi_disasm_fma_rrot_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x33d000)))
        bi_disasm_fma_rshift_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7ff000) == 0x6eb000)))
        bi_disasm_fma_vn_asst1_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe1c0) == 0x7240c0)))
        bi_disasm_fma_imul_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x2f4000)))
        bi_disasm_fma_atom_c_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x2f0000)))
        bi_disasm_fma_atom_c_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x2f6000)))
        bi_disasm_fma_atom_c_return_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x2f2000)))
        bi_disasm_fma_atom_c_return_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x6ec000)))
        bi_disasm_fma_atom_pre_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x2e4000)))
        bi_disasm_fma_csel_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x2e6000)))
        bi_disasm_fma_csel_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x6e4000)))
        bi_disasm_fma_csel_v2s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x6e6000)))
        bi_disasm_fma_csel_v2u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x6e8000)))
        bi_disasm_fma_v2f32_to_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fe000) == 0x27c000)))
        bi_disasm_fma_vn_asst1_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fde00) == 0x325800)))
        bi_disasm_fma_lshift_xor_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fde00) == 0x321800)))
        bi_disasm_fma_rshift_xor_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x325000)))
        bi_disasm_fma_lshift_xor_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x324800)))
        bi_disasm_fma_lshift_xor_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x325800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_lshift_xor_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x324000)))
        bi_disasm_fma_lshift_xor_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x321000)))
        bi_disasm_fma_rshift_xor_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x320800)))
        bi_disasm_fma_rshift_xor_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x321800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_rshift_xor_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fd800) == 0x320000)))
        bi_disasm_fma_rshift_xor_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fc000) == 0x2e0000)
        && !(0x8 & (1 << _BITS(bits, 12, 2)))
    ))
        bi_disasm_fma_csel_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7fc000) == 0x6e0000)
        && !(0x8 & (1 << _BITS(bits, 12, 2)))
    ))
        bi_disasm_fma_csel_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f8000) == 0x2e0000)
        && !(0xf7 & (1 << _BITS(bits, 12, 3)))
    ))
        bi_disasm_fma_csel_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f8000) == 0x6e0000)
        && !(0xf7 & (1 << _BITS(bits, 12, 3)))
    ))
        bi_disasm_fma_csel_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3e00) == 0x311800)))
        bi_disasm_fma_lshift_and_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3e00) == 0x313800)))
        bi_disasm_fma_lshift_or_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3e00) == 0x301800)))
        bi_disasm_fma_rshift_and_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3e00) == 0x303800)))
        bi_disasm_fma_rshift_or_v4i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x311000)))
        bi_disasm_fma_lshift_and_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x310800)))
        bi_disasm_fma_lshift_and_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x311800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_lshift_and_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x310000)))
        bi_disasm_fma_lshift_and_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x313000)))
        bi_disasm_fma_lshift_or_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x312800)))
        bi_disasm_fma_lshift_or_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x313800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_lshift_or_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x312000)))
        bi_disasm_fma_lshift_or_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x301000)))
        bi_disasm_fma_rshift_and_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x300800)))
        bi_disasm_fma_rshift_and_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x301800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_rshift_and_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x300000)))
        bi_disasm_fma_rshift_and_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x303000)))
        bi_disasm_fma_rshift_or_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x302800)))
        bi_disasm_fma_rshift_or_v2i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x303800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_fma_rshift_or_v2i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f3800) == 0x302000)))
        bi_disasm_fma_rshift_or_v4i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f0000) == 0x710000)))
        bi_disasm_fma_mkvec_v4i8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7e0000) == 0x2c0000)))
        bi_disasm_fma_fadd_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7e0000) == 0x6c0000)))
        bi_disasm_fma_fadd_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c0000) == 0x240000)))
        bi_disasm_fma_fcmp_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c0000) == 0x640000)))
        bi_disasm_fma_fcmp_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c0000) == 0x280000)))
        bi_disasm_fma_fma_rscale_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c0000) == 0x680000)
        && !(0x40 & (1 << _BITS(bits, 12, 3)))
    ))
        bi_disasm_fma_fma_rscale_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x600000) == 0x0)))
        bi_disasm_fma_fma_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x600000) == 0x400000)))
        bi_disasm_fma_fma_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else
        fprintf(fp, "INSTR_INVALID_ENC fma %X", bits);

    fputs("\n", fp);
}
void
bi_disasm_add(FILE *fp, unsigned bits, struct bifrost_regs *srcs, struct bifrost_regs *next_regs, unsigned staging_register, unsigned branch_offset, struct bi_constants *consts, bool last)
{
    fputs("    ", fp);

    if (unlikely(((bits & 0xfffff) == 0xd7874)))
        bi_disasm_add_barrier(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffff) == 0x3d964)))
        bi_disasm_add_nop(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3de58)))
        bi_disasm_add_cubeface2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0xd7860)))
        bi_disasm_add_doorbell(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0xd7850)))
        bi_disasm_add_eureka(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3cca0)))
        bi_disasm_add_f32_to_s32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3cca8)))
        bi_disasm_add_f32_to_u32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x66340)))
        bi_disasm_add_flogd_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x67c50)))
        bi_disasm_add_fpclass_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x67ab0)))
        bi_disasm_add_frcbrt_approx_b_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x67ab8)))
        bi_disasm_add_frcbrt_approx_c_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3dea0)))
        bi_disasm_add_iabs_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3deb0)))
        bi_disasm_add_iabs_v4s8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0xd7858)))
        bi_disasm_add_kaboom(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0xd7800)))
        bi_disasm_add_ld_gclk_u64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3d968)))
        bi_disasm_add_mov_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3d970)))
        bi_disasm_add_quiet_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3cd00)))
        bi_disasm_add_s32_to_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff8) == 0x3cd08)))
        bi_disasm_add_u32_to_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff7) == 0xd7820)))
        bi_disasm_add_imov_fma(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff0) == 0x3cd10)))
        bi_disasm_add_f16_to_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff0) == 0x67c40)))
        bi_disasm_add_fpclass_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff0) == 0x67aa0)))
        bi_disasm_add_fsincos_offset_u6(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff0) == 0x3df80)))
        bi_disasm_add_vn_asst2_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffff0) == 0x3dfa0)))
        bi_disasm_add_vn_asst2_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x67a88)))
        bi_disasm_add_fcos_table_u6(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x67a80)))
        bi_disasm_add_fsin_table_u6(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x3cce0)))
        bi_disasm_add_s16_to_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x3ccc0)))
        bi_disasm_add_s16_to_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x3cce8)))
        bi_disasm_add_u16_to_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x3ccc8)))
        bi_disasm_add_u16_to_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe8) == 0x3de80)))
        bi_disasm_add_vn_asst2_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x67ac0)))
        bi_disasm_add_fexp_table_u4(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x67ae0)))
        bi_disasm_add_flog_table_f32_4(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x3de20)))
        bi_disasm_add_frexpe_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x3de00)))
        bi_disasm_add_frexpe_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x3d9e0)))
        bi_disasm_add_ilogb_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x3d9c0)))
        bi_disasm_add_ilogb_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x3d9a0)))
        bi_disasm_add_logb_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffe0) == 0x3d980)))
        bi_disasm_add_logb_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffd8) == 0x3cc40)))
        bi_disasm_add_f16_to_s32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffd8) == 0x3cc48)))
        bi_disasm_add_f16_to_u32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3c980)))
        bi_disasm_add_f32_to_s32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3c988)))
        bi_disasm_add_f32_to_u32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3de88)))
        bi_disasm_add_iabs_v2s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3d900)))
        bi_disasm_add_quiet_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cbc0)))
        bi_disasm_add_s32_to_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cb80)))
        bi_disasm_add_s8_to_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cb40)))
        bi_disasm_add_s8_to_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3d948)))
        bi_disasm_add_swz_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cbc8)))
        bi_disasm_add_u32_to_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cb88)))
        bi_disasm_add_u8_to_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cb48)))
        bi_disasm_add_u8_to_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3ca80)))
        bi_disasm_add_v2f16_to_v2s16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3ca88)))
        bi_disasm_add_v2f16_to_v2u16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cb00)))
        bi_disasm_add_v2s16_to_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc8) == 0x3cb08)))
        bi_disasm_add_v2u16_to_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x3f0c0)))
        bi_disasm_add_clper_v6_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x75200)))
        bi_disasm_add_fadd_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67a00)))
        bi_disasm_add_fatan_assist_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67a40)))
        bi_disasm_add_fatan_table_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x66ac0)))
        bi_disasm_add_fexp_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67300)))
        bi_disasm_add_flog_table_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67b00)))
        bi_disasm_add_flog_table_f32_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x75080)))
        bi_disasm_add_fpow_sc_apply(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67200)))
        bi_disasm_add_frcbrt_approx_a_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67000)))
        bi_disasm_add_frcp_approx_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x67100)))
        bi_disasm_add_frsq_approx_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x3f8c0)))
        bi_disasm_add_shaddxh_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x3df40)))
        bi_disasm_add_swz_v4i8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffc0) == 0x3d700)))
        bi_disasm_add_wmask(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffa0) == 0x66000)
        && !(0x2 & (1 << _BITS(bits, 6, 1)))
    ))
        bi_disasm_add_frcp_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfffa0) == 0x66100)
        && !(0x2 & (1 << _BITS(bits, 6, 1)))
    ))
        bi_disasm_add_frsq_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff48) == 0x3c500)))
        bi_disasm_add_f16_to_s32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff48) == 0x3c508)))
        bi_disasm_add_f16_to_u32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x67340)))
        bi_disasm_add_flog_table_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x67b40)))
        bi_disasm_add_flog_table_f32_3(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x67240)))
        bi_disasm_add_frcbrt_approx_a_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x67040)))
        bi_disasm_add_frcp_approx_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x67140)))
        bi_disasm_add_frsq_approx_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x3d500)))
        bi_disasm_add_seg_add(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff40) == 0x3d540)))
        bi_disasm_add_seg_sub(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff20) == 0x3db20)))
        bi_disasm_add_frexpm_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff20) == 0x3da20)))
        bi_disasm_add_frexpm_f32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff20) == 0x3db00)))
        bi_disasm_add_frexpm_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff20) == 0x3da00)))
        bi_disasm_add_frexpm_v2f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c200)))
        bi_disasm_add_v2f16_to_v2s16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c208)))
        bi_disasm_add_v2f16_to_v2u16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c600)))
        bi_disasm_add_v2s16_to_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c800)))
        bi_disasm_add_v2s8_to_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c700)))
        bi_disasm_add_v2s8_to_v2s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c608)))
        bi_disasm_add_v2u16_to_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c808)))
        bi_disasm_add_v2u8_to_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff08) == 0x3c708)))
        bi_disasm_add_v2u8_to_v2u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff00) == 0xc8f00)))
        bi_disasm_add_atest(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff00) == 0x67800)))
        bi_disasm_add_fatan_assist_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff00) == 0x67900)))
        bi_disasm_add_fatan_table_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfff00) == 0x75300)))
        bi_disasm_add_mkvec_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffec0) == 0x67080)))
        bi_disasm_add_frcp_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffec0) == 0x67280)))
        bi_disasm_add_frsq_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffec0) == 0xbc600)))
        bi_disasm_add_iadd_s32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffec0) == 0xbc400)))
        bi_disasm_add_iadd_v4s8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffec0) == 0xbd600)))
        bi_disasm_add_isub_s32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffec0) == 0xbd400)))
        bi_disasm_add_isub_v4s8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffea0) == 0x3dc20)))
        bi_disasm_add_frexpe_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffea0) == 0x3dc00)))
        bi_disasm_add_frexpe_v2f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe40) == 0x67600)))
        bi_disasm_add_fpow_sc_det_f16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe40) == 0x67640)))
        bi_disasm_add_fpow_sc_det_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe40) == 0xbc600)))
        bi_disasm_add_iadd_u32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe40) == 0xbc400)))
        bi_disasm_add_iadd_v4u8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe40) == 0xbd600)))
        bi_disasm_add_isub_u32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe40) == 0xbd400)))
        bi_disasm_add_isub_v4u8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe3f) == 0x6f83c)))
        bi_disasm_add_branch_diverg(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe3f) == 0x6fa34)))
        bi_disasm_add_branch_no_diverg(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe3f) == 0x6fe34)))
        bi_disasm_add_jump(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe38) == 0x6fa38)))
        bi_disasm_add_branch_lowbits_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe30) == 0x6f800)))
        bi_disasm_add_branchz_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0xd7400)))
        bi_disasm_add_atom_cx(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0xca800)))
        bi_disasm_add_blend(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x6f800)
        && !(0x9 & (1 << _BITS(bits, 4, 2)))
    ))
        bi_disasm_add_branchz_i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x67400)))
        bi_disasm_add_fpow_sc_det_f16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x7be00)))
        bi_disasm_add_icmpf_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x7ba00)))
        bi_disasm_add_icmpm_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x74c00)))
        bi_disasm_add_ldexp_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x74e00)))
        bi_disasm_add_ldexp_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0xc8400)))
        bi_disasm_add_lea_attr_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0xc8600)))
        bi_disasm_add_lea_attr_tex_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x61000)))
        bi_disasm_add_load_i128(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x65000)))
        bi_disasm_add_load_i24(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x60c00)))
        bi_disasm_add_load_i32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x65200)))
        bi_disasm_add_load_i48(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x60e00)))
        bi_disasm_add_load_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x65400)))
        bi_disasm_add_load_i96(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0xefe00)))
        bi_disasm_add_shift_double_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x61200)))
        bi_disasm_add_store_i128(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x62800)))
        bi_disasm_add_store_i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x65800)))
        bi_disasm_add_store_i24(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x62c00)))
        bi_disasm_add_store_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x65a00)))
        bi_disasm_add_store_i48(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x62e00)))
        bi_disasm_add_store_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x62000)))
        bi_disasm_add_store_i8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffe00) == 0x65c00)))
        bi_disasm_add_store_i96(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffdc0) == 0x648c0)))
        bi_disasm_add_acmpstore_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffdc0) == 0x64900)))
        bi_disasm_add_acmpstore_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffdc0) == 0x644c0)))
        bi_disasm_add_acmpxchg_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffdc0) == 0x64500)))
        bi_disasm_add_acmpxchg_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffdc0) == 0x640c0)))
        bi_disasm_add_axchg_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffdc0) == 0x64100)))
        bi_disasm_add_axchg_i64(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffd00) == 0xca100)
        && !(0xc & (1 << _BITS(bits, 5, 2)))
    ))
        bi_disasm_add_var_tex_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffd00) == 0xca000)
        && !(0xc & (1 << _BITS(bits, 5, 2)))
    ))
        bi_disasm_add_var_tex_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbec00)))
        bi_disasm_add_iadd_s32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbec40)))
        bi_disasm_add_iadd_v2s16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbe800)))
        bi_disasm_add_iadd_v2s16_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbe840)))
        bi_disasm_add_iadd_v4s8_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbfc00)))
        bi_disasm_add_isub_s32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbfc40)))
        bi_disasm_add_isub_v2s16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbf800)))
        bi_disasm_add_isub_v2s16_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xbf840)))
        bi_disasm_add_isub_v4s8_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffcc0) == 0xcf8c0)))
        bi_disasm_add_ld_var_flat_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbec00)))
        bi_disasm_add_iadd_u32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbec40)))
        bi_disasm_add_iadd_v2u16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbe800)))
        bi_disasm_add_iadd_v2u16_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbe840)))
        bi_disasm_add_iadd_v4u8_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbfc00)))
        bi_disasm_add_isub_u32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbfc40)))
        bi_disasm_add_isub_v2u16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbf800)))
        bi_disasm_add_isub_v2u16_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc40) == 0xbf840)))
        bi_disasm_add_isub_v4u8_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x3e000)))
        bi_disasm_add_cube_ssel(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x3e400)))
        bi_disasm_add_cube_tsel(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0xcf800)))
        bi_disasm_add_ld_var_flat_imm_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0xc8000)))
        bi_disasm_add_lea_attr_imm_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x60800)))
        bi_disasm_add_load_i16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x63000)))
        bi_disasm_add_load_i16_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x61800)))
        bi_disasm_add_load_i16_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x61c00)))
        bi_disasm_add_load_i32_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x63400)))
        bi_disasm_add_load_i8_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x61400)))
        bi_disasm_add_load_i8_3(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0x74800)))
        bi_disasm_add_mux_v4i8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffc00) == 0xd7000)))
        bi_disasm_add_texc(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b300)))
        bi_disasm_add_icmp_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b200)))
        bi_disasm_add_icmp_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b280)))
        bi_disasm_add_icmp_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b100)))
        bi_disasm_add_icmp_v4i8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b000)))
        bi_disasm_add_icmp_v4s8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b080)))
        bi_disasm_add_icmp_v4u8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b900)))
        bi_disasm_add_icmpi_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b800)))
        bi_disasm_add_icmpi_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffb80) == 0x7b880)))
        bi_disasm_add_icmpi_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xffa38) == 0x6f238)))
        bi_disasm_add_branchc_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff8c0) == 0xbe000)))
        bi_disasm_add_iadd_s32_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff8c0) == 0xbc800)))
        bi_disasm_add_iadd_v2s16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff8c0) == 0xbe040)))
        bi_disasm_add_iadd_v4s8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff8c0) == 0xbf000)))
        bi_disasm_add_isub_s32_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff8c0) == 0xbd800)))
        bi_disasm_add_isub_v2s16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff8c0) == 0xbf040)))
        bi_disasm_add_isub_v4s8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff860) == 0x3e820)))
        bi_disasm_add_fround_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff860) == 0x3e800)))
        bi_disasm_add_fround_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff840) == 0xbe000)))
        bi_disasm_add_iadd_u32_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff840) == 0xbc800)))
        bi_disasm_add_iadd_v2u16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff840) == 0xbe040)))
        bi_disasm_add_iadd_v4u8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff840) == 0xbf000)))
        bi_disasm_add_isub_u32_2(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff840) == 0xbd800)))
        bi_disasm_add_isub_v2u16_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff840) == 0xbf040)))
        bi_disasm_add_isub_v4u8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff830) == 0x6f030)))
        bi_disasm_add_branchc_i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0xc8800)
        && !(0xe0 & (1 << _BITS(bits, 8, 3)))
    ))
        bi_disasm_add_discard_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0xc9000)))
        bi_disasm_add_ld_cvt(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0xcb000)))
        bi_disasm_add_ld_tile(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0x60000)))
        bi_disasm_add_load_i8_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0x63800)))
        bi_disasm_add_load_i8_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0x74000)))
        bi_disasm_add_mux_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0xc9800)))
        bi_disasm_add_st_cvt(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0xcb800)))
        bi_disasm_add_st_tile(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff800) == 0xd7800)
        && !(0x1 & (1 << _BITS(bits, 9, 2)))
    ))
        bi_disasm_add_zs_emit(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff600) == 0xd6600)))
        bi_disasm_add_lea_tex(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff038) == 0x6f008)
        && !(0xf0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branchz_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff038) == 0x6f000)
        && !(0xf0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branchz_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff030) == 0x6f000)
        && !(0x1f & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branchz_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff008) == 0x6f008)
        && !(0x9 & (1 << _BITS(bits, 4, 2)))
        && !(0xf0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branchz_s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff008) == 0x6f000)
        && !(0x9 & (1 << _BITS(bits, 4, 2)))
        && !(0xf0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branchz_u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff000) == 0x6f000)
        && !(0x9 & (1 << _BITS(bits, 4, 2)))
        && !(0x1f & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branchz_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff000) == 0x7a000)))
        bi_disasm_add_icmp_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xff000) == 0xd6000)))
        bi_disasm_add_lea_tex_imm(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfefc0) == 0xbc640)))
        bi_disasm_add_hadd_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfefc0) == 0xbc6c0)))
        bi_disasm_add_hadd_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfefc0) == 0xbc440)))
        bi_disasm_add_hadd_v4s8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfefc0) == 0xbc4c0)))
        bi_disasm_add_hadd_v4u8(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe9c0) == 0xbc840)))
        bi_disasm_add_hadd_v2s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe9c0) == 0xbc8c0)))
        bi_disasm_add_hadd_v2u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe800) == 0x78000)))
        bi_disasm_add_icmp_v2s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe800) == 0x78800)))
        bi_disasm_add_icmp_v2u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe600) == 0xc4400)))
        bi_disasm_add_ld_attr_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe600) == 0xc4600)))
        bi_disasm_add_ld_attr_tex_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe400) == 0xc4000)))
        bi_disasm_add_ld_attr_imm_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfe000) == 0x76000)))
        bi_disasm_add_v2f32_to_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc600) == 0xc0400)))
        bi_disasm_add_lea_attr_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc600) == 0xc0600)))
        bi_disasm_add_lea_attr_tex_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc400) == 0xc0000)))
        bi_disasm_add_lea_attr_imm_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc3e0) == 0xcc0a0)
        && !(0x2 & (1 << _BITS(bits, 3, 2)))
        && !(0xc0e0 & (1 << _BITS(bits, 10, 4)))
    ))
        bi_disasm_add_ld_var_special_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc0c0) == 0xcc0c0)
        && !(0xc0e0 & (1 << _BITS(bits, 10, 4)))
    ))
        bi_disasm_add_ld_var_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0x7c000)))
        bi_disasm_add_clper_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0xcc000)
        && !(0xc0e0 & (1 << _BITS(bits, 10, 4)))
    ))
        bi_disasm_add_ld_var_imm_1(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0x70000)))
        bi_disasm_add_mux_v2i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0xd8000)))
        bi_disasm_add_texs_2d_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0x58000)))
        bi_disasm_add_texs_2d_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0xdc000)))
        bi_disasm_add_texs_cube_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xfc000) == 0x5c000)))
        bi_disasm_add_texs_cube_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xe1 & (1 << _BITS(bits, 12, 3)))
        && !(0xf & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0x9e & (1 << _BITS(bits, 12, 3)))
        && !(0x1 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xe1 & (1 << _BITS(bits, 12, 3)))
        && !(0xed & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_i16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xfe & (1 << _BITS(bits, 12, 3)))
        && !(0xed & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_i32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xe9 & (1 << _BITS(bits, 12, 3)))
        && !(0xe0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_s16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xfe & (1 << _BITS(bits, 12, 3)))
        && !(0xe0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_s32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xf1 & (1 << _BITS(bits, 12, 3)))
        && !(0xf0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_u16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x68000)
        && !(0xfe & (1 << _BITS(bits, 12, 3)))
        && !(0xf0 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_branch_u32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x80000)))
        bi_disasm_add_fmax_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf8000) == 0x90000)))
        bi_disasm_add_fmin_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0600) == 0x0)))
        bi_disasm_add_fmax_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0600) == 0x10000)))
        bi_disasm_add_fmin_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0600) == 0x40400)))
        bi_disasm_add_ld_attr_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0600) == 0x40600)))
        bi_disasm_add_ld_attr_tex_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0400) == 0x40000)))
        bi_disasm_add_ld_attr_imm_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0000) == 0x20000)))
        bi_disasm_add_fadd_f32_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0000) == 0xa0000)))
        bi_disasm_add_fadd_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0000) == 0x30000)))
        bi_disasm_add_fcmp_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xf0000) == 0xb0000)))
        bi_disasm_add_fcmp_v2f16(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0xe8000) == 0x88000)
        && !(0x2 & (1 << _BITS(bits, 9, 3)))
    ))
        bi_disasm_add_fadd_rscale_f32(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f8c0) == 0x538c0)))
        bi_disasm_add_ld_var_flat_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7f800) == 0x53800)))
        bi_disasm_add_ld_var_flat_imm_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c3e0) == 0x500a0)
        && !(0x2 & (1 << _BITS(bits, 3, 2)))
        && !(0xc0e0 & (1 << _BITS(bits, 10, 4)))
    ))
        bi_disasm_add_ld_var_special_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c0c0) == 0x500c0)
        && !(0xc0e0 & (1 << _BITS(bits, 10, 4)))
    ))
        bi_disasm_add_ld_var_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else if (unlikely(((bits & 0x7c000) == 0x50000)
        && !(0xc0e0 & (1 << _BITS(bits, 10, 4)))
    ))
        bi_disasm_add_ld_var_imm_0(fp, bits, srcs, next_regs, staging_register, branch_offset, consts, last);
    else
        fprintf(fp, "INSTR_INVALID_ENC add %X", bits);

    fputs("\n", fp);
}
