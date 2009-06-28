
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

#include <orc/orcprogram.h>
#include <orc/orcdebug.h>
#include <orc/orcsse.h>

#define SIZE 65536

/* sse rules */

void
orc_sse_emit_loadil (OrcCompiler *p, int reg, int value)
{
  if (value == 0) {
    orc_sse_emit_660f (p, "pxor", 0xef, reg, reg);
  } else {
    orc_x86_emit_mov_imm_reg (p, 4, value, p->gp_tmpreg);
    orc_x86_emit_mov_reg_sse (p, p->gp_tmpreg, reg);
    orc_sse_emit_pshufd (p, 0, reg, reg);
  }
}

void
orc_sse_emit_loadib (OrcCompiler *p, int reg, int value)
{
  value &= 0xff;
  value |= (value<<8);
  value |= (value<<16);
  orc_sse_emit_loadil (p, reg, value);
}

void
orc_sse_emit_loadiw (OrcCompiler *p, int reg, int value)
{
  value &= 0xffff;
  value |= (value<<16);
  orc_sse_emit_loadil (p, reg, value);
}

void
orc_sse_emit_loadpb (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_sse (p, 4, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_sse_emit_660f (p, "punpcklbw", 0x60, reg, reg);

  orc_sse_emit_pshuflw (p, 0, reg, reg);
  orc_sse_emit_pshufd (p, 0, reg, reg);
}

void
orc_sse_emit_loadpw (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_sse (p, 4, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_sse_emit_pshuflw (p, 0, reg, reg);
  orc_sse_emit_pshufd (p, 0, reg, reg);
}

void
orc_sse_emit_loadpl (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_sse (p, 4, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_sse_emit_pshufd (p, 0, reg, reg);
}

void
orc_sse_emit_loadpq (OrcCompiler *p, int reg, int param)
{
  orc_x86_emit_mov_memoffset_sse (p, 8, 
      (int)ORC_STRUCT_OFFSET(OrcExecutor, params[param]),
      p->exec_reg, reg, FALSE);

  orc_sse_emit_pshufd (p, 0, reg, reg);
}

static void
sse_rule_copyx (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "movdqa", 0x6f,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

#define UNARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_sse_emit_660f (p, insn_name, code, \
      p->vars[insn->src_args[0]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}

#define BINARY(opcode,insn_name,code) \
static void \
sse_rule_ ## opcode (OrcCompiler *p, void *user, OrcInstruction *insn) \
{ \
  orc_sse_emit_660f (p, insn_name, code, \
      p->vars[insn->src_args[1]].alloc, \
      p->vars[insn->dest_args[0]].alloc); \
}


UNARY(absb,"pabsb",0x381c)
BINARY(addb,"paddb",0xfc)
BINARY(addssb,"paddsb",0xec)
BINARY(addusb,"paddusb",0xdc)
BINARY(andb,"pand",0xdb)
BINARY(andnb,"pandn",0xdf)
BINARY(avgub,"pavgb",0xe0)
BINARY(cmpeqb,"pcmpeqb",0x74)
BINARY(cmpgtsb,"pcmpgtb",0x64)
BINARY(maxsb,"pmaxsb",0x383c)
BINARY(maxub,"pmaxub",0xde)
BINARY(minsb,"pminsb",0x3838)
BINARY(minub,"pminub",0xda)
//BINARY(mullb,"pmullb",0xd5)
//BINARY(mulhsb,"pmulhb",0xe5)
//BINARY(mulhub,"pmulhub",0xe4)
BINARY(orb,"por",0xeb)
//UNARY(signb,"psignb",0x3808)
BINARY(subb,"psubb",0xf8)
BINARY(subssb,"psubsb",0xe8)
BINARY(subusb,"psubusb",0xd8)
BINARY(xorb,"pxor",0xef)

UNARY(absw,"pabsw",0x381d)
BINARY(addw,"paddw",0xfd)
BINARY(addssw,"paddsw",0xed)
BINARY(addusw,"paddusw",0xdd)
BINARY(andw,"pand",0xdb)
BINARY(andnw,"pandn",0xdf)
BINARY(avguw,"pavgw",0xe3)
BINARY(cmpeqw,"pcmpeqw",0x75)
BINARY(cmpgtsw,"pcmpgtw",0x65)
BINARY(maxsw,"pmaxsw",0xee)
BINARY(maxuw,"pmaxuw",0x383e)
BINARY(minsw,"pminsw",0xea)
BINARY(minuw,"pminuw",0x383a)
BINARY(mullw,"pmullw",0xd5)
BINARY(mulhsw,"pmulhw",0xe5)
BINARY(mulhuw,"pmulhuw",0xe4)
BINARY(orw,"por",0xeb)
//UNARY(signw,"psignw",0x3809)
BINARY(subw,"psubw",0xf9)
BINARY(subssw,"psubsw",0xe9)
BINARY(subusw,"psubusw",0xd9)
BINARY(xorw,"pxor",0xef)

UNARY(absl,"pabsd",0x381e)
BINARY(addl,"paddd",0xfe)
//BINARY(addssl,"paddsd",0xed)
//BINARY(addusl,"paddusd",0xdd)
BINARY(andl,"pand",0xdb)
BINARY(andnl,"pandn",0xdf)
//BINARY(avgul,"pavgd",0xe3)
BINARY(cmpeql,"pcmpeqd",0x76)
BINARY(cmpgtsl,"pcmpgtd",0x66)
BINARY(maxsl,"pmaxsd",0x383d)
BINARY(maxul,"pmaxud",0x383f)
BINARY(minsl,"pminsd",0x3839)
BINARY(minul,"pminud",0x383b)
BINARY(mulll,"pmulld",0x3840)
//BINARY(mulhsl,"pmulhd",0xe5)
//BINARY(mulhul,"pmulhud",0xe4)
BINARY(orl,"por",0xeb)
//UNARY(signl,"psignd",0x380a)
BINARY(subl,"psubd",0xfa)
//BINARY(subssl,"psubsd",0xe9)
//BINARY(subusl,"psubusd",0xd9)
BINARY(xorl,"pxor",0xef)


static void
sse_rule_accw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_sse_emit_660f (p, "paddw", 0xfd, src, dest);
}

static void
sse_rule_accl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  if (p->loop_shift == 0) {
    orc_sse_emit_shiftimm (p, "pslldq", 0x73, 7, 12, src);
  }
  orc_sse_emit_660f (p, "paddd", 0xfe, src, dest);
}

static void
sse_rule_accsadubl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src1 = p->vars[insn->src_args[0]].alloc;
  int src2 = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_sse_emit_660f (p, "movdqa", 0x6f, src1, p->tmpreg);
  orc_sse_emit_660f (p, "psadbw", 0xf6, src2, p->tmpreg);
  orc_sse_emit_660f (p, "paddd", 0xfe, p->tmpreg, dest);
}

static void
sse_rule_signX (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int imm_vals[] = { 0x01010101, 0x00010001, 0x00000001 };
  const char * names[] = { "psignb", "psignw", "psignd" };
  int codes[] = { 0x3808, 0x3809, 0x380a };

  if (src == dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, p->tmpreg);
    src = p->tmpreg;
  }

  orc_x86_emit_mov_imm_reg (p, 4, imm_vals[ORC_PTR_TO_INT(user)], p->gp_tmpreg);

  orc_x86_emit_mov_reg_sse (p, p->gp_tmpreg, dest);
  orc_sse_emit_pshufd (p, 0, dest, dest);

  orc_sse_emit_660f (p, names[ORC_PTR_TO_INT(user)], codes[ORC_PTR_TO_INT(user)], src, dest);
}

