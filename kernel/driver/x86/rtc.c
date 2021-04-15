/*
 * RTC CMOS driver
 *
 * This file is part of PanicOS.
 *
 * PanicOS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PanicOS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PanicOS.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <common/types.h>
#include <common/x86.h>
#include <defs.h>
#include <proc/kcall.h>

#define RTC_REG_SECOND 0
#define RTC_REG_MINUTE 2
#define RTC_REG_HOUR 4
#define RTC_REG_DAY_OF_WEEK 6
#define RTC_REG_DAY_OF_MONTH 7
#define RTC_REG_MONTH 8
#define RTC_REG_YEAR 9
#define RTC_REG_STATUS_B 0xb

static uint8_t cmos_read(unsigned int reg) {
	outb(0x70, reg);
	return inb(0x71);
}

static unsigned int rtc_get_year(void) {
	uint8_t regyear = cmos_read(RTC_REG_YEAR);
	return 2000 + (regyear >> 4) * 10 + (regyear & 0xf);
}

static unsigned int rtc_get_month(void) {
	uint8_t regmonth = cmos_read(RTC_REG_MONTH);
	return (regmonth >> 4) * 10 + (regmonth & 0xf);
}

static unsigned int rtc_get_day_of_week(void) {
	uint8_t regday = cmos_read(RTC_REG_DAY_OF_WEEK);
	return (regday >> 4) * 10 + (regday & 0xf);
}

static unsigned int rtc_get_day_of_month(void) {
	uint8_t regday = cmos_read(RTC_REG_DAY_OF_MONTH);
	return (regday >> 4) * 10 + (regday & 0xf);
}

static unsigned int rtc_get_hour(void) {
	uint8_t reghour = cmos_read(RTC_REG_HOUR);
	return (reghour >> 4) * 10 + (reghour & 0xf);
}

static unsigned int rtc_get_minute(void) {
	uint8_t regmin = cmos_read(RTC_REG_MINUTE);
	return (regmin >> 4) * 10 + (regmin & 0xf);
}

static unsigned int rtc_get_second(void) {
	uint8_t regsec = cmos_read(RTC_REG_SECOND);
	return (regsec >> 4) * 10 + (regsec & 0xf);
}

struct KernelTime {
	unsigned int year, month, day_of_week, day_of_month;
	unsigned int hour, minute, second;
};

static int date_kcall_handler(unsigned int p) {
	struct KernelTime* t = (void*)p;
	t->year = rtc_get_year();
	t->month = rtc_get_month();
	t->day_of_week = rtc_get_day_of_week();
	t->day_of_month = rtc_get_day_of_month();
	t->hour = rtc_get_hour();
	t->minute = rtc_get_minute();
	t->second = rtc_get_second();
	return 0;
}

void rtc_init(void) {
	if (cmos_read(RTC_REG_STATUS_B) & (1 << 2)) {
		cprintf("[rtc] WARNING: RTC not in BCD mode\n");
	}
	if (!(cmos_read(RTC_REG_STATUS_B) & (1 << 1))) {
		cprintf("[rtc] WARNING: RTC not in 24-hour mode\n");
	}
	cprintf("[rtc] Date %d-%d-%d %d:%d:%d\n", rtc_get_year(), rtc_get_month(),
			rtc_get_day_of_month(), rtc_get_hour(), rtc_get_minute(), rtc_get_second());
	kcall_set("date", date_kcall_handler);
}
