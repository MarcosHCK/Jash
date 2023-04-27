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
#include <parser/ast.h>

#define J_TYPE_CODEGEN (j_codegen_get_type ())
#define J_CODEGEN(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), J_TYPE_CODEGEN, JCodegen))
#define J_IS_CODEGEN(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), J_TYPE_CODEGEN))
typedef struct _JCodegen JCodegen;

#define J_CLOSURE_ERROR (j_closure_error_quark ())
#define J_CODEGEN_ERROR (j_codegen_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    J_CODEGEN_ERROR_FAILED,
    J_CODEGEN_ERROR_BLOCK_ALLOC,
    J_CODEGEN_ERROR_BLOCK_PROTECT,
    J_CODEGEN_ERROR_PROGRAM_ENCODE,
    J_CODEGEN_ERROR_PROGRAM_LINK,
  } JCodegenError;

  typedef enum
  {
    J_CLOSURE_ERROR_FAILED,
    J_CLOSURE_ERROR_CHDIR,
    J_CLOSURE_ERROR_DUP2,
    J_CLOSURE_ERROR_EXECVP,
    J_CLOSURE_ERROR_EXIT,
    J_CLOSURE_ERROR_FORK,
    J_CLOSURE_ERROR_IRQ,
    J_CLOSURE_ERROR_OPEN,
    J_CLOSURE_ERROR_PIPE,
    J_CLOSURE_ERROR_WAITPID,
  } JClosureError;

  typedef enum
  {
    J_CLOSURE_STATUS_REMOVE = 0,
    J_CLOSURE_STATUS_CONTINUE = 1,
  } JClosureStatus;

  G_GNUC_INTERNAL GQuark j_closure_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GValue* j_closure_error_value (const GError* error);
  G_GNUC_INTERNAL GQuark j_codegen_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GType j_codegen_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JCodegen* j_codegen_new ();
  G_GNUC_INTERNAL GClosure* j_codegen_emit (JCodegen* codegen, JAst* ast, gboolean interactive, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN__
