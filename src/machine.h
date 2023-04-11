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
#ifndef __JASH_MACHINE__
#define __JASH_MACHINE__ 1
#include <glib.h>
#include <code.h>

typedef struct _JMachine JMachine;
typedef enum _JMachineError JMachineError;

#define J_MACHINE_ERROR (j_machine_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  enum _JMachineError
  {
    J_MACHINE_ERROR_FAILED,
    J_MACHINE_ERROR_BG,
    J_MACHINE_ERROR_EXIT,
  };

  G_GNUC_INTERNAL GQuark j_machine_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JMachine* j_machine_new ();
  G_GNUC_INTERNAL JMachine* j_machine_ref (JMachine* machine);
  G_GNUC_INTERNAL void j_machine_unref (JMachine* machine);
  G_GNUC_INTERNAL void j_machine_push_instruction (JMachine* machine, JCode* code);
  G_GNUC_INTERNAL void j_machine_push_instructions (JMachine* machine, JCode** codes, guint n_codes);
  G_GNUC_INTERNAL gboolean j_machine_has_instructions (JMachine* machine);
  G_GNUC_INTERNAL gboolean j_machine_execute (JMachine* machine, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_MACHINE__
