// SPDX-License-Identifier: LGPL
/* Copyright (C) 2018-2019 Jiaxun Yang <jiaxun.yang@flygoat.com> */
/* Ryzen NB SMU Service Request Opreations */

#include "nb_smu_ops.h"

u32 smu_service_req(smu_t smu ,u32 id ,smu_service_args_t *args)
{
    u32 response = 0x0;
    DBG("SMU_SERVICE REQ_ID:0x%x\n", id);
    DBG("SMU_SERVICE REQ: arg0: 0x%x, arg1:0x%x, arg2:0x%x, arg3:0x%x, arg4: 0x%x, arg5: 0x%x\n",  \
        args->arg0, args->arg1, args->arg2, args->arg3, args->arg4, args->arg5);

    /* Clear the response */
    smn_reg_write(smu->nb, smu->rep, 0x0);
    /* Pass arguments */
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 0), args->arg0);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 1), args->arg1);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 2), args->arg2);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 3), args->arg3);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 4), args->arg4);
    smn_reg_write(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 5), args->arg5);
    /* Send message ID */
    smn_reg_write(smu->nb, smu->msg, id);
    /* Wait until reponse changed */
    while(response == 0x0) {
        response = smn_reg_read(smu->nb, smu->rep);
    }
    /* Read back arguments */
    args->arg0 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 0));
    args->arg1 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 1));
    args->arg2 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 2));
    args->arg3 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 3));
    args->arg4 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 4));
    args->arg5 = smn_reg_read(smu->nb, C2PMSG_ARGx_ADDR(smu->arg_base, 5));

    DBG("SMU_SERVICE REP: REP: 0x%x, arg0: 0x%x, arg1:0x%x, arg2:0x%x, arg3:0x%x, arg4: 0x%x, arg5: 0x%x\n",  \
        response, args->arg0, args->arg1, args->arg2, args->arg3, args->arg4, args->arg5);

    return response;
}

smu_t get_smu(nb_t nb, int smu_type) {
    smu_t smu;
    uint32_t rep; /* REP of test message */
    smu_service_args_t arg = {0, 0, 0, 0, 0, 0}; /* Test message shuld have no arguments */

    smu = (smu_t)malloc((sizeof(smu_t)));
    smu->nb = nb;
    /* Fill SMU information */
    switch(smu_type){
        case TYPE_MP1:
            smu->msg = MP1_C2PMSG_MESSAGE_ADDR;
            smu->rep = MP1_C2PMSG_RESPONSE_ADDR;
            smu->arg_base = MP1_C2PMSG_ARG_BASE;
            break;
        case TYPE_PSMU:
            smu->msg = PSMU_C2PMSG_MESSAGE_ADDR;
            smu->rep = PSMU_C2PMSG_RESPONSE_ADDR;
            smu->arg_base = PSMU_C2PMSG_ARG_BASE;
            break;
        default:
            DBG("Failed to get SMU, unknown SMU_TYPE: %s\n", smu_type);
            goto err;
            break;
    }
    /* Try to send a test message*/
    rep = smu_service_req(smu, SMU_TEST_MSG, &arg);
    if(rep == REP_MSG_OK){
        return smu;
    } else {
        DBG("Faild to get SMU: %s, test message REP: %x\n", smu_type, rep);
        goto err;
    }
err:
    free_smu(smu);
    return NULL;
}

void free_smu(smu_t smu) {
    free((void *)smu);
}