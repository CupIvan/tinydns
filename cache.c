#include <time.h>

#include "common.h"

uint32_t getTimestamp()
{
	return (uint32_t)time(0);
}

struct TCacheItem *cache      = NULL;
struct TCacheItem *cache_last = NULL;

void* cache_question(void *buf, uint16_t n)
{
	struct TCacheItem *ptr = (struct TCacheItem *)malloc(sizeof(TCacheItem) + n);
	if (!ptr) return NULL;

	memset(ptr, 0, sizeof(TCacheItem));
	ptr->  que = (uint16_t*)(ptr + 1);
	ptr->n_que = n;
	ptr->timestamp = getTimestamp();
	memcpy(ptr->que, buf, n);

	// search for star and calc part sizes
	char *star_ptr;
	star_ptr = (char*)ptr->que + sizeof(THeader);
	while (*star_ptr)
	{
		if (star_ptr[0] == 1 && star_ptr[1] == '*')
		{
			ptr->star_q_n1 = star_ptr - (char*)ptr->que;
			ptr->star_q_n2 = n - ptr->star_q_n1 - 2;
			break;
		}
		star_ptr++;
	}

	if (!cache) cache = cache_last = ptr;
	else { cache_last->next = ptr; cache_last = ptr; }

	return NULL;
}

int cache_answer(void *_buf, uint16_t n)
{
	uint16_t *buf = (uint16_t*)_buf;
	struct TCacheItem *ptr = cache;
	char     *star_ptr;

	while (ptr)
	{
		if (buf[0] == ptr->que[0])
		{
			ptr->  ans = (uint16_t*)malloc(n);
			ptr->n_ans = n;
			if (!ptr->ans) return 0;
			memcpy(ptr->ans, buf, n);
			// search for star and calc part sizes
			star_ptr = (char*)ptr->ans + sizeof(THeader);
			while (*star_ptr)
			{
				if (star_ptr[0] == 1 && star_ptr[1] == '*')
				{
					ptr->star_a_n1 = star_ptr - (char*)ptr->ans;
					ptr->star_a_n2 = n - ptr->star_a_n1 - 2;
					if (ptr->ans_right = (uint16_t*)malloc(ptr->star_a_n2))
						memcpy(ptr->ans_right, (char*)ptr->ans + ptr->star_a_n1 + 2, ptr->star_a_n2);
					break;
				}
				star_ptr++;
			}

			return 1;
		}
		ptr = ptr->next;
	}
	return 0;
}

void* cache_search(void *_buf, uint16_t *n)
{
	uint16_t *buf = (uint16_t*)_buf;
	struct TCacheItem *ptr      = cache;
	struct TCacheItem *ptr_prev = NULL;
	struct TCacheItem *ptr_tmp  = NULL;
	uint32_t time = getTimestamp();
	while (ptr)
	{
		if (config.cache_time)
		if (time - ptr->timestamp > config.cache_time)
		{
			ptr_tmp = ptr->next;
			if (ptr_prev == NULL)
			{
				cache = ptr->next;
				if (ptr->ans) free(ptr->ans);
				free(ptr);
			}
			else
			{
				ptr_prev->next = ptr->next;
				if (ptr->ans) free(ptr->ans);
				free(ptr);
			}
			ptr = ptr_tmp;
			continue;
		}

		if (ptr->ans)
		if (ptr->star_q_n1 > 0)
		{
			int16_t mid_sz = *n - ptr->star_q_n1 - ptr->star_q_n2; // size of middle
			if (mid_sz > 0)
			if (memcmp(&buf[1], &ptr->que[1], ptr->star_q_n1 - 2) == 0) // left part
			if (memcmp((char*)buf      + *n         - ptr->star_q_n2,
			           (char*)ptr->que + ptr->n_que - ptr->star_q_n2, ptr->star_q_n2) == 0) // right part
			{
				*n = ptr->star_a_n1 + mid_sz + ptr->star_a_n2;
				char *p = (char*)ptr->ans;
				p += ptr->star_a_n1; // skip left part
				// fill subdomain
				memcpy(p, (char*)buf + ptr->star_q_n1, mid_sz); p += mid_sz;
				// fill right part
				memcpy(p, ptr->ans_right, ptr->star_a_n2);
				return ptr->ans;
			}
		}
		else
		if (memcmp(&buf[1], &ptr->que[1], *n - 2) == 0)
		{
			*n = ptr->n_ans;
			return ptr->ans;
		}
		ptr_prev = ptr;
		ptr = ptr->next;
	}
	return NULL;
}
