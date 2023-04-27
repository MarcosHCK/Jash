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
#include <codegen/externs.h>
#if DEVELOPER == 1
# include <codegen/debug/gdb.h>
#endif // DEVELOPER
#include <wait.h>

#define J_CODEGEN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_CODEGEN, JCodegenClass))
#define J_IS_CODEGEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_CODEGEN))
#define J_CODEGEN_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_CODEGEN, JCodegenClass))
typedef struct _JCodegenClass JCodegenClass;
typedef struct _GValue JClosureErrorPrivate;
static void j_closure_error_private_init (JClosureErrorPrivate* priv);
#define j_closure_error_private_copy g_value_copy
#define j_closure_error_private_clear g_value_unset

struct _JCodegen
{
  GObject parent;

  /*<private>*/
  GHashTable* variables;
};

struct _JCodegenClass
{
  GObjectClass parent;
};

G_DEFINE_EXTENDED_ERROR (JClosureError, j_closure_error);
G_DEFINE_QUARK (j-codegen-error-quark, j_codegen_error);
G_DEFINE_FINAL_TYPE (JCodegen, j_codegen, G_TYPE_OBJECT);

static void j_closure_error_private_init (JClosureErrorPrivate* priv)
{
#ifdef HAVE_MEMSET
  memset (priv, 0, sizeof (GValue));
#else // !HAVE_MEMSET
  static GValue __null__ = G_VALUE_INIT;
    *priv = __null__;
#endif // HAVE_MEMSET
}

static void j_codegen_class_finalize (GObject* pself)
{
  JCodegen* self = (gpointer) pself;
  g_hash_table_unref (self->variables);
G_OBJECT_CLASS (j_codegen_parent_class)->finalize (pself);
}

static void j_codegen_class_dispose (GObject* pself)
{
  JCodegen* self = (gpointer) pself;
  g_hash_table_remove_all (self->variables);
G_OBJECT_CLASS (j_codegen_parent_class)->dispose (pself);
}

static void j_codegen_class_init (JCodegenClass* klass)
{
  G_OBJECT_CLASS (klass)->finalize = j_codegen_class_finalize;
  G_OBJECT_CLASS (klass)->dispose = j_codegen_class_dispose;
}

static void j_codegen_init (JCodegen* self)
{
  const GHashFunc func1 = (GHashFunc) g_bytes_hash;
  const GEqualFunc func2 = (GEqualFunc) g_bytes_equal;
  const GDestroyNotify notify = (GDestroyNotify) g_bytes_unref;

  self->variables = g_hash_table_new_full (func1, func2, notify, notify);
}

JCodegen* j_codegen_new ()
{
  return g_object_new (J_TYPE_CODEGEN, NULL);
}

GValue* j_closure_error_value (const GError* error)
{
  return j_closure_error_get_private (error);
}

static void closure_nofity (gpointer __null__, JClosure* jc)
{
  g_queue_clear (&jc->waitq);
  j_block_clear (&jc->block);
#if DEVELOPER == 1
  j_gdb_unregister (jc->debug_object);
  j_gdb_free (jc->debug_object);
#endif // DEVELOPER
}

static void closure_marshal (JClosure* jc, GValue* return_value, guint n_param_values, const GValue* param_values)
{
  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);
  g_return_if_fail (param_values != NULL);
  GClosure* gc = (gpointer) jc;
  GError** error = g_value_get_pointer (param_values);
  GError* tmperr = NULL;
  GList* list;

  while ((list = g_queue_peek_head_link (&jc->waitq)) != NULL)
    {
      gint pid = GPOINTER_TO_INT (list->data);
      gint status = 0;

      if ((j_waitpid (pid, &status, WNOHANG, &tmperr)), G_UNLIKELY (tmperr != NULL))
      {
        g_propagate_error (error, tmperr);
        g_value_set_int (return_value, J_CLOSURE_STATUS_REMOVE);
        return;
      }
      else
      {
        if (WIFEXITED (status) || WIFSIGNALED (status))
        {
          GList* link = (list);
          GList* next = (list = list->next);

          g_queue_unlink (&jc->waitq, link);
          g_list_free (link);
          waitpid (pid, &status, 0);

          if (WEXITSTATUS (status) != 0)
          {
            jc->condition |= 1;
          }
        }
        else
        {
          g_value_set_int (return_value, J_CLOSURE_STATUS_CONTINUE);
          return;
        }
      }
    }

  
  gint (*callback) (JClosure* self, GError** error, gpointer closure_data) = jc->entry;
  gint result = callback (jc, error, gc->data);

  jc->condition = 0;
  g_value_set_int (return_value, result);
}

GClosure* j_codegen_emit (JCodegen* codegen, JAst* ast, GError** error)
{
  g_return_val_if_fail (J_IS_CODEGEN (codegen), NULL);
  g_return_val_if_fail (ast != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  JCodegen* self = (codegen);
  JContext context = {0};
  JTag tag = {0};
  GPtrArray* children = NULL;
  GClosure* gc = NULL;
  JClosure* jc = NULL;
  GError* tmperr = NULL;
  guint i;

  int result = 0;
  size_t sz = 0;

  j_context_init (&context);
  j_tag_init (&context, &tag);
  j_context_generate (&context, ast, &tag);
  j_context_finish (&context);

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

  if (context.expansions->len > 0)
  g_closure_add_finalize_notifier (gc, children, (GClosureNotify) g_ptr_array_unref);
  g_closure_add_finalize_notifier (gc, codegen, (GClosureNotify) g_object_unref);
  g_closure_add_finalize_notifier (gc, NULL, (GClosureNotify) closure_nofity);
  g_closure_set_marshal (gc, (GClosureMarshal) closure_marshal);

  if (G_LIKELY (gc->floating))
    {
      g_closure_ref (gc);
      g_closure_sink (gc);
    }

  g_queue_init (&jc->waitq);
  j_block_init (&jc->block, sz);

  if ((result = dasm_encode (&context, j_block_ptr (&jc->block))), G_UNLIKELY (result != 0))
    {
      g_set_error_literal (error, J_CODEGEN_ERROR, J_CODEGEN_ERROR_PROGRAM_ENCODE, "dasm_encode()!: failed");
      g_closure_unref (gc);
      j_context_clear (&context);
    }
#if DEVELOPER == 1
  g_file_set_contents ("closure", j_block_ptr (&jc->block), j_block_sz (&jc->block), &tmperr);
  g_assert_no_error (tmperr);
  j_context_emit_debuginfo (&context);
  j_gdb_register (jc->debug_object = j_gdb_builder_end (&context.debug_builder));
#endif // DEVELOPER
  jc->expansions = (children == NULL) ? NULL : (gpointer) children->pdata;
  jc->entry = j_tag_as_offset (&context, &tag) + j_block_ptr (&jc->block);
return (j_block_protect (&jc->block), j_context_clear (&context), gc);
}
