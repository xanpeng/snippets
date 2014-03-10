/*
 * Copyright (C) 2011 Colin Patrick McCabe <cmccabe@alumni.cmu.edu>
 *
 * This is licensed under the Apache License, Version 2.0.  See file COPYING.
 */

#include "util.h"

#include <stdlib.h>

void *xcalloc(size_t nmemb, size_t size)
{
  void *ret = calloc(nmemb, size);
  if (!ret)
    abort();
  return ret;
}
