#include "tuya_rf.h"
#include "radio.h"

#include "esphome/core/log.h"

namespace esphome {
namespace tuya_rf {

static const char *TAG = "tuya_rf";


void TuyaRfComponent::setup()
{
  Radio_Init();

  if (!this->receiver_disabled_)
    StartRx();
}



void TuyaRfComponent::loop()
{

  if (this->receiver_disabled_)
    return;


  uint16_t pulses[512];

  int len = radio_receive(pulses, 512);


  //
  // Không đủ dữ liệu thì bỏ
  //

  if (len < 60)
    return;


  //
  // tìm sync gap (~10ms)
  //

  int start = -1;

  for (int i = 0; i < len; i++)
  {
    if (pulses[i] > 9000)
    {
      start = i;
      break;
    }
  }


  if (start < 0)
    return;


  //
  // frame RF chuẩn
  //

  int frame_len = 140;

  if (start + frame_len > len)
      frame_len = len - start;



  //
  // Dump RAW ổn định
  //

  ESP_LOGI("remote.raw","Received Raw:");

  for (int i = 0; i < frame_len; i++)
  {
      ESP_LOGI("remote.raw","%d,", pulses[start+i]);
  }


}



void TuyaRfComponent::set_receiver(bool enabled)
{

  this->receiver_disabled_ = !enabled;

  if(enabled)
      StartRx();

}



void TuyaRfComponent::send_internal(uint32_t code,uint32_t times)
{

  StartTx();

  delay(20);

  StartRx();

}



}
}
