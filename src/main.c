#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/display.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/pm/device.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#define LED0_NODE DT_ALIAS(led0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
// static struct device *gpio_c;
static const struct device *display_dev;

#define LVGL_STACK_SIZE 4096
#define LVGL_PRIORITY 5
K_THREAD_STACK_DEFINE(lvgl_stack_area, LVGL_STACK_SIZE);
struct k_thread lvgl_thread_data;
k_tid_t lvgl_tid;

uint8_t buf[100];

lv_obj_t *canvas;
lv_obj_t *label;
lv_timer_t *info_update_timer;
void info_update(lv_timer_t *timer)
{
	static uint8_t cnt = 0;
	static uint8_t y = 0;
	static uint8_t x = 0;
	uint8_t buf[20];
	sprintf(buf, "#ffffff %02d#", cnt);
	lv_label_set_text(label, buf);
	lv_obj_align(label, LV_ALIGN_TOP_LEFT, x++, y);
	if (x == 72)
	{
		x = 0;
		y++;
	}
	if (y == 144)
		y = 0;
	cnt++;
	if (cnt == 100)
		cnt = 0;
}
void lvgl_entry_point(void *, void *, void *)
{

/*Create a buffer for the canvas*/
#define CANVAS_WIDTH 72
#define CANVAS_HEIGHT 144
	static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];
	canvas = lv_canvas_create(lv_scr_act());
	lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);
	lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
	for (uint8_t y = 108; y < 117; y++)
	{
		for (uint8_t x = 0; x < 36; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x00ffffff));
		}
	}
	for (uint8_t y = 108; y < 117; y++)
	{
		for (uint8_t x = 36; x < 72; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x0000ffff));
		}
	}
	for (uint8_t y = 117; y < 126; y++)
	{
		for (uint8_t x = 0; x < 36; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x00ff00ff));
		}
	}
	for (uint8_t y = 117; y < 126; y++)
	{
		for (uint8_t x = 36; x < 72; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x00ffff00));
		}
	}
	for (uint8_t y = 126; y < 135; y++)
	{
		for (uint8_t x = 0; x < 36; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x00ff0000));
		}
	}
	for (uint8_t y = 126; y < 135; y++)
	{
		for (uint8_t x = 36; x < 72; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x0000ff00));
		}
	}
	for (uint8_t y = 135; y < 144; y++)
	{
		for (uint8_t x = 0; x < 36; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x000000ff));
		}
	}
	for (uint8_t y = 135; y < 144; y++)
	{
		for (uint8_t x = 36; x < 72; x++)
		{
			lv_canvas_set_px_color(canvas, x, y, lv_color_hex(0x00000000));
		}
	}

	label = lv_label_create(lv_scr_act());
	lv_label_set_recolor(label, true);
	lv_label_set_text(label, "#ffffff 00#");

	lv_obj_align(label, LV_ALIGN_TOP_LEFT, 1, 0);
	lv_task_handler();

	info_update_timer = lv_timer_create(info_update, 500, NULL);
	lv_timer_set_repeat_count(info_update_timer, -1);
	lv_timer_ready(info_update_timer);
	lv_task_handler();
	for (;;)
	{
		lv_task_handler();
		lv_timer_handler();
		k_msleep(5);
	}
}

int main()
{
	LOG_INF("LPM009M360A");
	int ret;
	if (!gpio_is_ready_dt(&led))
	{
		return 0;
	}
	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0)
	{
		return 0;
	}

	display_dev = DEVICE_DT_GET(DT_NODELABEL(lcd0));
	if (!device_is_ready(display_dev))
	{
		LOG_ERR("Device %s not found. Aborting sample.",
				display_dev->name);
	}
	display_blanking_off(display_dev);

	// struct display_buffer_descriptor buf_desc;
	// (void)memset(buf, 0xFFu, 5);

	// buf_desc.buf_size = 90;
	// buf_desc.pitch = 8;
	// buf_desc.width = 72;
	// buf_desc.height = 10;
	// display_write(display_dev, 0, 0, &buf_desc, buf);
	lvgl_tid = k_thread_create(&lvgl_thread_data, lvgl_stack_area,
							   K_THREAD_STACK_SIZEOF(lvgl_stack_area),
							   lvgl_entry_point,
							   NULL, NULL, NULL,
							   LVGL_PRIORITY, 0, K_NO_WAIT);
	for (;;)
	{
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0)
		{
			return 0;
		}
		k_msleep(500);
	}
	return 0;
}
