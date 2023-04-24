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
#include <codegen/system.h>
#include <codegen/walker.h>
#if DEVELOPER == 1
# include <bfd.h>
# include <codegen/debug/gdb.h>
#endif // DEVELOPER
#include <lexer/token.h>

#ifdef __CODEGEN__
|.arch x64
|.include codegen/backend/common.dasc.c

|.define Self, gpointer:rbp[-1]
|.define Error, gpointer:rbp[-2]
|.define Pipes, gpointer:rbp[-3]

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
# define globl___aux_start (0)
# define globl___code_start (0)
# define globl___data_start (0)
# define globl___code_end (0)
# define globl___aux_end (0)
# define globl___data_end (0)
# define globl__MAX (0)
static const unsigned char actions[];
static const char* const globl_names[];
static const char* const extern_names[];
#endif // __CODEGEN__
#include <dynasm/dasm_x86.h>

const guint j_gdb_default_arch = bfd_arch_i386;
const guint j_gdb_default_mach = bfd_mach_x86_64;
typedef union _JInvokeStdfile JInvokeStdfile;

#define stacksz_r \
  ( \
    0 \
    + sizeof (gpointer) /* self */ \
    + sizeof (gpointer) /* error */ \
  )

static void emit_invoke_child_side (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions);
static void emit_invoke_parent_side (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions);
static void emit_invoke_stdfile (Dst_DECL, JWalker* walker, JInvoke* invoke, guint type, guint fileno, JInvokeStdfile* desc);
static void emit_invoke (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions);
static void emit_symbol_once_close_s (Dst_DECL);
static void emit_symbol_once_dup2_s (Dst_DECL);
static void emit_symbol_once_execvp_s (Dst_DECL);
static void emit_symbol_once_fork_s (Dst_DECL);
static void emit_symbol_once_open_s (Dst_DECL);
static void emit_symbol_once_pipe_s (Dst_DECL);
static gint take_string (Dst_DECL, gconstpointer data, gsize length);
G_STATIC_ASSERT (J_CONTEXT_LABEL_MAIN == globl_entry);

void j_context_complete (Dst_DECL)
{
#if __CODEGEN__
  |.code
  |=>(Dst->nextstage):
  | mov rax, RetRemove
  | ret
#endif // __CODEGEN__

  j_context_complete_common (Dst);
}

#if DEVELOPER == 1

void j_context_debug_build (Dst_DECL)
{
  JGdbSection* aux = NULL;
  gpointer aux_start = Dst->labels [globl___aux_start];
  gpointer aux_end = Dst->labels [globl___aux_end];
  gssize aux_size = aux_end - aux_start;
  JGdbSection* code = NULL;
  gpointer code_start = Dst->labels [globl___code_start];
  gpointer code_end = Dst->labels [globl___code_end];
  gssize code_size = code_end - code_start;
  JGdbSection* data = NULL;
  gpointer data_start = Dst->labels [globl___data_start];
  gpointer data_end = Dst->labels [globl___data_end];
  gssize data_size = data_end - data_start;
  JGdbSymbol* symbol;
  guint i;

  g_assert (aux_start < aux_end);
  g_assert (code_start < code_end);
  g_assert (data_start < data_end);

  aux = j_gdb_builder_decl_section (&Dst->debug_builder, "aux", aux_start, aux_size);
  code = j_gdb_builder_decl_section (&Dst->debug_builder, "code", code_start, code_size);
  data = j_gdb_builder_decl_section (&Dst->debug_builder, "data", data_start, data_size);

  for (i = 0; i < globl__MAX; ++i)
    {
      gpointer base = (gpointer) Dst->labels [i];
      gpointer name = (gpointer) globl_names [i];

      if (base == NULL) continue;
      else if (aux_end >= base && base >= aux_start) j_gdb_builder_decl_function (&Dst->debug_builder, name, base, aux);
      else if (code_end >= base && base >= code_start) j_gdb_builder_decl_function (&Dst->debug_builder, name, base, code);
      else if (data_end >= base && base >= data_start) j_gdb_builder_decl_function (&Dst->debug_builder, name, base, data);
      else g_assert_not_reached ();
    }

  j_gdb_builder_fill_section (&Dst->debug_builder, aux_start, aux_size, aux);
  j_gdb_builder_fill_section (&Dst->debug_builder, code_start, code_size, code);
  j_gdb_builder_fill_section (&Dst->debug_builder, data_start, data_size, data);
}

#endif // DEVELOPER

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
      arguments [i++] = take_string (Dst, list->data, strlen (list->data) + 1);
    }

  for (list = g_queue_peek_head_link (&walker->expansions), i = 0; list; list = list->next)
    {
      expansions [i++] = Dst->expansions->len;
      g_ptr_array_add (Dst->expansions, list->data);
    }

