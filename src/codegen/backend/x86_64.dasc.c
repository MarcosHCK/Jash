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
#include <codegen/backend/common.dasc.c>
static const unsigned char actions[];
static const char* const globl_names[];
static const char* const extern_names[];
#endif // __CODEGEN__
#include <dynasm/dasm_x86.h>

const guint j_gdb_default_arch = bfd_arch_i386;
const guint j_gdb_default_mach = bfd_mach_x86_64;
static guint emit_invoke (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions);
static guint emit_string (Dst_DECL, gconstpointer data, gsize length);
typedef union _JInvokeStdfile JInvokeStdfile;

#if __CODEGEN__
|.macro pushchild, pid
| mov c_arg2, pid
| mov c_arg1, Self
| lea c_arg1, JClosure:c_arg1->waitq
| call extern g_queue_push_tail
|.endmacro
|.macro SYSCALL_WRAPPER_ONCE, name
||static inline guint emit_symbol_once_ .. name (Dst_DECL) G_GNUC_CONST;
||static inline guint emit_symbol_once_ .. name (Dst_DECL)
||{
||  static const gchar key [] = G_STRINGIFY ( .. name .. );
||  static const gchar fail [] = " .. name .. ()!";
||               gpointer value;
||  if (j_context_once_enter (Dst, (gchar*) key, &value))
||    {
||      guint funcpc = j_context_allocpc (Dst);
||      guint failpc = j_context_allocpc (Dst);
        |.aux
        |=>(funcpc):
        | sub rsp, 8
        | call extern name
        | test rax, rax
        | jns >1
        | lea c_arg1, [=>(failpc)]
        | call extern fail
        |1:
        | add rsp, 8
        | ret
        |.data
        |=>(failpc):
||      j_context_store (Dst, fail, sizeof (fail));
        |.code
||      j_context_once_leave (Dst, (gchar*) key, &value, GUINT_TO_POINTER (funcpc));
||    }
||return GPOINTER_TO_UINT (value);
||}
|.endmacro

| SYSCALL_WRAPPER_ONCE chdir
| SYSCALL_WRAPPER_ONCE close
| SYSCALL_WRAPPER_ONCE dup2
| SYSCALL_WRAPPER_ONCE execvp
| SYSCALL_WRAPPER_ONCE fork
| SYSCALL_WRAPPER_ONCE pipe
| SYSCALL_WRAPPER_ONCE open

||static inline guint emit_symbol_once_close_all (Dst_DECL)
||{
||  static const gchar key [] = "close_all";
||                gpointer value;
||  if (j_context_once_enter (Dst, (gchar*) key, &value))
||    {
||      guint funcpc = j_context_allocpc (Dst);
        guint closepc = emit_symbol_once_close (Dst);
||;
        |.aux
        |=>(funcpc):
        | push r12
        | push rbx
        | sub rsp, 8
        | mov r12, c_arg1
        | mov rbx, c_arg2
        |1:
        | mov c_arg1, [r12+rbx*4-(sizeof (int))]
  || G_STATIC_ASSERT (sizeof (int) == 4);
        | call =>(closepc)
        | dec rbx
        | jnz <1
        | add rsp, 8
        | pop rbx
        | pop r12
        | ret
        |.code
||      j_context_once_leave (Dst, (gchar*) key, &value, GUINT_TO_POINTER (funcpc));
||    }
||return GPOINTER_TO_UINT (value);
||}
#endif // __CODEGEN__

void j_context_complete (Dst_DECL)
{
#if __CODEGEN__
  |=>(Dst->nextstage):
  | mov rax, RetRemove
  | ret
#endif // __CODEGEN__

  j_context_complete_common (Dst);
}

