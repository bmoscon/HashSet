/*
 * hash_set_test.c
 *
 * Hash Set Test
 * 
 *
 * Copyright (C) 2013  Bryant Moscon - bmoscon@gmail.com
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to 
 * deal in the Software without restriction, including without limitation the 
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions, and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution, and in the same 
 *    place and form as other copyright, license and disclaimer information.
 *
 * 3. The end-user documentation included with the redistribution, if any, must 
 *    include the following acknowledgment: "This product includes software 
 *    developed by Bryant Moscon (http://www.bryantmoscon.org/)", in the same 
 *    place and form as other third-party acknowledgments. Alternately, this 
 *    acknowledgment may appear in the software itself, in the same form and 
 *    location as other such third-party acknowledgments.
 *
 * 4. Except as contained in this notice, the name of the author, Bryant Moscon,
 *    shall not be used in advertising or otherwise to promote the sale, use or 
 *    other dealings in this Software without prior written authorization from 
 *    the author.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "../src/hash_set.h"


#define SET_LEN 503
#define SET_SIZE 50000

uint32_t chksum(const void *str) 
{
  char *s = (char *)str;
  int len = strlen(s);
  int i;
  uint32_t c = 0;

  for (i = 0; i < len; ++i) {
    c = (c >> 1) + ((c & 1) << (32-1));
    c += s[i];
  }
  return (c);
}

uint32_t hash_int(const void *number) {
  return (*(int *)number);
}

int main()
{
  hash_set_st *set = hash_set_init(SET_LEN, hash_int);
  int i;
  hash_set_it *it;
  void **set_dump;

  printf("verifying inserts with overflow...\n");
  for(i = 0; i < SET_SIZE; ++i) {
    assert(hash_set_insert(set, &i, sizeof(int)));
  }

  printf("verifying duplicate inserts fail...\n");
  for(i = 0; i < SET_SIZE; ++i) {
    assert(hash_set_insert(set, &i, sizeof(int)) == DUPLICATE);
  }

  printf("verifying data structure integrity...\n");
  assert(set->len == SET_LEN);
  if (SET_SIZE > SET_LEN) {
    assert(set->overflow >= SET_SIZE - SET_LEN);
  }
  assert(set->entries == SET_SIZE);
  
  printf("verifying values exist...\n");
  for(i = 0; i < SET_SIZE; ++i) {
    assert(hash_set_exists(set, &i, sizeof(int)));
  }

  printf("verifying non-present values fail existence test...\n");
  for(i = SET_SIZE; i < SET_SIZE*10; ++i) {
    assert(hash_set_exists(set, &i, sizeof(int)) == FALSE);
  }

  printf("iterator test...\n");
  it = it_init(set);
  assert(it);

  // start at 1 beause init'ing the iterator 
  // gives us the first value, so we only have 49,999 more to test
  for (i = 1; i < SET_SIZE; ++i) {
    assert(*(int *)it_value(it) < SET_SIZE);
    assert(it_next(it) == OK);
  }
  
  it_free(it);

  it = it_init(set);
  assert(it);
  set_dump = hash_set_dump(set);
  
  printf("verifying iterator values...\n");

  for (i = 0; i < SET_SIZE; ++i) {
    assert(*(int *)it_value(it) == *(int *)set_dump[i]);
    it_next(it);
  }


  hash_set_dump_free(set_dump);
  hash_set_free(set);

  printf("\n-------- PASS --------\n");
  
  return 0;
}
