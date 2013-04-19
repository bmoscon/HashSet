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


hash_set_st* hash_set_init(size_t size, uint32_t (*hash_fp)(void *)) 
{
  hash_set_st *ret = NULL;

  assert(size > 0);

  ret = (hash_set_st *)malloc(sizeof(hash_set_st));
  assert(ret);

  ret->entries = 0;
  ret->overflow = 0;
  ret->hash_fp = hash_fp;
  ret->len = size;
  ret->array = calloc(size, sizeof(bucket_st));
  assert(ret->array);

  return (ret);
}


void hash_set_free(hash_set_st *set)
{
  bucket_st *next = NULL;
  size_t i;
  
  if (!set) {
    return;
  }

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
  free(set);
}

void hash_set_insert(hash_set_st *set, void *val, size_t size)
{
  uint32_t hash;
  uint32_t index;
  
  hash = set->hash_fp(val);
  index = hash % set->len;

  if (!set->array[index].hash) {
    set->array[index].hash = hash;
    set->array[index].value = malloc(size);
    assert(set->array[index].value);
    memcpy(set->array[index].value, val, size);
    set->array[index].size = size;
  } else {
    if (set->array[index].hash == hash) {
      if (size == set->array[index].size) {
	if (memcmp(set->array[index].value, val, size) == 0) {
	  // mp duplicates allowed
	  return;
	}
      } 
    }
    
    if (set->array[index].next == NULL) {
      set->array[index].next = malloc(sizeof(bucket_st));
      assert(set->array[index].next);
      set->array[index].next->hash = hash;
      set->array[index].next->next = NULL;
      ++set->overflow;
    } else {
      bucket_st *b = set->array[index].next;
      
      while (b->next) {
	if (b->hash == hash) {
	  if (b->size == size) {
	    if (memcmp(b->value, val, size == 0)) {
	      // no duplicates allowed
	      return;
	    }
	  }
	}
	
	b = b->next;
      }

      if (b->hash == hash) {
	if (b->size == size) {
	  if (memcmp(b->value, val, size == 0)) {
	    // no duplicates allowed
	    return;
	  }
	}
      }
      
      b->next = malloc(sizeof(bucket_st));
      assert(b->next);
      b->next->hash = hash;
      b->next->next = NULL;
      ++set->overflow;
    }
  }
  ++set->entries;
}

int hash_set_exists(hash_set_st *set, void *val, size_t size)
{
  uint32_t hash;
  uint32_t index;
  bucket_st *b;
  hash = set->hash_fp(val);
  index = hash % set->len;
  
  b = &(set->array[index]);
  
  while (b) {
    if (size == b->size) {
      if (b->hash == hash) {
	if (memcmp(b->value, val, size) == 0) {
	  return (1);
	}
      }
    }
    b = b->next;
  }
  
  return (0);
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
    return (-1);
  }
  
  while (!b->hash) {
    ++i;
    if (i > it->set->len) {
      return (-1);
    }
    b = &(it->set->array[i]);
  }

  it->current = b;
  it->index = i;
  
  return (0);
}


hash_set_it* it_init(hash_set_st *set)
{
  hash_set_it* ret;
  ret = malloc(sizeof(hash_set_it));

  assert(ret);

  ret->index = 0;
  ret->current = NULL;
  ret->set = set;

  if (init_bucket(ret) != 0) {
    return (NULL);
  } 
  
  return (ret);
}


int it_next(hash_set_it *it)
{
  bucket_st *b;
  uint32_t index;

  // check if there are overflowed buckets in our current position in the array
  if (it->current->next) {
    it->current = it->current->next;
    return (0);
  }

  index = it->index + 1;
  if (index > it->set->len) {
    return (-1);
  }
  b = &(it->set->array[index]);
  
  while (!b->hash) {
    ++index;
    if (index > it->set->len) {
      return (-1);
    }
    b = &(it->set->array[index]);
  }
  
  it->current = b;
  it->index = index;
  
  return (0);
}

void it_prev(hash_set_it *it)
{

}

void* it_value(hash_set_it *it)
{
  return (it->current->value);
}

void it_free(hash_set_it *it)
{
  free(it);
}
