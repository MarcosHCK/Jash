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
#include <code.h>
#include <profile.h>

typedef struct _JCodeArrayIter JCodeArrayIter;
#define J_CODE_ARRAY_ITER_INIT(ptr_array) { ptr_array, 0, 0, }
#define J_CODE_MARK(TYPE_NAME) (J_CODE_MARK_ ## TYPE_NAME + J_CODE_TYPE_MAX_TYPE)

struct _JCodeArrayIter
{
  GPtrArray* ptr_array;
  guint current;
  guint next;
};

static gboolean j_code_array_iter_next (JCodeArrayIter* iter, JCode** value)
{
  if (iter->next < iter->ptr_array->len)
    {
      iter->current = iter->next;
      return (((value == NULL) ? ++iter->next : (*value = g_ptr_array_index (iter->ptr_array, iter->current), ++iter->next)), TRUE);
    }
return FALSE;
}

static void j_code_array_iter_remove (JCodeArrayIter* iter, guint index)
{
  if (iter->current >= index)
    {
      --iter->next;
      --iter->current;
    }
  g_ptr_array_remove_index (iter->ptr_array, index);
}

#define j_code_array_iter_get_index(iter) ({ JCodeArrayIter* __iter = ((iter)); __iter->current; })
#define j_code_array_iter_remove_current(iter) ({ JCodeArrayIter* __iter = ((iter)); j_code_array_iter_remove (__iter, __iter->current); })

gint j_parser_profile_inplace_expansions (GPtrArray* codes, GError** error)
{
  JCodeArrayIter iter = J_CODE_ARRAY_ITER_INIT (codes);
  JCode* code = NULL;
  GError* tmperr = NULL;

  while (j_code_array_iter_next (&iter, &code))
  switch (code->type)
    {
      case J_CODE_MARK (PIPE):
      case J_CODE_MARK (CHAIN_BEGIN):
      case J_CODE_MARK (CHAIN_FINISH):
      case J_CODE_MARK (EXPANSION_BEGIN):
      case J_CODE_MARK (EXPANSION_FINISH):
        j_code_array_iter_remove_current (&iter);
        break;
    }
return J_PARSER_PROFILER_FINISH;
}

gint j_parser_profile_null_branches (GPtrArray* codes, GError** error)
{
  JCodeArrayIter iter = J_CODE_ARRAY_ITER_INIT (codes);
  JCode* code = NULL;
  GError* tmperr = NULL;

  while (j_code_array_iter_next (&iter, &code))
  switch (code->type)
    {
      case J_CODE_TYPE_IF:
      case J_CODE_TYPE_IFN:
        if (code->int_argument == 0)
          j_code_array_iter_remove_current (&iter);
          break;
    }
return J_PARSER_PROFILER_FINISH;
}

#if DEVELOPER == 1

G_GNUC_INTERNAL gint _j_parser_profile_catch_leaks (GPtrArray* codes, GError** error);

gint _j_parser_profile_catch_leaks (GPtrArray* codes, GError** error)
{
  JCodeArrayIter iter = J_CODE_ARRAY_ITER_INIT (codes);
  JCode* code = NULL;
  GError* tmperr = NULL;

  while (j_code_array_iter_next (&iter, &code))
  if (code->type > J_CODE_MARK (FIRST_MARK))
    g_error ("(" G_STRLOC "): Code mark leaked!");
return J_PARSER_PROFILER_FINISH;
}

#endif // DEVELOPER
