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
#include <lexer/token.h>

typedef struct _JWalker JWalker;

#define J_WALKER_INIT { NULL, G_QUEUE_INIT, }

#define j_walker_adjust(walker,last_) \
    G_STMT_START \
      { \
        JWalker* __walker = ((walker)); \
        JToken* __peek = g_queue_peek_tail (&__walker->queue); \
        JToken* __last = (__peek) ? __peek : ((last_)); \
          __walker->last = __last; \
      } \
    G_STMT_END

#define j_walker_clear(walker) (({ JWalker* __walker = ((walker)); g_queue_clear (&__walker->queue); }))
#define j_walker_emplace(walker,token) (({ JWalker* __walker = ((walker)); JToken* __token = ((token)); g_queue_push_tail (&__walker->queue, __token); }))
#define j_walker_emplace_link(walker,link) (({ JWalker* __walker = ((walker)); GList* __link = ((link)); g_queue_push_tail_link (&__walker->queue, __link); }))
#define j_walker_last(walker) ({ JWalker* __walker = ((walker)); __walker->last; })
#define j_walker_leave(walker,token) (({ JWalker* __walker = ((walker)); JToken* __token = ((token)); g_queue_push_head (&__walker->queue, __token); }))
#define j_walker_leave_link(walker,link) (({ JWalker* __walker = ((walker)); GList* __link = ((link)); g_queue_push_head_link (&__walker->queue, __link); }))
#define j_walker_length(walker) (({ JWalker* __walker = ((walker)); __walker->queue.length; }))
#define j_walker_peek_back(walker) ({ JWalker* __walker = ((walker)); (JToken*) g_queue_peek_tail (&__walker->queue); })
#define j_walker_peek_front(walker) ({ JWalker* __walker = ((walker)); (JToken*) g_queue_peek_head (&__walker->queue); })
#define j_walker_peek_index(walker,index) ({ JWalker* __walker = ((walker)); (JToken*) g_queue_peek_nth (&__walker->queue, ((index))); })
#define j_walker_take(walker) (({ JWalker* __walker = ((walker)); (JToken*) g_queue_pop_head (&__walker->queue); }))
#define j_walker_take_link(walker) (({ JWalker* __walker = ((walker)); (GList*) g_queue_pop_head_link (&__walker->queue); }))
#define j_walker_withdraw(walker) (({ JWalker* __walker = ((walker)); (JToken*) g_queue_pop_tail (&__walker->queue); }))
#define j_walker_withdraw_link(walker) (({ JWalker* __walker = ((walker)); (GList*) g_queue_pop_tail_link (&__walker->queue); }))

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JWalker
  {
    JToken* last;
    GQueue queue;
  };

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_WALKER__
