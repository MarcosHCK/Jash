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
#include <jobqueue.h>

typedef struct _Job Job;

struct _JJobQueue
{
  guint ref_count;
  gint next_job;
  GQueue jobs;
};

struct _Job
{
  gint order;
  JMachine* machine;
};

#define J_CODE_META(TYPE_NAME) (J_CODE_META_ ## TYPE_NAME + J_CODE_TYPE_MAX_TYPE)

JJobQueue* j_job_queue_new ()
{
  JJobQueue* queue;
  queue = g_slice_new (JJobQueue);
  queue->ref_count = 1;
  queue->next_job = 1;
return (g_queue_init (&queue->jobs), queue);
}

JJobQueue* j_job_queue_ref (JJobQueue* queue)
{
  g_return_val_if_fail (queue != NULL, NULL);
  JJobQueue* self = (queue);
return (g_atomic_int_inc (& self->ref_count), self);
}

static void job_free (gpointer mem)
{
  j_machine_unref (((Job*) mem)->machine);
}

void j_job_queue_unref (JJobQueue* queue)
{
  g_return_if_fail (queue != NULL);
  JJobQueue* self = (queue);

  if (g_atomic_int_dec_and_test (& self->ref_count))
    {
      g_queue_clear_full (&self->jobs, job_free);
      g_slice_free (JJobQueue, self);
    }
}

static void iterate (JJobQueue* self, JMachine* machine, GError** error)
{
  GError* tmperr = NULL;
  gboolean continue_;

  do
  {
    if ((continue_ = j_machine_execute (machine, &tmperr)), G_UNLIKELY (tmperr != NULL))
      {
        g_propagate_error (error, tmperr);
        return;
      }
  } while ((g_thread_yield (), continue_));
}

void j_job_queue_execute (JJobQueue* queue, JCode** codes, guint n_codes, GError** error)
{
  JJobQueue* self = (queue);
  JMachine* last = NULL;

  g_return_if_fail (queue != NULL);
  g_return_if_fail (codes != NULL);
  guint i;

  last = j_machine_new ();

  for (i = 0; i < n_codes; ++i)
  if (codes [i]->type < J_CODE_META (FIRST_META))
    j_machine_push_instruction (last, codes [i]);
  else
    {
      const gint type = codes [i]->type;

      switch ((JCodeType) type)
        {
          case J_CODE_META (CD): g_assert_not_reached ();
          case J_CODE_META (EXIT): g_assert_not_reached ();
          case J_CODE_META (FG): g_assert_not_reached ();
          case J_CODE_META (JOBS): g_assert_not_reached ();
          case J_CODE_META (BG): g_assert_not_reached ();
          default: g_assert_not_reached ();
        }
    }
return iterate (self, last, error);
}
