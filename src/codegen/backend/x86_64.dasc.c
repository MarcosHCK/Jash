/* Copyright 2023 MarcosHCK
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
#include <codegen/externs.h>
#include <codegen/walker.h>
#if DEVELOPER == 1
# include <bfd.h>
# include <codegen/debug/gdb.h>
#endif // DEVELOPER
#include <lexer/token.h>

#ifndef __INTELLISENSE__
|.arch x64
|.include codegen/backend/common.dasc.c
|
|.define self, gpointer:rbp [-1]
|.define runner, gpointer:rbp [-2]
|.define error, gpointer:rbp [-3]
|.define tmperr, gpointer:rbp [-4]
|.define pipes, gpointer:rbp [-5]
|.define rtmp, r10
|.define rsv1, rbx
|.define rsv2, r15
|
|.if PLATFORM == 'linux'
| .define c_arg1, rdi
| .define c_arg2, rsi
| .define c_arg3, rdx
| .define c_arg4, rcx
|.elif PLATFORM == 'win32'
| .define c_arg1, rcx
| .define c_arg2, rdx
| .define c_arg3, r8
| .define c_arg4, r9
|.endif
|
||#include <dynasm/dasm_x86.h>
||
||const guint j_gdb_default_arch = bfd_arch_i386;
||const guint j_gdb_default_mach = bfd_mach_x86_64;
||typedef union _JInvokeStdfile JInvokeStdfile;
||
|.macro j_step_adjust_io
|   j_step_adjust_io_file stdin, STDIN_FILENO
|   j_step_adjust_io_file stdout, STDOUT_FILENO
||
||  if (walker->n_pipes > 0)
||    {
|       mov c_arg1, pipes
|       mov c_arg2, (walker->n_pipes)
|       call extern j_pipe_clear_many
||    }
|.endmacro
|.macro j_step_adjust_io_file, file, fileno
||  switch (invoke-> .. file .. _type)
||  {
||    case J_INVOKE_STD_FILE_TYPE_FILE:
||    if (invoke-> .. file .. .filename == NULL)
||      break;
||    else
||      {
|         lea c_arg1, [=>(j_tag_once_string_as_pc (Dst, invoke-> .. file .. .filename))]
|         mov c_arg2, (j_invoke_get_open_flags (fileno, invoke->stdout_mode == J_INVOKE_STD_FILE_MODE_APPEND))
|         mov c_arg3, (j_invoke_get_open_mode (fileno, invoke->stdout_mode == J_INVOKE_STD_FILE_MODE_APPEND)) 
|         lea c_arg4, tmperr
|         call extern j_open
|
|         mov c_arg2, tmperr
|         test c_arg2, c_arg2
|         jz >9
|           mov c_arg1, error
|           call extern g_propagate_error
|           leave
|           ret
|         9:
|           sub rsp, #gpointer * 2
|           mov [rsp], rax
|           mov c_arg1, rax
|           mov c_arg2, fileno
|           lea c_arg3, tmperr
|           call extern j_dup2
|
|           mov c_arg2, tmperr
|           test c_arg2, c_arg2
|           jz >9
|             mov c_arg1, error
|             call extern g_propagate_error
|             leave
|             ret
|           9:
|             mov c_arg1, [rsp]
|             mov c_arg2, 0
|             add rsp, #gpointer * 2
|             call extern g_close
||        break;
||      }
||    case J_INVOKE_STD_FILE_TYPE_PIPE:
|       mov c_arg1, pipes
|       lea c_arg1, JPipe:c_arg1 [invoke-> .. file .. .fd] [(gint) fileno]
|       movsxd c_arg1, dword [c_arg1]
|       mov c_arg2, (fileno)
|       lea c_arg3, tmperr
|       call extern j_dup2
|
|       mov c_arg2, tmperr
|       test c_arg2, c_arg2
|       jz >9
|         mov c_arg1, error
|         call extern g_propagate_error
|         leave
|         ret
|       9:
||      break;
||  }
|.endmacro
|.macro j_step_branch_put_last
|   mov rax, RetRemove
|   ret
|.endmacro
|.macro j_step_branch_set_fail, closure
|   lea rtmp, [=>(j_tag_once_symbol_as_pc (Dst, j_branch_fail))]
|   mov JClosure:closure->entry, rtmp
|.endmacro
|.macro j_step_branch_set_tag, closure, tag
|   lea rtmp, [=>(j_tag_as_pc (tag))]
|   mov JClosure:closure->entry, rtmp
|.endmacro
|.macro j_step_fork
|   lea c_arg1, tmperr
|   call extern j_fork
|
|   mov c_arg2, tmperr
|   test c_arg2, c_arg2
|   jz >9
|     mov c_arg1, error
|     call extern g_propagate_error
|     leave
|     ret
|   9:
|.endmacro
|.macro j_step_fork_and_report, error_code
|   j_step_fork
|   test rax, rax
|   jz >9
|     leave
|     ret
|   9:
|     j_step_adjust_io
|     j_step_report error_code
|     leave
|     ret
|.endmacro
|.macro j_step_load_arg, index_, register
||  G_STMT_START
||    {
||      JArgument* __arguments = & invoke->target;
||      JArgument* __argument = & __arguments [index_];
||
||      switch (__argument->type)
||      {
||        case J_ARGUMENT_TYPE_DATA:
|           lea register, [=>(j_tag_as_pc (& argument_tags [__argument->index]))]
||          break;
||        case J_ARGUMENT_TYPE_EXPANSION:
|           mov register, self
|           mov register, JClosure:register->expansions
|           mov register, gpointer:register [expansion_indices [__argument->index]]
||          break;
||      }
||    }
||  G_STMT_END;
|.endmacro
|.macro j_step_report, error_code
|   mov c_arg1, error
|   mov c_arg2, error_code
|   call extern j_set_closure_error_exit
|.endmacro
||
||void j_context_emit_absolute_jump (Dst_DECL, gpointer address, const JTag* tag)
||{
|=>(j_tag_as_pc (tag)):
|   mov64 rax, ((guintptr) address)
|   jmp rax
||}
||
||void j_context_emit_chain_empty (Dst_DECL, const JTag* tag, const JTag* tag_next)
||{
|=>(j_tag_as_pc (tag)):
|   j_step_branch_set_tag, c_arg1, tag_next
|   mov rax, RetContinue
|   ret
||}
||
||void j_context_emit_chain_last (Dst_DECL, const JTag* tag)
||{
|=>(j_tag_as_pc (tag)):
|   j_step_branch_put_last
||}
||
||void j_context_emit_chain_step (Dst_DECL, JWalker* walker, const JTag* tag, const JTag* tag_next)
||{
#define J_VARARRAY_DECL(name,ctype,pre) \
||  ctype* name = NULL; \
||  ctype* name##_dyn = NULL; \
||  ctype name##_stat [(pre)];
#define J_VARARRAY_INIT(name,now) \
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
#define J_VARARRAY_CLEAR(name) g_clear_pointer (& name##_dyn, g_free)
||
||  J_VARARRAY_DECL (argument_tags, JTag, 32);
||  J_VARARRAY_INIT (argument_tags, g_queue_get_length (&walker->arguments));
||  J_VARARRAY_DECL (expansion_indices, guint, 32);
||  J_VARARRAY_INIT (expansion_indices, g_queue_get_length (&walker->expansions));
||  J_VARARRAY_DECL (invocation_tags, JTag, 32);
||  J_VARARRAY_INIT (invocation_tags, g_queue_get_length (&walker->invocations));
||
||  GList* list;
||  guint i, j;
||
||  for (list = g_queue_peek_head_link (&walker->arguments), i = 0; list; list = list->next, ++i)
||    {
||      j_tag_once_string (Dst, & argument_tags [i], list->data);
||    }
||
||  for (list = g_queue_peek_head_link (&walker->expansions), i = 0; list; list = list->next, ++i)
||    {
||      expansion_indices [i] = Dst->expansions->len;
||      g_ptr_array_add (Dst->expansions, list->data);
||    }
||
||/*
|| * Stack (should be 16-bytes aligned):
|| * > JClosure* self; (argument #1)
|| * > JRunner* runner; (argument #2)
|| * > GError** error; (argument #3)
|| * > GError* tmperr; (local variable)
|| * > JPipes* pipes; (local variable) (if any)
|| * > JPipes pipes_ []; (local variable) (if any)
|| * before self goes other two 8-bytes slots
|| * - return address (pushed by call, caller)
|| * - frame pointer (pushed at function entry, callee)
|| */
||  gsize stacksize = 0
|| + sizeof (JClosure*)
|| + sizeof (GError**)
|| + sizeof (GError*)
|| + sizeof (JPipe*) * (walker->n_pipes > 0 ? 1 : 0)
|| + sizeof (JPipe) * (walker->n_pipes)
||  ; stacksize += 16 - (stacksize % 16);
||
|=>(j_tag_as_pc (tag)):
|   push rbp
|   mov rbp, rsp
|   sub rsp, stacksize
|   mov self, c_arg1
|   mov runner, c_arg2
|   mov error, c_arg3
|   mov qword tmperr, 0
||
||  if (walker->n_pipes > 0)
||    {
|       mov pipes, rsp
|       mov c_arg1, rsp
|       mov c_arg2, (walker->n_pipes)
|       lea c_arg3, tmperr
|       call extern j_pipe_init_many
|
|       mov c_arg2, tmperr
|       test c_arg2, c_arg2
|       jz >1
|         mov c_arg1, error
|         call extern g_propagate_error
|         mov rax, self
|         j_step_branch_set_fail rax
|         leave
|         mov rax, RetRemove
|         ret
|       1:
||    }
||
||  for (i = 0; i < g_queue_get_length (&walker->invocations); ++i)
||    {
||      j_tag_init (Dst, & invocation_tags [i]);
||
|       mov c_arg1, self
|       mov c_arg2, runner
|       mov c_arg3, pipes
|       lea c_arg4, tmperr
|       call =>(j_tag_as_pc (& invocation_tags [i]))
|
|       mov c_arg2, tmperr
|       test c_arg2, c_arg2
|       jz >1
|         mov c_arg1, error
|         call extern g_propagate_error
|         mov rax, self
|         j_step_branch_set_fail rax
|         leave
|         mov rax, RetRemove
|         ret
|       1:
|         mov c_arg2, rax
|         mov c_arg1, self
|         lea c_arg1, JClosure:c_arg1->waitq
|         call extern g_queue_push_tail
||    }
||
||  if (walker->n_pipes > 0)
||    {
|       mov pipes, rsp
|       mov c_arg1, rsp
|       mov c_arg2, (walker->n_pipes)
|       call extern j_pipe_clear_many
||    }
||
|   mov rax, self
|   j_step_branch_set_tag rax, tag_next
|   leave
|   mov rax, RetContinue
|   ret
||
||  for (list = g_queue_peek_head_link (&walker->invocations), i = 0; list; list = list->next, ++i)
||    {
||      JInvoke* invoke = list->data;
||      gsize framesz = stacksize - sizeof (JPipe) * (walker->n_pipes);
||          framesz += 16 - (framesz % 16);
|=>(j_tag_as_pc (& invocation_tags [i])):
|       push rbp
|       mov rbp, rsp
|       sub rsp, framesz
|       mov self, c_arg1
|       mov runner, c_arg2
|       mov error, c_arg4
|       mov qword tmperr, 0
||
||      if (walker->n_pipes > 0)
||        {
|           mov pipes, c_arg3
||        }
||
||      if (invoke->target_type == J_INVOKE_TARGET_TYPE_REGULAR)
||        {
|           j_step_fork
|           test rax, rax
|           jz >1
|             leave
|             ret
|           1:
|             j_step_adjust_io
||            guint n_arguments = invoke->n_arguments + 1;
||            guint allocsz = (n_arguments + 1) * sizeof (gchar*);
||                 allocsz += 16 + (allocsz % 16);
||            gboolean use_malloc = allocsz > 1024;
||
||            if (use_malloc)
||              {
|                 mov c_arg1, allocsz
|                 call extern g_malloc
||              }
||            else
||              {
|                 sub rsp, allocsz
|                 mov rax, rsp
||              }
||
||            for (j = 0; j < n_arguments; ++j)
||              {
|                 j_step_load_arg j, rcx
|                 mov gpointer:Rq (use_malloc ? 0 : 4) [j], rcx
||              }
|
|             mov qword gpointer:Rq (use_malloc ? 0 : 4) [n_arguments], 0
|             mov c_arg1, gpointer:Rq (use_malloc ? 0 : 4) [0]
|             lea c_arg2, gpointer:Rq (use_malloc ? 0 : 4) [0]
|             lea c_arg3, tmperr
|             call extern j_execvp
|
|             mov c_arg1, error
|             mov c_arg2, tmperr
|             call extern g_propagate_error
|             leave
|             ret
||        }
||      else if (invoke->target_type == J_INVOKE_TARGET_TYPE_BUILTIN)
||        {
||          const gchar* value = invoke->target.builtin;
||
||          if (value == J_TOKEN_BUILTIN_AGAIN)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_CD)
||            {
||              if (walker->n_pipes > 0 || invoke->n_arguments == 0)
||                {
|                   j_step_fork_and_report 0
||                }
||              else
||                {
|                   j_step_load_arg, 1, c_arg1
|                   lea c_arg2, tmperr
|                   call extern j_chdir
|
|                   mov c_arg2, tmperr
|                   test c_arg2, c_arg2
|                   jnz >1
|                     j_step_fork_and_report 0
|                   1:
|                     sub rsp, #gpointer * 2
|                     mov [rsp], c_arg2
|                     mov qword tmperr, 0
|                     j_step_fork
|                     test rax, rax
|                     jz >1
|                       mov c_arg1, [rsp]
|                       call extern g_error_free
|                       leave
|                       ret
|                     1:
|                       mov c_arg1, error
|                       mov c_arg2, [rsp]
|                       call extern g_propagate_error
|                       mov qword error, 0
|                       j_step_adjust_io
|                       leave
|                       ret
||                }
||            }
||          else if (value == J_TOKEN_BUILTIN_EXIT)
||            {
||              if (walker->n_pipes > 0)
||                {
|                   j_step_fork_and_report 0
||                }
||              else
||                {
|                   j_step_load_arg, 1, c_arg1
|                   lea c_arg2, tmperr
|                   call extern j_parse_int
|
|                   mov c_arg2, tmperr
|                   test c_arg2, c_arg2
|                   jnz >1
|                     j_step_report rax
|                     leave
|                     ret
|                   1:
|                     sub rsp, #gpointer * 2
|                     mov [rsp], c_arg2
|                     mov qword tmperr, 0
|                     j_step_fork
|                     test rax, rax
|                     jz >1
|                       mov c_arg1, [rsp]
|                       call extern g_error_free
|                       leave
|                       ret
|                     1:
|                       mov c_arg1, error
|                       mov c_arg2, [rsp]
|                       call extern g_propagate_error
|                       mov qword error, 0
|                       j_step_adjust_io
|                       leave
|                       ret
||                }
||            }
||          else if (value == J_TOKEN_BUILTIN_FALSE)
||            {
|               j_step_fork_and_report 1
||            }
||          else if (value == J_TOKEN_BUILTIN_FG)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_GET)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_HELP)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_HISTORY)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_JOBS)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_SET)
||            {
||              g_assert_not_reached ();
||            }
||          else if (value == J_TOKEN_BUILTIN_TRUE)
||            {
|               j_step_fork_and_report 0
||            }
||          else if (value == J_TOKEN_BUILTIN_UNSET)
||            {
||              g_assert_not_reached ();
||            }
||          else g_assert_not_reached ();
||        }
||      else g_assert_not_reached ();
||    }
||
||  J_VARARRAY_CLEAR (argument_tags);
||  J_VARARRAY_CLEAR (expansion_indices);
||  J_VARARRAY_CLEAR (invocation_tags);
#undef J_VARARRAY_DECL
#undef J_VARARRAY_INIT
#undef J_VARARRAY_CLEAR
||}
||
||void j_context_emit_test (Dst_DECL, const JTag* tag, const JTag* tag_direct, const JTag* tag_reverse)
||{
|=>(j_tag_as_pc (tag)):
|   movsxd rax, dword JClosure:c_arg1->condition
|   test rax, rax
|   jz >1
|     j_step_branch_set_tag, c_arg1, tag_reverse
|     mov rax, RetContinue
|     ret
|   1:
|     j_step_branch_set_tag, c_arg1, tag_direct
|     mov rax, RetContinue
|     ret
||}
||
||void j_once_init_branch_fail (Dst_DECL)
||{
|   j_step_branch_put_last
||}
||
#endif // __INTELLISENSE__
