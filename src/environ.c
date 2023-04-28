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
#include <environ.h>

#define J_ENVIRON_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_ENVIRON, JEnvironClass))
#define J_IS_ENVIRON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_ENVIRON))
#define J_ENVIRON_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_ENVIRON, JEnvironClass))
typedef struct _JEnvironClass JEnvironClass;
typedef struct _JJob JJob;
static int j_job_id_cmp (guint id1, guint id2);
static void j_job_free (gpointer job);

struct _JEnviron
{
  GObject parent;

  /*<private>*/
  GHashTable* variables;
  GTree* jobs_tree;
  guint interactive : 1;
};

struct _JEnvironClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (JEnviron, j_environ, G_TYPE_OBJECT);

static void j_environ_class_dispose (GObject* pself)
{
  JEnviron* self = (gpointer) pself;
  g_hash_table_remove_all (self->variables);
  g_tree_remove_all (self->jobs_tree);
G_OBJECT_CLASS (j_environ_parent_class)->dispose (pself);
}

static void j_environ_class_finalize (GObject* pself)
{
  JEnviron* self = (gpointer) pself;
  g_hash_table_unref (self->variables);
  g_tree_unref (self->jobs_tree);
G_OBJECT_CLASS (j_environ_parent_class)->finalize (pself);
}

static void j_environ_class_init (JEnvironClass* klass)
{
  G_OBJECT_CLASS (klass)->dispose = j_environ_class_dispose;
  G_OBJECT_CLASS (klass)->finalize = j_environ_class_finalize;
}

static void j_environ_init (JEnviron* self)
{
  GHashFunc func1 = (GHashFunc) g_bytes_hash;
  GEqualFunc func2 = (GEqualFunc) g_bytes_equal;
  GCompareDataFunc func3 = (GCompareDataFunc) j_job_id_cmp;
  GDestroyNotify notify1 = (GDestroyNotify) g_bytes_unref;
  GDestroyNotify notify2 = (GDestroyNotify) j_job_free;

  self->variables = g_hash_table_new_full (func1, func2, notify1, notify1);
  self->jobs_tree = g_tree_new_full (func3, NULL, NULL, notify2);
}

static int j_job_id_cmp (guint id1, guint id2)
{ 
  return (id1 < id2) ? -1 : ((id1 == id2) ? 0 : 1); 
}

static void j_job_free (gpointer job)
{
  g_assert_not_reached ();
}

JEnviron* j_environ_new (void)
{
  return g_object_new (J_TYPE_ENVIRON, NULL);
}

GBytes* j_environ_get (JEnviron* environ, GBytes* key)
{
  g_return_val_if_fail (J_IS_ENVIRON (environ), NULL);
  g_return_val_if_fail (key != NULL, NULL);
return g_hash_table_lookup (environ->variables, key);
}

gboolean j_environ_get_interactive (JEnviron* environ)
{
  g_return_val_if_fail (J_IS_ENVIRON (environ), 0);
return environ->interactive;
}

void j_environ_set (JEnviron* environ, GBytes* key, GBytes* value)
{
  g_return_if_fail (J_IS_ENVIRON (environ));
  g_return_if_fail (key != NULL);
  g_return_if_fail (value != NULL);
  g_hash_table_insert (environ->variables, key, value);
}

void j_environ_set_interactive (JEnviron* environ, gboolean value)
{
  g_return_if_fail (J_IS_ENVIRON (environ));
  environ->interactive = value;
}
