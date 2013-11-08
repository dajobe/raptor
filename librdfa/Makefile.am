# -*- Mode: Makefile -*-
#
# Makefile for librdfa convienience library
#

noinst_LTLIBRARIES = librdfa.la

librdfa_la_SOURCES = \
context.c \
curie.c \
iri.c \
language.c \
lists.c \
namespace.c \
rdfa.c \
rdfa_utils.c \
strtok_r.c \
subject.c \
triple.c \
rdfa.h \
rdfa_utils.h \
strtok_r.h \
config.h

if NEED_STRTOK_R
librdfa_la_SOURCES += \
strtok_r.c \
strtok_r.h
endif


# Do not need:
#   mingw32_utils.c
#   mingw32_utils.h
