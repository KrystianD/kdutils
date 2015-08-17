/*
 * numconv.h
 * Copyright (C) 2015 Krystian Dużyński <krystian.duzynski@gmail.com>
 */

#ifndef __NUMCONV_H__
#define __NUMCONV_H__

void xtoa(unsigned long val, char *buf, unsigned radix, int is_neg);
char *ultoa(unsigned long val, char *buf, int radix);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);

#endif
