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
#if DEVELOPER == 1
# include <codegen/debug/gdb.h>
#endif // DEVELOPER

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
  GClosure** children = jc->children;
  guint i, top = jc->n_children;

  j_block_clear (&jc->block);

  for (i = 0; i < top; ++i)
    {
      g_closure_unref (children [i]);
    }
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
  GPtrArray* children = NULL;
#if DEVELOPER == 1
  JGdb* gdb = NULL;
#endif // DEVELOPER
  GClosure* gc = NULL;
  JClosure* jc = NULL;
  GError* tmperr = NULL;
  guint i;

  int result = 0;
  size_t sz = 0;

  j_context_init (&context);
  j_context_prolog (&context);
  j_context_generate (&context, ast);
  j_context_epilog (&context);

  if (context.expansions->len > 0)
    {
      GDestroyNotify notify;

      notify = (GDestroyNotify) g_closure_unref;
      children = g_ptr_array_new_with_free_func (notify);
    }

  for (i = 0; i < context.expansions->len; ++i)
    {
      gpointer* expansion = & g_ptr_array_index (context.expansions, i);
      JAst* ast = G_STRUCT_MEMBER (JAst*, expansion, 0);
      GClosure* closure = NULL;

      if ((closure = j_codegen_emit (self, ast, &tmperr)), G_LIKELY (tmperr == NULL))
        g_ptr_array_add (children, closure);
      else
        {
          g_propagate_error (error, tmperr);
          g_ptr_array_unref (children);
          j_context_clear (&context);
          return NULL;
        }
    }

  if ((result = dasm_link (&context, &sz)), G_UNLIKELY (result != 0))
    {
      g_set_error_literal (error, J_CODEGEN_ERROR, J_CODEGEN_ERROR_PROGRAM_LINK, "dasm_link()!: failed");
      g_ptr_array_unref (children);
      j_context_clear (&context);
    }

  gc = g_closure_new_simple (sizeof (JClosure), g_object_ref (codegen));
  jc = (JClosure*) gc;

  g_closure_add_finalize_notifier (gc, codegen, (GClosureNotify) g_object_unref);
  g_closure_add_finalize_notifier (gc, NULL, (GClosureNotify) closure_notify);
#if DEVELOPER == 1
  g_closure_add_finalize_notifier (gc, gdb = j_gdb_new (), (GClosureNotify) j_gdb_free);
#endif // DEVELOPER
  g_closure_set_marshal (gc, (GClosureMarshal) closure_marshal);

  if (context.expansions->len == 0)
    jc->n_children = 0;
  else
    {
      jc->n_children = (guint) children->len;
      jc->children = (GClosure**) g_ptr_array_free (children, FALSE);
    }

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

#if DEVELOPER == 1
  JGdbSection* section;
  JGdbSymbol* symbol;

  section = j_gdb_decl_section (gdb, "text", jc->block.ptr, jc->block.sz);
  symbol = j_gdb_decl_function (gdb, "closure_entry", context.labels [J_CONTEXT_LABEL_MAIN], section);

  j_gdb_finish (gdb);
  j_gdb_register (gdb);
#endif // DEVELOPER
  jc->entry = G_CALLBACK (context.labels [J_CONTEXT_LABEL_MAIN]);
return (j_block_protect (&jc->block), j_context_clear (&context), gc);
}
