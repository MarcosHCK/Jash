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
#ifndef __JASH_CODEGEN__
#define __JASH_CODEGEN__ 1
#include <glib-object.h>

typedef struct _JCodegen JCodegen;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JCodegen
  {
    gpointer context;
    gpointer* labels;
    guint maxpc;
    guint nextpc;
    guint n_labels;
  };

  #define J_CODEGEN_LABEL_MAIN (0)

  G_GNUC_INTERNAL void j_codegen_init (JCodegen* codegen);
  G_GNUC_INTERNAL void j_codegen_clear (JCodegen* codegen);
  G_GNUC_INTERNAL void j_codegen_prologue (JCodegen* codegen);
  G_GNUC_INTERNAL void j_codegen_epilogue (JCodegen* codegen);
  G_GNUC_INTERNAL void j_codegen_generate (JCodegen* codegen, Ast ast);
  G_GNUC_INTERNAL GClosure* j_codegen_emit (JCodegen* codegen);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN__
