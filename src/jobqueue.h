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
#ifndef __JASH_JOB_QUEUE__
#define __JASH_JOB_QUEUE__ 1
#include <glib.h>
#include <machine.h>

typedef struct _JJobQueue JJobQueue;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL JJobQueue* j_job_queue_new ();
  G_GNUC_INTERNAL JJobQueue* j_job_queue_ref (JJobQueue* queue);
  G_GNUC_INTERNAL void j_job_queue_unref (JJobQueue* queue);
  G_GNUC_INTERNAL void j_job_queue_broadcast (JJobQueue* queue, JCode* code);
  G_GNUC_INTERNAL void j_job_queue_broadcast_many (JJobQueue* queue, JCode* codes, guint n_codes);
  G_GNUC_INTERNAL gboolean j_job_queue_execute (JJobQueue* queue);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_JOB_QUEUE__
