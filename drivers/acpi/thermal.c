/*
 *  acpi_thermal.c - ACPI Thermal Zone Driver ($Revision: 41 $)
 *
 *  Copyright (C) 2001, 2002 Andy Grover <andrew.grover@intel.com>
 *  Copyright (C) 2001, 2002 Paul Diefenbaugh <paul.s.diefenbaugh@intel.com>
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or (at
 *  your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *  This driver fully implements the ACPI thermal policy as described in the
 *  ACPI 2.0 Specification.
 *
 *  TBD: 1. Implement passive cooling hysteresis.
 *       2. Enhance passive cooling (CPU) states/limit interface to support
 *          concepts of 'multiple limiters', upper/lower limits, etc.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/kmod.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

#include <acpi/acpi_bus.h>
#include <acpi/acpi_drivers.h>

#define ACPI_THERMAL_COMPONENT		0x04000000
#define ACPI_THERMAL_CLASS		"thermal_zone"
#define ACPI_THERMAL_DRIVER_NAME	"ACPI Thermal Zone Driver"
#define ACPI_THERMAL_DEVICE_NAME	"Thermal Zone"
#define ACPI_THERMAL_FILE_STATE		"state"
#define ACPI_THERMAL_FILE_TEMPERATURE	"temperature"
#define ACPI_THERMAL_FILE_TRIP_POINTS	"trip_points"
#define ACPI_THERMAL_FILE_COOLING_MODE	"cooling_mode"
#define ACPI_THERMAL_FILE_POLLING_FREQ	"polling_frequency"
#define ACPI_THERMAL_NOTIFY_TEMPERATURE	0x80
#define ACPI_THERMAL_NOTIFY_THRESHOLDS	0x81
#define ACPI_THERMAL_NOTIFY_DEVICES	0x82
#define ACPI_THERMAL_NOTIFY_CRITICAL	0xF0
#define ACPI_THERMAL_NOTIFY_HOT		0xF1
#define ACPI_THERMAL_MODE_ACTIVE	0x00
#define ACPI_THERMAL_MODE_PASSIVE	0x01
#define ACPI_THERMAL_MODE_CRITICAL   	0xff
#define ACPI_THERMAL_PATH_POWEROFF	"/sbin/poweroff"

#define ACPI_THERMAL_MAX_ACTIVE	10
#define ACPI_THERMAL_MAX_LIMIT_STR_LEN 65

#define KELVIN_TO_CELSIUS(t)    (long)(((long)t-2732>=0) ? ((long)t-2732+5)/10 : ((long)t-2732-5)/10)
#define CELSIUS_TO_KELVIN(t)	((t+273)*10)

#define _COMPONENT		ACPI_THERMAL_COMPONENT
ACPI_MODULE_NAME		("acpi_thermal")

MODULE_AUTHOR("Paul Diefenbaugh");
MODULE_DESCRIPTION(ACPI_THERMAL_DRIVER_NAME);
MODULE_LICENSE("GPL");

static int tzp;
module_param(tzp, int, 0);
MODULE_PARM_DESC(tzp, "Thermal zone polling frequency, in 1/10 seconds.\n");


static int acpi_thermal_add (struct acpi_device *device);
static int acpi_thermal_remove (struct acpi_device *device, int type);
static int acpi_thermal_state_open_fs(struct inode *inode, struct file *file);
static int acpi_thermal_temp_open_fs(struct inode *inode, struct file *file);
static int acpi_thermal_trip_open_fs(struct inode *inode, struct file *file);
static ssize_t acpi_thermal_write_trip_points (struct file*,const char __user *,size_t,loff_t *);
static int acpi_thermal_cooling_open_fs(struct inode *inode, struct file *file);
static ssize_t acpi_thermal_write_cooling_mode (struct file*,const char __user *,size_t,loff_t *);
static int acpi_thermal_polling_open_fs(struct inode *inode, struct file *file);
static ssize_t acpi_thermal_write_polling(struct file*,const char __user *,size_t,loff_t *);

static struct acpi_driver acpi_thermal_driver = {
	.name =		ACPI_THERMAL_DRIVER_NAME,
	.class =	ACPI_THERMAL_CLASS,
	.ids =		ACPI_THERMAL_HID,
	.ops =		{
				.add =		acpi_thermal_add,
				.remove =	acpi_thermal_remove,
			},
};

struct acpi_thermal_state {
	u8			critical:1;
	u8			hot:1;
	u8			passive:1;
	u8			active:1;
	u8			reserved:4;
	int			active_index;
};

struct acpi_thermal_state_flags {
	u8			valid:1;
	u8			enabled:1;
	u8			reserved:6;
};

struct acpi_thermal_critical {
	struct acpi_thermal_state_flags flags;
	unsigned long		temperature;
};

struct acpi_thermal_hot {
	struct acpi_thermal_state_flags flags;
	unsigned long		temperature;
};

struct acpi_thermal_passive {
	struct acpi_thermal_state_flags flags;
	unsigned long		temperature;
	unsigned long		tc1;
	unsigned long		tc2;
	unsigned long		tsp;
	struct acpi_handle_list	devices;
};

struct acpi_thermal_active {
	struct acpi_thermal_state_flags flags;
	unsigned long		temperature;
	struct acpi_handle_list	devices;
};

struct acpi_thermal_trips {
	struct acpi_thermal_critical critical;
	struct acpi_thermal_hot	hot;
	struct acpi_thermal_passive passive;
	struct acpi_thermal_active active[ACPI_THERMAL_MAX_ACTIVE];
};

struct acpi_thermal_flags {
	u8			cooling_mode:1;		/* _SCP */
	u8			devices:1;		/* _TZD */
	u8			reserved:6;
};

