/*
 * hash_set.h
 *
 * Hash Set Definitions
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

#ifndef __HASH_SET__
#define __HASH_SET__

#include <stdint.h>
#include <stdlib.h>


typedef struct bucket_st {
  uint32_t hash;
  size_t size;
  void *value;
  struct bucket_st *next;
} bucket_st;


typedef struct hash_set_st {
  uint32_t entries;
  uint32_t overflow;
  size_t len;
  uint32_t (*hash_fp)(const void *);
  bucket_st *array;
} hash_set_st;


// Iterator Definition 
typedef struct hash_set_it {
  uint32_t index;
  const bucket_st *current;
  const hash_set_st* set;
} hash_set_it;

enum bool {
  FALSE = 0,
  TRUE
};

enum hash_return {
  ERROR = 0,
  OK = 1,
  DUPLICATE,
  END
};


hash_set_st* hash_set_init(const size_t size, uint32_t (* const hash_fp)(const void *));
void hash_set_free(hash_set_st *set);
int hash_set_exists(const hash_set_st *set, const void *val, const size_t size);
int hash_set_insert(hash_set_st *set, const void *val, const size_t size);
void hash_set_clear(hash_set_st *set);
void** hash_set_dump(const hash_set_st *set);
void hash_set_dump_free(void **d);


// Hash Set Iterator Function Definitions
hash_set_it* it_init(const hash_set_st *set);
int it_next(hash_set_it *it);
void* it_value(hash_set_it *it);
void it_free(hash_set_it *it);





#endif