static void
sse_rule_shift (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int type = ORC_PTR_TO_INT(user);
  int imm_code1[] = { 0x71, 0x71, 0x71, 0x72, 0x72, 0x72 };
  int imm_code2[] = { 6, 2, 4, 6, 2, 4 };
  int reg_code[] = { 0xf1, 0xd1, 0xe1, 0xf2, 0xd2, 0xe2 };
  const char *code[] = { "psllw", "psrlw", "psraw", "pslld", "psrld", "psrad" };

  if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_CONST) {
    orc_sse_emit_shiftimm (p, code[type], imm_code1[type], imm_code2[type],
        p->vars[insn->src_args[1]].value,
        p->vars[insn->dest_args[0]].alloc);
  } else if (p->vars[insn->src_args[1]].vartype == ORC_VAR_TYPE_PARAM) {
    /* FIXME this is a gross hack to reload the register with a
     * 64-bit version of the parameter. */
    orc_x86_emit_mov_memoffset_sse (p, 4, 
        (int)ORC_STRUCT_OFFSET(OrcExecutor, params[insn->src_args[1]]),
        p->exec_reg, p->tmpreg, FALSE);

    orc_sse_emit_660f (p, code[type], reg_code[type],
        p->tmpreg,
        p->vars[insn->dest_args[0]].alloc);
  } else {
    ORC_COMPILER_ERROR(p,"rule only works with constants or params");
  }
}

