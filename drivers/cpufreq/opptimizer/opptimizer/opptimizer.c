/*
 opptimizer_n9.ko - The OPP Mannagement API
 version 1.5.3
 by Lance Colton <lance.colton@gmail.com>
 License: GNU GPLv3
 <http://www.gnu.org/licenses/gpl-3.0.html>

 Latest Source & changelog:
 https://gitorious.org/opptimizer-n9/opptimizer-n9

 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/cpufreq.h>
#include <plat/common.h>
#include <plat/opp.h>
#include <plat/clock.h>
#include "../../../../arch/arm/mach-omap2/voltage.h"
#include <linux/smp_lock.h>

#include "../symsearch/symsearch.h"

#define DRIVER_AUTHOR "Lance Colton <lance.colton@gmail.com>\n"
#define DRIVER_DESCRIPTION "opptimizer.ko - The OPP Management API\n\
https://gitorious.org/opptimizer-n9/opptimizer-n9 for source\n\
This module uses SYMSEARCH by Skrilax_CZ\n\
Made possible by Jeffrey Kawika Patricio and Tiago Sousa\n"
#define DRIVER_VERSION "1.5.3"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESCRIPTION);
MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL");

// opp.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(int,
						opp_get_opp_count_fp, enum opp_t opp_type);
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct omap_opp *,
						opp_find_freq_floor_fp, enum opp_t opp_type, unsigned long *freq);
SYMSEARCH_DECLARE_FUNCTION_STATIC(unsigned long,
						opp_get_voltage_fp, const struct omap_opp *opp);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int,
						opp_disable_fp, struct omap_opp *opp);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int,
						opp_enable_fp, struct omap_opp *opp);
//smartreflex-class1p5.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(void,
						sr_class1p5_reset_calib_fp, int vdd, bool reset, bool recal);
//voltage.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(unsigned long,
						omap_voltageprocessor_get_voltage_fp, int vp_id);
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct omap_volt_data *,
						omap_get_volt_data_fp, int vdd, unsigned long volt);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int,
						omap_voltage_scale_fp, int vdd, struct omap_volt_data *vdata_target, struct omap_volt_data *vdata_current);
SYMSEARCH_DECLARE_FUNCTION_STATIC(void,
						vc_setup_on_voltage_fp, u32 vdd, unsigned long target_volt);
//cpu-omap.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(unsigned int,
						omap_getspeed_fp, unsigned int cpu);
//clock.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(long,
						clk_round_rate_fp, struct clk *clk, unsigned long rate);
SYMSEARCH_DECLARE_FUNCTION_STATIC(int,
						clk_set_rate_fp, struct clk *clk, unsigned long rate);
//clkdev.c
SYMSEARCH_DECLARE_FUNCTION_STATIC(struct clk*,
						clk_get_fp, struct device *dev, const char *con_id);
//cpufreq.h
SYMSEARCH_DECLARE_FUNCTION_STATIC(int,
						cpufreq_update_policy_fp,unsigned int cpu);

static int opp_count, enabled_opp_count, main_index, cpufreq_index;

unsigned long default_max_rate;

struct omap_volt_data default_vdata;

static struct cpufreq_frequency_table *freq_table;
static struct cpufreq_policy *policy;

#define MPU_CLK		"arm_fck"
#define BUF_SIZE PAGE_SIZE
static char *buf;

/**
 * struct omap_opp - OMAP OPP description structure
 * @enabled:	true/false - marking this OPP as enabled/disabled
 * @rate:	Frequency in hertz
 * @u_volt:	Nominal voltage in microvolts corresponding to this OPP
 * @opp_id:	opp identifier (deprecated)
 *
 * This structure stores the OPP information for a given domain.
 */
struct omap_opp {
	bool enabled;
	unsigned long rate;
	unsigned long u_volt;
	u8 opp_id;
};

/**
 * omap_volt_data - Omap voltage specific data.
 *
 * @u_volt_nominal	: The possible voltage value in uVolts
 * @u_volt_dyn_nominal	: The run time optimized nominal voltage for device.
 *			  this dynamic nominal is the nominal voltage
 *			  specialized for that device at that time.
 * @u_volt_dyn_margin	: margin to add on top of calib voltage for this opp
 * @u_volt_calib	: Calibrated voltage for this opp
 * @sr_nvalue		: Smartreflex N target value at voltage <voltage>
 * @sr_errminlimit	: Error min limit value for smartreflex. This value
 *			  differs at differnet opp and thus is linked
 *			  with voltage.
 * @vp_errorgain	: Error gain value for the voltage processor. This
 *			  field also differs according to the voltage/opp.
 */
