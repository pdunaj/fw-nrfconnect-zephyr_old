
#include <zephyr.h>
#include <misc/util.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(sched_demo, 4);

#define THREAD_COUNT 2
#define STACK_SIZE 1024

static K_THREAD_STACK_ARRAY_DEFINE(stack, THREAD_COUNT, STACK_SIZE);
static struct k_thread thread[THREAD_COUNT];


static void thread_fn(void *p1, void *p2, void *p3)
{
	size_t id = (size_t)p1;

	while (true) {
		LOG_INF("%zu: %p", id, k_current_get());
		k_busy_wait(100000);
	}
}

void main(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(thread); i++) {
		k_thread_create(&thread[i],
				stack[i], STACK_SIZE,
				(k_thread_entry_t)thread_fn,
				(void *)i, NULL, NULL,
				K_PRIO_PREEMPT(0), 0, K_NO_WAIT);
		LOG_INF("Thread %zu created", i);
	}
	LOG_INF("Start");
}
