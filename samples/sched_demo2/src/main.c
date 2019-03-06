
#include <zephyr.h>
#include <misc/util.h>
#include <logging/log.h>

#include <gpio.h>

LOG_MODULE_REGISTER(sched_demo, 4);

#define THREAD_COUNT 16
#define STACK_SIZE 1024

static K_THREAD_STACK_ARRAY_DEFINE(stack, THREAD_COUNT, STACK_SIZE);
static struct k_thread thread[THREAD_COUNT];
static const int prio[THREAD_COUNT];/// = {-1, -1, -1, -1};

static struct device *gpio_dev;

static void thread_fn(void *p1, void *p2, void *p3)
{
	size_t id = (size_t)p1;

	while (true) {
		LOG_INF("%zu: %p", id, k_current_get());
		k_busy_wait(100000);
		gpio_pin_write(gpio_dev, 31, 1);
		k_yield();
		gpio_pin_write(gpio_dev, 31, 0);
	}
}

void main(void)
{
	gpio_dev = device_get_binding(DT_GPIO_P0_DEV_NAME);
	if (!gpio_dev) {
		LOG_ERR("No GPIO");
		return;
	}

	int err = gpio_pin_configure(gpio_dev, 31, GPIO_DIR_OUT);
	if (err) {
		LOG_ERR("Cannot configure pin");
		return;
	}

	for (size_t i = 0; i < ARRAY_SIZE(thread); i++) {
		k_thread_create(&thread[i],
				&stack[i][0], STACK_SIZE,
				(k_thread_entry_t)thread_fn,
				(void *)i, NULL, NULL,
				prio[i], 0, K_NO_WAIT);
		LOG_INF("Thread %zu created", i);
	}
	LOG_INF("Start");
}