//struct omap_volt_data {
//	unsigned long	u_volt_nominal;
//	unsigned long	u_volt_dyn_nominal;
//	unsigned long	u_volt_dyn_margin;
//	unsigned long	u_volt_calib;
//	u32		sr_nvalue;
//	u8		sr_errminlimit;
//	u8		vp_errorgain;
//	bool		abb;
//	u32		sr_val;
//	u32		sr_error;
//};


static int proc_opptimizer_read(char *buffer, char **buffer_location,
							  off_t offset, int count, int *eof, void *data)
{
	int ret = 0;
	unsigned long freq = ULONG_MAX;
	struct omap_opp *opp = ERR_PTR(-ENODEV);
	struct omap_volt_data *vdata;

	opp = opp_find_freq_floor_fp(OPP_MPU, &freq);
	if (IS_ERR(opp)) {
		ret = 0;
	}

	vdata = omap_get_volt_data_fp(0, opp_get_voltage_fp(opp));

	ret += scnprintf(buffer+ret, count-ret, "opp rate: %lu\n", opp->rate);
	ret += scnprintf(buffer+ret, count-ret, "freq table [0]: %u\n", freq_table[0].frequency);
	ret += scnprintf(buffer+ret, count-ret, "policy->max: %u\n", policy->max);
	ret += scnprintf(buffer+ret, count-ret, "cpuinfo.max_freq: %u\n", policy->cpuinfo.max_freq);
	ret += scnprintf(buffer+ret, count-ret, "user_policy.max: %u\n", policy->user_policy.max);
	ret += scnprintf(buffer+ret, count-ret, "omap_voltageprocessor_get_voltage: %lu\n", omap_voltageprocessor_get_voltage_fp(0));
	ret += scnprintf(buffer+ret, count-ret, "vdata->u_volt_nominal: %10ld\n", vdata->u_volt_nominal);
	ret += scnprintf(buffer+ret, count-ret, "vdata->u_volt_dyn_nominal: %10ld\n", vdata->u_volt_dyn_nominal);
	ret += scnprintf(buffer+ret, count-ret, "vdata->u_volt_dyn_margin: %10ld\n", vdata->u_volt_dyn_margin);
	ret += scnprintf(buffer+ret, count-ret, "vdata->u_volt_calib: %10ld\n", vdata->u_volt_calib);
	ret += scnprintf(buffer+ret, count-ret, "vdata->sr_nvalue: 0x%08x\n", vdata->sr_nvalue);
	ret += scnprintf(buffer+ret, count-ret, "vdata->sr_errminlimit: %u\n", vdata->sr_errminlimit);
	ret += scnprintf(buffer+ret, count-ret, "vdata->vp_errorgain: 0x%08x\n", vdata->vp_errorgain);
	ret += scnprintf(buffer+ret, count-ret, "vdata->sr_error: 0x%08x\n", vdata->sr_error);
	ret += scnprintf(buffer+ret, count-ret, "vdata->sr_val: 0x%08x\n", vdata->sr_val);
	ret += scnprintf(buffer+ret, count-ret, "vdata->abb: %2s\n", (vdata->abb) ? "yes" : "no");
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->u_volt_nominal: %10ld\n", default_vdata.u_volt_nominal);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->u_volt_dyn_nominal: %10ld\n", default_vdata.u_volt_dyn_nominal);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->u_volt_dyn_margin: %10ld\n", default_vdata.u_volt_dyn_margin);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->u_volt_calib: %10ld\n", default_vdata.u_volt_calib);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->sr_nvalue: 0x%08x\n", default_vdata.sr_nvalue);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->sr_errminlimit: %u\n", default_vdata.sr_errminlimit);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->vp_errorgain: 0x%08x\n", default_vdata.vp_errorgain);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->sr_error: 0x%08x\n", default_vdata.sr_error);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->sr_val: 0x%08x\n", default_vdata.sr_val);
	ret += scnprintf(buffer+ret, count-ret, "Default_vdata->abb: %2s\n", (default_vdata.abb) ? "yes" : "no");
	ret += scnprintf(buffer+ret, count+ret, "v%s by @CreamyG31337\n", DRIVER_VERSION);
	return ret;
};