void j_context_emit (Dst_DECL, JWalker* walker)
{
  GList* list;
  guint i;
#define DECL_VARARRAY(name,ctype,pre) \
  ctype* name = NULL; \
  ctype* name##_dyn = NULL; \
  ctype name##_stat [(pre)];
#define INIT_VARARRAY(name,now) \
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
#define CLEAR_VARARRAY(name) g_clear_pointer (& name##_dyn, g_free)

  DECL_VARARRAY (arguments, guint, 32);
  DECL_VARARRAY (expansions, guint, 32);
  DECL_VARARRAY (invocations, guint, 32);
  INIT_VARARRAY (arguments, g_queue_get_length (&walker->arguments));
  INIT_VARARRAY (expansions, g_queue_get_length (&walker->expansions));
  INIT_VARARRAY (invocations, g_queue_get_length (&walker->invocations));

  for (list = g_queue_peek_head_link (&walker->arguments), i = 0; list; list = list->next)
    {
      arguments [i++] = emit_string (Dst, list->data, strlen (list->data) + 1);
    }

  for (list = g_queue_peek_head_link (&walker->expansions), i = 0; list; list = list->next)
    {
      expansions [i++] = Dst->expansions->len;
      g_ptr_array_add (Dst->expansions, list->data);
    }

  for (list = g_queue_peek_head_link (&walker->invocations), i = 0; list; list = list->next)
    {
      invocations [i++] = emit_invoke (Dst, walker, list->data, arguments, expansions);
    }

  /*
   * Stack:
   * > rbp (frame pointer)
   * > JClosure* (self : argument)
   * > GError** (error : argument)
   * > JPipe [walker->n_pipes] (pipes : local variable)
   * > gchar* [invoke->n_arguments] (arguemnts : local variable)
   */
  guint stacksize = 0
    + sizeof (guintptr)
    + sizeof (JClosure*)
    + sizeof (GError**)
    + sizeof (JPipe*) * (walker->n_pipes > 0 ? 1 : 0)
    + sizeof (JPipe) * (walker->n_pipes)
  /* AMD64 System V ABI demands 16 bytes alignment */
  ; stacksize += 16 - (stacksize % 16);

#if __CODEGEN__
  |=>(Dst->nextstage):
  | push rbp
  | mov rbp, rsp
  | sub rsp, (stacksize)
  | mov Self, c_arg1
  | mov Error, c_arg2
#endif // __CODEGEN__

  if (walker->n_pipes > 0)
    {
#if __CODEGEN__
      | mov Pipes, rsp
#endif // __CODEGEN__

      for (i = 0; i < walker->n_pipes; ++i)
        {
#if __CODEGEN__
          | mov c_arg1, Pipes
          | lea c_arg1, JPipe:c_arg1 [i]
          | call =>(emit_symbol_once_pipe (Dst))
#endif // __CODEGEN__
        }
    }

  Dst->nextstage = j_context_allocpc (Dst);

  for (i = 0; i < g_queue_get_length (&walker->invocations); ++i)
    {
#if __CODEGEN__
      | call =>(invocations [i])
      | mov c_arg2, rax
      | mov rax, Self
      | lea c_arg1, JClosure:rax->waitq
      | call extern g_queue_push_tail
#endif // __CODEGEN__
    }
  if (walker->n_pipes > 0)
    {
#if __CODEGEN__
      | mov c_arg1, Pipes
      | mov c_arg2, (walker->n_pipes * 2)
      | call =>(emit_symbol_once_close_all (Dst))
#endif // __CODEGEN__
    }
#if __CODEGEN__
  | mov rax, Self
  | lea rcx, [=>(Dst->nextstage)]
  | leave
  | mov JClosure:rax->entry, rcx
  | mov rax, RetContinue
  | ret
#endif // __CODEGEN__
  CLEAR_VARARRAY (invocations);
  CLEAR_VARARRAY (expansions);
  CLEAR_VARARRAY (arguments);
#undef DECL_VARARRAY
#undef INIT_VARARRAY
#undef CLEAR_VARARRAY
}

void j_context_ljmp (Dst_DECL, gpointer address)
{
#if __CODEGEN__
  |=>(Dst->nextstage):
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

static void adjust_stdfile (Dst_DECL, JWalker* walker, JInvoke* invoke, guint type, guint fileno, JInvokeStdfile* desc)
{
  switch (type)
  {
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
        | mov c_arg2, (fileno)
        | call =>(emit_symbol_once_dup2 (Dst))
#endif // __CODEGEN__
        break;
      }

    case J_INVOKE_STD_FILE_TYPE_FILE:
      if (desc->filename == NULL)
        break;
      else
      {
        const gchar* name = desc->filename;
        const gsize length = strlen (name) + 1;

        guint namepc = emit_string (Dst, name, length);
        guint mode = 0;

        switch (fileno)
          {
            default: g_assert_not_reached ();
            case STDIN_FILENO: mode = O_RDONLY; break;
            case STDOUT_FILENO: mode = O_WRONLY;
              {
                switch (invoke->stdout_mode)
                {
                  default: g_assert_not_reached ();
                  case J_INVOKE_STD_FILE_MODE_APPEND: mode |= O_APPEND; break;
                  case J_INVOKE_STD_FILE_MODE_REPLACE: mode |= O_CREAT; break;
                } break;
              }
          }

#if __CODEGEN__
        | lea c_arg1, [=>(namepc)]
        | mov c_arg2, (mode)
        | call =>(emit_symbol_once_open (Dst))
        | push rax
        | mov c_arg1, rax
        | mov c_arg2, (fileno)
        | call =>(emit_symbol_once_dup2 (Dst))
        | pop c_arg1
        | call =>(emit_symbol_once_close (Dst))
#endif // __CODEGEN__
        break;
      }
  }
}

