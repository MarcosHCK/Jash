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
#include <fdchannel.h>
#include <machine.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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
  GHashTable* variables;
  GBytes* empty_var;
  GList* children;
};

G_DEFINE_QUARK (j-machine-error-quark, j_machine_error);

JMachine* j_machine_new ()
{
  JMachine* self;
  self = g_slice_new (JMachine);
  self->ref_count = 1;
  self->pipe_r = -1;
  self->pipe_w = -1;
  self->proc_in = -1;
  self->proc_out = -1;
  self->variables = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
  self->empty_var = g_bytes_new_static ("", 1);
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
return (g_atomic_int_inc (&self->ref_count), machine);
}

void j_machine_unref (JMachine* machine)
{
  g_return_if_fail (machine != NULL);
  JMachine* self = (machine);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_list_free (self->children);
      g_bytes_unref (self->empty_var);
      g_hash_table_remove_all (self->variables);
      g_hash_table_unref (self->variables);
      g_queue_clear (&self->flags);
      g_queue_clear_full (&self->arguments, (GDestroyNotify) g_free);
      g_queue_clear_full (&self->instructions, (GDestroyNotify) j_code_unref);
      g_slice_free (JMachine, self);
    }
}

void j_machine_push_instruction (JMachine* machine, JCode* code)
{
  g_return_if_fail (machine != NULL);
  g_return_if_fail (code != NULL);
  JMachine* self = (machine);

  g_queue_push_tail (& self->instructions, j_code_ref (code));
}

void j_machine_push_instructions (JMachine* machine, JCode** codes, guint n_codes)
{
  g_return_if_fail (machine != NULL);
  g_return_if_fail (codes != NULL);
  JMachine* self = (machine);
  guint i;

  for (i = 0; i < n_codes; i++)
    {
      GQueue* queue = & self->instructions;
      JCode* code = codes [i];

      g_queue_push_tail (queue, j_code_ref (code));
    }
}

gboolean j_machine_has_instructions (JMachine* machine)
{
  g_return_val_if_fail (machine != NULL, FALSE);
  JMachine* self = (machine);
return (g_queue_get_length (& self->instructions) > 0);
}

gboolean j_machine_execute (JMachine* machine, GError** error)
{
  g_return_val_if_fail (machine != NULL, FALSE);
  JMachine* self = (machine);
  JCode* code = NULL;

  if ((code = g_queue_peek_head (& self->instructions)) != NULL)
    {
      if (code->type == J_CODE_TYPE_SYNC)
        {
          GList* list = NULL;
          gint return_code;
          pid_t pid_;

          for (list = self->children; list; list = list->next)
            {
              const gpointer ptr = list->data;
              const gint pid = GPOINTER_TO_INT (ptr);

              if ((pid_ = waitpid (pid, &return_code, WNOHANG)) < 0)
                {
                  int e = errno;

                  g_set_error_literal (error, j_machine_error_quark (), e, g_strerror (e));
                  return FALSE;
                }
              else
                {
                  if (WIFEXITED (return_code) || WIFSIGNALED (return_code))
                    {

                    }
                }
            }

          g_queue_pop_head (& self->instructions);
          return j_machine_execute (self, error);
        }
      else
        {
          while ((code = g_queue_peek_head (& self->instructions)) != NULL)
            {
              switch ((JCodeType) code->type)
                {
                  case J_CODE_TYPE_DUMP: g_printerr ("instruction 'J_CODE_TYPE_DUMP'\n"); break;
                  case J_CODE_TYPE_END: g_printerr ("instruction 'J_CODE_TYPE_END'\n"); break;
                  case J_CODE_TYPE_EXEC: g_printerr ("instruction 'J_CODE_TYPE_EXEC' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_FSI: g_printerr ("instruction 'J_CODE_TYPE_FSI' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_FSO: g_printerr ("instruction 'J_CODE_TYPE_FSO' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_FSOA: g_printerr ("instruction 'J_CODE_TYPE_FSOA' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_GET: g_printerr ("instruction 'J_CODE_TYPE_GET' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_IF: g_printerr ("instruction 'J_CODE_TYPE_IF' %i\n", code->int_argument); break;
                  case J_CODE_TYPE_IFN: g_printerr ("instruction 'J_CODE_TYPE_IFN' %i\n", code->int_argument); break;
                  case J_CODE_TYPE_LF: g_printerr ("instruction 'J_CODE_TYPE_LF'\n"); break;
                  case J_CODE_TYPE_LSET: g_printerr ("instruction 'J_CODE_TYPE_LSET' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_LT: g_printerr ("instruction 'J_CODE_TYPE_LT'\n"); break;
                  case J_CODE_TYPE_PAP: g_printerr ("instruction 'J_CODE_TYPE_PAP'\n"); break;
                  case J_CODE_TYPE_PAS: g_printerr ("instruction 'J_CODE_TYPE_PAS' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_PIPE: g_printerr ("instruction 'J_CODE_TYPE_PIPE'\n"); break;
                  case J_CODE_TYPE_PPRC: g_printerr ("instruction 'J_CODE_TYPE_PPRC'\n"); break;
                  case J_CODE_TYPE_PSI: g_printerr ("instruction 'J_CODE_TYPE_PSI'\n"); break;
                  case J_CODE_TYPE_PSO: g_printerr ("instruction 'J_CODE_TYPE_PSO'\n"); break;
                  case J_CODE_TYPE_SET: g_printerr ("instruction 'J_CODE_TYPE_SET' '%s'\n", code->string_argument); break;
                  case J_CODE_TYPE_SYNC: g_printerr ("instruction 'J_CODE_TYPE_SYNC'\n"); return TRUE;
                  case J_CODE_TYPE_USET: g_printerr ("instruction 'J_CODE_TYPE_USET'\n"); break;
                }

              g_queue_pop_head (& self->instructions);
            }
        }
    }
return (g_queue_get_length (&self->instructions) > 0);
}