static void
sse_rule_convsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "punpcklbw", 0x60,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);

  orc_sse_emit_shiftimm (p, "psraw", 0x71, 4, 8,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* FIXME need a zero register */

  if (0) {
    orc_sse_emit_660f (p, "punpcklbw", 0x60,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->dest_args[0]].alloc);

    orc_sse_emit_shiftimm (p, "psrlw", 0x71, 2, 8,
        p->vars[insn->dest_args[0]].alloc);
  } else {
    if (p->vars[insn->src_args[0]].alloc !=
        p->vars[insn->dest_args[0]].alloc) {
      orc_sse_emit_660f (p, "movdqa", 0x6f,
          p->vars[insn->src_args[0]].alloc,
          p->vars[insn->dest_args[0]].alloc);
    }
    orc_sse_emit_660f (p, "pxor", 0xef,
        p->tmpreg, p->tmpreg);
    orc_sse_emit_660f (p, "punpcklbw", 0x60,
        p->tmpreg,
        p->vars[insn->dest_args[0]].alloc);
  }
}

static void
sse_rule_convssswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "packsswb", 0x63,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convsuswb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "packuswb", 0x67,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convwb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_shiftimm (p, "psllw", 0x71, 6, 8,
      p->vars[insn->dest_args[0]].alloc);
  orc_sse_emit_shiftimm (p, "psrlw", 0x71, 2, 8,
      p->vars[insn->dest_args[0]].alloc);
  orc_sse_emit_660f (p, "packuswb", 0x67,
      p->vars[insn->dest_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "punpcklwd", 0x61,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);

  orc_sse_emit_shiftimm (p, "psrad", 0x72, 4, 16,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convuwl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  /* FIXME need a zero register */

  if (0) {
    orc_sse_emit_660f (p, "punpcklwd", 0x61,
        p->vars[insn->src_args[0]].alloc,
        p->vars[insn->dest_args[0]].alloc);
    orc_sse_emit_shiftimm (p, "psrld", 0x72, 2, 16,
        p->vars[insn->dest_args[0]].alloc);
  } else {
    if (p->vars[insn->src_args[0]].alloc !=
        p->vars[insn->dest_args[0]].alloc) {
      orc_sse_emit_660f (p, "movdqa", 0x6f,
          p->vars[insn->src_args[0]].alloc,
          p->vars[insn->dest_args[0]].alloc);
    }
    orc_sse_emit_660f (p, "pxor", 0xef,
        p->tmpreg, p->tmpreg);
    orc_sse_emit_660f (p, "punpcklwd", 0x61,
        p->tmpreg,
        p->vars[insn->dest_args[0]].alloc);
  }
}

static void
sse_rule_convlw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_shiftimm (p, "pslld", 0x72, 6, 16,
      p->vars[insn->dest_args[0]].alloc);
  orc_sse_emit_shiftimm (p, "psrad", 0x72, 4, 16,
      p->vars[insn->dest_args[0]].alloc);
  orc_sse_emit_660f (p, "packssdw", 0x6b, dest, dest);
}

static void
sse_rule_convssslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "packssdw", 0x6b,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_convsuslw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  orc_sse_emit_660f (p, "packusdw", 0x382b,
      p->vars[insn->src_args[0]].alloc,
      p->vars[insn->dest_args[0]].alloc);
}

static void
sse_rule_mulsbw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "punpcklbw", 0x60, src, tmp);
  orc_sse_emit_shiftimm (p, "psraw", 0x71, 4, 8, tmp);
  orc_sse_emit_660f (p, "punpcklbw", 0x60, dest, dest);
  orc_sse_emit_shiftimm (p, "psraw", 0x71, 4, 8, dest);
  orc_sse_emit_660f (p, "pmullw", 0xd5, tmp, dest);
}

