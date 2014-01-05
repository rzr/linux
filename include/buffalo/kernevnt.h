#ifndef _KERNEVNT_H_
#define _KERNEVNT_H_

/* routines are in kernel/arch/ppc/platforms/buffalo/kernevnt.c */

void kernevnt_LanAct(int linkspd, int isFull);
#ifdef CONFIG_MD
void kernevnt_RadiRecovery(int devno,int on, int isRecovery, int major, int minor);
void kernevnt_RadiScan(int devno, int on);
void kernevnt_RadiDegraded(int devno, int major, int minor);
#endif

// drivers/block/ll_rw_blk.c
void kernevnt_IOErr(const char *kdevname, const char *dir, unsigned long sector, unsigned int errcnt);  /* 2006.9.5 :add errcnt */
void kernevnt_FlashUpdate(int on);
void kernevnt_DriveDead(const char *drvname);
void kernevnt_I2cErr(void);
void kernevnt_MiconInt(void);
void kernevnt_EnetOverload(const char *name);

#endif
