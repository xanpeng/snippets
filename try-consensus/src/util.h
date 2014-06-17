/*
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

#ifndef CONSENSUS_FUN_UTIL_DOT_H
#define CONSENSUS_FUN_UTIL_DOT_H

#include <unistd.h> /* for size_t */

/** Allocate a zero-initialized chunk of memory or die.
 *
 * @param nmemb		Number of members, as in calloc
 * @param size		Size of members, as in calloc
 *
 * @return		allocated area, as in calloc
 */
void *xcalloc(size_t nmemb, size_t size);

#endif
