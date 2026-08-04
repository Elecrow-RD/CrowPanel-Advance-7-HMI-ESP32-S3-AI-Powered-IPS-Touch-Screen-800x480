#ifndef _FIRFILTER_H_
#define _FIRFILTER_H_

/*————————————————————————————————————————Header file declaration————————————————————————————————————————*/
#include <stdint.h>
/*——————————————————————————————————————Header file declaration end——————————————————————————————————————*/
/*——————————————————————————————————————————Variable declaration—————————————————————————————————————————*/
#define FIR_TAG "FIRFILTER"
#define FIR_INFO(fmt, ...)   ESP_LOGI(FIR_TAG, fmt, ##__VA_ARGS__)
#define FIR_DEBUG(fmt, ...)  ESP_LOGD(FIR_TAG, fmt, ##__VA_ARGS__)
#define FIR_ERROR(fmt, ...)  ESP_LOGE(FIR_TAG, fmt, ##__VA_ARGS__)

#define FIR_FILTER_LENGTH 10
typedef struct {
    float buf[FIR_FILTER_LENGTH];
    uint8_t bufIndex;
    float out;
}FIRFilter;

void FIRFilter_Init(FIRFilter *fir);
float FIRFilter_Update(FIRFilter *fir, float inp);
/*———————————————————————————————————————Variable declaration end——————————————-—————————————————————————*/
#endif