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
#include <runtime/runner.h>

#define J_RUNNER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_RUNNER, JRunnerClass))
#define J_IS_RUNNER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_RUNNER))
#define J_RUNNER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_RUNNER, JRunnerClass))
typedef struct _JRunnerClass JRunnerClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
static gint next_order = 1;
typedef struct _Job Job;

struct _JRunner
{
  GObject parent;
  GQueue background;
  GTree* background_ref;
  GHashTable* variables;
  guint interactive : 1;
};

struct _JRunnerClass
{
  GObjectClass parent;
};

struct _Job
{
  guint order;
  GQueue waitq;
};

enum
{
  prop_0,
  prop_interactive,
  prop_number,
};

G_DEFINE_FINAL_TYPE (JRunner, j_runner, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};

static void job_free (Job* job)
{
  g_queue_clear (&job->waitq);
  g_slice_free (Job, job);
}

static void j_runner_class_dispose (GObject* pself)
{
  JRunner* self = (gpointer) pself;
  g_queue_clear_full (&self->background, (GDestroyNotify) job_free);
  g_tree_remove_all (self->background_ref);
  g_hash_table_remove_all (self->variables);
G_OBJECT_CLASS (j_runner_parent_class)->dispose (pself);
}

static void j_runner_class_finalize (GObject* pself)
{
  JRunner* self = (gpointer) pself;
  g_tree_unref (self->background_ref);
  g_hash_table_unref (self->variables);
G_OBJECT_CLASS (j_runner_parent_class)->finalize (pself);
}

static void j_runner_class_get_property (GObject* pself, guint property_id, GValue* value, GParamSpec* pspec)
{
  JRunner* self = (gpointer) pself;

  switch (property_id)
  {
    case prop_interactive:
      g_value_set_boolean (value, j_runner_get_interactive (self));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
      break;
  }
}

static void j_runner_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  JRunner* self = (gpointer) pself;

  switch (property_id)
  {
    case prop_interactive:
      self->interactive = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
      break;
  }
}