static void
sse_rule_mulubw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "punpcklbw", 0x60, src, tmp);
  orc_sse_emit_shiftimm (p, "psrlw", 0x71, 2, 8, tmp);
  orc_sse_emit_660f (p, "punpcklbw", 0x60, dest, dest);
  orc_sse_emit_shiftimm (p, "psrlw", 0x71, 2, 8, dest);
  orc_sse_emit_660f (p, "pmullw", 0xd5, tmp, dest);
}

static void
sse_rule_mulswl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  orc_sse_emit_660f (p, "pmulhw", 0xe5, src, tmp);
  orc_sse_emit_660f (p, "pmullw", 0xd5, src, dest);
  orc_sse_emit_660f (p, "punpcklwd", 0x61, tmp, dest);
}

static void
sse_rule_select0lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */
  /* same as convlw */

  if (dest != src) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_shiftimm (p, "pslld", 0x72, 6, 16, dest);
  orc_sse_emit_shiftimm (p, "psrad", 0x72, 4, 16, dest);
  orc_sse_emit_660f (p, "packssdw", 0x6b, dest, dest);
}

static void
sse_rule_select1lw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_shiftimm (p, "psrad", 0x72, 4, 16, dest);
  orc_sse_emit_660f (p, "packssdw", 0x6b, dest, dest);
}

static void
sse_rule_select0wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */
  /* same as convwb */

  if (dest != src) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_shiftimm (p, "psllw", 0x71, 6, 8, dest);
  orc_sse_emit_shiftimm (p, "psraw", 0x71, 4, 8, dest);
  orc_sse_emit_660f (p, "packsswb", 0x63, dest, dest);
}

static void
sse_rule_select1wb (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  /* FIXME slow */

  if (dest != src) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_shiftimm (p, "psraw", 0x71, 4, 8, dest);
  orc_sse_emit_660f (p, "packsswb", 0x63, dest, dest);
}

static void
sse_rule_mergebw (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_sse_emit_660f (p, "punpcklbw", 0x60, src, dest);
}

static void
sse_rule_mergewl (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;

  orc_sse_emit_660f (p, "punpcklwd", 0x61, src, dest);
}

static void
sse_emit_load_mask (OrcCompiler *p, unsigned int mask1, unsigned int mask2)
{
  int tmp = p->tmpreg;
  int tmp2 = X86_XMM7;

  orc_x86_emit_mov_imm_reg (p, 4, mask1, p->gp_tmpreg);
  orc_x86_emit_mov_reg_sse (p, p->gp_tmpreg, p->tmpreg);
  orc_sse_emit_pshufd (p, 0, tmp, tmp);
  orc_x86_emit_mov_imm_reg (p, 4, mask2, p->gp_tmpreg);
  orc_x86_emit_mov_reg_sse (p, p->gp_tmpreg, tmp2);
  orc_sse_emit_660f (p, "punpcklbw", 0x60, tmp2, tmp2);
  orc_sse_emit_660f (p, "punpcklwd", 0x61, tmp2, tmp2);
  orc_sse_emit_660f (p, "paddb", 0xfc, tmp2, tmp);
}

