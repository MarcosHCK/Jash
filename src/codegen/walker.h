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
#ifndef __JASH_CODEGEN_WALKER__
#define __JASH_CODEGEN_WALKER__ 1
#include <glib.h>

typedef union _JArgument JArgument;
typedef struct _JInvoke JInvoke;
typedef struct _JWalker JWalker;

#define J_INVOKE_STDFILE_TYPE_BITS (1)
#define J_INVOKE_STDFILE_MODE_BITS (1)
#define J_INVOKE_TARGET_TYPE_BITS (1)
#define J_INVOKE_N_ARGUMENTS_BITS ((sizeof (guint) * 8) \
                                  - (J_INVOKE_STDFILE_TYPE_BITS * 2 \
                                  +  J_INVOKE_STDFILE_MODE_BITS \
                                  +  J_INVOKE_TARGET_TYPE_BITS))

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JInvoke
  {
    union _JInvokeStdfile
    {
      const gchar* filename;
      guint fd;
    } stdin, stdout;

    guint stdin_type : J_INVOKE_STDFILE_TYPE_BITS;
    guint stdout_mode : J_INVOKE_STDFILE_MODE_BITS;
    guint stdout_type : J_INVOKE_STDFILE_TYPE_BITS;
    guint target_type : J_INVOKE_TARGET_TYPE_BITS;
    guint n_arguments : J_INVOKE_N_ARGUMENTS_BITS;

    union _JArgument
    {
      const gchar* builtin;

      struct
      {
        guint type : 1;
        guint index : (sizeof (guint) * 8 - 1);
      };
    } target, first_argument;
  };

  struct _JWalker
  {
    GQueue arguments;
    GQueue expansions;
    GQueue invocations;
    guint detach : 1;
    guint n_pipes : (sizeof (guint) * 8 - 1);
  };

  #define J_INVOKE_INIT { NULL, NULL, 0, 0, 0, NULL, NULL, }
  #define J_WALKER_INIT { G_QUEUE_INIT, G_QUEUE_INIT, G_QUEUE_INIT, FALSE, 0, }

  enum
  {
    J_ARGUMENT_TYPE_DATA = 0,
    J_ARGUMENT_TYPE_EXPANSION = 1,
  };

  enum
  {
    J_INVOKE_STD_FILE_MODE_APPEND = 0,
    J_INVOKE_STD_FILE_MODE_REPLACE = 1,
  };

  enum
  {
    J_INVOKE_STD_FILE_TYPE_FILE = 0,
    J_INVOKE_STD_FILE_TYPE_PIPE = 1,
  };

  enum
  {
    J_INVOKE_TARGET_TYPE_BUILTIN = 0,
    J_INVOKE_TARGET_TYPE_REGULAR = 1,
  };

  #define j_walker_clear(walker) \
      (({ \
          JWalker* __walker = ((walker)); \
          g_queue_clear (&__walker->arguments); \
          g_queue_clear (&__walker->expansions); \
          g_queue_clear_full (&__walker->invocations, (GDestroyNotify) g_free); \
          __walker->detach = 0; \
          __walker->n_pipes = 0; \
        }))

  #define j_walker_add_pipe(walker) (({ JWalker* __walker = ((walker)); __walker->n_pipes++; }))
  #define j_walker_mark_detach(walker) (({ JWalker* __walker = ((walker)); __walker->detach = TRUE; }))

  static inline JInvoke* j_invoke_new (guint n_arguments)
  {
    g_assert (g_bit_storage (n_arguments) < J_INVOKE_N_ARGUMENTS_BITS);
    JInvoke* self = NULL;

    const gsize s_size = G_SIZEOF_MEMBER (JInvoke, target) + G_STRUCT_OFFSET (JInvoke, target);
    const gsize a_size = G_SIZEOF_MEMBER (JInvoke, first_argument) * n_arguments;
  return (self = g_malloc (s_size + a_size), self->n_arguments = n_arguments, self);
  }

  static inline guint j_walker_add_argument (JWalker* walker, const gchar* value)
  {
    guint index = g_queue_get_length (&walker->arguments);
                  g_queue_push_tail (&walker->arguments, (gpointer) value);
        return index;
  }

  static inline guint j_walker_add_expansion (JWalker* walker, JAst* ast)
  {
    guint index = g_queue_get_length (&walker->expansions);
                  g_queue_push_tail (&walker->expansions, ast);
        return index;
  }

  static inline guint j_walker_add_invoke (JWalker* walker, JInvoke* invoke)
  {
    guint index = g_queue_get_length (&walker->invocations);
                  g_queue_push_tail (&walker->invocations, invoke);
        return index;
  }

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_WALKER__