static void j_runner_class_init (JRunnerClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = j_runner_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = j_runner_class_finalize;
  G_OBJECT_CLASS (klass)->get_property = j_runner_class_get_property;
  G_OBJECT_CLASS (klass)->set_property = j_runner_class_set_property;

  const guint flags1 = G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE;
  properties [prop_interactive] = g_param_spec_boolean ("interactive", "interactive", "interactive", FALSE, flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
}

static gint uintcmp (guint a, guint b)
{
  return (a < b) ? -1 : ((a == b) ? 0 : 1);
}

static void j_runner_init (JRunner* self)
{
  GHashFunc func1 = (GHashFunc) g_bytes_hash;
  GEqualFunc func2 = (GEqualFunc) g_bytes_equal;
  GCompareDataFunc func3 = (GCompareDataFunc) uintcmp;
  GDestroyNotify notify1 = (GDestroyNotify) g_bytes_unref;

  self->variables = g_hash_table_new_full (func1, func2, notify1, notify1);
  self->background_ref = g_tree_new_full (func3, NULL, NULL, NULL);
}

JRunner* j_runner_new (gboolean interactive)
{
  return g_object_new (J_TYPE_RUNNER, "interactive", interactive, NULL);
}

gboolean j_runner_get_interactive (JRunner* runner)
{
  g_return_val_if_fail (J_IS_RUNNER (runner), FALSE);
return runner->interactive;
}

gboolean j_runner_job_pop (JRunner* runner, GQueue* waitq)
{
  g_return_val_if_fail (J_IS_RUNNER (runner), FALSE);
  g_return_val_if_fail (waitq != NULL, FALSE);
  JRunner* self = (runner);
  GList* list;
  Job* job;

  if ((list = g_queue_pop_head_link (&self->background)) != NULL)
    {
      g_list_free ((job = list->data, list));
      g_tree_remove (self->background_ref, GUINT_TO_POINTER (job->order));

      while ((list = g_queue_pop_head_link (&job->waitq)) != NULL)
        g_queue_push_tail_link (waitq, list);
      return (job_free (job), TRUE);
    }
return (FALSE);
}

gboolean j_runner_job_pop_nth (JRunner* runner, GQueue* waitq, gint order)
{
  g_return_val_if_fail (J_IS_RUNNER (runner), FALSE);
  g_return_val_if_fail (waitq != NULL, FALSE);
  JRunner* self = (runner);
  GList* list;
  Job* job;

  if ((list = g_tree_lookup (self->background_ref, GUINT_TO_POINTER (order))) != NULL)
    {
      g_list_free ((job = list->data, list));
      g_tree_remove (self->background_ref, GUINT_TO_POINTER (job->order));

      while ((list = g_queue_pop_head_link (&job->waitq)) != NULL)
        g_queue_push_tail_link (waitq, list);
      return (job_free (job), TRUE);
    }
return (FALSE);
}

void j_runner_job_push (JRunner* runner, GQueue* waitq)
{
  g_return_if_fail (J_IS_RUNNER (runner));
  g_return_if_fail (waitq != NULL);
  JRunner* self = (runner);
  Job* job = g_slice_new0 (Job);
  guint order = g_atomic_int_add (&next_order, 1);
  GList* list;

  while ((list = g_queue_pop_head_link (waitq)) != NULL)
    g_queue_push_tail_link (&job->waitq, list);

  list = g_list_alloc ();
  list->data = job;
  job->order = order;

  g_queue_push_head_link (&self->background, list);
  g_tree_insert (self->background_ref, GUINT_TO_POINTER (order), list);
}

gboolean j_runner_run (JRunner* runner, GClosure* closure, gint* exit_code, GError** error)
{
  g_return_val_if_fail (J_IS_RUNNER (runner), FALSE);
  g_return_val_if_fail (closure != NULL, FALSE);
  g_return_val_if_fail (exit_code != NULL, FALSE);
  GValue param_values [2] = {0};
  GValue return_value [1] = {0};
  GValue* exit_value = NULL;
  gboolean exit_thrown = FALSE;
  GError* tmperr = NULL;

  g_value_init (param_values + 0, J_TYPE_RUNNER);
  g_value_init (param_values + 1, G_TYPE_POINTER);
  g_value_init (return_value + 0, G_TYPE_INT);

  g_value_set_object (param_values + 0, runner);
  g_value_set_pointer (param_values + 1, &tmperr);

  do
  {
    if ((g_closure_invoke (closure, return_value, 2, param_values, NULL)), G_UNLIKELY (tmperr != NULL))
    {
      if (!g_error_matches (tmperr, J_CLOSURE_ERROR, J_CLOSURE_ERROR_EXIT))
        {
          g_propagate_error (error, tmperr);
          break;
        }
      else
        {
          exit_thrown = TRUE;
          exit_value = j_closure_error_value (tmperr);
          exit_code [0] = g_value_get_int (exit_value);
            _g_error_free0 (tmperr);
        }
    }
  } while (g_value_get_int (return_value) == J_CLOSURE_STATUS_CONTINUE);
return (g_value_unset (return_value), g_value_unset (param_values + 0), g_value_unset (param_values + 1), exit_thrown);
}

GBytes* j_runner_variable_get (JRunner* runner, GBytes* key)
{
  g_return_val_if_fail (J_IS_RUNNER (runner), NULL);
  g_return_val_if_fail (key != NULL, NULL);
return g_hash_table_lookup (runner->variables, key);
}

void j_runner_variable_set (JRunner* runner, GBytes* key, GBytes* value)
{
  g_return_if_fail (J_IS_RUNNER (runner));
  g_return_if_fail (key != NULL);
  g_return_if_fail (value != NULL);
  g_hash_table_insert (runner->variables, key, value);
}
