typedef struct TCacheItem
{
	struct TCacheItem *next;
	uint16_t  *que;
	uint16_t  *ans;
	uint16_t   n_que;
	uint16_t   n_ans;
	uint32_t   timestamp;
	// num bytes before/after star
	uint16_t   star_q_n1;
	uint16_t   star_q_n2;
	uint16_t   star_a_n1;
	uint16_t   star_a_n2;
	uint16_t  *ans_right;
} TCacheItem;

int   cache_answer(void *_buf, uint16_t n);
void* cache_question(void *buf, uint16_t n);
void* cache_search(void *_buf, uint16_t *n);
