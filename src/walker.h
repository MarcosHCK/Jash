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
#ifndef __JASH_WALKER__
#define __JASH_WALKER__ 1
#include <glib.h>
#include <token.h>

typedef struct _Walker Walker;

#define WALKER_INIT { NULL, G_QUEUE_INIT, }

#define walker_clear(walker) (({ Walker* __walker = ((walker)); g_queue_clear (&__walker->queue); }))
#define walker_length(walker) (({ Walker* __walker = ((walker)); __walker->queue.length; }))
#define walker_peek_back(walker) ({ Walker* __walker = ((walker)); (JToken*) g_queue_peek_tail (&__walker->queue); })
#define walker_peek_front(walker) ({ Walker* __walker = ((walker)); (JToken*) g_queue_peek_head (&__walker->queue); })
#define walker_peek_index(walker,index) ({ Walker* __walker = ((walker)); (JToken*) g_queue_peek_nth (&__walker->queue, ((index))); })
#define walker_emplace(walker,token) (({ Walker* __walker = ((walker)); JToken* __token = ((token)); g_queue_push_tail (&__walker->queue, __token); }))
#define walker_emplace_link(walker,link) (({ Walker* __walker = ((walker)); GList* __link = ((link)); g_queue_push_tail_link (&__walker->queue, __link); }))
#define walker_leave(walker,token) (({ Walker* __walker = ((walker)); JToken* __token = ((token)); g_queue_push_head (&__walker->queue, __token); }))
#define walker_leave_link(walker,link) (({ Walker* __walker = ((walker)); GList* __link = ((link)); g_queue_push_head_link (&__walker->queue, __link); }))
#define walker_take(walker) (({ Walker* __walker = ((walker)); (JToken*) g_queue_pop_head (&__walker->queue); }))
#define walker_take_link(walker) (({ Walker* __walker = ((walker)); (GList*) g_queue_pop_head_link (&__walker->queue); }))
#define walker_withdraw(walker) (({ Walker* __walker = ((walker)); (JToken*) g_queue_pop_tail (&__walker->queue); }))
#define walker_withdraw_link(walker) (({ Walker* __walker = ((walker)); (GList*) g_queue_pop_tail_link (&__walker->queue); }))
#define walker_last(walker) ({ Walker* __walker = ((walker)); __walker->last; })

#define walker_adjust(walker,last_) \
    G_STMT_START \
      { \
        Walker* __walker = ((walker)); \
        JToken* __peek = g_queue_peek_tail (&__walker->queue); \
        JToken* __last = (__peek) ? __peek : ((last_)); \
          __walker->last = __last; \
      } \
    G_STMT_END

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _Walker
  {
    JToken* last;
    GQueue queue;
  };

  #if !DEVELOPER
  # define dumpwalker(walker)
  #else // DEVELOPER

    static void dumpwalker (Walker* walker)
    {
      GList* list;
      gchar* escp;
      guint i;

      const gchar* types [] =
        {
          "J_TOKEN_TYPE_BUILTIN", "J_TOKEN_TYPE_COMMENT",
          "J_TOKEN_TYPE_KEYWORD", "J_TOKEN_TYPE_LITERAL",
          "J_TOKEN_TYPE_OPERATOR", "J_TOKEN_TYPE_SEPARATOR",
          "J_TOKEN_TYPE_QUOTED",
        };

      G_STATIC_ASSERT (J_TOKEN_TYPE_BUILTIN == 0);
      G_STATIC_ASSERT (J_TOKEN_TYPE_QUOTED == G_N_ELEMENTS (types) - 1);

      for (list = g_queue_peek_head_link (&walker->queue), i = 0;
          list != NULL;
          list = list->next, ++i)
      {
        const JToken* token = list->data;
        const gchar* value = token->value;
        const guint type = token->type;

        g_printerr ("  walker[%i] = '%s' (%s)\n", i, escp = g_strescape (value, NULL), types [type]);
        g_free (escp);
      }
    }

  # define dumpwalker(walker) ({ g_printerr ("(" G_STRLOC "): dumpwalker()!\n"); (dumpwalker) ((walker)); })
  #endif // !DEVELOPER

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_WALKER__
