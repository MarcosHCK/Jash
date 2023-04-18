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
#include <codegen/codegen.h>
#include <codegen/context.h>
#include <codegen/walker.h>

#ifdef __CODEGEN__
|.arch x64
|.include codegen/backend/common.h
#else // !__CODEGEN__
# define DASM_MAXSECTION (0)
# define globl_entry (J_CONTEXT_LABEL_MAIN)
# define globl__MAX (0)
static const unsigned char actions[];
static const char* const globl_names[];
static const char* const extern_names[];
#endif // __CODEGEN__
#include <dynasm/dasm_x86.h>

#define stacksz_r \
  ( \
    0 \
    + sizeof (gpointer) /* self */ \
    + sizeof (gpointer) /* error */ \
  )

G_STATIC_ASSERT (J_CONTEXT_LABEL_MAIN == globl_entry);

void j_context_clear (Dst_DECL)
{
  dasm_free (Dst);
  g_free (Dst->labels);
}

void j_context_store (Dst_DECL, gpointer buffer, gsize bufsz)
{
  gsize i;

  const gsize n_dwords = (bufsz / 4);
  const guint32* dwords = & G_STRUCT_MEMBER (guint32, buffer, 0);

  for (i = 0; i < n_dwords; i++)
    {
#if __CODEGEN__
      |.dword (dwords [i])
#endif // __CODEGEN__
    }

  const gsize n_bytes = (bufsz % 4);
  const guint8* bytes = & G_STRUCT_MEMBER (guint8, buffer, n_dwords * 4);

  for (i = 0; i < n_bytes; i++)
    {
#if __CODEGEN__
      |.byte (bytes [i])
#endif // __CODEGEN__
    }
}

void j_context_epilog (Dst_DECL)
{
#if __CODEGEN__
  | mov rax, RetRemove
  | leave
  | ret
#endif // __CODEGEN__
}

void j_context_init (Dst_DECL)
{
  Dst->n_labels = globl__MAX;
  Dst->labels = g_new (gpointer, globl__MAX);
  Dst->nextpc = 0;
  Dst->maxpc = 2;

  dasm_init (Dst, DASM_MAXSECTION);
  dasm_setupglobal (Dst, Dst->labels, Dst->n_labels);
  dasm_setup (Dst, actions);
  dasm_growpc (Dst, Dst->maxpc);

#if __CODEGEN__
  |.main
  |->entry:
#endif // __CODEGEN__
}

void j_context_ljmp (Dst_DECL, gpointer address)
{
#if __CODEGEN__
  | mov64 rax, ((guint64) address)
  | jmp rax
#endif // __CODEGEN__
}

void j_context_prolog (Dst_DECL)
{
#if __CODEGEN__
  | push rbp
  | mov rbp, rsp
  | sub rsp, (stacksz_r)
  | mov Self, c_arg1
  | mov Error, c_arg2
#endif // __CODEGEN__
}
