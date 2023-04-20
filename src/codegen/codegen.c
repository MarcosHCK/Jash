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
#ifdef G_OS_WIN32
# include <windows.h>
#else // !G_OS_WIN32
# include <sys/mman.h>
# if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#   define MAP_ANONYMOUS MAP_ANON
# endif // !MAP_ANONYMOUS && MAP_ANON
#endif // G_OS_WIN32

#define J_CODEGEN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_CODEGEN, JCodegenClass))
#define J_IS_CODEGEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_CODEGEN))
#define J_CODEGEN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_CODEGEN, JCodegenClass))
typedef struct _JCodegenClass JCodegenClass;

struct _JCodegen
{
  GObject parent;
};

struct _JCodegenClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (JCodegen, j_codegen, G_TYPE_OBJECT);
G_DEFINE_QUARK (j-codegen-error-quark, j_codegen_error);

static void j_codegen_class_init (JCodegenClass* klass) { }
static void j_codegen_init (JCodegen* self) { }

JCodegen* j_codegen_new ()
{
  return g_object_new (J_TYPE_CODEGEN, NULL);
}

static void closure_notify (gpointer notify_data, JClosure* jc)
{
  j_block_clear (&jc->block);
}

static void closure_marshal (JClosure* jc, GValue* return_value, guint n_param_values, const GValue* param_values)
{
  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);
  g_return_if_fail (param_values != NULL);

  GClosure* gc = (gpointer) jc;
  gboolean (* callback) (JClosure* self, GError** error, gpointer closure_data) = jc->entry;
  gboolean result = callback (jc, g_value_get_pointer (param_values), gc->data);

  g_value_set_boolean (return_value, result);
}

GClosure* j_codegen_emit (JCodegen* codegen, JAst* ast, GError** error)
{
  g_return_val_if_fail (J_IS_CODEGEN (codegen), NULL);
  g_return_val_if_fail (ast != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  JCodegen* self = (codegen);
  JContext context = {0};
  GClosure* gc = NULL;
  JClosure* jc = NULL;
  int result = 0;
  size_t sz = 0;

  j_context_init (&context, self);
  j_context_prolog (&context);
  j_context_generate (&context, ast);
  j_context_epilog (&context);

  if ((result = dasm_link (&context, &sz)), G_UNLIKELY (result != 0))
    {
      g_set_error_literal (error, J_CODEGEN_ERROR, J_CODEGEN_ERROR_PROGRAM_LINK, "dasm_link()!: failed");
      j_context_clear (&context);
    }

  gc = g_closure_new_simple (sizeof (JClosure), g_object_ref (codegen));
  jc = (JClosure*) gc;

  g_closure_add_finalize_notifier (gc, codegen, (GClosureNotify) g_object_unref);
  g_closure_add_finalize_notifier (gc, NULL, (GClosureNotify) closure_notify);
  g_closure_set_marshal (gc, (GClosureMarshal) closure_marshal);

  if (G_LIKELY (gc->floating))
    {
      g_closure_ref (gc);
      g_closure_sink (gc);
    }

  j_block_init (&jc->block, sz);

  if ((result = dasm_encode (&context, j_block_ptr (&jc->block))), G_UNLIKELY (result != 0))
    {
      g_set_error_literal (error, J_CODEGEN_ERROR, J_CODEGEN_ERROR_PROGRAM_ENCODE, "dasm_encode()!: failed");
      g_closure_unref (gc);
      j_context_clear (&context);
    }

  jc->entry = G_CALLBACK (context.labels [J_CONTEXT_LABEL_MAIN]);
return (j_block_protect (&jc->block), j_context_clear (&context), gc);
}
