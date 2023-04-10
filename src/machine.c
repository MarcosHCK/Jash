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
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>

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

  //M gustaria poder hacer q esta HashTable fuera privada
  GHashTable* dictionary;
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
  self->dictionary = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
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

void j_machine_push_instruction (JMachine* machine, JCode* code)
{
  g_queue_push_tail(machine, code);
}

void j_machine_push_instructions (JMachine* machine, JCode* codes, guint n_codes)
{
  for (unsigned int i = 0; i < n_codes; i++)
    g_queue_push_tail(&(machine->instructions), &codes[i]);
}

gboolean j_machine_execute (JMachine* machine)
{
  if(machine->instructions.length == 0)return TRUE;
  JCode* Actual_Inst = g_queue_pop_head(&(machine->instructions));
  switch(Actual_Inst->type)
  {
    case(J_CODE_TYPE_PSI):
      machine->proc_in = machine->pipe_r;
      break;
    case(J_CODE_TYPE_PSO):
      machine->proc_out = machine->pipe_w;
      break;
    case(J_CODE_TYPE_FSI):
      machine->pipe_r = open(Actual_Inst->string_argument);
      break;
    case(J_CODE_TYPE_FSO):
      machine->pipe_w = open(Actual_Inst->string_argument);
      break;
    case(J_CODE_TYPE_PIPE):
      int xtr[2];
      pipe(xtr);
      machine->pipe_r = xtr[0];
      machine->pipe_w = xtr[1];
      break;
    case(J_CODE_TYPE_PAS):
      g_queue_push_head(&(machine->arguments), Actual_Inst->string_argument);
      break;
    case(J_CODE_TYPE_PAP):
      //M falta implementarlo
      break;
    case(J_CODE_TYPE_SET):
      gchar* key = g_strdup(Actual_Inst->string_argument);
      gchar* argument = g_strdup(g_queue_pop_head(&(machine->arguments)));
      g_hash_table_insert(machine->dictionary, key, argument);
      break;
    case(J_CODE_TYPE_GET):
      char* value = g_hash_table_lookup(machine->dictionary, Actual_Inst->string_argument);
      g_queue_push_head(&(machine->arguments), value);
      break;
    case(J_CODE_TYPE_USET):
      g_hash_table_insert(machine->dictionary, Actual_Inst->string_argument, NULL);
      break;
    case(J_CODE_TYPE_DUMP):
      value = g_queue_pop_head(&(machine->arguments));
      FILE* file = fdopen(machine->proc_out);
      fprintf(file, value);
      fclose(file);
      break;
    case(J_CODE_TYPE_EXEC):
      pid_t pid_hijo = fork();
      if (pid_hijo == 0)
      {
        char** args;
        Get_Arguments(machine, args);
        execvp(Actual_Inst->string_argument, args);
      }else
      {
        g_list_append(machine->children, pid_hijo);
      }
      g_queue_free(&(machine->arguments));
      break;
    case(J_CODE_TYPE_SYNC):
    int status = 0;
      if(Processes_Running(machine, &status))
        g_queue_push_head(&(machine->instructions), Actual_Inst);
        //si se borra d la cola d instrucciones con el pop a lo mejor esto se destruye
        break;
  }
  return (machine->instructions.length == 0);
}
void Get_Arguments(JMachine* machine, char* args[])
{
  GQueue* pila = &(machine->arguments);
  int n = pila->length;
  args = (char**)malloc(n*sizeof(char*));
  for(int i = 0; i < n; i++)
    //1- El metodo g_queue_peek_nth es 0indexed o 1indexed?
    //2- El metodo g_queue_peek_nth es O(1) o O(n)?
    args[i] = g_queue_peek_nth(pila, i + 1);
}
gboolean Processes_Running(JMachine* machine, int status)
{
  pid_t pid;
  for(GList* elem = machine->children; elem; elem = elem->next)
  {
    pid = elem->data;
    if(!waitpid(pid, &status, WNOHANG))return TRUE;
  }
  return FALSE;
}
