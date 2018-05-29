#include <stdlib.h>     /* realloc, free, exit, NULL */

typedef struct TCacheItem
{
	struct TCacheItem *next;
	uint16_t          *que;
	uint16_t          *ans;
	uint16_t           n_que;
	uint16_t           n_ans;
} TCacheItem;

struct TCacheItem *cache      = NULL;
struct TCacheItem *cache_last = NULL;

void* cache_question(void *buf, uint16_t n)
{
	struct TCacheItem *ptr = (struct TCacheItem *)malloc(sizeof(TCacheItem) + n);
	if (!ptr) return NULL;

	memset(ptr, 0, sizeof(TCacheItem));
	ptr->  que = (uint16_t*)(ptr + 1);
	ptr->n_que = n;
	memcpy(ptr->que, buf, n);

	if (!cache) cache = cache_last = ptr;
	else { cache_last->next = ptr; cache_last = ptr; }

	return NULL;
}

int cache_answer(void *_buf, uint16_t n)
{
	uint16_t *buf = (uint16_t*)_buf;
	struct TCacheItem *ptr = cache;

	while (ptr)
	{
		if (buf[0] == ptr->que[0])
		{
			ptr->  ans = (uint16_t*)malloc(n);
			ptr->n_ans = n;
			if (!ptr->ans) return 0;
			memcpy(ptr->ans, buf, n);
			return 1;
		}
		ptr = ptr->next;
	}
	return 0;
}

void* cache_search(void *_buf, uint16_t *n)
{
	uint16_t *buf = (uint16_t*)_buf;
	struct TCacheItem *ptr = cache;

	while (ptr)
	{
		if (memcmp(&buf[1], &ptr->que[1], *n) == 0)
		{
			*n = ptr->n_ans;
			return ptr->ans;
		}
		ptr = ptr->next;
	}
	return NULL;
}