struct acpi_thermal {
	acpi_handle		handle;
	acpi_bus_id		name;
	unsigned long		temperature;
	unsigned long		last_temperature;
	unsigned long		polling_frequency;
	u8			cooling_mode;
	volatile u8		zombie;
	struct acpi_thermal_flags flags;
	struct acpi_thermal_state state;
	struct acpi_thermal_trips trips;
	struct acpi_handle_list	devices;
	struct timer_list	timer;
};

static struct file_operations acpi_thermal_state_fops = {
	.open		= acpi_thermal_state_open_fs,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct file_operations acpi_thermal_temp_fops = {
	.open		= acpi_thermal_temp_open_fs,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct file_operations acpi_thermal_trip_fops = {
	.open		= acpi_thermal_trip_open_fs,
	.read		= seq_read,
	.write		= acpi_thermal_write_trip_points,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct file_operations acpi_thermal_cooling_fops = {
	.open		= acpi_thermal_cooling_open_fs,
	.read		= seq_read,
	.write		= acpi_thermal_write_cooling_mode,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static struct file_operations acpi_thermal_polling_fops = {
	.open		= acpi_thermal_polling_open_fs,
	.read		= seq_read,
	.write		= acpi_thermal_write_polling,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/* --------------------------------------------------------------------------
                             Thermal Zone Management
   -------------------------------------------------------------------------- */

static int
acpi_thermal_get_temperature (
	struct acpi_thermal *tz)
{
	acpi_status		status = AE_OK;

	ACPI_FUNCTION_TRACE("acpi_thermal_get_temperature");

	if (!tz)
		return_VALUE(-EINVAL);

	tz->last_temperature = tz->temperature;

	status = acpi_evaluate_integer(tz->handle, "_TMP", NULL, &tz->temperature);
	if (ACPI_FAILURE(status))
		return_VALUE(-ENODEV);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Temperature is %lu dK\n", tz->temperature));

	return_VALUE(0);
}


static int
acpi_thermal_get_polling_frequency (
	struct acpi_thermal	*tz)
{
	acpi_status		status = AE_OK;

	ACPI_FUNCTION_TRACE("acpi_thermal_get_polling_frequency");

	if (!tz)
		return_VALUE(-EINVAL);

	status = acpi_evaluate_integer(tz->handle, "_TZP", NULL, &tz->polling_frequency);
	if (ACPI_FAILURE(status))
		return_VALUE(-ENODEV);

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Polling frequency is %lu dS\n", tz->polling_frequency));

	return_VALUE(0);
}


static int
acpi_thermal_set_polling (
	struct acpi_thermal	*tz,
	int			seconds)
{
	ACPI_FUNCTION_TRACE("acpi_thermal_set_polling");

	if (!tz)
		return_VALUE(-EINVAL);

	tz->polling_frequency = seconds * 10;	/* Convert value to deci-seconds */

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Polling frequency set to %lu seconds\n", tz->polling_frequency));

	return_VALUE(0);
}


static int
acpi_thermal_set_cooling_mode (
	struct acpi_thermal	*tz,
	int			mode)
{
	acpi_status		status = AE_OK;
	union acpi_object	arg0 = {ACPI_TYPE_INTEGER};
	struct acpi_object_list	arg_list = {1, &arg0};
	acpi_handle		handle = NULL;

	ACPI_FUNCTION_TRACE("acpi_thermal_set_cooling_mode");

	if (!tz)
		return_VALUE(-EINVAL);

	status = acpi_get_handle(tz->handle, "_SCP", &handle);
	if (ACPI_FAILURE(status)) {
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "_SCP not present\n"));
		return_VALUE(-ENODEV);
	}

	arg0.integer.value = mode;

	status = acpi_evaluate_object(handle, NULL, &arg_list, NULL);
	if (ACPI_FAILURE(status))
		return_VALUE(-ENODEV);

	tz->cooling_mode = mode;

	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Cooling mode [%s]\n", 
		mode?"passive":"active"));

	return_VALUE(0);
}


static int
acpi_thermal_get_trip_points (
	struct acpi_thermal *tz)
{
	acpi_status		status = AE_OK;
	int			i = 0;

