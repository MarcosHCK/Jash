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
#include <codegen_common.h>

G_LOCK_DEFINE_STATIC (lowtable);

void j_codegen_breakpoint ()
{
  G_BREAKPOINT ();
}

void j_codegen_clear (Dst_DECL)
{
  dasm_free (Dst);
  g_free (Dst->labels);
}

static inline gboolean adjust (gint32* offset, gconstpointer address, gconstpointer callback, int type)
{
  gsize address_ = GPOINTER_TO_SIZE (address);
  gsize callback_ = GPOINTER_TO_SIZE (callback);
  gssize offset_ = 0;

  G_STATIC_ASSERT (sizeof (gsize) == GLIB_SIZEOF_VOID_P);

  if (!type)
    {
      if (callback_ > (gsize) G_MAXINT32)
        return FALSE;
      else
        *offset = (gint32) callback_;
    }
  else
    {
      offset_ = callback_ - address_ - 4;

      if (offset_ > (gssize) G_MAXINT32
        || offset_ < (gssize) G_MININT32)
        return FALSE;
      else
        *offset = (gint32) offset_;
    }
return TRUE;
}

static inline gboolean lowget (gint32* offset, gconstpointer address, gconstpointer callback, int type)
{
  static GHashTable* table = NULL;
  const GHashFunc func1 = (GHashFunc) g_direct_hash;
  const GEqualFunc func2 = (GEqualFunc) g_direct_equal;
  const GDestroyNotify notify = (GDestroyNotify) g_closure_unref;

  JCodegenClosure* jc;

  G_LOCK (lowtable);
  table = (table) ? table : g_hash_table_new_full (func1, func2, NULL, notify);

  if ((jc = g_hash_table_lookup (table, callback)) == NULL)
    {
      JCodegen codegen = {0};
      gpointer key = NULL;

      j_codegen_init (&codegen);
      j_codegen_absjump (&codegen, callback);

      key = (gpointer) callback;
      jc = (gpointer) j_codegen_emit (&codegen);

      g_hash_table_insert (table, key, jc);
      j_codegen_clear (&codegen);
    }

  G_UNLOCK (lowtable);
return adjust (offset, address, jc->entry, type);
}

gint32 j_codegen_extern_find (Dst_DECL, gconstpointer address, const gchar* name, int type)
{
  JCodegenExtern* extern_ = NULL;
  gboolean good = FALSE;
  gint32 offset = 0;

  if ((extern_ = (gpointer) j_codegen_extern_lookup (name, strlen (name))) == NULL)
    g_error ("(" G_STRLOC "): Unknown extern '%s'", name);
  if ((good = (gboolean) adjust (&offset, address, extern_->callback, type)) == FALSE)
  if ((good = (gboolean) lowget (&offset, address, extern_->callback, type)) == FALSE)
    g_error ("(" G_STRLOC "): Extern '%s' offset above 2 GB limit", name);
return offset;
}
