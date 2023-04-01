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
return (g_atomic_int_inc (&self->ref_count), queue);
}

static void job_free (gpointer mem)
{
  j_machine_unref (((Job*) mem)->machine);
}

void j_job_queue_unref (JJobQueue* queue)
{
  g_return_if_fail (queue != NULL);
  JJobQueue* self = (queue);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_queue_clear (&self->jobs);
      g_slice_free (JJobQueue, self);
    }
}

void j_job_queue_broadcast (JJobQueue* queue, JCode* code)
{
  g_return_if_fail (queue != NULL);
  g_return_if_fail (code != NULL);
  JJobQueue* self = (queue);
  GList* list = NULL;

  for (list = self->jobs.head; list; list->next)
    {
      JMachine* machine = list->data;
      //j_machine_push_instruction (machine, code);
    }
}

void j_job_queue_broadcast_many (JJobQueue* queue, JCode* codes, guint n_codes)
{
  g_return_if_fail (queue != NULL);
  g_return_if_fail (codes != NULL);
  JJobQueue* self = (queue);
  GList* list = NULL;

  for (list = self->jobs.head; list; list->next)
    {
      JMachine* machine = list->data;
      //j_machine_push_instructions (machine, codes, n_codes);
    }
}

gboolean j_job_queue_execute (JJobQueue* queue)
{
  g_return_val_if_fail (queue != NULL, FALSE);
  JJobQueue* self = (queue);
  gboolean keep = FALSE;
  GList* list = NULL;

  for (list = self->jobs.head; list; list = list->next)
    {
      JMachine* machine = list->data;
/*
      if (j_machine_execute (machine))
        keep = TRUE;
*/
    }
return (keep);
}
