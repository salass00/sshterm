/*
 * SSHTerm - SSH2 shell client
 *
 * Copyright (C) 2019-2020 Fredrik Wikstrom <fredrik@a500.org>
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the SSHTerm
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <proto/exec.h>

#include <stdlib.h>
#include <string.h>

static APTR mempool;

static void __attribute__((constructor)) malloc_init(void)
{
	mempool = IExec->AllocSysObjectTags(ASOT_MEMPOOL,
		ASOPOOL_Name,      "SSHTerm memory pool",
		ASO_MemoryOvr,     MEMF_SHARED,
		ASOPOOL_MFlags,    MEMF_SHARED,
		ASOPOOL_Puddle,    4096,
		ASOPOOL_Threshold, 2048,
		ASOPOOL_Protected, TRUE,
		TAG_END);
	if (mempool == NULL)
		exit(EXIT_FAILURE);
}

static void __attribute__((destructor)) malloc_cleanup(void)
{
	if (mempool != NULL)
		IExec->FreeSysObject(ASOT_MEMPOOL, mempool);
}

void *malloc(size_t size)
{
	void *ptr = IExec->AllocPooled(mempool, size + 8);
	if (ptr == NULL)
		return NULL;

	*(size_t *)ptr = size;

	return (char *)ptr + 8;
}

void *calloc(size_t num, size_t size)
{
	size *= num;

	void *ptr = malloc(size);
	if (ptr != NULL)
		memset(ptr, 0, size);

	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	if (ptr == NULL)
		return malloc(size);

	size_t osize = *(size_t *)((char *)ptr - 8);
	if (osize >= size)
		return ptr;

	void *new = malloc(size);
	if (new != NULL)
	{
		memcpy(new, ptr, osize);
		free(ptr);
	}

	return new;
}

void free(void *ptr)
{
	if (ptr != NULL)
	{
		ptr = (char *)ptr - 8;

		IExec->FreePooled(mempool, ptr, *(size_t *)ptr);
	}
}

char *strdup(const char *src)
{
	char *result;

	result = malloc(strlen(src) + 1);
	if (result != NULL)
		strcpy(result, src);

	return result;
}

