PACKAGE = krb5-cracklib
VERSION = 0.1.4

abs_top_srcdir = $(dir $(abspath $(firstword $(MAKEFILE_LIST))))

include $(abs_top_srcdir)/ensc-lib/build-simple.mk

krb5plugindir = ${libdir}/krb5/plugins/pwqual

AM_CFLAGS = -std=gnu99 -Wall -W -Wno-unused-parameter
AM_CPPFLAGS = -D_GNU_SOURCE

CFLAGS = -O2 -flto
LDFLAGS = -flto -fuse-linker-plugin -fvisibility=hidden -Wl,-as-needed

VPATH += $(abs_top_srcdir)

KRB5_CONFIG = krb5-config
KRB5_CFLAGS = $(shell $(KRB5_CONFIG) --cflags krb5)
KRB5_LIBS = $(shell $(KRB5_CONFIG) --libs krb5)

CRACKLIB_CFLAGS =
CRACKLIB_LIBS = -lcrack

COM_ERR_MODULE = com_err
COM_ERR_CFLAGS = $(call _pkg_get_cflags,$(COM_ERR_MODULE))
COM_ERR_LIBS   = $(call _pkg_get_libs,$(COM_ERR_MODULE))

_pre_process = echo '$1' | $(CC) $(AM_CPPFLAGS) $(CPPFLAGS) $2 -E -
_find_symbol = $(shell $(call _pre_process,'$2',$3) | grep -q '\<$1\>' && echo '-DHAVE_$1=1')

libexec_PROGRAMS = krb5-checkpass
krb5plugin_MODULES = cracklib.so

LDFLAGS_SHARED = -fPIC -shared

CPPFLAGS_FascistCheckUser = \
  $(call _find_symbol,FascistCheckUser,'\#include <crack.h>',$(CRACKLIB_CFLAGS)) \

CFLAGS_krb5-checkpass = \
  $(CPPFLAGS_FascistCheckUser) \
  $(COM_ERR_CFLAGS)

LIBS_krb5-checkpass = \
  -lcrack \
  $(COM_ERR_LIBS)

SOURCES_krb5-checkpass = \
  checkpass.c \
  ensc-lib/io.c \
  ensc-lib/io.h \

SOURCES_cracklib.so = \
  pwqual-cracklib.c \
  ensc-lib/io.c \
  ensc-lib/io.h \

CPPFLAGS_cracklib.so = \
  -DCHECKPASS_PROG=\"${libexecdir}/krb5-checkpass\"

CFLAGS_cracklib.so = \
  $(KRB5_CFLAGS)

LDFLAGS_cracklib.so = \
  $(LDFLAGS_SHARED) \
  -Wl,-soname=pwqual-cracklib.so

LIBS_cracklib.so = \
  $(KRB5_LIBS) \
  $(COM_ERR_LIBS) \

ALL_SOURCES = \
  $(SOURCES_krb5-checkpass) \
  $(SOURCES_cracklib.so) \
  COPYING \
  README \
  Makefile \
  ensc-lib/build-simple.mk

all:	$(libexec_PROGRAMS) $(krb5plugin_MODULES)

dist:	$(PACKAGE)-$(VERSION).tar.xz

clean:
	rm -f $(libexec_PROGRAMS) $(krb5plugin_MODULES)

krb5-checkpass:	$(SOURCES_krb5-checkpass)
	$(CC) $(call _buildflags,C) $(filter %.c,$^) -o $@ $(LIBS_$@)

cracklib.so:	$(SOURCES_cracklib.so)
	$(CC) $(call _buildflags,C) $(filter %.c,$^) -o $@ $(LIBS_$@)

$(PACKAGE)-$(VERSION).tar.xz:	$(PACKAGE)-$(VERSION).tar
	@rm -f $@
	$(XZ) -c $< -c > $@

$(PACKAGE)-$(VERSION).tar:	$(ALL_SOURCES)
	$(call _createtarball,$@,$(PACKAGE)-$(VERSION),$(sort $^))

$(eval $(call _installrule,libexec))
$(eval $(call _installrule,krb5plugin))
