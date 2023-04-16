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
noinst_LTLIBRARIES=libcodegen.la

noinst_HEADERS=\
	ast.h \
	codegen.h \
	codegen_common.h \
	datachannel.h \
	fdchannel.h \
	histcontrol.h \
	lexer.h \
	parser.h \
	readline.h \
	token.h \
	walker.h \
	$(VOID)

noinst_DATA=\
	codegen_x86_64.c \
	$(VOID)

#
# Binaries and libraries
# - sources
#

jash_SOURCES=\
	datachannel.c \
	fdchannel.c \
	histcontrol.c \
	jash.c \
	lexer.c \
	parser.c \
	readline.c \
	token.c \
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
	libcodegen.la \
	$(VOID)

libcodegen_la_SOURCES=\
	codegen_closure.c \
	codegen_common.c \
	codegen_externs.c \
	codegen_${host_cpu}.c \
	$(VOID)
libcodegen_la_CFLAGS=\
	-DG_LOG_DOMAIN=\"Jash.Codegen\" \
	-DG_LOG_USE_STRUCTURED=1 \
	-D__CODEGEN__ \
	-flto \
	$(GLIB_CFLAGS) \
	$(VOID)
libcodegen_la_LDFLAGS=\
	-flto \
	$(VOID)
libcodegen_la_LIBADD=\
	$(GLIB_LIBS) \
	$(VOID)

#
# Extensions
# - .stringlist -> .c
#

SUFFIXES=.stringlist .dasc.c .c
DYNASM_CFLAGS=-D DEBUG=\"${DEBUG}\" -D DEVELOPER=\"${DEVELOPER}\"

.dasc.c.c:
	$(DYNASM) $(DYNASM_CFLAGS) -o $@ $<
.stringlist.c:
	$(GPERF) --output-file $@ -PCI $<
