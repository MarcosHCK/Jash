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
#ifndef __JASH_CODEGEN_COMMON__
#define __JASH_CODEGEN_COMMON__ 1
#include <ast.h>
#include <codegen.h>

typedef struct _JCodegenClosure JCodegenClosure;
typedef struct _JCodegenExtern JCodegenExtern;

#define DASM_FDEF G_GNUC_INTERNAL
#define DASM_EXTERN(ctx, addr, idx, type) \
    (j_codegen_extern_find ((ctx), (addr), extern_names [(idx)], (type)))
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
            exit (1); \
          (sz) = _sz; \
        } \
      } \
    G_STMT_END
#define DASM_M_FREE(ctx, p, sz) g_free (p)

#define Dst_DECL JCodegen* Dst
#define Dst_REF (Dst->context)

#include <dynasm/dasm_proto.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JCodegenClosure
  {
    GClosure closure;
    GCallback entry;
    gpointer block;
    gsize blocksz;
  };

  struct _JCodegenExtern
  {
    int name;
    GCallback callback;
  };

  G_GNUC_INTERNAL int j_codegen_allocpc (JCodegen* codegen);

  /* Backend-specific */
  G_GNUC_INTERNAL void j_codegen_absjump (JCodegen* codegen, gconstpointer value);

  /* DynASM externs - API */
  G_GNUC_INTERNAL gint32 j_codegen_extern_find (JCodegen* codegen, gconstpointer address, const gchar* name, int type);
  G_GNUC_INTERNAL const JCodegenExtern* j_codegen_extern_lookup (const gchar *str, size_t len);

  /* DynASM externs - callbacks */
  G_GNUC_INTERNAL void j_codegen_breakpoint ();

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_COMMON__
