
#include <zephyr.h>
#include <misc/util.h>
#include <logging/log.h>

#include <gpio.h>

LOG_MODULE_REGISTER(sched_demo, 4);

#define THREAD_COUNT 1
#define STACK_SIZE 1024

static K_THREAD_STACK_ARRAY_DEFINE(stack, THREAD_COUNT, STACK_SIZE);
static struct k_thread thread[THREAD_COUNT];
static const int prio[THREAD_COUNT] = {-1};

static struct device *gpio_dev;
static struct gpio_callback gpio_cb;

static void thread_fn(void *p1, void *p2, void *p3)
{
	size_t id = (size_t)p1;

	while (true) {
		LOG_INF("%zu: %p", id, k_current_get());
		gpio_pin_enable_callback(gpio_dev, SW0_GPIO_PIN);
		k_thread_suspend(k_current_get());
		gpio_pin_write(gpio_dev, 31, 0);
	}
}

static void button_pressed(struct device *gpio_dev, struct gpio_callback *cb,
			   u32_t pins)
{
	gpio_pin_disable_callback(gpio_dev, SW0_GPIO_PIN);
	gpio_pin_write(gpio_dev, 31, 1);
	k_thread_resume(&thread[0]);
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
	gpio_pin_write(gpio_dev, 31, 0);

	/* SW0_GPIO_PIN -> 11 for pca10056 */
	err = gpio_pin_configure(gpio_dev, SW0_GPIO_PIN,
				 GPIO_PUD_PULL_UP | GPIO_DIR_IN | GPIO_INT |
				 GPIO_INT_EDGE | GPIO_INT_ACTIVE_LOW);
	if (err) {
		LOG_ERR("Cannot configure in pin");
		return;
	}
	gpio_init_callback(&gpio_cb, button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(gpio_dev, &gpio_cb);
	gpio_pin_enable_callback(gpio_dev, SW0_GPIO_PIN);

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
