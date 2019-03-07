
#include <zephyr.h>
#include <misc/util.h>
#include <logging/log.h>

#include <gpio.h>

LOG_MODULE_REGISTER(sched_demo, 4);


static struct device *gpio_dev;
static struct gpio_callback gpio_cb;

static void work_fn(struct k_work *work)
{
	gpio_pin_write(gpio_dev, 31, 0);
	LOG_INF("work");
	gpio_pin_enable_callback(gpio_dev, SW0_GPIO_PIN);
}
static K_WORK_DEFINE(work, work_fn);

static void button_pressed(struct device *gpio_dev, struct gpio_callback *cb,
			   u32_t pins)
{
	gpio_pin_disable_callback(gpio_dev, SW0_GPIO_PIN);
	gpio_pin_write(gpio_dev, 31, 1);
	k_work_submit(&work);
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

	LOG_INF("Start");
}
