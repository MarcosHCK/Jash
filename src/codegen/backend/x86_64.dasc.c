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

|.define Self, [rbp - (sizeof (gpointer) * 0)]
|.define Error, [rbp - (sizeof (gpointer) * 1)]

|.if PLATFORM == 'linux'
|.define c_arg1, rdi
|.define c_arg2, rsi
|.define c_arg3, rdx
|.define c_arg4, rcx
|.define c_arg5, r8
|.define c_arg6, r9
|.elif PLATFORM == 'win32'
|.define c_arg1, rcx
|.define c_arg2, rdx
|.define c_arg3, r8
|.define c_arg4, r9
|.endif
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
  g_object_unref (Dst->codegen);
  g_hash_table_remove_all (Dst->strtab);
  g_hash_table_unref (Dst->strtab);
}

void j_context_epilog (Dst_DECL)
{
#if __CODEGEN__
  | mov rax, RetRemove
  | leave
  | ret
#endif // __CODEGEN__
}

void j_context_emit (Dst_DECL, JWalker* walker)
{
  GList* list;
  guint i;

#define DECL_MIXVAR(name,ctype,pre) \
  ctype* name = NULL; \
  ctype* name##_dyn = NULL; \
  ctype name##_stat [(pre)];
#define INIT_MIXVAR(name,now) \
  G_STMT_START \
    { \
      gsize __now = ((now)); \
 ; \
      if (G_N_ELEMENTS (name##_stat) >= __now) \
        name = name##_stat; \
      else \
        { \
          name##_dyn = g_malloc (sizeof (*name) * __now); \
          name = name##_dyn; \
        } \
    } \
  G_STMT_END
#define CLEAR_MIXVAR(name) g_clear_pointer (& name##_dyn, g_free)

  DECL_MIXVAR (arguments, guint, 32);
  DECL_MIXVAR (expansions, guint, 32);

  INIT_MIXVAR (arguments, g_queue_get_length (&walker->arguments));
  INIT_MIXVAR (expansions, g_queue_get_length (&walker->expansions));

  for (list = g_queue_peek_head_link (&walker->arguments), i = 0; list; list = list->next)
    {
      gpointer index_p;
      guint index_s;

      if (g_hash_table_lookup_extended (Dst->strtab, list->data, NULL, &index_p))
        arguments [i++] = GPOINTER_TO_UINT (index_p);
      else
        {
          index_s = j_context_allocpc (Dst);

#if __CODEGEN__
          |.data
          |=>(index_s):
#endif // __CODEGEN__

          g_hash_table_insert (Dst->strtab, list->data, GUINT_TO_POINTER (index_s));
          j_context_store (Dst, list->data, strlen (list->data));
        }
    }

  for (list = g_queue_peek_head_link (&walker->expansions), i = 0; list; list = list->next)
    {
      GError* tmperr = NULL;
      GClosure* expansion;

      expansions [i++] = j_context_allocpc (Dst);
      expansion = j_codegen_emit (Dst->codegen, list->data, &tmperr);

      g_assert_no_error (tmperr);

#if __CODEGEN__
      |.expansions
      |=>(expansions [i - 1]):
# if G_BYTE_ORDER == G_LITTLE_ENDIAN
      | .dword (((guintptr) expansion) & G_MAXUINT32)
      | .dword (((guintptr) expansion) >> 32)
# elif G_BYTE_ORDER == G_BIG_ENDIAN
      | .dword (((guintptr) expansion) >> 32)
      | .dword (((guintptr) expansion) & G_MAXUINT32)
# else
# error Unsupported byte order
# endif // G_BYTE_ORDER
#endif // __CODEGEN__
    }

  const guint chainpc = j_context_allocpc (Dst);
  const guint n_invokes = g_queue_get_length (&walker->invocations);
  const gsize pipes_size = walker->n_pipes * sizeof (gint) * 2;
  const gsize invokes_size = n_invokes * sizeof (gint);

  G_STMT_START
    {
#if __CODEGEN__
      |.code
      |=>(chainpc):
      | push rbx
      | sub rsp, (pipes_size + invokes_size)

      | mov rbx, (walker->n_pipes - 1)
      |1:

      G_STATIC_ASSERT (sizeof (gint) * 2 == 8);

      | lea c_arg1, [rsp+rbx*8+invokes_size]
      | call extern pipe
      | sub rbx, 1
      | jns <1

      | add rsp, (pipes_size + invokes_size)
      | pop rbx
#endif // __CODEGEN__
    }
  G_STMT_END;

  CLEAR_MIXVAR (expansions);
  CLEAR_MIXVAR (arguments);
#undef DECL_MIXVAR
#undef INIT_MIXVAR
#undef CLEAR_MIXVAR
}

void j_context_init (Dst_DECL, JCodegen* codegen)
{
  Dst->labels = g_new (gpointer, globl__MAX);
  Dst->n_labels = globl__MAX;
  Dst->nextpc = 0;
  Dst->maxpc = 2;
  Dst->codegen = g_object_ref (codegen);
  Dst->strtab = g_hash_table_new (g_str_hash, g_str_equal);

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
