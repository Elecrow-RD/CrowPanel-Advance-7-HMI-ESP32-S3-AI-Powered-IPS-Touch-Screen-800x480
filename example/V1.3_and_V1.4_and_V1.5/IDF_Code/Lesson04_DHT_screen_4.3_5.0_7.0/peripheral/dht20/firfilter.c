/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include "firfilter.h"
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/

/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/
// Moving average = 10 TAPS
static float filter_taps[FIR_FILTER_LENGTH] = {0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f};
// Moving average = 16 TAPS
//static float filter_taps[FIR_FILTER_LENGTH] = {0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f, 0.0625f};
/*————————————————————————————————————————Variable declaration end———————————————————————————————————————*/

/*—————————————————————————————————————————Functional function———————————————————————————————————————————*/
void FIRFilter_Init(FIRFilter *fir)
{
    /* Clear filter buffer */
    for(uint8_t n = 0; n < FIR_FILTER_LENGTH; n++) 
    {
        fir->buf[n] = 0.0f;
    }
    /* Reset buffer index */
    fir->bufIndex = 0;
    /* Clear filter output */
    fir->out = 0.0f;
}

float FIRFilter_Update(FIRFilter *fir, float inp)
{
    /* Store latest sample in buffer */
    fir->buf[fir->bufIndex] = inp;
    /* Increment buffer index and wrap around if necessary */
    fir->bufIndex++;
    if(fir->bufIndex == FIR_FILTER_LENGTH) 
        fir->bufIndex = 0;
    /* Compute new output sample (via convolution) */
    fir->out = 0.0f;
    uint8_t sumIndex = fir->bufIndex;
    for(uint8_t n = 0; n < FIR_FILTER_LENGTH; n++) 
    {
        /* Decrement index and wrap if necessary */
        if(sumIndex > 0) 
            sumIndex--;
        else 
            sumIndex = FIR_FILTER_LENGTH - 1;

        /* Multiply impulse response with shifted input sample and add to output */
        fir->out += filter_taps[n] * fir->buf[sumIndex];
    }
    /* Return filtered output */
    return fir->out;
}
/*———————————————————————————————————————Functional function end—————————————————————————————————————————*/