#if __CODEGEN__
  |.code
  |=>(Dst->nextstage):
  | push rbp
  | mov rbp, rsp
  | sub rsp, stacksz_r
  | mov Self, c_arg1
  | mov Error, c_arg2
#endif // __CODEGEN__

  if (walker->n_pipes > 0)
    {
#if __CODEGEN__
      |.aux
      |->close_all:
      | push rbx
      | mov rbx, c_arg1
#endif // __CODEGEN__

      for (i = 0; i < walker->n_pipes; ++i)
        {
          guint j;

          for (j = 0; j < 2; ++j)
          {
#if __CODEGEN__
            | movsxd c_arg1, dword JPipe:rbx [i] [j]
            | call ->close_0
#endif // __CODEGEN__   
          }
        }
#if __CODEGEN__
      | pop rbx
      | ret
      || j_context_symbol_once (Dst, close_s);
      |.aux
      |->close_0:
      | test c_arg1, c_arg1
      | js >1
      | call ->close_s
      |1:
      | ret
#endif // __CODEGEN__

#if __CODEGEN__
      |.code
      | sub rsp, (#gpointer + (walker->n_pipes * #JPipe))
      | mov Pipes, rsp
#endif // __CODEGEN__
      for (i = 0; i < walker->n_pipes; ++i)
        {
#if __CODEGEN__
          | mov c_arg1, Pipes
          | lea c_arg1, JPipe:c_arg1 [i]
          | call ->pipe_s
          || j_context_symbol_once (Dst, pipe_s);
#endif // __CODEGEN__
        }
    }

  Dst->nextstage = j_context_allocpc (Dst);

  for (list = g_queue_peek_head_link (&walker->invocations); list; list = list->next)
    {
      emit_invoke (Dst, walker, (JInvoke*) list->data, arguments, expansions);
    }

  if (walker->n_pipes > 0)
    {
#if __CODEGEN__
      | mov c_arg1, Pipes
      | call ->close_all
#endif // __CODEGEN__
    }

#if __CODEGEN__
  | mov rax, Self
  | leave
  | lea rcx, [=>(Dst->nextstage)]
  | mov JClosure:rax->entry, rcx
  | mov rax, RetContinue
  | ret
#endif // __CODEGEN__
  CLEAR_MIXVAR (expansions);
  CLEAR_MIXVAR (arguments);
#undef DECL_MIXVAR
#undef INIT_MIXVAR
#undef CLEAR_MIXVAR
}

void j_context_ljmp (Dst_DECL, gpointer address)
{
#if __CODEGEN__
  | mov64 rax, ((guint64) address)
  | jmp rax
#endif // __CODEGEN__
}