	ACPI_FUNCTION_TRACE("acpi_thermal_get_trip_points");

	if (!tz)
		return_VALUE(-EINVAL);

	/* Critical Shutdown (required) */

	status = acpi_evaluate_integer(tz->handle, "_CRT", NULL, 
		&tz->trips.critical.temperature);
	if (ACPI_FAILURE(status)) {
		tz->trips.critical.flags.valid = 0;
		ACPI_DEBUG_PRINT((ACPI_DB_ERROR, "No critical threshold\n"));
		return_VALUE(-ENODEV);
	}
	else {
		tz->trips.critical.flags.valid = 1;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found critical threshold [%lu]\n", tz->trips.critical.temperature));
	}

	/* Critical Sleep (optional) */

	status = acpi_evaluate_integer(tz->handle, "_HOT", NULL, &tz->trips.hot.temperature);
	if (ACPI_FAILURE(status)) {
		tz->trips.hot.flags.valid = 0;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "No hot threshold\n"));
	}
	else {
		tz->trips.hot.flags.valid = 1;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found hot threshold [%lu]\n", tz->trips.hot.temperature));
	}

	/* Passive: Processors (optional) */

	status = acpi_evaluate_integer(tz->handle, "_PSV", NULL, &tz->trips.passive.temperature);
	if (ACPI_FAILURE(status)) {
		tz->trips.passive.flags.valid = 0;
		ACPI_DEBUG_PRINT((ACPI_DB_INFO, "No passive threshold\n"));
	}
	else {
		tz->trips.passive.flags.valid = 1;

		status = acpi_evaluate_integer(tz->handle, "_TC1", NULL, &tz->trips.passive.tc1);
		if (ACPI_FAILURE(status))
			tz->trips.passive.flags.valid = 0;

		status = acpi_evaluate_integer(tz->handle, "_TC2", NULL, &tz->trips.passive.tc2);
		if (ACPI_FAILURE(status))
			tz->trips.passive.flags.valid = 0;

		status = acpi_evaluate_integer(tz->handle, "_TSP", NULL, &tz->trips.passive.tsp);
		if (ACPI_FAILURE(status))
			tz->trips.passive.flags.valid = 0;

		status = acpi_evaluate_reference(tz->handle, "_PSL", NULL, &tz->trips.passive.devices);
		if (ACPI_FAILURE(status))
			tz->trips.passive.flags.valid = 0;

		if (!tz->trips.passive.flags.valid)
			ACPI_DEBUG_PRINT((ACPI_DB_WARN, "Invalid passive threshold\n"));
		else
			ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found passive threshold [%lu]\n", tz->trips.passive.temperature));
	}

	/* Active: Fans, etc. (optional) */

	for (i=0; i<ACPI_THERMAL_MAX_ACTIVE; i++) {

		char name[5] = {'_','A','C',('0'+i),'\0'};

		status = acpi_evaluate_integer(tz->handle, name, NULL, &tz->trips.active[i].temperature);
		if (ACPI_FAILURE(status))
			break;

		name[2] = 'L';
		status = acpi_evaluate_reference(tz->handle, name, NULL, &tz->trips.active[i].devices);
		if (ACPI_SUCCESS(status)) {
			tz->trips.active[i].flags.valid = 1;
			ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Found active threshold [%d]:[%lu]\n", i, tz->trips.active[i].temperature));
		}
		else
			ACPI_DEBUG_PRINT((ACPI_DB_ERROR, "Invalid active threshold [%d]\n", i));
	}

	return_VALUE(0);
}


