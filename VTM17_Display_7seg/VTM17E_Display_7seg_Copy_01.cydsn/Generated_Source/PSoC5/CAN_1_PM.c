/*******************************************************************************
* File Name: CAN_1_PM.c
* Version 3.0
*
* Description:
*  This file contains the setup, control and status commands to support
*  component operations in the low power mode.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "CAN_1.h"

static CAN_1_BACKUP_STRUCT CAN_1_backup =
{
    0u,
    #if (CY_PSOC3 || CY_PSOC5)
        0u,
        CAN_1_INIT_INTERRUPT_MASK,
        CAN_1_MODE_MASK,
        (((uint32) ((uint32) CAN_1_SYNC_EDGE    << CAN_1_EDGE_MODE_SHIFT))     |
        ((uint32) ((uint32) CAN_1_SAMPLING_MODE << CAN_1_SAMPLE_MODE_SHIFT))   |
        ((uint32) ((uint32) CAN_1_CFG_REG_SJW   << CAN_1_CFG_REG_SJW_SHIFT))   |
        ((uint32) ((uint32) CAN_1_RESET_TYPE    << CAN_1_RESET_SHIFT))         |
        ((uint32) ((uint32) CAN_1_CFG_REG_TSEG2 << CAN_1_CFG_REG_TSEG2_SHIFT)) |
        ((uint32) ((uint32) CAN_1_CFG_REG_TSEG1 << CAN_1_CFG_REG_TSEG1_SHIFT)) |
        ((uint32) ((uint32) CAN_1_ARBITER       << CAN_1_ARBITER_SHIFT))       |
        ((uint32) ((uint32) CAN_1_BITRATE       << CAN_1_BITRATE_SHIFT)))
    #endif /* CY_PSOC3 || CY_PSOC5 */
};


/*******************************************************************************
* Function Name: CAN_1_SaveConfig
********************************************************************************
*
* Summary:
*  Save the CAN configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_1_backup - Modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_1_SaveConfig(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        CAN_1_backup.intSr = (CY_GET_REG32(CAN_1_INT_SR_PTR));
        CAN_1_backup.intEn = (CY_GET_REG32(CAN_1_INT_EN_PTR));
        CAN_1_backup.cmd = (CY_GET_REG32(CAN_1_CMD_PTR));
        CAN_1_backup.cfg = (CY_GET_REG32(CAN_1_CFG_PTR));
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_1_RestoreConfig
********************************************************************************
*
* Summary:
*  Restore the CAN configuration.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_1_backup - Used when non-retention registers are restored.
*
* Side Effects:
*  If this API is called without first calling SaveConfig then default values
*  from Customizer will be in the following registers: CAN_1_INT_SR,
*  CAN_1_INT_EN, CAN_1_CMD, CAN_1_CFG.
*
*******************************************************************************/
void CAN_1_RestoreConfig(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        CY_SET_REG32(CAN_1_INT_SR_PTR, CAN_1_backup.intSr);
        CY_SET_REG32(CAN_1_INT_EN_PTR, CAN_1_backup.intEn);
        CY_SET_REG32(CAN_1_CMD_PTR, CAN_1_backup.cmd);
        CY_SET_REG32(CAN_1_CFG_PTR, CAN_1_backup.cfg);
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_1_Sleep
********************************************************************************
*
* Summary:
*  Prepares CAN Component to go to sleep.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_1_backup - Modified when non-retention registers are saved.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_1_Sleep(void) 
{
    #if (!(CY_PSOC3 || CY_PSOC5))
        uint8 i;
    #endif /* (!(CY_PSOC3 || CY_PSOC5)) */

    if (0u != (CY_GET_REG32(CAN_1_CMD_PTR) & CAN_1_MODE_MASK))
    {
        CAN_1_backup.enableState = 1u;
    }
    else /* CAN block is disabled */
    {
        CAN_1_backup.enableState = 0u;
    }

    #if (CY_PSOC3 || CY_PSOC5)
        CAN_1_SaveConfig();
        (void) CAN_1_Stop();
    #else /* CY_PSOC4 */
        /* Abort respective pending TX message requests */
        for (i = 0u; i < CAN_1_NUMBER_OF_TX_MAILBOXES; i++)
        {
            CAN_1_TX_ABORT_MESSAGE(i);
        }

        /* Sets CAN controller to Stop mode */
        (void) CAN_1_Stop();

        /* Clear Global Interrupt enable Flag */
        (void) CAN_1_GlobalIntDisable();
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/*******************************************************************************
* Function Name: CAN_1_Wakeup
********************************************************************************
*
* Summary:
*  Prepares CAN Component to wake up.
*
* Parameters:
*  None.
*
* Return:
*  None.
*
* Global Variables:
*  CAN_1_backup - Used when non-retention registers are restored.
*
* Reentrant:
*  No.
*
*******************************************************************************/
void CAN_1_Wakeup(void) 
{
    #if (CY_PSOC3 || CY_PSOC5)
        /* Enable CAN block in Active mode */
        CAN_1_PM_ACT_CFG_REG |= CAN_1_ACT_PWR_EN;
        /* Enable CAN block in Alternate Active (Standby) mode */
        CAN_1_PM_STBY_CFG_REG |= CAN_1_STBY_PWR_EN;

        CAN_1_RestoreConfig();

        /* Reconfigure Rx and Tx Buffers control registers */
        (void) CAN_1_RxTxBuffersConfig();

        if (CAN_1_backup.enableState != 0u)
        {
            /* Enable component's operation */
            (void) CAN_1_Enable();
        } /* Do nothing if component's block was disabled before */
    #else /* CY_PSOC4 */
        /* Clear all INT_STATUS bits */
        CAN_1_INT_SR_REG = CAN_1_INT_STATUS_MASK;

        /* Set Global Interrupt enable Flag */
        (void) CAN_1_GlobalIntEnable();

        if (CAN_1_backup.enableState != 0u)
        {
            (void) CAN_1_Enable();
        } /* Do nothing if component's block was disabled before */
    #endif /* CY_PSOC3 || CY_PSOC5 */
}


/* [] END OF FILE */
