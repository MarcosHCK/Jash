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
#ifndef __JASH_CODEGEN_BACKEND_COMMON__
#define __JASH_CODEGEN_BACKEND_COMMON__ 1
#include <codegen/codegen.h>
#include <codegen/context.h>
#include <codegen/walker.h>

#if __CODEGEN__
|.actionlist actions
|.externnames extern_names
|.globals globl_
|.globalnames globl_names
|.section main, code, data

|.type Jc, JClosure
|.define Self, [rsp + (sizeof (gpointer) * 0)]
|.define Error, [rsp + (sizeof (gpointer) * 1)]

|.define RetContinue, 1
|.define RetRemove, 0

|.if PLATFORM == 'linux'
|.define c_arg1, rdi
|.define c_arg2, rsi
|.define c_arg3, rdx
|.define c_arg4, rcx
|.define c_arg5, r8
|.define c_arg6, r9
|.elif PLATFORM == 'win32'
|.define c_arg1, rcx
|.define c_arg2, rdx
|.define c_arg3, r8
|.define c_arg4, r9
|.endif
#endif // __CODEGEN__

#endif // __JASH_CODEGEN_BACKEND_COMMON__
