/* Copyright 2023 MarcosHCK
 * Copyright 2023 DavierSB
 * This file is part of JASH.
 *
 * JASH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JASH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JASH. If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <codegen_common.h>

#ifdef __CODEGEN__
|.arch x64
|.section code
|.globals globl_
|.actionlist actions
|.globalnames globl_names
|.externnames extern_names
#else
# define DASM_MAXSECTION (0)
# define globl_main (J_CODEGEN_LABEL_MAIN)
# define globl__MAX (0)
static const unsigned char actions[];
static const char* const globl_names[];
static const char* const extern_names[];
#endif // __CODEGEN__
#include <dynasm/dasm_x86.h>

#define stacksz_r \
  ( \
    0 \
    + sizeof (gpointer) /* jash */ \
    + sizeof (gpointer) /* error */ \
  )

G_STATIC_ASSERT (J_CODEGEN_LABEL_MAIN == globl_main);

#if __CODEGEN__
|.define Jash, [rsp + (sizeof (gpointer) * 0)]
|.define Error, [rsp + (sizeof (gpointer) * 1)]
|.define RetContinue, 1
|.define RetRemove, 0
#endif // __CODEGEN__

static void dumpbuffer (Dst_DECL, gconstpointer data, gsize buffsz)
{
  gsize n_dwords = (buffsz / 4);
  gsize n_bytes = (buffsz % 4);
  gsize i;

  const guint32* dwords = & G_STRUCT_MEMBER (guint32, data, 0);
  const guint8* bytes = & G_STRUCT_MEMBER (guint8, data, n_dwords * 4);

  for (i = 0; i < n_dwords; i++)
    {
#if __CODEGEN__
      |.dword (dwords [i])
#endif // __CODEGEN__
    }

  for (i = 0; i < n_bytes; i++)
    {
#if __CODEGEN__
      |.byte (bytes [i])
#endif // __CODEGEN__
    }
}

void j_codegen_init (Dst_DECL)
{
  Dst->n_labels = globl__MAX;
  Dst->labels = g_new (gpointer, globl__MAX);

  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, Dst->labels, globl__MAX);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, 0);
}

void j_codegen_absjump (Dst_DECL, gconstpointer value)
{
#if __CODEGEN__
  |.code
  |->main:
  | mov64 rax, ((guint64) value)
  | jmp rax
#endif // __CODEGEN__
}

void j_codegen_prologue (Dst_DECL)
{
#if __CODEGEN__
  |.code
  |->main:
  | sub rsp, (stacksz_r)
# ifdef G_OS_UNIX
  | mov Jash, rdi
  | mov Error, rsi
# else // !G_OS_UNIX
  | mov Jash, rcx
  | mov Error, rdx
# endif // G_OS_UNIX
  | call extern breakpoint
#endif // __CODEGEN__
}

void j_codegen_epilogue (Dst_DECL)
{
#if __CODEGEN__
  | add rsp, (stacksz_r)
  | mov rax, RetContinue
  | ret
#endif // __CODEGEN__
}

void j_codegen_generate (Dst_DECL, Ast ast)
{
}