static int
acpi_thermal_get_devices (
	struct acpi_thermal	*tz)
{
	acpi_status		status = AE_OK;

	ACPI_FUNCTION_TRACE("acpi_thermal_get_devices");

	if (!tz)
		return_VALUE(-EINVAL);

	status = acpi_evaluate_reference(tz->handle, "_TZD", NULL, &tz->devices);
	if (ACPI_FAILURE(status))
		return_VALUE(-ENODEV);

	return_VALUE(0);
}


static int
acpi_thermal_call_usermode (
	char			*path)
{
	char			*argv[2] = {NULL, NULL};
	char			*envp[3] = {NULL, NULL, NULL};

	ACPI_FUNCTION_TRACE("acpi_thermal_call_usermode");

	if (!path)
		return_VALUE(-EINVAL);

	argv[0] = path;

	/* minimal command environment */
	envp[0] = "HOME=/";
	envp[1] = "PATH=/sbin:/bin:/usr/sbin:/usr/bin";
	
	call_usermodehelper(argv[0], argv, envp, 0);

	return_VALUE(0);
}


static int
acpi_thermal_critical (
	struct acpi_thermal	*tz)
{
	int			result = 0;
	struct acpi_device	*device = NULL;

	ACPI_FUNCTION_TRACE("acpi_thermal_critical");

	if (!tz || !tz->trips.critical.flags.valid)
		return_VALUE(-EINVAL);

	if (tz->temperature >= tz->trips.critical.temperature) {
		ACPI_DEBUG_PRINT((ACPI_DB_WARN, "Critical trip point\n"));
		tz->trips.critical.flags.enabled = 1;
	}
	else if (tz->trips.critical.flags.enabled)
		tz->trips.critical.flags.enabled = 0;

	result = acpi_bus_get_device(tz->handle, &device);
	if (result)
		return_VALUE(result);

	printk(KERN_EMERG "Critical temperature reached (%ld C), shutting down.\n", KELVIN_TO_CELSIUS(tz->temperature));
	acpi_bus_generate_event(device, ACPI_THERMAL_NOTIFY_CRITICAL, tz->trips.critical.flags.enabled);

	acpi_thermal_call_usermode(ACPI_THERMAL_PATH_POWEROFF);

	return_VALUE(0);
}


static int
acpi_thermal_hot (
	struct acpi_thermal	*tz)
{
	int			result = 0;
	struct acpi_device	*device = NULL;

	ACPI_FUNCTION_TRACE("acpi_thermal_hot");

	if (!tz || !tz->trips.hot.flags.valid)
		return_VALUE(-EINVAL);

	if (tz->temperature >= tz->trips.hot.temperature) {
		ACPI_DEBUG_PRINT((ACPI_DB_WARN, "Hot trip point\n"));
		tz->trips.hot.flags.enabled = 1;
	}
	else if (tz->trips.hot.flags.enabled)
		tz->trips.hot.flags.enabled = 0;

	result = acpi_bus_get_device(tz->handle, &device);
	if (result)
		return_VALUE(result);

	acpi_bus_generate_event(device, ACPI_THERMAL_NOTIFY_HOT, tz->trips.hot.flags.enabled);

	/* TBD: Call user-mode "sleep(S4)" function */

	return_VALUE(0);
}


static int
acpi_thermal_passive (
	struct acpi_thermal	*tz)
{
	int			result = 0;
	struct acpi_thermal_passive *passive = NULL;
	int			trend = 0;
	int			i = 0;

	ACPI_FUNCTION_TRACE("acpi_thermal_passive");

	if (!tz || !tz->trips.passive.flags.valid)
		return_VALUE(-EINVAL);

	passive = &(tz->trips.passive);

	/*
	 * Above Trip?
	 * -----------
	 * Calcul