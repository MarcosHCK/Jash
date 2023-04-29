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
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
typedef struct _GValue JClosureErrorPrivate;
static void j_closure_error_private_init (JClosureErrorPrivate* priv);
#define j_closure_error_private_copy g_value_copy
#define j_closure_error_private_clear g_value_unset

struct _JCodegen
{
  GObject parent;
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

static void j_codegen_class_init (JCodegenClass* klass) { }
static void j_codegen_init (JCodegen* self) { }

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
  guint i;

  for (i = 0; i < jc->expansions_count; ++i)
    _g_free0 (jc->expansion_values [i]);
    _g_free0 (jc->expansion_values);
    _g_free0 (jc->expansion_pipes);

  g_queue_clear (&jc->waitq);
#if DEVELOPER == 1
  j_gdb_unregister (jc->debug_object);
  j_gdb_free (jc->debug_object);
#endif // DEVELOPER
  j_block_clear (&jc->block);
}

static void closure_marshal (JClosure* jc, GValue* return_value, guint n_param_values, const GValue* param_values)
{
  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 2);
  g_return_if_fail (param_values != NULL);
  GClosure* gc = (gpointer) jc;
  JRunner* runner = g_value_get_object (param_values + 0);
  GError** error = g_value_get_pointer (param_values + 1);
  GError* tmperr = NULL;
  GList* link;

  while ((link = g_queue_peek_head_link (&jc->waitq)) != NULL)
    {
      gint pid = GPOINTER_TO_INT (link->data);
      gint status = 0;

      if ((j_waitpid (pid, &status, WNOHANG, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          g_value_set_int (return_value, J_CLOSURE_STATUS_REMOVE);
          return;
        }
      else
        {
          if (!WIFEXITED (status) && !WIFSIGNALED (status))
            {
              g_value_set_int (return_value, J_CLOSURE_STATUS_CONTINUE);
              return;
            }
          else
            {
              g_queue_unlink (&jc->waitq, link);
              g_list_free (link);
              waitpid (pid, &status, 0);

              if (WEXITSTATUS (status) != 0 || WIFSIGNALED (status))
                {
                  jc->condition |= 1;
                }
            }
        }
    }

  g_value_set_int (return_value, jc->entry (jc, runner, error));
  jc->condition = 0;
}

GClosure* j_codegen_emit (JCodegen* codegen, JAst* ast, GError** error)
{
  g_return_val_if_fail (J_IS_CODEGEN (codegen), NULL);
  g_return_val_if_fail (ast != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  JCodegen* self = (codegen);
  JContext context = {0};
  JTag tag = {0};
  GClosure* gc = NULL;
  JClosure* jc = NULL;
  GError* tmperr = NULL;
  size_t sz = 0;
  gint result = 0;
  guint i;

  j_context_init (&context);
  j_tag_init (&context, &tag);

  j_context_generate (&context, ast, &tag);
  j_context_finish (&context);

  if ((result = dasm_link (&context, &sz)), G_UNLIKELY (result != 0))
    {
      g_set_error_literal (error, J_CODEGEN_ERROR, J_CODEGEN_ERROR_PROGRAM_LINK, "dasm_link()!: failed");
      j_context_clear (&context);
    }

  gc = g_closure_new_simple (sizeof (JClosure), g_object_ref (codegen));
  jc = (JClosure*) gc;

  jc->expansion_pipes = (context.max_expansions == 0) ? NULL : g_new (JPipeEnd, context.max_expansions);
  jc->expansion_values = (context.max_expansions == 0) ? NULL : g_new0 (gchar*, context.max_expansions);
  jc->expansions_count = context.max_expansions;

  if (jc->expansion_pipes != NULL)
    {
#if HAVE_MEMSET
      memset (jc->expansion_pipes, -1, sizeof (JPipeEnd) * context.max_expansions);
#else // HAVE_MEMSET
      JPipeEnd* ptr = jc->expansion_pipes;
      guint blocksz = 8;
      guint n_block = (context.max_expansions + (blocksz - 1)) / blocksz;
      guint i;

      switch (context.max_expansions % blocksz)
        {
          case 0: do
          {
                    *ptr++ = -1;
            case 7: *ptr++ = -1;
            case 6: *ptr++ = -1;
            case 5: *ptr++ = -1;
            case 4: *ptr++ = -1;
            case 3: *ptr++ = -1;
            case 2: *ptr++ = -1;
            case 1: *ptr++ = -1;
          } while (--n_block > 0);
        }
#endif // HAME_MEMSET
    }

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
  j_context_emit_debuginfo (&context);
  j_gdb_register (jc->debug_object = j_gdb_builder_end (&context.debug_builder));
#endif // DEVELOPER
  jc->entry = j_tag_as_offset (&context, &tag) + j_block_ptr (&jc->block);
return (j_block_protect (&jc->block), j_context_clear (&context), gc);
}
