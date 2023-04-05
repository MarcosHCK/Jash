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
#include <machine.h>

struct _JMachine
{
  guint ref_count;

  gint pipe_r;
  gint pipe_w;
  gint proc_in;
  gint proc_out;

  GQueue instructions;
  GQueue arguments;
  GQueue flags;
  GList* children;
};

JMachine* j_machine_new ()
{
  JMachine* self;

  self = g_slice_new (JMachine);
  self->ref_count = 1;
  self->pipe_r = -1;
  self->pipe_w = -1;
  self->proc_in = -1;
  self->proc_out = -1;
  self->children = NULL;

  g_queue_init (&self->instructions);
  g_queue_init (&self->arguments);
  g_queue_init (&self->flags);
return self;
}

JMachine* j_machine_ref (JMachine* machine)
{
  g_return_val_if_fail (machine != NULL, NULL);
  JMachine* self = (machine);

  g_atomic_int_inc (&self->ref_count);
    return machine;
}

void j_machine_unref (JMachine* machine)
{
  g_return_if_fail (machine != NULL);
  JMachine* self = (machine);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_list_free (self->children);
      g_queue_clear (&self->flags);
      g_queue_clear_full (&self->arguments, (GDestroyNotify) g_free);
      g_queue_clear_full (&self->instructions, (GDestroyNotify) j_code_unref);
      g_slice_free (JMachine, self);
    }
}
