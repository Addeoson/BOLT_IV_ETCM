/*
 * etcm_ecap.c
 *
 * Basic frequency counting, no averaging or filtering!
 *
 *  Created on: Feb 11, 2020
 *      Author: Tyler
 */

#include "ecap_etcm.h"

#define GPIO_NUM_WHL_SPD_FRNT   2      // check input x-bar if change
#define GPIO_WHL_SPD_FRNT       GPIO_2_GPIO2

/*
 * Globals
 */
volatile uint32_t cap1Count;
volatile uint32_t cap2Count;
volatile uint32_t cap3Count;
volatile uint32_t cap4Count;

volatile float signalPeriod;
volatile float signalFreq;

//
// initECAP - Configure eCAP
//
void initECAP()
{
    //
    // Configure eCAP input
    //
    XBAR_setInputPin(XBAR_INPUT7, GPIO_NUM_WHL_SPD_FRNT);
    GPIO_setPinConfig(GPIO_WHL_SPD_FRNT);
    GPIO_setDirectionMode(GPIO_NUM_WHL_SPD_FRNT, GPIO_DIR_MODE_IN);
    GPIO_setQualificationMode(GPIO_NUM_WHL_SPD_FRNT, GPIO_QUAL_ASYNC);

    //
    // Disable ,clear all capture flags and interrupts
    //
    ECAP_disableInterrupt(ECAP1_BASE,
                          (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                           ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                           ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                           ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                           ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                           ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                           ECAP_ISR_SOURCE_COUNTER_COMPARE));
    ECAP_clearInterrupt(ECAP1_BASE,
                        (ECAP_ISR_SOURCE_CAPTURE_EVENT_1  |
                         ECAP_ISR_SOURCE_CAPTURE_EVENT_2  |
                         ECAP_ISR_SOURCE_CAPTURE_EVENT_3  |
                         ECAP_ISR_SOURCE_CAPTURE_EVENT_4  |
                         ECAP_ISR_SOURCE_COUNTER_OVERFLOW |
                         ECAP_ISR_SOURCE_COUNTER_PERIOD   |
                         ECAP_ISR_SOURCE_COUNTER_COMPARE));

    //
    // Disable CAP1-CAP4 register loads
    //
    ECAP_disableTimeStampCapture(ECAP1_BASE);

    //
    // Configure eCAP
    //    Enable capture mode.
    //    One shot mode, stop capture at event 4.
    //    Set polarity of the events to rising, falling, rising, falling edge.
    //    Set capture in time difference mode.
    //    Select input from XBAR7.
    //    Enable eCAP module.
    //    Enable interrupt.
    //
    ECAP_stopCounter(ECAP1_BASE);
    ECAP_enableCaptureMode(ECAP1_BASE);

    ECAP_setCaptureMode(ECAP1_BASE, ECAP_ONE_SHOT_CAPTURE_MODE, ECAP_EVENT_2);



    ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_1, ECAP_EVNT_FALLING_EDGE);
    ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_2, ECAP_EVNT_FALLING_EDGE);
    ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_3, ECAP_EVNT_FALLING_EDGE);
    ECAP_setEventPolarity(ECAP1_BASE, ECAP_EVENT_4, ECAP_EVNT_FALLING_EDGE);

    ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_1);
    ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_2);
    ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_3);
    ECAP_enableCounterResetOnEvent(ECAP1_BASE, ECAP_EVENT_4);

    // Set eCap input pin to GPIO16 (Lpad pin 33)
    XBAR_setInputPin(XBAR_INPUT7, GPIO_NUM_WHL_SPD_FRNT);

    ECAP_enableLoadCounter(ECAP1_BASE);
    ECAP_setSyncOutMode(ECAP1_BASE, ECAP_SYNC_OUT_DISABLED);
    ECAP_startCounter(ECAP1_BASE);
    ECAP_enableTimeStampCapture(ECAP1_BASE);
    ECAP_setEventPrescaler(ECAP1_BASE, 0);
    ECAP_reArm(ECAP1_BASE);

    ECAP_enableInterrupt(ECAP1_BASE, ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
}



//
// eCAP 1 ISR
//
__interrupt void ecap1ISR(void)
{
    //
    // Get the capture counts.
    //
    cap1Count = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_1);
    cap2Count = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_2);
    //cap3Count = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_3);
    //cap4Count = ECAP_getEventTimeStamp(ECAP1_BASE, ECAP_EVENT_4);

    // Calculate signal period and frequency
    signalPeriod = cap2Count / (float)DEVICE_SYSCLK_FREQ;
    signalFreq = 1.0 / signalPeriod;


    //ecap1IntCount++;

    //
    // Count correct captures
    //
    //ecap1PassCount++;

    //
    // Clear interrupt flags for more interrupts.
    //
    ECAP_clearInterrupt(ECAP1_BASE,ECAP_ISR_SOURCE_CAPTURE_EVENT_2);
    ECAP_clearGlobalInterrupt(ECAP1_BASE);

    //
    // Start eCAP
    //
    ECAP_reArm(ECAP1_BASE);

    //
    // Acknowledge the group interrupt for more interrupts.
    //
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP4);
}
