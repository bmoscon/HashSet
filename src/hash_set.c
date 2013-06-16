/*
 * hash_set.c
 *
 * Hash Set Implementation
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

#include <assert.h>
#include <string.h>
#include <assert.h>

#include "hash_set.h"

#define BASE_SIZE 1024


hash_set_st* hash_set_init(uint32_t (* const hash_fp)(const void *)) 
{
  hash_set_st *ret = NULL;

  ret = (hash_set_st *)malloc(sizeof(hash_set_st));
  assert(ret);

  ret->entries = 0;
  ret->overflow = 0;
  ret->hash_fp = hash_fp;
  ret->len = BASE_SIZE;
  ret->array = calloc(ret->len, sizeof(bucket_st));
  assert(ret->array);

  return (ret);
}

static void hash_set_free_array(hash_set_st *set)
{
  bucket_st *next = NULL;
  size_t i;
  
  for (i = 0; i < set->len; ++i) {
    next = set->array[i].next;
    free(set->array[i].value);
    while (next) {
      set->array[i].next = next->next;
      free(next->value);
      free(next);
      next = set->array[i].next;
    }
  }

  free(set->array);
}

void hash_set_free(hash_set_st *set)
{ 
  if (!set) {
    return;
  }

  hash_set_free_array(set);

  free(set);
}

static int hash_set_realloc(hash_set_st *set)
{
  // allocate new set
  hash_set_st new_set;

  new_set.entries = 0;
  new_set.overflow = 0;
  new_set.hash_fp = set->hash_fp;
  new_set.len = set->len * 2;
  new_set.array = calloc(new_set.len, sizeof(bucket_st));
  assert(new_set.array);
  
  // copy over old set
  int set_index;
  bucket_st *b;
  
  for (set_index = 0; set_index < set->len; ++set_index) {
    b = &(set->array[set_index]);
    
    while (b && b->value) {
      hash_set_insert(&new_set, b->value, b->size);
      b = b->next;
    } 
  }
  
  // delete old array
  hash_set_free_array(set);
  
  // set new fields
  set->entries = new_set.entries;
  set->overflow = new_set.overflow;
  set->hash_fp = new_set.hash_fp;
  set->len = new_set.len;
  set->array = new_set.array;
  return (OK);
}

int hash_set_insert(hash_set_st *set, const void *val, const size_t size)
{
  uint32_t hash;
  uint32_t index;

  if (hash_set_exists(set, val, size)) {
    return (DUPLICATE);
  }
  
  if ((set->entries - set->overflow) > (set->len * .75)) {
    assert(hash_set_realloc(set) == OK);
  }
  
  hash = set->hash_fp(val);
  index = hash &  (set->len - 1);
  
  if (!set->array[index].value) {
    set->array[index].hash = hash;
    set->array[index].value = malloc(size);
    assert(set->array[index].value);
    memcpy(set->array[index].value, val, size);
    set->array[index].size = size;
  } else {
    if (set->array[index].next == NULL) {
      set->array[index].next = malloc(sizeof(bucket_st));
      assert(set->array[index].next);
      set->array[index].next->hash = hash;
      set->array[index].next->next = NULL;
      set->array[index].next->value = malloc(size);
      assert(set->array[index].next->value);
      memcpy(set->array[index].next->value, val, size);
      set->array[index].next->size = size;
      ++set->overflow;
    } else {
      bucket_st *b = set->array[index].next;
      while (b->next) {
	b = b->next;
      }
      
      b->next = malloc(sizeof(bucket_st));
      assert(b->next);
      b->next->hash = hash;
      b->next->next = NULL;
      b->next->value = malloc(size);
      assert(b->next->value);
      memcpy(b->next->value, val, size);
      b->next->size = size;
      
      ++set->overflow;
    }
  }

  ++set->entries;
  return (OK);
}

int hash_set_exists(const hash_set_st *set, const void *val, const size_t size)
{
  uint32_t hash;
  uint32_t index;
  bucket_st *b;

  hash = set->hash_fp(val);
  index = hash & (set->len - 1);

  b = &(set->array[index]);

  while (b && b->value) {
    if (size == b->size) {
      if (b->hash == hash) {
	if (memcmp(b->value, val, size) == 0) {
	  return (TRUE);
	}
      }
    }
    
    b = b->next;
  }

  return (FALSE);
}

void hash_set_clear(hash_set_st *set)
{
  bucket_st *next = NULL;
  size_t i;
  
  if (!set) {
    return;
  }

  if (set->overflow) {
    // free all 'overflow' bucket entries
    for (i = 0; i < set->len; ++i) {
      next = set->array[i].next;
      free(set->array[i].value);
      while (next) {
	set->array[i].next = next->next;
	free(next->value);
	free(next);
	next = set->array[i].next;
      }
    } 
  }

  memset(set->array, 0, set->len * sizeof(bucket_st));
  
  set->overflow = 0;
  set->entries = 0;

}


void** hash_set_dump(const hash_set_st *set)
{
  int set_index;
  int array_index = 0;
  bucket_st *b;
  void **array = malloc(sizeof(void*) * set->entries);
  assert(array);

  for (set_index = 0; set_index < set->len; ++set_index) {
    b = &(set->array[set_index]);
    
    while (b && b->value) {
      array[array_index++] = b->value;
      b = b->next;
    } 
  }
  
  return (array);
}

void hash_set_dump_free(void **d)
{
  free(d);
}

/*
 * Helper function that finds first non-empty bucket and inits 
 * the iterator accordingly
 *
 */
static int init_bucket(hash_set_it *it)
{
  bucket_st *b = it->set->array;
  uint32_t i = 0;

  if (!b) {
    return (ERROR);
  }
  
  while (!b->value) {
    ++i;
    if (i > it->set->len) {
      return (ERROR);
    }
    b = &(it->set->array[i]);
  }

  it->current = b;
  it->index = i;
  
  return (OK);
}


hash_set_it* it_init(const hash_set_st *set)
{
  hash_set_it* ret;

  if (!set) {
    return (NULL);
  }

  ret = malloc(sizeof(hash_set_it));

  assert(ret);

  ret->index = 0;
  ret->current = NULL;
  ret->set = set;

  if (init_bucket(ret) != OK) {
    free(ret);
    return (NULL);
  } 
  
  return (ret);
}


int it_next(hash_set_it *it)
{
  bucket_st *b;
  uint32_t index;

  if (!it || !it->current) {
    return (ERROR);
  }

  // check if there are overflowed buckets in our current position in the array
  if (it->current->next) {
    it->current = it->current->next;
    return (OK);
  }

  
  // no more buckets in our current index, so increment index 
  // and seach for non-empty bucket
  index = it->index + 1;
  if (index > it->set->len) {
    return (END);
  }
  b = &(it->set->array[index]);
  
  while (!b->value) {
    ++index;
    if (index > it->set->len) {
      return (END);
    }
    b = &(it->set->array[index]);
  }
  
  it->current = b;
  it->index = index;
  
  return (OK);
}

const void* it_value(const hash_set_it *it)
{
  if (it && it->current) {
    return (it->current->value);
  }
  
  return (NULL);
}

void it_free(hash_set_it *it)
{
  free(it);
}
