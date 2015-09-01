// ----------------------------------------------------------------------------
// Copyright (C)  qbrobotics. All rights reserved.
// www.qbrobotics.com
// ----------------------------------------------------------------------------


/**
* \file         main.c
*
* \brief        Firmware main file.
* \date         May 16, 2012
* \author       qbrobotics
* \copyright    (C)  qbrobotics. All rights reserved.
*/

/**
* \mainpage     Firmware
* \brief        This is the firmware of the qb move.
* \version      0.1 beta 4
*
* \author       _qbrobotics_
*
* \date         May 16, 2012
*
* \details      This is the firmware of the qb move.
*
* \copyright    (C)  qbrobotics. All rights reserved.
*
*/


// ----------------------------------------------------------------------------
// This version changes:
//      - not reported


//=================================================================     includes

#include <device.h>
#include <globals.h> // ALL GLOBAL DEFINITIONS, STRUCTURES AND MACROS HERE
#include <interruptions.h>
#include <command_processing.h>
#include <utils.h>

//==============================================================================
//                                                                 MAIN FUNCTION
//==============================================================================

int i;              //iterator


void main()
{
//====================================     initializations - psoc and components

    // EEPROM

    EEPROM_Start();
    memRecall();                                        // recall configuration

    // RS485

    CyDelay(100);
    FTDI_ENABLE_REG_Write(0x01);

    UART_RS485_Stop();                                  // stop UART
    UART_RS485_Start();                                 // start UART
    UART_RS485_Init();

    UART_RS485_ClearRxBuffer();
    UART_RS485_ClearTxBuffer();

    ISR_RS485_RX_StartEx(ISR_RS485_RX_ExInterrupt);     // RS485 isr function

    // PWM

    PWM_MOTORS_Start();
    PWM_MOTORS_WriteCompare1(0);
    PWM_MOTORS_WriteCompare2(0);
    MOTOR_DIR_Write(0);
    MOTOR_ON_OFF_Write(0x00);

    // SSI encoder initializations

    COUNTER_ENC_1_Start();

    SHIFTREG_ENC_1_Start();
    SHIFTREG_ENC_2_Start();
    SHIFTREG_ENC_3_Start();

    // ADC

    ADC_Start();                            // start ADC
    ADC_StartConvert();

    // Temporization

    MY_TIMER_Start();
    PACER_TIMER_Start();

    RS485_CTS_Write(0);

    CYGlobalIntEnable;                      // enable interrupts

//========================================     initializations - clean variables

    // Activate encoder counters
    RESET_COUNTERS_Write(0x00);
    CyDelay(10);

    for (i = 0; i < NUM_OF_MOTORS; i++) {
        g_ref.pos[i] = 0;
    }

    if (c_mem.emg_calibration_flag) {
        if ((c_mem.input_mode == INPUT_MODE_EMG_PROPORTIONAL) ||
            (c_mem.input_mode == INPUT_MODE_EMG_INTEGRAL) ||
            (c_mem.input_mode == INPUT_MODE_EMG_FCFS) ||
            (c_mem.input_mode == INPUT_MODE_EMG_FCFS_ADV)) {
            g_ref.onoff = 0x00;
        } else {
            g_ref.onoff = c_mem.activ;
        }
    } else {
        g_ref.onoff = c_mem.activ;
    }

    for (i = 0; i < NUM_OF_SENSORS; i++) {
        g_meas.pos[i] = 0;
        g_meas.rot[i] = 0;
    }

    g_meas.emg[0] = 0;
    g_meas.emg[1] = 0;

    g_rx.length   = 0;

    // Calculate conversion factor
    device.tension_conv_factor = ((0.25 * 101.0 * 1000) / 1638.4); //derives from datasheet calculations
    device.pwm_limit = PWM_MAX_VALUE;
    device.tension_valid = FALSE;

    reset_last_value_flag = 0;


//=========================================================     application loop

    for(;;)
    {
        // Put the FF reset pin to LOW
        RESET_FF_Write(0x00);

        // Call function scheduler
        function_scheduler();

        //  Wait until the FF is set to 1
        while(FF_STATUS_Read() == 0);

        // Command a FF reset
        RESET_FF_Write(0x01);

        // Wait for FF to be reset
        while(FF_STATUS_Read() == 1);

        if(UART_RS485_ReadRxStatus() & UART_RS485_RX_STS_SOFT_BUFF_OVER)
            UART_RS485_ClearRxBuffer();
    }
}



/* [] END OF FILE */