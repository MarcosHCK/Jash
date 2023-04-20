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
|.section main, code, data, expansions

|.type Jc, JClosure
|.define RetContinue, 1
|.define RetRemove, 0
#endif // __CODEGEN__

#endif // __JASH_CODEGEN_BACKEND_COMMON__