static void
sse_rule_select0lw_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  sse_emit_load_mask (p, 0x05040100, 0x08000800);

  if (src != dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
sse_rule_select1lw_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  sse_emit_load_mask (p, 0x07060302, 0x08000800);

  if (src != dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
sse_rule_select0wb_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  sse_emit_load_mask (p, 0x06040200, 0x08000800);

  if (src != dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
sse_rule_select1wb_sse3 (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  sse_emit_load_mask (p, 0x07050301, 0x08000800);

  if (src != dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }

  orc_sse_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

/* slow rules */

static void
sse_rule_swapw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  sse_emit_load_mask (p, 0x02030001, 0x0c080400);

  if (src != dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }
  orc_sse_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
sse_rule_swapl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[0]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;
  
  /* FIXME slow */

  sse_emit_load_mask (p, 0x00010203, 0x0c080400);

  if (src != dest) {
    orc_sse_emit_660f (p, "movdqa", 0x6f, src, dest);
  }
  orc_sse_emit_660f (p, "pshufb", 0x3800, tmp, dest);
}

static void
sse_rule_maxuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_x86_emit_mov_imm_reg (p, 4, 0x80008000, p->gp_tmpreg);
  orc_x86_emit_mov_reg_sse (p, p->gp_tmpreg, tmp);
  orc_sse_emit_pshufd (p, 0, tmp, tmp);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  orc_sse_emit_660f (p, "pmaxsw", 0xee, src, dest);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_minuw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_loadiw (p, tmp, 0x8000);

  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  orc_sse_emit_660f (p, "pminsw", 0xea, src, dest);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_avgsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_loadib (p, tmp, 0x80);

  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  orc_sse_emit_660f (p, "pavgb", 0xe0, src, dest);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_avgsw_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_loadiw (p, tmp, 0x8000);

  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
  orc_sse_emit_660f (p, "pavgw", 0xe3, src, dest);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_maxsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  orc_sse_emit_660f (p, "pcmpgtb", 0x64, src, tmp);
  orc_sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  orc_sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  orc_sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_minsb_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "movdqa", 0x6f, src, tmp);
  orc_sse_emit_660f (p, "pcmpgtb", 0x64, dest, tmp);
  orc_sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  orc_sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  orc_sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_maxsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  orc_sse_emit_660f (p, "pcmpgtd", 0x66, src, tmp);
  orc_sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  orc_sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  orc_sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_minsl_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_660f (p, "movdqa", 0x6f, src, tmp);
  orc_sse_emit_660f (p, "pcmpgtd", 0x66, dest, tmp);
  orc_sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  orc_sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  orc_sse_emit_660f (p, "por", 0xeb, tmp, dest);
}

static void
sse_rule_maxul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_loadil (p, tmp, 0x80000000);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);

  orc_sse_emit_660f (p, "movdqa", 0x6f, dest, tmp);
  orc_sse_emit_660f (p, "pcmpgtd", 0x66, src, tmp);
  orc_sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  orc_sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  orc_sse_emit_660f (p, "por", 0xeb, tmp, dest);

  orc_sse_emit_loadil (p, tmp, 0x80000000);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}

static void
sse_rule_minul_slow (OrcCompiler *p, void *user, OrcInstruction *insn)
{
  int src = p->vars[insn->src_args[1]].alloc;
  int dest = p->vars[insn->dest_args[0]].alloc;
  int tmp = p->tmpreg;

  orc_sse_emit_loadil (p, tmp, 0x80000000);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);

  orc_sse_emit_660f (p, "movdqa", 0x6f, src, tmp);
  orc_sse_emit_660f (p, "pcmpgtd", 0x66, dest, tmp);
  orc_sse_emit_660f (p, "pand", 0xdb, tmp, dest);
  orc_sse_emit_660f (p, "pandn", 0xdf, src, tmp);
  orc_sse_emit_660f (p, "por", 0xeb, tmp, dest);

  orc_sse_emit_loadil (p, tmp, 0x80000000);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, src);
  orc_sse_emit_660f (p, "pxor", 0xef, tmp, dest);
}


