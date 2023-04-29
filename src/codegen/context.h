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
#ifndef __JASH_CODEGEN_CONTEXT__
#define __JASH_CODEGEN_CONTEXT__ 1
#include <codegen/block.h>
#include <codegen/codegen.h>
#if DEVELOPER == 1
# include <codegen/debug/gdb.h>
# define DASM_CHECKS
#endif // DEVELOPER
#include <codegen/tag.h>
#include <runtime/runner.h>

typedef struct _JContext JContext;
typedef struct _JClosure JClosure;
typedef struct _JExtern JExtern;
typedef const gchar JOnceID;
typedef struct _JOnceInit JOnceInit;
typedef gint JPipe [2];
typedef gint JPipeEnd;
typedef struct _JWalker JWalker;

typedef void (*JCallback) ();
typedef JClosureStatus (*JClosureCallback) (JClosure* closure, JRunner* runner, GError** error);

#define Dst_DECL JContext* Dst
#define Dst_REF (Dst->state)

#define DASM_FDEF G_GNUC_INTERNAL
#define DASM_EXTERN(ctx, addr, idx, type) \
    (j_extern_search ((ctx), (addr), extern_names [(idx)], (type)))
#define DASM_M_GROW(ctx, t, p, sz, need) \
    G_STMT_START \
      { \
        size_t _sz = (sz), _need = (need); \
        if (_sz < _need) \
        { \
          if (_sz < 16) _sz = 16; \
          while (_sz < _need) \
            _sz += _sz; \
          (p) = (t *) g_realloc ((p), _sz); \
          if ((p) == NULL) \
            g_assert_not_reached (); \
          (sz) = _sz; \
        } \
      } \
    G_STMT_END
#define DASM_M_FREE(ctx, p, sz) g_free (p)
#include <dynasm/dasm_proto.h>

#if __cplusplus
extern "C"
{
#endif // __cplusplus

  struct _JContext
  {
    dasm_State* state;

    gpointer* labels;
    guint n_labels;
  
    guint maxpc;
    guint nextpc;

    guint max_expansions;
    GHashTable* symbols;
    GHashTable* strtab;
#if DEVELOPER == 1
    GHashTable* debug_info;
    JGdbBuilder debug_builder;
#endif // DEVELOPER
  };

  struct _JClosure
  {
    GClosure closure;
    JBlock block;
    JClosureCallback entry;
    JPipeEnd* expansion_pipes;
    gchar** expansion_values;
    guint expansions_count;
    GQueue waitq;
    gboolean condition;
#if DEVELOPER == 1
    JGdb* debug_object;
#endif // DEVELOPER
  };

  struct _JExtern
  {
    gint name;
    JCallback address;
  };

  struct _JOnceInit
  {
    gint name;
    void (*callback) (Dst_DECL);
  };

  #define j_context_allocpc(context) \
    (({ \
        JContext* __context = ((context)); \
        guint __nextpc = __context->nextpc++; \
 ; \
        if (__nextpc == __context->maxpc) \
          { \
            dasm_growpc (__context, __context->maxpc *= 2); \
          } \
 ; \
        __nextpc; \
      }))

  #define J_CALLBACK(func) ((JCallback) ((func)))

  G_GNUC_INTERNAL void j_context_clear (Dst_DECL);
  G_GNUC_INTERNAL void j_context_emit_absolute_jump (Dst_DECL, gpointer address, const JTag* tag);
#if DEVELOPER == 1
  G_GNUC_INTERNAL void j_context_emit_debuginfo (Dst_DECL);
#endif // DEVELOPER
  G_GNUC_INTERNAL void j_context_emit_chain_empty (Dst_DECL, const JTag* tag, const JTag* tag_next);
  G_GNUC_INTERNAL void j_context_emit_chain_last (Dst_DECL, const JTag* tag);
  G_GNUC_INTERNAL void j_context_emit_chain_last_and_report (Dst_DECL, guint exit_code, const JTag* tag);
  G_GNUC_INTERNAL void j_context_emit_chain_step (Dst_DECL, JWalker* walker, const JTag* tag, const JTag* tag_next);
  G_GNUC_INTERNAL void j_context_emit_chain_step_expansions (Dst_DECL, JWalker* walker, const JTag* tag, const JTag* tag_next);
  G_GNUC_INTERNAL void j_context_emit_chain_step_expression (Dst_DECL, JWalker* walker, const JTag* tag, const JTag* tag_next);
  G_GNUC_INTERNAL void j_context_emit_test (Dst_DECL, const JTag* tag, const JTag* tag_direct, const JTag* tag_reverse);
  G_GNUC_INTERNAL void j_context_finish (Dst_DECL);
  G_GNUC_INTERNAL void j_context_generate (Dst_DECL, JAst* ast, const JTag* tag);
  G_GNUC_INTERNAL void j_context_init (Dst_DECL);
  G_GNUC_INTERNAL void j_context_store (Dst_DECL, gconstpointer buffer, gsize bufsz);

  G_GNUC_INTERNAL const JExtern* j_extern_lookup (const gchar* name, size_t length);
  G_GNUC_INTERNAL const gint32 j_extern_search (Dst_DECL, gconstpointer address, const gchar* name, int type);

  G_GNUC_INTERNAL void j_once_init (Dst_DECL, GHashTable* table, JOnceID* once, JTag* tag);
  G_GNUC_INTERNAL void j_once_init_branch_fail (Dst_DECL);
  G_GNUC_INTERNAL const JOnceInit* j_once_lookup (const gchar* name, size_t length);

  G_GNUC_INTERNAL void j_set_closure_error_again (GError** error, const gchar* value);
  G_GNUC_INTERNAL void j_set_closure_error_chdir (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);
  G_GNUC_INTERNAL void j_set_closure_error_dup2 (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);
  G_GNUC_INTERNAL void j_set_closure_error_execvp (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);
  G_GNUC_INTERNAL void j_set_closure_error_exit (GError** error, int value);
  G_GNUC_INTERNAL void j_set_closure_error_fork (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);
  G_GNUC_INTERNAL void j_set_closure_error_open (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);
  G_GNUC_INTERNAL void j_set_closure_error_pipe (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);
  G_GNUC_INTERNAL void j_set_closure_error_waitpid (GError** error, int errno_value, const gchar* fmt, ...) G_GNUC_PRINTF (3, 4);

  #define j_tag_as_offset(context,src) \
    (({ \
        guint __pc = j_tag_as_pc ((src)); \
        dasm_getpclabel ((context), __pc); \
      }))
  #define j_tag_init(context,dst) \
    (({ \
        JTag* __dst = ((dst)); \
        guint __pc = j_context_allocpc ((context)); \
          *__dst = GUINT_TO_POINTER (__pc); \
       }))
  #define j_tag_once_symbol(context,dst,name) \
    (({ \
        JContext* __context = ((context)); \
        JOnceID* __once = G_STRINGIFY (name); \
        j_once_init (__context, __context->symbols, __once, (dst)); \
      }))
  #define j_tag_once_symbol_as_pc(context,name) \
    (({ \
        JTag __tag; \
        j_tag_once_symbol ((context), &__tag, name); \
        j_tag_as_pc (&__tag); \
      }))
  #define j_tag_once_string(context,dst,value) \
    (({ \
        JContext* __context = ((context)); \
        JOnceID* __once = ((value)); \
        j_once_init (__context, __context->strtab, __once, (dst)); \
      }))
  #define j_tag_once_string_as_pc(context,name) \
    (({ \
        JTag __tag; \
        j_tag_once_string ((context), &__tag, name); \
        j_tag_as_pc (&__tag); \
      }))

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_CONTEXT__
