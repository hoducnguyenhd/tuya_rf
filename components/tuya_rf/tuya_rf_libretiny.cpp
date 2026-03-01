#include "tuya_rf.h"
#include "radio.h"
#include "esphome/core/log.h"

namespace esphome {
namespace tuya_rf {

static const char *TAG = "tuya_rf";


void TuyaRfComponent::setup() {

  radio_init();

  if (!this->receiver_disabled_)
    StartRx();

}



void TuyaRfComponent::loop() {

  if (this->receiver_disabled_)
    return;


  uint16_t raw[512];

  int len = GetRfRawData(raw);


  if(len < 50)
      return;



  //
  // tìm sync ~10ms
  //

  int start = -1;

  for(int i=0;i<len;i++)
  {
      if(raw[i] > 9000)
      {
          start = i;
          break;
      }
  }


  if(start < 0)
      return;



  //
  // frame chuẩn
  //

  int frame = 140;

  if(start+frame > len)
      frame = len-start;



  ESP_LOGI("remote.raw","Received Raw:");

  for(int i=0;i<frame;i++)
  {
      ESP_LOGI("remote.raw","%d,",raw[start+i]);
  }


}



void TuyaRfComponent::set_receiver(bool enabled) {

  this->receiver_disabled_ = !enabled;

  if(enabled)
    StartRx();

}



void TuyaRfComponent::send_internal(uint32_t code, uint32_t times) {

  StartTx();

  delay(20);

  StartRx();

}



}
}
