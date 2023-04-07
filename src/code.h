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
#ifndef __JASH_CODE__
#define __JASH_CODE__ 1
#include <glib.h>

typedef struct _JCode JCode;
typedef enum _JCodeType JCodeType;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  /**
   * JCode: (copy-func j_code_ref) (free-func j_code_unref)
   * @ref_count: object's reference count.
   * @type: (type Jash.CodeType): instruction type.
   * @size: instruction size (sizeof (JCode) + argument_size).
   * @argument: (skip): instruction argument as a pointer.
   * @string_argument: instruction argument as an string pointer.
   * @uintptr_argument: (skip): instruction argument a pointer-sized unsigned integer.
   * @int_argument: instruction argument as a literal signed integer.
   * @uint_argument: instruction argument as a literal unsigned integer.
   *
   * A single instruction as understood by #JMachine.
   */
  struct _JCode
  {
    guint ref_count;
    guint type : 5;
    guint size : sizeof (guint) * 8 - 5;

    union
    {
      gpointer argument;
      gchar* string_argument;
      guintptr uintptr_argument;
      gint int_argument;
      guint uint_argument;
    };
  };

  /**
   * JCodeType:
   * @J_CODE_TYPE_DUMP: writes the value which is on top of arguments stack directly into #JMachine:proc_out.
   * @J_CODE_TYPE_END: finishes a if-ifn sequence by poping flags' stack top value.
   * @J_CODE_TYPE_EXEC: executes a program (given by #JCode.string_argument) using all arguments in arguments' stack.
   *  Uses #JMachine:proc_out and #JMachine:proc_in as process standard output and input, respectively and
   *  cleans them up (by setting them to shell's I/O).
   * @J_CODE_TYPE_FSI: opens a file (whose name is given by #JCode.string_argument) and sets it's descriptor as next
   *  to be executed (by @J_CODE_TYPE_EXEC) program's standard input (done by setting #JMachine:proc_out).
   * @J_CODE_TYPE_FSO: similar to @J_CODE_TYPE_FSI but modifies program's standard output.
   * @J_CODE_TYPE_FSOA: similar to @J_CODE_TYPE_FSO, differing only in the way output file is opened (@J_CODE_TYPE_FSO opens
   *  it with overwrite mode, @J_CODE_TYPE_FSOA does it with append mode).
   * @J_CODE_TYPE_GET: reads a previously saved shell variable (whose name is given by #JCode.string_argument) and pushes it
   *  into arguments stack (if the variable wasn't set then an empty string is pushed).
   * @J_CODE_TYPE_IF: performs a conditional jump (currently only forwards) if the value on top of flags stack is considered
   *  false (equals to zero) by omitting n instructions given by #JCode.int_argument (@J_CODE_TYPE_IF instruction
   *  currently being executed doesn't counts).
   * @J_CODE_TYPE_IFN: same behavior as @J_CODE_TYPE_IF, but performs the jump if flags stack's value is considered false (
   *  different from zero).
   * @J_CODE_TYPE_LF: pushes a false value (a value different from zero, currently 1) on flags stack.
   * @J_CODE_TYPE_LSET: lists shell set variables (appends a variables as a 'NAME=VALUE' pairs, one line at the time) and pushes
   *  is into arguments stack (as a single, multi line string).
   * @J_CODE_TYPE_LT: pushes a true value (zero) on flags stack.
   * @J_CODE_TYPE_PAP: reads the contents (reads and appends it into a buffer until EOF is hit) of #JCode.pipe_r and pushes
   *  the result string into arguments stack.
   * @J_CODE_TYPE_PAS: pushes an string (given by #JCode.string_argument) on arguments stack.
   * @J_CODE_TYPE_PIPE: creates a pipe (creating two file descriptors pointing to both ends) and stores them in #JCode.pipe_r
   *  and #JCode.pipe_w.
   * @J_CODE_TYPE_PPRC: pushes lasted executed (by @J_CODE_TYPE_EXEC) program's return code into flags stack (of course any
   *  @J_CODE_TYPE_PPRC must be preceded by an @J_CODE_TYPE_SYNC instruction, but it doesn't means those two are always
   *  together, saying, one following the other).
   * @J_CODE_TYPE_PSI: similar to @J_CODE_TYPE_FSI but uses an already existing pipe read end's file descriptor instead
   *  of opening a file (just copies #JMachine:pipe_r to #JMachine:proc_in).
   * @J_CODE_TYPE_PSO: similar to @J_CODE_TYPE_FSO but uses an already existing pipe write end's file descriptor instead
   *  of opening a file (just copies #JMachine:pipe_w to #JMachine:proc_out).
   * @J_CODE_TYPE_SET: sets a shell variable (whose name is given by #JCode.string_argument) to the value on top of arguments
   *  stack.
   * @J_CODE_TYPE_SYNC: synchronizes all currently executing programs (by @J_CODE_TYPE_EXEC) by stopping all subsequent
   *  instructions execution until of all child program have finish.
   * @J_CODE_TYPE_USET: clears a shell variable's value (whose name is given by #JCode.string_argument).
   *
   * Instruction codes recognized by #JMachine.
   */
  enum _JCodeType
  {
    J_CODE_TYPE_DUMP,
    J_CODE_TYPE_END,
    J_CODE_TYPE_EXEC,
    J_CODE_TYPE_FSI,
    J_CODE_TYPE_FSO,
    J_CODE_TYPE_FSOA,
    J_CODE_TYPE_GET,
    J_CODE_TYPE_IF,
    J_CODE_TYPE_IFN,
    J_CODE_TYPE_LF,
    J_CODE_TYPE_LSET,
    J_CODE_TYPE_LT,
    J_CODE_TYPE_PAP,
    J_CODE_TYPE_PAS,
    J_CODE_TYPE_PIPE,
    J_CODE_TYPE_PPRC,
    J_CODE_TYPE_PSI,
    J_CODE_TYPE_PSO,
    J_CODE_TYPE_SET,
    J_CODE_TYPE_SYNC,
    J_CODE_TYPE_USET,
    J_CODE_TYPE_MAX_CODE,
  };

  G_GNUC_INTERNAL JCode* j_code_new (JCodeType type, gsize argument_size);
  G_GNUC_INTERNAL JCode* j_code_new_string (JCodeType type, const gchar* value);
  G_GNUC_INTERNAL JCode* j_code_ref (JCode* code);
  G_GNUC_INTERNAL void j_code_unref (JCode* code);

  #define j_code_new_simple(type) \
    (G_GNUC_EXTENSION ({ \
        JCodeType __type = ((type)); \
        JCode* __code = j_code_new (__type, 0); \
          __code; \
      }))

  #define j_code_new_int(type,value) \
    (G_GNUC_EXTENSION ({ \
        JCodeType __type = ((type)); \
        gint __value = ((value)); \
        JCode* __code = j_code_new (__type, 0); \
          __code->uintptr_argument = (guintptr) __value; \
          __code; \
      }))

  #define j_code_new_uint(type,value) \
    (G_GNUC_EXTENSION ({ \
        JCodeType __type = ((type)); \
        gint __value = ((value)); \
        JCode* __code = j_code_new (__type, 0); \
          __code->uintptr_argument = (guintptr) __value; \
          __code; \
      }))

  #define J_CODE_META_CD (1)
  #define J_CODE_META_EXIT (2)
  #define J_CODE_META_FG (3)
  #define J_CODE_META_BG (4)
  #define J_CODE_META_DETACH (5)

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODE__