void j_context_store (Dst_DECL, gconstpointer buffer, gsize bufsz)
{
  gsize i;

  const gsize n_dwords = (bufsz / 4);
  const guint32* dwords = & G_STRUCT_MEMBER (guint32, buffer, 0);

  for (i = 0; i < n_dwords; i++)
    {
#if __CODEGEN__
      |.dword (GUINT32_TO_LE (dwords [i]))
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

static void emit_invoke_child_side (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions)
{
  JArgument* args = & invoke->target;
  guint n_arguments = invoke->n_arguments + 1;
  guint i, first_arg = 0;

  emit_invoke_stdfile (Dst, walker, invoke, invoke->stdin_type, STDIN_FILENO, & invoke->stdin);
  emit_invoke_stdfile (Dst, walker, invoke, invoke->stdout_type, STDOUT_FILENO, & invoke->stdout);

  if (walker->n_pipes > 0)
    {
#if __CODEGEN__
      | mov c_arg1, Pipes
      | call ->close_all
#endif // __CODEGEN__
    }
#if __CODEGEN__
  | sub rsp, (GLIB_SIZEOF_VOID_P * (n_arguments + 1))
#endif // __CODEGEN__

  if (invoke->target_type == J_INVOKE_TARGET_TYPE_BUILTIN)
    {
      ++first_arg;
      g_assert_not_reached ();
    }

  for (i = first_arg; i < n_arguments; ++i)
    {
      switch (args [i].type)
      {
        case J_ARGUMENT_TYPE_DATA:
#if __CODEGEN__
          | lea rax, [=>(arguments [args [i].index])]
#endif // __CODEGEN__
          break;
        case J_ARGUMENT_TYPE_EXPANSION:
          {
#if __CODEGEN__
            | mov rax, Self
            | mov rax, JClosure:rax->expansions
            | mov rax, gpointer:rax [expansions [args [i].index]]
#endif // __CODEGEN__
            break;
          }
      }
#if __CODEGEN__
      | mov gpointer:rsp [i], rax
#endif // __CODEGEN__
    }
#if __CODEGEN__
    | mov qword gpointer:rsp [n_arguments], 0
#endif // __CODEGEN__

  if (first_arg == 0)
    {
#if __CODEGEN__
      | mov c_arg1, [rsp]
      | lea c_arg2, [rsp]
      | call ->execvp_s
      || j_context_symbol_once (Dst, execvp_s);
#endif // __CODEGEN__
    }
}

static void emit_invoke_parent_side (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions)
{
#if __CODEGEN__
  | mov rcx, gpointer:rbp[-1]
  | lea c_arg1, JClosure:rcx->waitq
  | mov c_arg2, rax
  | call extern g_queue_push_head
#endif // __CODEGEN__
}

static void emit_invoke_stdfile (Dst_DECL, JWalker* walker, JInvoke* invoke, guint type, guint fileno, JInvokeStdfile* desc)
{
  switch (type)
  {
    case J_INVOKE_STD_FILE_TYPE_FILE:
      if (desc->filename == NULL)
        break;
      else
      {
        const gchar* name = desc->filename;
        const gsize length = strlen (name) + 1;

        guint namepc = take_string (Dst, name, length);
        guint mode = 0;

        switch (fileno)
          {
            default: g_assert_not_reached ();
            case STDIN_FILENO: mode = O_RDONLY; break;
            case STDOUT_FILENO:
              {
                mode = O_WRONLY;
                switch (invoke->stdout_mode)
                {
                  default: g_assert_not_reached ();
                  case J_INVOKE_STD_FILE_MODE_APPEND: mode |= O_APPEND; break;
                  case J_INVOKE_STD_FILE_MODE_REPLACE: mode |= O_CREAT; break;
                }

                break;
              }
          }

#if __CODEGEN__
        | lea c_arg1, [=>(namepc)]
        | mov c_arg2, (mode)
        | call ->open_s
        || j_context_symbol_once (Dst, open_s);
        | mov c_arg1, rax
        | mov c_arg2, (fileno)
        | call ->dup2_s
        || j_context_symbol_once (Dst, dup2_s);
#endif // __CODEGEN__
        break;
      }

    case J_INVOKE_STD_FILE_TYPE_PIPE:
      {
        guint offset;

        switch (fileno)
          {
            case STDIN_FILENO: offset = 0; break;
            case STDOUT_FILENO: offset = 1; break;
            default: g_assert_not_reached ();
          }
#if __CODEGEN__
        | mov rax, Pipes
        | lea rax, JPipe:rax [desc->fd] [offset]
        | movsxd c_arg1, dword [rax]
        | mov dword [rax], -1
        | mov c_arg2, (fileno)
        | call ->dup2_s
        || j_context_symbol_once (Dst, dup2_s);
#endif // __CODEGEN__
        break;
      }
  }
}

static void emit_invoke (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions)
{
  /* do fork */
#if __CODEGEN__
  |9:
  | call ->fork_s
  || j_context_symbol_once (Dst, fork_s);
  | test rax, rax
  | jnz >9
#endif // __CODEGEN__

  emit_invoke_child_side (Dst, walker, invoke, arguments, expansions);

#if __CODEGEN__
  |9:
#endif // __CODEGEN__

  emit_invoke_parent_side (Dst, walker, invoke, arguments, expansions);
}

static gint take_string (Dst_DECL, gconstpointer data, gsize length)
{
  gpointer lpc = NULL;
  guint pc = 0;

  if (g_hash_table_lookup_extended (Dst->strtab, data, NULL, &lpc))
    return GPOINTER_TO_UINT (lpc);
  else
    {
      pc = j_context_allocpc (Dst);
#if __CODEGEN__
      |.data
      |=>(pc):
#endif // __CODEGEN__
      j_context_store (Dst, data, length);
      g_hash_table_insert (Dst->strtab, (gpointer) data, GUINT_TO_POINTER (pc));
#if __CODEGEN__
      |.code
#endif // __CODEGEN__
    }
return pc;
}

#if __CODEGEN__
|.macro GROUP1_ONCE_FUNC, dy_name
||static const gchar __ .. dy_name .. _fail__ [] = " .. dy_name .. ()!";
||static void emit_symbol_once_ .. dy_name .. _s (Dst_DECL)
||{
  |.aux
  |->dy_name .. _s:
  | call extern dy_name
  | test eax, eax
  | js >1
  | ret
  |1:
  | lea c_arg1, [->__ .. dy_name .. _fail__]
  | call extern fail
  |.data
  |->__ .. dy_name .. _fail__:
|| j_context_store (Dst, __ .. dy_name .. _fail__, G_N_ELEMENTS (__ .. dy_name .. _fail__));
  |.code
||}
|.endmacro
| GROUP1_ONCE_FUNC close
| GROUP1_ONCE_FUNC dup2
| GROUP1_ONCE_FUNC execvp
| GROUP1_ONCE_FUNC fork
| GROUP1_ONCE_FUNC open
| GROUP1_ONCE_FUNC pipe
#endif // __CODEGEN__
