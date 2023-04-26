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

|.define Self, gpointer:rbp [-1]
|.define Error, gpointer:rbp [-2]
|.define Tmperr, gpointer:rbp [-3]
|.define Pipes, gpointer:rbp [-4]

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
|
||#include <dynasm/dasm_x86.h>
|
||const guint j_gdb_default_arch = bfd_arch_i386;
||const guint j_gdb_default_mach = bfd_mach_x86_64;
||typedef union _JInvokeStdfile JInvokeStdfile;
|
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
||;
|      .aux
|      =>(funcpc):
|        sub rsp, #gpointer
|        call extern name
|        test rax, rax
|        jns >1
|        xor c_arg1, c_arg1
|        lea c_arg2, [=>(failpc)]
|        call extern j_fail_with_errno
|      1:
|        add rsp, #gpointer
|        ret
|      .data
|      =>(failpc):
||      j_context_store (Dst, fail, sizeof (fail));
|      .code
||;
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

||static inline guint emit_symbol_once_close_pipes (Dst_DECL)
||{
||  static const gchar key [] = "close_all";
||                gpointer value;
||  if (j_context_once_enter (Dst, (gchar*) key, &value))
||    {
||      guint funcpc = j_context_allocpc (Dst);
||      guint closepc = emit_symbol_once_close (Dst);
||;
|      .aux
|      =>(funcpc):
|       push r12
|       push rbx
|       sub rsp, #gpointer
|       mov r12, c_arg1
|       mov rbx, c_arg2
|     1:
|       mov c_arg1, [r12+rbx*8+((-sizeof (JPipe)) + (sizeof (int) * 0))]
|       call =>(closepc)
|       mov c_arg1, [r12+rbx*8+((-sizeof (JPipe)) + (sizeof (int) * 1))]
|       call =>(closepc)
|       dec rbx
|       jnz <1
|       add rsp, #gpointer
|       pop rbx
|       pop r12
|       ret
|      .code
||;
||      j_context_once_leave (Dst, (gchar*) key, &value, GUINT_TO_POINTER (funcpc));
||    }
||return GPOINTER_TO_UINT (value);
||}
|
||void j_context_complete (Dst_DECL)
||{
|=>(Dst->nextstage):
|   mov rax, RetRemove
|   ret
||  j_context_complete_common (Dst);
||}
|
||static guint emit_string (Dst_DECL, gconstpointer data, gsize length)
||{
||  gpointer lpc = NULL;
||  guint pc = 0;
||
||  if (g_hash_table_lookup_extended (Dst->strtab, data, NULL, &lpc))
||    return GPOINTER_TO_UINT (lpc);
||  else
||    {
||      pc = j_context_allocpc (Dst);
|     .data
|     =>(pc):
||      j_context_store (Dst, data, length);
||      g_hash_table_insert (Dst->strtab, (gpointer) data, GUINT_TO_POINTER (pc));
|     .code
||    }
||return pc;
||}
|
||static void adjust_stdfile (Dst_DECL, JWalker* walker, JInvoke* invoke, guint type, guint fileno, JInvokeStdfile* desc)
||{
||  switch (type)
||  {
||    case J_INVOKE_STD_FILE_TYPE_FILE:
||      if (desc->filename == NULL)
||        break;
||      else
||    {
||        const gchar* name = desc->filename;
||        const gsize length = strlen (name) + 1;
||
||        guint namepc = emit_string (Dst, name, length);
||        guint mode = 0;
||
||        switch (fileno)
||          {
||            default: g_assert_not_reached ();
||            case STDIN_FILENO: mode = O_RDONLY; break;
||            case STDOUT_FILENO: mode = O_WRONLY;
||              {
||                switch (invoke->stdout_mode)
||                {
||                  default: g_assert_not_reached ();
||                  case J_INVOKE_STD_FILE_MODE_APPEND: mode |= O_APPEND; break;
||                  case J_INVOKE_STD_FILE_MODE_REPLACE: mode |= O_CREAT; break;
||                } break;
||              }
||          }
||
|       lea c_arg1, [=>(namepc)]
|       mov c_arg2, (mode)
|       call =>(emit_symbol_once_open (Dst))
|       sub rsp, 2 * #gpointer
|       mov [rsp], rax
|       mov c_arg1, rax
|       mov c_arg2, (fileno)
|       call =>(emit_symbol_once_dup2 (Dst))
|       mov c_arg1, [rsp]
|       add rsp, 2 * #gpointer
|       call =>(emit_symbol_once_close (Dst))
||       break;
||    }
|
||    case J_INVOKE_STD_FILE_TYPE_PIPE:
||    {
||      guint offset;
||
||      switch (fileno)
||      {
||        case STDIN_FILENO: offset = 0; break;
||        case STDOUT_FILENO: offset = 1; break;
||        default: g_assert_not_reached ();
||      }
||
|       mov rax, Pipes
|       lea rax, JPipe:rax [desc->fd] [offset]
|       movsxd c_arg1, dword [rax]
|       mov c_arg2, (fileno)
|       call =>(emit_symbol_once_dup2 (Dst))
||      break;
||    }
||  }
||}
|
||void j_context_emit (Dst_DECL, JWalker* walker)
||{
||#define DECL_VARARRAY(name,ctype,pre) \
||  ctype* name = NULL; \
||  ctype* name##_dyn = NULL; \
||  ctype name##_stat [(pre)];
||#define INIT_VARARRAY(name,now) \
||  G_STMT_START \
||    { \
||      gsize __now = ((now)); \
|| ; \
||      if (G_N_ELEMENTS (name##_stat) >= __now) \
||        name = name##_stat; \
||      else \
||        { \
||          name##_dyn = g_malloc (sizeof (*name) * __now); \
||          name = name##_dyn; \
||        } \
||    } \
||  G_STMT_END
||#define CLEAR_VARARRAY(name) g_clear_pointer (& name##_dyn, g_free)
||
||  DECL_VARARRAY (arguments, guint, 32);
||  DECL_VARARRAY (expansions, guint, 32);
||  DECL_VARARRAY (invocations, guint, 32);
||
||  GList* list;
||  guint i;
||
||  INIT_VARARRAY (arguments, g_queue_get_length (&walker->arguments));
||  INIT_VARARRAY (expansions, g_queue_get_length (&walker->expansions));
||  INIT_VARARRAY (invocations, g_queue_get_length (&walker->invocations));
||
||  for (list = g_queue_peek_head_link (&walker->arguments), i = 0; list; list = list->next)
||    {
||      arguments [i++] = emit_string (Dst, list->data, strlen (list->data) + 1);
||    }
||
||  for (list = g_queue_peek_head_link (&walker->expansions), i = 0; list; list = list->next)
||    {
||      expansions [i++] = Dst->expansions->len;
||      g_ptr_array_add (Dst->expansions, list->data);
||    }
||
||  /*
||   * Stack:
||   * > JClosure* (self : argument)
||   * > GError** (error : argument)
||   * > GError* (tmperr : local variable)
||   * > JPipe [walker->n_pipes] (pipes : local variable)
||   */
||  guint stacksize = 0
||    + sizeof (JClosure*)
||    + sizeof (GError**)
||    + sizeof (GError*)
||    + sizeof (JPipe*) * (walker->n_pipes > 0 ? 1 : 0)
||    + sizeof (JPipe) * (walker->n_pipes)
||  /* AMD64 System V ABI demands 16 bytes alignment */
||  ; stacksize += 16 - (stacksize % 16);
|
|=>(Dst->nextstage):
|   push rbp
|   mov rbp, rsp
|   sub rsp, (stacksize)
|   mov Self, c_arg1
|   mov Error, c_arg2
|   mov qword Tmperr, 0
|
||  if (walker->n_pipes > 0)
||  {
|     mov Pipes, rsp
|
||    for (i = 0; i < walker->n_pipes; ++i)
||    {
|       mov c_arg1, Pipes
|       lea c_arg1, JPipe:c_arg1 [i]
|       call =>(emit_symbol_once_pipe (Dst))
||    }
||  }
|
||  Dst->nextstage = j_context_allocpc (Dst);
|
||  for (i = 0; i < g_queue_get_length (&walker->invocations); ++i)
||    {
||      invocations [i] = j_context_allocpc (Dst);
|
|       mov c_arg1, Self
|       mov c_arg2, Pipes
|       lea c_arg3, Tmperr
|       call =>(invocations [i])
|
|       mov rcx, Tmperr
|       test rcx, rcx
|       jz >1
|
|       mov c_arg2, rcx
|       mov c_arg1, Error
|       call extern g_propagate_error
|       jmp >2
|     1:
|       mov c_arg2, rax
|       mov rax, Self
|       lea c_arg1, JClosure:rax->waitq
|       call extern g_queue_push_tail
||    }
|
||  if (walker->n_pipes > 0)
||    {
|       mov c_arg1, Pipes
|       mov c_arg2, (walker->n_pipes)
|       call =>(emit_symbol_once_close_pipes (Dst))
||    }
|
| 2:
|   mov rax, Self
|   lea rcx, [=>(Dst->nextstage)]
|   leave
|   mov JClosure:rax->entry, rcx
|   mov rax, RetContinue
|   ret
|
|.macro adjustio
||  adjust_stdfile (Dst, walker, invoke, invoke->stdin_type, STDIN_FILENO, & invoke->stdin);
||  adjust_stdfile (Dst, walker, invoke, invoke->stdout_type, STDOUT_FILENO, & invoke->stdout);
||  guint closepipespc = emit_symbol_once_close_pipes (Dst);
||  if (walker->n_pipes > 0)
||    {
|       mov c_arg1, Pipes
|       mov c_arg2, (walker->n_pipes)
|       call =>(closepipespc)
||    }
|.endmacro
|.macro splitproc
|   call =>(emit_symbol_once_fork (Dst))
|   test rax, rax
|   jz >1
|   leave
|   ret
| 1:
|.endmacro
|.macro splitadjust
|   splitproc
|   adjustio
|.endmacro
||
|.macro loadarg, register, index_
||  const JArgument* args = & invoke->target;
||  const JArgument* arg = & args [index_];
||
||  switch (arg->type)
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
|.endmacro
|.macro loadargs
||  guint n_arguments = (invoke->n_arguments + 1);
||  guint stacksize_ = (n_arguments + 1) * sizeof (gchar*);
||    stacksize_ += 16 - (stacksize_ % 16);
||  guint i_;
|
|   sub rsp, (stacksize_)
|
||  for (i_ = 0; i_ < n_arguments; ++i_)
||    {
|       loadarg rax, i_
|       mov gpointer:rsp [i_], rax
||    }
|   mov qword gpointer:rsp [n_arguments], 0
|.endmacro
|
||for (list = g_queue_peek_head_link (&walker->invocations), i = 0; list; list = list->next, ++i)
||  {
||    JInvoke* invoke = list->data;
|=>(invocations [i]):
    /* Keep stack 16 bytes aligned */
|     push rbp
|     mov rbp, rsp
    /* Allocate an extra pointer in behalf of alignment */
|     sub rsp, #gpointer * 4
|     mov Self, c_arg1
|     mov Pipes, c_arg2
|     mov Error, c_arg3
|
||    if (invoke->target_type == J_INVOKE_TARGET_TYPE_REGULAR)
||      {
|         splitadjust
|         loadargs
|         mov c_arg1, gpointer:rsp [0]
|         lea c_arg2, gpointer:rsp [0]
|         call =>(emit_symbol_once_execvp (Dst))
||      }
||    else
||      {
||        guint messagepc = 0;
||        const gchar* message = NULL;
||        const gchar* value = invoke->target.builtin;
|
||        if (value == J_TOKEN_BUILTIN_AGAIN)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_CD)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_EXIT)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_FALSE)
||          {
|             splitadjust
|             mov c_arg1, Error
|             mov c_arg2, 1
|             call extern j_set_closure_error_exit
|             leave
|             ret
||          }
||        else if (value == J_TOKEN_BUILTIN_FG)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_GET)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_HELP)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_HISTORY)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_JOBS)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_SET)
||          {
||            g_assert_not_reached ();
||          }
||        else if (value == J_TOKEN_BUILTIN_TRUE)
||          {
|             splitadjust
|             mov c_arg1, Error
|             mov c_arg2, 0
|             call extern j_set_closure_error_exit
|             leave
|             ret
||          }
||        else if (value == J_TOKEN_BUILTIN_UNSET)
||          {
||            g_assert_not_reached ();
||          }
||      }
||  }
|
||CLEAR_VARARRAY (invocations);
||CLEAR_VARARRAY (expansions);
||CLEAR_VARARRAY (arguments);
#undef DECL_VARARRAY
#undef INIT_VARARRAY
#undef CLEAR_VARARRAY
||}
|
||void j_context_ljmp (Dst_DECL, gpointer address)
||{
|=>(Dst->nextstage):
|   mov64 rax, ((guint64) address)
|   jmp rax
||}
|
||void j_context_store (Dst_DECL, gconstpointer buffer, gsize bufsz)
||{
||  const gsize n_dwords = (bufsz / 4);
||  const guint32* dwords = & G_STRUCT_MEMBER (guint32, buffer, 0);
||  gsize i;
||
||  for (i = 0; i < n_dwords; i++)
||    {
|     .dword (GUINT32_TO_LE (dwords [i]))
||    }
||
||  const gsize n_bytes = (bufsz % 4);
||  const guint8* bytes = & G_STRUCT_MEMBER (guint8, buffer, n_dwords * 4);
||
||  for (i = 0; i < n_bytes; i++)
||    {
|     .byte (bytes [i])
||    }
||}
#endif // __CODEGEN__