static int proc_opptimizer_write(struct file *filp, const char __user *buffer,
						 unsigned long len, void *data)
{
	unsigned long rate, freq = ULONG_MAX;
	unsigned long u_volt_current, u_volt_req = 0;
	struct omap_opp *opp = ERR_PTR(-ENODEV);
	struct cpufreq_freqs freqs;
	static struct clk *mpu_clk;
	struct omap_volt_data *volt_data;
	int ret;

	lock_kernel();

	mpu_clk = clk_get_fp(NULL, MPU_CLK);
	if (IS_ERR(mpu_clk))
		return PTR_ERR(mpu_clk);

	if(!len || len >= BUF_SIZE)
		return -ENOSPC;
	if(copy_from_user(buf, buffer, len))
		return -EFAULT;
	buf[len] = 0;
	if(sscanf(buf, "%lu %lu", &rate, &u_volt_req) >= 1) {
		opp = opp_find_freq_floor_fp(OPP_MPU, &freq);//this will get the highest speed one
		if (IS_ERR(opp)) {
			return -ENODEV;
		}
		//opp_disable_fp(opp);
		volt_data = omap_get_volt_data_fp(0, opp_get_voltage_fp(opp));
		if(rate > 1700000000 || rate < 800000000){ //800mhz - 1.7ghz limits; i assume typo out of that range
			printk(KERN_INFO "opptimizer: rate too high or low!\n");
			return len;
		}

		freq_table[0].frequency = policy->max = policy->cpuinfo.max_freq = policy->user_policy.max = rate / 1000; // break locks
		freqs.cpu = 0;//only 1 cpu
		freqs.old = omap_getspeed_fp(0); //this one is already divided by 1k,
		freqs.new = rate / 1000;  //clk_round_rate_fp(mpu_clk, rate) / 1000;
		if (freqs.old != freqs.new){
			//cpufreq_notify_transition(&freqs, CPUFREQ_PRECHANGE);
		}
		if (freqs.new < freqs.old){
			//set rate before (probably) lowering voltage
			opp->rate = rate;//not really sure what happens when i set this directly...
			//ret = clk_set_rate_fp(mpu_clk, freqs.new * 1000);
		}
		//set new voltage 1st
		//do we need to deal with dvfs_mutex ?? as long as we do the calibrate after, i think not
		if (u_volt_req != 0){
			struct omap_volt_data vdata_current;
			memcpy(&vdata_current, volt_data, sizeof(vdata_current));
			u_volt_current = omap_voltageprocessor_get_voltage_fp(0);//why am i doing this??
			vdata_current.u_volt_calib = u_volt_current;//vdata_current now contains current volt data
			if (u_volt_req >= 1425000){
				u_volt_req = 1425000;//max for now
			}
			if (u_volt_req <= 1000000){
				u_volt_req = 1000000;//min for now
			}
			volt_data->u_volt_calib = u_volt_req;
			volt_data->u_volt_dyn_nominal = u_volt_req;
			volt_data->u_volt_dyn_margin = 0;
			volt_data->sr_errminlimit = 0x16; // default is 0xF9
			//volt_data->vp_errorgain = 0xFF;	//default is 0x16 ; hopefully 1st error will be the limit then.
			if (volt_data->u_volt_calib != u_volt_current) {
				omap_voltage_scale_fp(VDD1, volt_data, &vdata_current);
			}
			vc_setup_on_voltage_fp(VDD1, volt_data->u_volt_calib);
		}
		else{//fix SR, set back to default voltage
			struct omap_volt_data vdata_current;
			memcpy(&vdata_current, volt_data, sizeof(vdata_current));
			u_volt_current = omap_voltageprocessor_get_voltage_fp(0);
			vdata_current.u_volt_calib = u_volt_current;
			volt_data->u_volt_calib = default_vdata.u_volt_calib;
			volt_data->u_volt_dyn_nominal = default_vdata.u_volt_dyn_nominal;
			volt_data->u_volt_dyn_margin = default_vdata.u_volt_dyn_margin;
			volt_data->sr_errminlimit = default_vdata.sr_errminlimit;
			if (default_vdata.u_volt_calib != u_volt_current) {
				printk(KERN_INFO "opptimizer: returning to default voltage\n");
				omap_voltage_scale_fp(VDD1, volt_data, &vdata_current);
			}
			vc_setup_on_voltage_fp(VDD1, volt_data->u_volt_calib);
		}

		if (freqs.new > freqs.old){
			//set rate after raising voltage
			opp->rate = rate;//not really sure what happens when i set this directly...
			//ret = clk_set_rate_fp(mpu_clk, freqs.new * 1000);
		}
		if (freqs.old != freqs.new){
			//cpufreq_notify_transition(&freqs, CPUFREQ_POSTCHANGE);
			printk(KERN_INFO "opptimizer: updated max rate to %dmhz \n",freqs.new / 1000);
		}
		//opp_enable_fp(opp);
		sr_class1p5_reset_calib_fp(VDD1, true, true); //request smartreflex recalibrate, wipe old settings
	} else
		printk(KERN_INFO "opptimizer: incorrect parameters\n");

	//fix cpufreq_stats

	cpufreq_update_policy_fp(0);

	unlock_kernel();

	return len;
};


