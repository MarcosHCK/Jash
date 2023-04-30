/* Copyright 2023 MarcosHCK
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
#ifndef __JASH_CODEGEN_CLOSURE__
#define __JASH_CODEGEN_CLOSURE__ 1
#include <codegen/block.h>
#if DEVELOPER == 1
# include <codegen/debug/gdb.h>
#endif // DEVELOPER
#include <runtime/runner.h>

typedef struct _JClosure JClosure;
typedef gint JPipeEnd;

typedef enum
{
  J_CLOSURE_ERROR_FAILED,
  J_CLOSURE_ERROR_CHDIR,
  J_CLOSURE_ERROR_DUP2,
  J_CLOSURE_ERROR_EXECVP,
  J_CLOSURE_ERROR_FORK,
  J_CLOSURE_ERROR_IRQ,
  J_CLOSURE_ERROR_OPEN,
  J_CLOSURE_ERROR_PIPE,
  J_CLOSURE_ERROR_WAITPID,
} JClosureError;

typedef enum
{
  J_CLOSURE_STATUS_REMOVE = 0,
  J_CLOSURE_STATUS_CONTINUE = (1 << 1),
  J_CLOSURE_STATUS_WAITING = (1 << 2),
} JClosureStatus;

typedef JClosureStatus (*JClosureCallback) (JClosure* closure, JRunner* runner, GError** error);

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JClosure
  {
    GClosure closure;
    JBlock block;
    gboolean condition;
    gpointer* detachables;
    guint detachables_count;
    JClosureCallback entry;
    JPipeEnd* expansion_pipes;
    gchar** expansion_values;
    guint expansions_count;
    GQueue waitq;
#if DEVELOPER == 1
    JGdb* debug_object;
#endif // DEVELOPER
  };

#if __cplusplus
}
#endif // __cplusplus
 
#endif // __JASH_CODEGEN_CLOSURE__
   