void
orc_compiler_sse_register_rules (OrcTarget *target)
{
  OrcRuleSet *rule_set;

#define REG(x) \
  orc_rule_register (rule_set, #x , sse_rule_ ## x, NULL)

  /* SSE 2 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_SSE_SSE2);

  REG(addb);
  REG(addssb);
  REG(addusb);
  REG(andb);
  REG(andnb);
  REG(avgub);
  REG(cmpeqb);
  REG(cmpgtsb);
  REG(maxub);
  REG(minub);
  REG(orb);
  REG(subb);
  REG(subssb);
  REG(subusb);
  REG(xorb);

  REG(addw);
  REG(addssw);
  REG(addusw);
  REG(andw);
  REG(andnw);
  REG(avguw);
  REG(cmpeqw);
  REG(cmpgtsw);
  REG(maxsw);
  REG(minsw);
  REG(mullw);
  REG(mulhsw);
  REG(mulhuw);
  REG(orw);
  REG(subw);
  REG(subssw);
  REG(subusw);
  REG(xorw);

  REG(addl);
  REG(andl);
  REG(andnl);
  REG(cmpeql);
  REG(cmpgtsl);
  REG(orl);
  REG(subl);
  REG(xorl);

  REG(select0lw);
  REG(select1lw);
  REG(select0wb);
  REG(select1wb);
  REG(mergebw);
  REG(mergewl);

  orc_rule_register (rule_set, "copyb", sse_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyw", sse_rule_copyx, NULL);
  orc_rule_register (rule_set, "copyl", sse_rule_copyx, NULL);

  orc_rule_register (rule_set, "shlw", sse_rule_shift, (void *)0);
  orc_rule_register (rule_set, "shruw", sse_rule_shift, (void *)1);
  orc_rule_register (rule_set, "shrsw", sse_rule_shift, (void *)2);
  orc_rule_register (rule_set, "shll", sse_rule_shift, (void *)3);
  orc_rule_register (rule_set, "shrul", sse_rule_shift, (void *)4);
  orc_rule_register (rule_set, "shrsl", sse_rule_shift, (void *)5);

  orc_rule_register (rule_set, "convsbw", sse_rule_convsbw, NULL);
  orc_rule_register (rule_set, "convubw", sse_rule_convubw, NULL);
  orc_rule_register (rule_set, "convssswb", sse_rule_convssswb, NULL);
  orc_rule_register (rule_set, "convsuswb", sse_rule_convsuswb, NULL);
  orc_rule_register (rule_set, "convwb", sse_rule_convwb, NULL);

  orc_rule_register (rule_set, "convswl", sse_rule_convswl, NULL);
  orc_rule_register (rule_set, "convuwl", sse_rule_convuwl, NULL);
  orc_rule_register (rule_set, "convssslw", sse_rule_convssslw, NULL);

  orc_rule_register (rule_set, "mulsbw", sse_rule_mulsbw, NULL);
  orc_rule_register (rule_set, "mulubw", sse_rule_mulubw, NULL);
  orc_rule_register (rule_set, "mulswl", sse_rule_mulswl, NULL);

  orc_rule_register (rule_set, "accw", sse_rule_accw, NULL);
  orc_rule_register (rule_set, "accl", sse_rule_accl, NULL);
  orc_rule_register (rule_set, "accsadubl", sse_rule_accsadubl, NULL);

  /* slow rules */
  orc_rule_register (rule_set, "maxuw", sse_rule_maxuw_slow, NULL);
  orc_rule_register (rule_set, "minuw", sse_rule_minuw_slow, NULL);
  orc_rule_register (rule_set, "avgsb", sse_rule_avgsb_slow, NULL);
  orc_rule_register (rule_set, "avgsw", sse_rule_avgsw_slow, NULL);
  orc_rule_register (rule_set, "maxsb", sse_rule_maxsb_slow, NULL);
  orc_rule_register (rule_set, "minsb", sse_rule_minsb_slow, NULL);
  orc_rule_register (rule_set, "maxsl", sse_rule_maxsl_slow, NULL);
  orc_rule_register (rule_set, "minsl", sse_rule_minsl_slow, NULL);
  orc_rule_register (rule_set, "maxul", sse_rule_maxul_slow, NULL);
  orc_rule_register (rule_set, "minul", sse_rule_minul_slow, NULL);
  orc_rule_register (rule_set, "convlw", sse_rule_convlw, NULL);

  /* SSE 3 -- no rules */

  /* SSSE 3 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_SSE_SSSE3);

  orc_rule_register (rule_set, "signb", sse_rule_signX, (void *)0);
  orc_rule_register (rule_set, "signw", sse_rule_signX, (void *)1);
  orc_rule_register (rule_set, "signl", sse_rule_signX, (void *)2);
  REG(absb);
  REG(absw);
  REG(absl);
  orc_rule_register (rule_set, "swapw", sse_rule_swapw_slow, NULL);
  orc_rule_register (rule_set, "swapl", sse_rule_swapl_slow, NULL);
  orc_rule_register (rule_set, "select0lw", sse_rule_select0lw_sse3, NULL);
  orc_rule_register (rule_set, "select1lw", sse_rule_select1lw_sse3, NULL);
  orc_rule_register (rule_set, "select0wb", sse_rule_select0wb_sse3, NULL);
  orc_rule_register (rule_set, "select1wb", sse_rule_select1wb_sse3, NULL);

  /* SSE 4.1 */
  rule_set = orc_rule_set_new (orc_opcode_set_get("sys"), target,
      ORC_TARGET_SSE_SSE4_1);

  REG(maxsb);
  REG(minsb);
  REG(maxuw);
  REG(minuw);
  REG(maxsl);
  REG(maxul);
  REG(minsl);
  REG(minul);
  REG(mulll);
  orc_rule_register (rule_set, "convsuslw", sse_rule_convsuslw, NULL);

  /* SSE 4.2 -- no rules */

  /* SSE 4a -- no rules */
}

