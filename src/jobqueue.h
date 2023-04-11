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
#include <code.h>
#include <machine.h>

typedef struct _JJobQueue JJobQueue;
typedef enum _JCodeMetaType JCodeMetaType;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  /**
   * JCodeMetaType:
   * @J_CODE_META_CD: changes current working directory.
   * @J_CODE_META_EXIT: quits current shell session (if there are running jobs user will be warned instead).
   * @J_CODE_META_FG: brings a background job to foreground (if foreground job isn't running) and restarts it if it was paused.
   * @J_CODE_META_JOBS: prints currently executing jobs.
   * @J_CODE_META_BG: detach current foreground job and stacks it into background (also pauses it).
   *
   * Metacodes introduced to be understood by #JJobQueue instead of being transfered to foreground #JMachine.
   */
  enum _JCodeMetaType
  {
    J_CODE_META_FIRST_META,
    J_CODE_META_CD,
    J_CODE_META_EXIT,
    J_CODE_META_FG,
    J_CODE_META_JOBS,
    J_CODE_META_BG,
    J_CODE_META_MAX_META,
  };

  G_GNUC_INTERNAL JJobQueue* j_job_queue_new ();
  G_GNUC_INTERNAL JJobQueue* j_job_queue_ref (JJobQueue* queue);
  G_GNUC_INTERNAL void j_job_queue_unref (JJobQueue* queue);
  G_GNUC_INTERNAL void j_job_queue_execute (JJobQueue* queue, JCode** codes, guint n_codes, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_JOB_QUEUE__
