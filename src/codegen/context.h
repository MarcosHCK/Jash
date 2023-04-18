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

#define J_CONTEXT_LABEL_MAIN (0)

#if __cplusplus
extern "C"
{
#endif // __cplusplus

  struct _JContext
  {
    dasm_State* state;
    gpointer* labels;
    guint maxpc;
    guint nextpc;
    guint n_labels;
  };

  struct _JClosure
  {
    GClosure closure;
    gpointer entry;
    JBlock block;
  };

  struct _JExtern
  {
    gint name;
    GCallback address;
  };

  G_GNUC_INTERNAL void j_context_clear (Dst_DECL);
  G_GNUC_INTERNAL void j_context_store (Dst_DECL, gpointer buffer, gsize bufsz);
  G_GNUC_INTERNAL void j_context_epilog (Dst_DECL);
  G_GNUC_INTERNAL void j_context_generate (Dst_DECL, JAst* ast);
  G_GNUC_INTERNAL void j_context_init (Dst_DECL);
  G_GNUC_INTERNAL void j_context_ljmp (Dst_DECL, gpointer address);
  G_GNUC_INTERNAL void j_context_prolog (Dst_DECL);

  G_GNUC_INTERNAL const JExtern* j_extern_lookup (const gchar* name, size_t length);
  G_GNUC_INTERNAL const gint32 j_extern_search (Dst_DECL, gconstpointer address, const gchar* name, int type);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_CONTEXT__
