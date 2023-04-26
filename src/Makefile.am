# Copyright 2023 MarcosHCK
# Copyright 2023 DavierSB
# This file is part of JASH.
#
# JASH is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# JASH is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with JASH. If not, see <http://www.gnu.org/licenses/>.
#

VOID=

#
# Binaries and libraries
# - declare
#

bin_PROGRAMS=jash

noinst_LTLIBRARIES=\
	codegen/liba.la \
	lexer/liba.la \
	parser/liba.la \
	term/liba.la \
	$(VOID)

noinst_HEADERS=\
	codegen/block.h \
	codegen/branch.h \
	codegen/codegen.h \
	codegen/context.h \
	codegen/debug/gdb.h \
	codegen/system.h \
	codegen/walker.h \
  lexer/datachannel.h \
  lexer/lexer.h \
  lexer/token.h \
  parser/ast.h \
  parser/parser.h \
  parser/walker.h \
  term/histcontrol.h \
  term/readline.h \
	$(VOID)

noinst_DATA=\
	codegen/backend/common.dasc.c \
	codegen/backend/x86_64.dasc.c \
	$(VOID)

#
# Binaries and libraries
# - sources
#

jash_SOURCES=\
  jash.c \
	$(VOID)
jash_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-flto \
	$(GLIB_CFLAGS) \
	$(GOBJECT_CFLAGS) \
	$(READLINE_CFLAGS) \
	$(VOID)
jash_LDFLAGS=\
	-flto \
	$(VOID)
jash_LDADD=\
	$(GLIB_LIBS) \
	$(GOBJECT_LIBS) \
	$(READLINE_LIBS) \
	codegen/liba.la \
	lexer/liba.la \
	parser/liba.la \
	term/liba.la \
	$(VOID)

codegen_liba_la_SOURCES=\
	codegen/backend/${target_cpu}.c \
	codegen/block.c \
	codegen/branch.c \
	codegen/builtin.c \
	codegen/codegen.c \
	codegen/extern.c \
	codegen/generate.c \
	$(VOID)
codegen_liba_la_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash.Codegen\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-flto \
	-D__CODEGEN__=1 \
	$(GLIB_CFLAGS) \
	$(VOID)
codegen_liba_la_LDFLAGS=\
	-flto \
	$(VOID)
codegen_liba_la_LIBADD=\
	$(GLIB_LIBS) \
	$(VOID)

if DEBUG
codegen_liba_la_SOURCES+= codegen/debug/builder.c
codegen_liba_la_SOURCES+= codegen/debug/gdb.c
codegen_liba_la_LIBADD+= -lbfd
else
noinst_DATA+= codegen/debug/builder.c
noinst_DATA+= codegen/debug/gdb.c
endif

lexer_liba_la_SOURCES=\
	lexer/datachannel.c \
  lexer/lexer.c \
  lexer/token.c \
	$(VOID)
lexer_liba_la_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash.Lexer\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-flto \
	$(GLIB_CFLAGS) \
	$(VOID)
lexer_liba_la_LDFLAGS=\
	-flto \
	$(VOID)
lexer_liba_la_LIBADD=\
	$(GLIB_LIBS) \
	$(VOID)

parser_liba_la_SOURCES=\
  parser/parser.c \
	$(VOID)
parser_liba_la_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash.Parser\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-flto \
	$(GLIB_CFLAGS) \
	$(VOID)
parser_liba_la_LDFLAGS=\
	-flto \
	$(VOID)
parser_liba_la_LIBADD=\
	$(GLIB_LIBS) \
	$(VOID)

term_liba_la_SOURCES=\
	term/histcontrol.c \
	term/readline.c \
	$(VOID)
term_liba_la_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash.Term\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-flto \
	$(GLIB_CFLAGS) \
	$(VOID)
term_liba_la_LDFLAGS=\
	-flto \
	$(VOID)
term_liba_la_LIBADD=\
	$(GLIB_LIBS) \
	$(VOID)

#
# Extensions
# - .stringlist -> .c
#

SUFFIXES=.stringlist .dasc.c .c
DYNASM_CFLAGS=-D DEBUG=\"${DEBUG}\" -D DEVELOPER=\"${DEVELOPER}\" -D PLATFORM=\"${DYNASM_PLATFORM}\"

.dasc.c.c:
	$(DYNASM) $(DYNASM_CFLAGS) -o $@ $<
.stringlist.c:
	$(GPERF) --output-file $@ -PCI $<
