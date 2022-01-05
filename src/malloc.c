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

struct memchunk {
	ULONG size;
	APTR ptr;
};

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
	struct memchunk *mc;
	void *ptr = NULL;

	size = (size + 7) & ~7;

	mc = IExec->AllocPooled(mempool, sizeof(*mc) + size);
	if (mc != NULL)
	{
		ptr = mc + 1;

		mc->size = size;
		mc->ptr = ptr;
	}

	return ptr;
}

void *calloc(size_t num, size_t size)
{
	void *ptr;

	size *= num;
	size = (size + 7) & ~7;

	ptr = malloc(size);
	if (ptr != NULL)
	{
		memset(ptr, 0, size);
	}

	return ptr;
}

void *realloc(void *ptr, size_t size)
{
	void *new;

	if (ptr != NULL)
	{
		struct memchunk *mc;

		mc = (struct memchunk *)ptr - 1;

		if (mc->ptr != ptr)
			IExec->Alert(AN_MemCorrupt);

		if (mc->size >= size)
			return ptr;

		new = malloc(size);
		if (new != NULL)
		{
			memcpy(new, ptr, mc->size);
			free(ptr);
		}
	}
	else
	{
		new = malloc(size);
	}

	return new;
}

void free(void *ptr)
{
	if (ptr != NULL)
	{
		struct memchunk *mc;

		mc = (struct memchunk *)ptr - 1;

		if (mc->ptr != ptr)
			IExec->Alert(AN_MemCorrupt);

		IExec->FreePooled(mempool, mc, sizeof(*mc) + mc->size);
	}
}

char *strdup(const char *src)
{
	size_t len;
	char *dst;

	len = strlen(src);
	dst = malloc(len + 1);
	if (dst != NULL)
	{
		memcpy(dst, src, len);
		dst[len] = '\0';
	}

	return dst;
}

char *strndup(const char *src, size_t maxlen)
{
	size_t len;
	char *dst;

	len = strnlen(src, maxlen);
	dst = malloc(len + 1);
	if (dst != NULL)
	{
		memcpy(dst, src, len);
		dst[len] = '\0';
	}

	return dst;
}