static int __init opptimizer_init(void)
{
	unsigned long freq = ULONG_MAX;
	struct omap_opp *opp = ERR_PTR(-ENODEV);
	struct proc_dir_entry *proc_entry;
	struct omap_volt_data *volt_data;


	printk(KERN_INFO " %s %s\n", DRIVER_DESCRIPTION, DRIVER_VERSION);
	printk(KERN_INFO " Created by %s\n", DRIVER_AUTHOR);

	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, opp_get_opp_count, opp_get_opp_count_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, opp_find_freq_floor, opp_find_freq_floor_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, opp_get_voltage, opp_get_voltage_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, sr_class1p5_reset_calib, sr_class1p5_reset_calib_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, omap_get_volt_data, omap_get_volt_data_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, omap_getspeed, omap_getspeed_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, clk_round_rate, clk_round_rate_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, clk_set_rate, clk_set_rate_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, clk_get, clk_get_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, omap_voltageprocessor_get_voltage, omap_voltageprocessor_get_voltage_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, omap_voltage_scale, omap_voltage_scale_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, vc_setup_on_voltage, vc_setup_on_voltage_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, opp_disable, opp_disable_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, opp_enable, opp_enable_fp);
	//SYMSEARCH_BIND_FUNCTION_TO(opptimizer, cpufreq_frequency_get_table, cpufreq_frequency_get_table_fp);
	//SYMSEARCH_BIND_FUNCTION_TO(opptimizer, cpufreq_stats_create_table, cpufreq_stats_create_table_fp);
	SYMSEARCH_BIND_FUNCTION_TO(opptimizer, cpufreq_update_policy, cpufreq_update_policy_fp);



	freq_table = cpufreq_frequency_get_table(0);
	policy = cpufreq_cpu_get(0);


	opp_count = enabled_opp_count = (opp_get_opp_count_fp(OPP_MPU));
	if (enabled_opp_count == opp_count) {
		main_index = cpufreq_index = (enabled_opp_count-1);
	} else {
		main_index = enabled_opp_count;
		cpufreq_index = (enabled_opp_count-1);
	}

	opp = opp_find_freq_floor_fp(OPP_MPU, &freq);
	if (!opp || IS_ERR(opp)) {
		return -ENODEV;
	}

	default_max_rate = opp->rate;

	volt_data = omap_get_volt_data_fp(0, opp_get_voltage_fp(opp));

	memcpy(&default_vdata, volt_data, sizeof(default_vdata));

	buf = (char *)vmalloc(BUF_SIZE);

	proc_entry = create_proc_read_entry("opptimizer", 0644, NULL, proc_opptimizer_read, NULL);
	proc_entry->write_proc = proc_opptimizer_write;

	return 0;
};

static void __exit opptimizer_exit(void)
{
	unsigned long temp_rate, freq = ULONG_MAX;
	struct omap_opp *opp = ERR_PTR(-ENODEV);
	struct omap_volt_data *vdata_current;

	remove_proc_entry("opptimizer", NULL);

	vfree(buf);

	opp = opp_find_freq_floor_fp(OPP_MPU, &freq);
	if (!opp || IS_ERR(opp)) {
		return;
	}

	vdata_current = omap_get_volt_data_fp(0, opp_get_voltage_fp(opp));

	if(opp->rate < default_max_rate){
		//change voltage 1st because we are actually speeding up
		if (default_vdata.u_volt_calib != vdata_current->u_volt_calib) {
			omap_voltage_scale_fp(VDD1, &default_vdata, vdata_current);
		}
		opp->rate = default_max_rate;
		freq_table[0].frequency =
			policy->max = policy->cpuinfo.max_freq =
			policy->user_policy.max = default_max_rate / 1000;
	}else{
		//change voltage 2nd because we need to slow down first
		opp->rate = default_max_rate;
		freq_table[0].frequency =
			policy->max = policy->cpuinfo.max_freq =
			policy->user_policy.max = default_max_rate / 1000;
		if (default_vdata.u_volt_calib != vdata_current->u_volt_calib) {
			omap_voltage_scale_fp(VDD1, &default_vdata, vdata_current);
		}
	}
	printk(KERN_INFO " opptimizer: Reseting values to default... Goodbye!\n");
};

module_init(opptimizer_init);
module_exit(opptimizer_exit);