static void adjust_iofiles (Dst_DECL, JWalker* walker, JInvoke* invoke)
{
  guint closeallpc = emit_symbol_once_close_all (Dst);
  adjust_stdfile (Dst, walker, invoke, invoke->stdin_type, STDIN_FILENO, & invoke->stdin);
  adjust_stdfile (Dst, walker, invoke, invoke->stdout_type, STDOUT_FILENO, & invoke->stdout);

  if (walker->n_pipes > 0)
    {
#if __CODEGEN__
      | mov c_arg1, Pipes
      | mov c_arg2, (walker->n_pipes * 2)
      | call =>closeallpc
#endif // __CODEGEN__
    }
}

enum
{
  REPORT_STATUS_IMMEDIATE = (1 << 0),
  REPORT_STATUS_MESSAGE = (1 << 1),
  REPORT_STATUS_VA_MESSAGE = (1 << 2),
};

static guint emit_invoke (Dst_DECL, JWalker* walker, JInvoke* invoke, guint* arguments, guint* expansions)
{
  guint invokepc = j_context_allocpc (Dst);
  guint forkpc = emit_symbol_once_fork (Dst);
  gboolean report = FALSE;

#if __CODEGEN__
  |=>(invokepc):
  | sub rsp, (8 /* Respect alignment */)
  |.macro loadarg, register, index_
  ||  G_STMT_START
  ||  {
  ||    const JArgument* args = & invoke->target;
  ||    const JArgument* arg = & args [index_];
  ||
  ||    switch (arg->type)
  ||    {
  ||      case J_ARGUMENT_TYPE_DATA:
  ||        {
  |           lea register, [=>(arguments [arg->index])]
  ||          break;
  ||        }
  ||      case J_ARGUMENT_TYPE_EXPANSION:
  ||        {
  |           mov register, Self
  |           mov register, JClosure:register->expansions
  |           mov register, gpointer:register [expansions [arg->index]]
  ||          break;
  ||        }
  ||    }
  ||  }
  ||  G_STMT_END;
  |.endmacro
#endif // __CODEGEN__

  if (invoke->target_type == J_INVOKE_TARGET_TYPE_BUILTIN)
    {
      const gchar* value = invoke->target.builtin;

      if (value == J_TOKEN_BUILTIN_CD)
        {
          if (invoke->n_arguments > 1)
            g_assert_not_reached ();
          else if (invoke->n_arguments != 1)
            report = TRUE;
          else
          {
#if __CODEGEN__
            | loadarg c_arg1, 1
            | call extern j_chdir
            | mov [rsp], rax
            |1:
#endif // __CODEGEN__
            report = TRUE;
          }
        }
    }

  /* do fork */
#if __CODEGEN__
  | call =>forkpc
  | test rax, rax
  | jz >1
  | add rsp, 8
  | ret
  |1:
#endif // __CODEGEN__

  adjust_iofiles (Dst, walker, invoke);

  if (report == TRUE)
    {
#if __CODEGEN__
      | mov c_arg1, [rsp]
      | call extern exit
#endif // __CODEGEN__
      return invokepc;
    }
  else
    {
      JArgument* args = & invoke->target;
      guint n_arguments = invoke->n_arguments + 1;
      guint stacksize = sizeof (gchar*) * (n_arguments + 1);
          stacksize += 16 - (stacksize % 16);
      guint i = 0;

#if __CODEGEN__
      | sub rsp, (stacksize)
#endif // __CODEGEN__

      for (i = (invoke->target_type == J_INVOKE_TARGET_TYPE_BUILTIN ? 1 : 0); i < n_arguments; ++i)
        {
#if __CODEGEN__
          | loadarg rax, i
          | mov gpointer:rsp [i], rax
#endif // __CODEGEN__
        }
#if __CODEGEN__
        | mov qword gpointer:rsp [n_arguments], 0
#endif // __CODEGEN__

      if (invoke->target_type == J_INVOKE_TARGET_TYPE_REGULAR)
        {
#if __CODEGEN__
          | mov c_arg1, [rsp]
          | lea c_arg2, [rsp]
          | call =>(emit_symbol_once_execvp (Dst))
#endif // __CODEGEN__
        }
      else
        {
          g_assert_not_reached ();
        }
    }
return invokepc;
}

static guint emit_string (Dst_DECL, gconstpointer data, gsize length)
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
