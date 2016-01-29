/**
 * $Id: $
 *
 * @brief Red Pitaya library Logic analyzer acquisition module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include "la_acq.h"


#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "common.h"
#include "la_acq.h"
#include "generate.h"

int rp_LaAcqOpen(const char *a_dev, rp_handle_uio_t *handle) {

    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(a_dev)+1) * sizeof(char));
    strncpy(handle->dev, a_dev, strlen(a_dev)+1);

    if(strncmp(c_dummy_dev, a_dev, sizeof(c_dummy_dev))==0){
        handle->regset = (rp_la_acq_regset_t*) malloc(sizeof(rp_la_acq_regset_t));
    }
    else{
        // try opening the device
        handle->fd = open(handle->dev, O_RDWR);
        if (handle->fd==-1) {
            return -1;
        }

        // get regset pointer
        handle->regset = mmap(NULL, sizeof(rp_la_acq_regset_t), PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == MAP_FAILED) {
            return -1;
        }
        if(rp_LaAcqReset(handle)!=RP_OK){
            return -1;
        }
    }
    return RP_OK;
}

int rp_LaAcqClose(rp_handle_uio_t *handle) {
    int r=RP_OK;

    if(strncmp(c_dummy_dev, handle->dev, sizeof(c_dummy_dev))==0){



    }
    else{
        // release regset
        if(munmap((void *) handle->regset, sizeof(rp_la_acq_regset_t))!=0){
            r=-1;
        }

        // close device
        if(close (handle->fd)!=0){
            r=-1;
        }

        // free device path
        free(handle->dev);

        // free name
        free(handle->name);

    }
    return r;
}

/** Control registers setter & getter */
static int rp_LaAcqSetControl(rp_handle_uio_t *handle, rp_ctl_regset_t a_reg) {
    rp_ctl_regset_t *regset = (rp_ctl_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->ctl);
    iowrite32(a_reg.ctl, &regset->ctl);
    return RP_OK;
}


static int rp_LaAcqGetControl(rp_handle_uio_t *handle, rp_ctl_regset_t * a_reg) {
    rp_ctl_regset_t *regset = (rp_ctl_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->ctl);
    a_reg->ctl = ioread32(&regset->ctl);
    return RP_OK;
}


/** Acq. control */
int rp_LaAcqReset(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_RST_MASK;
    return rp_LaAcqSetControl(handle,reg);
}

int rp_LaAcqRunAcq(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_STA_MASK;
    return rp_LaAcqSetControl(handle,reg);
}

int rp_LaAcqStopAcq(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_STO_MASK;
    return rp_LaAcqSetControl(handle,reg);
}

int rp_LaAcqTriggerAcq(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_SWT_MASK;
    return rp_LaAcqSetControl(handle,reg);
}

int rp_LaAcqAcqIsStopped(rp_handle_uio_t *handle, bool * status){
    rp_ctl_regset_t reg;
    rp_LaAcqGetControl(handle, &reg);
    if(reg.ctl&RP_CTL_STA_MASK){
        *status=false;
    }
    else{
        *status=true;
    }
    return RP_OK;
}

/** Configuration registers setter & getter */
int rp_LaAcqSetConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t a_reg) {
    rp_la_cfg_regset_t *regset = (rp_la_cfg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->cfg);
    iowrite32(a_reg.acq, &regset->acq);
    iowrite32(a_reg.pre, &regset->pre);
    iowrite32(a_reg.pst, &regset->pst);
    return RP_OK;
}

int rp_LaAcqGetConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t * a_reg) {
    rp_la_cfg_regset_t *regset = (rp_la_cfg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->cfg);
    a_reg->acq = ioread32(&regset->acq);
    a_reg->pre = ioread32(&regset->pre);
    a_reg->pst = ioread32(&regset->pst);
    return RP_OK;
}

int rp_LaAcqGlobalTrigEnable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_global_trig_regset_t *regset = (rp_global_trig_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->gtrg);
    uint32_t tmp;
    tmp=ioread32(&regset->msk);
    tmp|=a_mask;
    iowrite32(tmp, &regset->msk);
    return RP_OK;
}

int rp_LaAcqGlobalTrigDisable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_global_trig_regset_t *regset = (rp_global_trig_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->gtrg);
    uint32_t tmp;
    tmp=ioread32(&regset->msk);
    tmp&=~a_mask;
    iowrite32(tmp, &regset->msk);
    return RP_OK;
}


/** Trigger settings setter & getter */
int rp_LaAcqSetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t a_reg) {
    rp_la_trg_regset_t *regset = (rp_la_trg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->trg);
    iowrite32(a_reg.cmp_msk, &regset->cmp_msk);
    iowrite32(a_reg.cmp_val, &regset->cmp_val);
    iowrite32(a_reg.edg_pos, &regset->edg_pos);
    iowrite32(a_reg.edg_neg, &regset->edg_neg);
    return RP_OK;
}

int rp_LaAcqGetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t * a_reg) {
    rp_la_trg_regset_t *regset = (rp_la_trg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->trg);
    a_reg->cmp_msk = ioread32(&regset->cmp_msk);
    a_reg->cmp_val = ioread32(&regset->cmp_val);
    a_reg->edg_pos = ioread32(&regset->edg_pos);
    a_reg->edg_neg = ioread32(&regset->edg_neg);
    return RP_OK;
}

/** Decimation settings setter & getter */
int rp_LaAcqSetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t a_reg) {
    rp_la_decimation_regset_t *regset = (rp_la_decimation_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->dec);
    iowrite32(a_reg.avg, &regset->avg);
    iowrite32(a_reg.dec, &regset->dec);
    iowrite32(a_reg.shr, &regset->shr);
    return RP_OK;
}

int rp_LaAcqGetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t * a_reg) {
    rp_la_decimation_regset_t *regset = (rp_la_decimation_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->dec);
    a_reg->avg = ioread32(&regset->avg);
    a_reg->dec = ioread32(&regset->dec);
    a_reg->shr = ioread32(&regset->shr);
    return RP_OK;
}

/** Data buffer pointers */
/*
int rp_LaAcqGetDataPointers(rp_handle_uio_t *handle, rp_data_ptrs_regset_t * a_reg) {
    rp_data_ptrs_regset_t *regset = (rp_data_ptrs_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->dpt);
    a_reg->start = ioread32(&regset->start);
    a_reg->trig = ioread32(&regset->trig);
    a_reg->stopped = ioread32(&regset->stopped);
    return RP_OK;
}
*/

int rp_LaAcqFpgaRegDump(rp_handle_uio_t *handle)
{
    return FpgaRegDump(0,(uint32_t*)handle->regset,(sizeof(rp_la_acq_regset_t)/sizeof(uint32_t)));
}
