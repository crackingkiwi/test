/*

receive and Identify NEC IR remote signal

*/

//#include <ESP8266WiFi.h>
//#include <WiFiClient.h>
//#include <ESP8266WiFiMulti.h>
//#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>
#include <osapi.h>
#include <os_type.h>

#define READY_TO_START          1
#define COMPLETE_CODE_OBTAINED  2
#define CODE_IS_COMING_IN       3

#define RECEIVER_PIN            D5
#define TIME_OUT_TIME           25//milliseconds
#define REPEAT_FLAG_NOREPEAT    0
#define TOLERANCE               8 //12.5%
#define MAX_CODE_LENGTH         1024
#define MAX_BIT_DATA            512
#define MAX_HEX_DATA            256

static uint8_t flag;
static uint32_t current_time;
static os_timer_t timer0;
static uint16_t code[MAX_CODE_LENGTH];
static uint16_t current_length;

const uint16_t NEC_HEAD[] = {9000, 4500};
const uint16_t NEC_HEAD_LENGTH=2;
const uint16_t NEC_FOOT[] = {560};
const uint16_t NEC_FOOT_LENGTH=1;
const uint16_t NEC_ZERO[] = {560, 560};
const uint16_t NEC_ZERO_LENGTH=2;
const uint16_t NEC_ONE[] = {560,1680};
const uint16_t NEC_ONE_LENGTH=2; 

void IRAM_ATTR gpio_ISR()
{
  uint32_t last_time;
  //os_intr_lock();
  os_timer_disarm(&timer0);
  os_timer_arm(&timer0,TIME_OUT_TIME, REPEAT_FLAG_NOREPEAT);
  switch(flag)
  {
    case COMPLETE_CODE_OBTAINED: break;//do nothing
    case READY_TO_START:
      current_time=micros();
      current_length = 0;
      flag=CODE_IS_COMING_IN;
      break;
    case CODE_IS_COMING_IN:
      last_time=current_time;
      current_time=micros();
      code[current_length]=current_time - last_time;
      current_length++;
      break;
    default:break;
  }
  //os_intr_unlock();
}//gpio_interrupt

void IRAM_ATTR timer0_timeout_function() 
//what to do after long time without signal
{
  //Serial.println("Time out!");
  os_intr_lock();
  os_timer_disarm(&timer0);
  if(flag==COMPLETE_CODE_OBTAINED)
  {
    Serial.println("Error: Code is complete when timeout.");
  }
  flag=COMPLETE_CODE_OBTAINED; //flag 1 or 3 set as 2
  os_intr_unlock();
}

uint8_t can_match(uint16_t received, uint16_t standard)
{
  //Serial.print("compare:");Serial.print(received);
  //Serial.print(" and ");Serial.println(standard);
  if(received > standard + standard/TOLERANCE 
  || received < standard - standard/TOLERANCE)
    return 0;
  return 1;
}

uint8_t check_header_footer(const uint16_t *head, uint8_t head_length, const uint16_t *foot, uint8_t foot_length)
{
  uint8_t i;
  
  for(i=0;i<head_length;i++)
  {
    //Serial.print("ihead=");Serial.println(i);
    if(!can_match(code[i],head[i])) return 0;
  }
  for(i=0; i<foot_length; i++)
  {
    //Serial.print("ifoot=");Serial.println(i);
    //Serial.print("current_length-foot_length+i=");
    //Serial.print(code[current_length-foot_length+i]);
    if(!can_match(code[current_length-foot_length+i],foot[i])) return 0;
  }
  return 1;
}

uint8_t is_logic_zero(uint16_t current)
{
  for(uint16_t i=0; i<NEC_ZERO_LENGTH; i++)
  {
    if(!can_match(code[current+i],NEC_ZERO[i])) return 0;
  }
  return 1;
}

uint8_t is_logic_one(uint16_t current)
{
  for(uint16_t i=0; i<NEC_ONE_LENGTH; i++)
  {
    if(!can_match(code[current+i],NEC_ONE[i])) return 0;
  }
  return 1;
}

uint8_t process_code()
{
  //check header and footer
  uint8_t head_length, foot_length;
  head_length=NEC_HEAD_LENGTH;
  foot_length=NEC_FOOT_LENGTH;
  //Serial.print("head_length=");Serial.println(head_length);
  //Serial.print("foot_length=");Serial.println(foot_length);
  if(check_header_footer(NEC_HEAD, head_length, NEC_FOOT, foot_length))
  {
    Serial.println("NEC Protocol");//check passed
  }
  else
  {
    //do nothing
    return 0;
  }
  //converter to bit and check code integrity
  uint16_t current = head_length;
  uint16_t i=0;
  uint8_t bit_data[MAX_BIT_DATA];
  char hex_data[MAX_HEX_DATA];
  uint16_t bit_data_length=0;
  uint16_t hex_data_length=0;
  uint16_t a_hex_number;
  while(current < current_length-foot_length)
  {
    if(is_logic_zero(current)) 
    {
      bit_data[i]=0;
    }
    else if(is_logic_one(current)) 
    {
      bit_data[i]=1;
    }
    else {Serial.println("Code corrupt: neither 1 nor 0"); return 0;}
    current = current + 2;
    i++;
  }
  if(current!=current_length-foot_length)
  {
    Serial.println("Code corrupt: can't convert to bits");
    return 0;
  }
  Serial.print("bit_data["); Serial.print(i); Serial.print("]=");
  for(uint16_t j=0; j<i; j++)
  {
    Serial.print(bit_data[j]);
  }
  Serial.println("");
  if(i%8!=0) 
  {
    Serial.println("Code corrupt: not in Bytes");
  }
  bit_data_length=i;
  //convert to hex or bytes
  i = 0; 
  hex_data_length=0;
  while(i<bit_data_length)
  {
    a_hex_number =
    (bit_data[i]<<3) + (bit_data[i+1]<<2) +
    (bit_data[i+2]<<1) + bit_data[i+3];
    if(a_hex_number<10)
      hex_data[hex_data_length] = a_hex_number + 48;
    else
      hex_data[hex_data_length] = a_hex_number + 55;
    i=i+4; hex_data_length++;
  }
  Serial.print("hex_data["); Serial.print(hex_data_length); Serial.print("]=");
  for(i=0; i<hex_data_length; i++)
  {
    Serial.print(hex_data[i]);
  }
  Serial.println("");
  return 1; 
}

void setup()
{
  Serial.begin(115200);
  pinMode(RECEIVER_PIN, 0);//set receiver pin as input
  flag = READY_TO_START;
  current_time = micros();
  current_length=0;
  os_timer_setfn(&timer0, reinterpret_cast<os_timer_func_t *>(timer0_timeout_function), NULL);
  attachInterrupt(RECEIVER_PIN, gpio_ISR, CHANGE);
}
void loop() 
{
  if(flag!=COMPLETE_CODE_OBTAINED);//do nothing
  if(flag==COMPLETE_CODE_OBTAINED)
  {
    Serial.print("\n");
    if(process_code())
    {
      Serial.print("code[");Serial.print(current_length);Serial.print("]=");
      for(uint16_t i=0; i<current_length; i++)
      {
        Serial.print(code[i]);Serial.print(" ");
      }
      Serial.println("");//Serial.println("Data ready");
    }
    flag=READY_TO_START;
  }
}
