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

  if(this->receiver_disabled_)
    return;

  this->handleReceived();

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

  RestartRx();

}




//
// ===== PRODUCTION PERFECT SNIFF =====
//


void TuyaRfComponent::handleReceived()
{

  uint16_t pulses[512];

  int count = radio_receive(pulses,512);


  //
  // Không đủ dữ liệu
  //

  if(count < 40)
    return;



  //
  // Tìm sync gap
  //

  int sync_index = -1;

  for(int i=0;i<count;i++)
  {
      if(pulses[i] > 9000)
      {
          sync_index = i;
          break;
      }
  }


  if(sync_index < 0)
      return;



  //
  // Frame length tối ưu EV1527/PT2262
  //

  int frame_len = 160;

  if(sync_index + frame_len > count)
      frame_len = count - sync_index;



  //
  // Dump RAW
  //

  ESP_LOGI("remote.raw","Received Raw:");

  for(int i=0;i<frame_len;i++)
  {
      ESP_LOGI("remote.raw","%d,",pulses[sync_index+i]);
  }



  //
  // Reset RX nhanh
  //

  RestartRx();

}



}
}
