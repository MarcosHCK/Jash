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
#ifndef __JASH_CODEGEN_CONTEXT__
#define __JASH_CODEGEN_CONTEXT__ 1
#include <codegen/block.h>
#include <codegen/codegen.h>

typedef struct _JContext JContext;
typedef struct _JClosure JClosure;
typedef struct _JExtern JExtern;
typedef struct _JWalker JWalker;

typedef void (*JCallback) ();

#define Dst_DECL JContext* Dst
#define Dst_REF (Dst->state)

#if DEVELOPER == 1
# define DASM_CHECKS
# include <codegen/debug/gdb.h>
#endif // DEVELOPER

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

#define J_CONTEXT_LABEL_MAIN (0)

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
    guint nextstage;

    GPtrArray* expansions;
    GHashTable* symbols;
    GHashTable* strtab;
#if DEVELOPER == 1
    JGdbBuilder debug_builder;
#endif // DEVELOPER
  };

  struct _JClosure
  {
    GClosure closure;
    GQueue waitq;
    JBlock block;
    gpointer entry;
#if DEVELOPER == 1
    JGdb* debug_object;
#endif // DEVELOPER

    union
    {
      GClosure* closure;
      gchar* expanded;
    } *expansions;
  };

  struct _JExtern
  {
    gint name;
    JCallback address;
  };

  G_GNUC_INTERNAL void j_context_clear (Dst_DECL);
  G_GNUC_INTERNAL void j_context_complete (Dst_DECL);
#if DEVELOPER == 1
  G_GNUC_INTERNAL void j_context_debug_build (Dst_DECL);
#endif // DEVELOPER
  G_GNUC_INTERNAL void j_context_emit (Dst_DECL, JWalker* walker);
  G_GNUC_INTERNAL void j_context_generate (Dst_DECL, JAst* ast);
  G_GNUC_INTERNAL void j_context_init (Dst_DECL);
  G_GNUC_INTERNAL void j_context_ljmp (Dst_DECL, gpointer address);
  G_GNUC_INTERNAL void j_context_store (Dst_DECL, gconstpointer buffer, gsize bufsz);

  G_GNUC_INTERNAL const JExtern* j_extern_lookup (const gchar* name, size_t length);
  G_GNUC_INTERNAL const gint32 j_extern_search (Dst_DECL, gconstpointer address, const gchar* name, int type);

  #define J_CALLBACK(func) ((JCallback) ((func)))

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
  #define j_context_symbol_once(context,name) \
    (({ \
        JContext* __context = ((context)); \
        gpointer __name = G_STRINGIFY (name); \
        gpointer __value = GINT_TO_POINTER (globl_##name ); \
 ; \
        if (!g_hash_table_contains (__context->symbols, __name)) \
          { \
            emit_symbol_once_##name (__context); \
            g_hash_table_insert (__context->symbols, __name, __value); \
          } \
      }))

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_CONTEXT__
