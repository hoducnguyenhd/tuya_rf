#include "tuya_rf.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "radio.h"

#ifdef USE_LIBRETINY

namespace esphome {
namespace tuya_rf {

static const char *const TAG = "tuya_rf";


// ================= ISR =================

void IRAM_ATTR HOT RemoteReceiverComponentStore::gpio_intr(RemoteReceiverComponentStore *arg)
{
  const uint32_t now = micros();

  const uint32_t next =
      (arg->buffer_write_at + 1) % arg->buffer_size;

  const bool level =
      !arg->pin.digital_read();

  if(level!=next%2)
      return;

  if(next==arg->buffer_read_at)
      return;

  const uint32_t last_change =
      arg->buffer[arg->buffer_write_at];

  const uint32_t time_since_change =
      now-last_change;

  if(time_since_change<=arg->filter_us)
      return;

  arg->buffer[arg->buffer_write_at=next]=now;
}



// ================= RECEIVER CONTROL =================

void TuyaRfComponent::turn_on_receiver()
{
 if(this->receiver_disabled_){
   this->receiver_disabled_=false;
   this->set_receiver(true);
 }
}



void TuyaRfComponent::turn_off_receiver()
{
 if(!this->receiver_disabled_){
   this->receiver_disabled_=true;
   this->set_receiver(false);
 }
}



void TuyaRfComponent::set_receiver(bool on)
{

 auto &s=this->store_;

 if(on){

  if(s.buffer==NULL){

   s.buffer=new uint32_t[s.buffer_size];

   memset((void*)s.buffer,0,
          s.buffer_size*sizeof(uint32_t));

  }

  if(!this->RemoteReceiverBase::pin_->digital_read())
       s.buffer_write_at=s.buffer_read_at=1;
  else
       s.buffer_write_at=s.buffer_read_at=0;


  this->RemoteReceiverBase::pin_->attach_interrupt(
      RemoteReceiverComponentStore::gpio_intr,
      &this->store_,
      gpio::INTERRUPT_ANY_EDGE);


  this->high_freq_.start();

  if(!this->transmitting_)
       StartRx();

 }
 else{

  if(!this->transmitting_)
       CMT2300A_GoStby();

  this->RemoteReceiverBase::pin_->detach_interrupt();

  this->high_freq_.stop();

 }

}



// ================= SETUP =================

void TuyaRfComponent::setup()
{

 this->RemoteTransmitterBase::pin_->setup();

 this->RemoteTransmitterBase::pin_->digital_write(false);

 this->RemoteReceiverBase::pin_->setup();

 auto &s=this->store_;

 s.filter_us=this->filter_us_;

 s.pin=this->RemoteReceiverBase::pin_->to_isr();

 s.buffer_size=this->buffer_size_;

 if(s.buffer_size%2!=0)
     s.buffer_size++;

 this->set_receiver(!this->receiver_disabled_);

}



// ================= CONFIG =================

void TuyaRfComponent::dump_config()
{
 ESP_LOGCONFIG(TAG,"Tuya RF");
}



// ================= TIMING =================

void TuyaRfComponent::await_target_time_()
{
 const uint32_t now=micros();

 if(this->target_time_==0)
    this->target_time_=now;
 else
    while(this->target_time_>micros());
}



void TuyaRfComponent::mark_(uint32_t usec)
{

 this->await_target_time_();

 this->RemoteTransmitterBase::pin_->digital_write(false);

 this->target_time_+=usec;

}



void TuyaRfComponent::space_(uint32_t usec)
{

 this->await_target_time_();

 this->RemoteTransmitterBase::pin_->digital_write(true);

 this->target_time_+=usec;

}



// ================= TX =================

void IRAM_ATTR TuyaRfComponent::send_internal(
    uint32_t send_times,
    uint32_t send_wait)
{

 InterruptLock lock;

 this->transmitting_=true;

 int res=StartTx();

 if(res!=0){
   this->transmitting_=false;
   return;
 }

 this->target_time_=0;

 this->space_(2500);


 for(uint32_t i=0;i<send_times;i++){

  for(int32_t item :
      this->RemoteTransmitterBase::temp_.get_data()){

   if(item>0)
       this->mark_(item);
   else
       this->space_(-item);

   App.feed_wdt();
  }

  if(i+1<send_times && send_wait>0)
      this->space_(send_wait);

 }


 this->space_(2000);


 this->transmitting_=false;


 if(this->receiver_disabled_)
      CMT2300A_GoStby();
 else
      StartRx();

}



// ================= RX LOOP =================

void TuyaRfComponent::loop()
{

 if(this->receiver_disabled_)
      return;

 auto &s=this->store_;

 const uint32_t write_at=s.buffer_write_at;

 const uint32_t dist=
 (s.buffer_size+write_at-s.buffer_read_at)
 %s.buffer_size;

 if(dist<=1)
      return;


 bool receive_end=false;

 uint32_t new_write_at=old_write_at_;


 while(new_write_at!=write_at){

  uint32_t prev=
  new_write_at==0?
  s.buffer_size-1:
  new_write_at-1;

  uint32_t diff=
  s.buffer[new_write_at]
  -s.buffer[prev];


  if(new_write_at%2==0){

   if(diff>=start_pulse_min_us_){

    if(diff>=end_pulse_us_){

     if(receive_started_){

       receive_end=true;
       new_write_at=prev;
       break;

     }

    }
    else if(diff<start_pulse_max_us_){

     s.buffer_read_at=prev;
     receive_started_=true;

    }

   }

  }


  if(!receive_started_)
      s.buffer_read_at=prev;

  new_write_at=
  (new_write_at+1)%s.buffer_size;

 }


 old_write_at_=new_write_at;


 if(!receive_end)
      return;



 receive_started_=false;



 uint32_t prev=s.buffer_read_at;

 s.buffer_read_at=
 (s.buffer_read_at+1)%s.buffer_size;



 this->RemoteReceiverBase::temp_.clear();



 int32_t multiplier=
 s.buffer_read_at%2==0?1:-1;



 while(prev!=new_write_at){

  int32_t delta=
  s.buffer[s.buffer_read_at]
  -s.buffer[prev];


  this->RemoteReceiverBase::temp_.push_back(
      multiplier*delta);


  prev=s.buffer_read_at;

  s.buffer_read_at=
  (s.buffer_read_at+1)%s.buffer_size;

  multiplier*=-1;

 }



//
// ===== SYNC FILTER =====
//

int sync=-1;

for(size_t i=0;
i<this->RemoteReceiverBase::temp_.size();
i++){

 if(abs(this->RemoteReceiverBase::temp_[i])>9000){

   sync=i;
   break;

 }

}


if(sync>=0){

 std::vector<int32_t> clean;

 for(int i=0;i<140;i++){

  int idx=sync+i;

  if(idx>=this->RemoteReceiverBase::temp_.size())
       break;

  clean.push_back(
      this->RemoteReceiverBase::temp_[idx]);

 }

 this->RemoteReceiverBase::temp_=clean;

}


 this->call_listeners_dumpers_();

}



}
}

#endif
