/*
 * ESP32AMSEncoder.ino
 * 
 * Main interface for controlling AS5x47 
 * 
 * Created by: 
 *       K.C.Y
 * Date:
 *       2017/06/30
 */

#include "Global.h"
#include "Fifo.h"

#define ABI_FIFO_ID       0
#define PCM_FIFO_ID       1
#define PCM_BUFFER_SIZE   1024

#define PUB_UNIT_COUNT      5

static hw_timer_t* s_HwTimer = NULL;
static boolean s_MicRecording = false;

char s_PCMBuffer[PCM_BUFFER_SIZE] = { 0 };
char s_ABIBuffer[256] = { 0 };

void setup()
{
  Serial.begin(115200);
  
  /* Setup WIFI & Bluemix */
  //setup_wifi();
  //setup_bluemix();
#if 1
  /* Initialize FIFO */
  fifo_init(ABI_FIFO_ID, 4608);
  fifo_init(PCM_FIFO_ID, 8192);
  
  /* Initialize Mic */
  pdm_mic_init(MIC_CLK_IO_NUM, MIC_DATA_IO_NUM, 48000);
  
  /* Initialize AS5147 */
  as5x47_init(AS5X47_A_IO_NUM, AS5X47_B_IO_NUM, AS5X47_I_IO_NUM, AS5X47_SPI_CS_IO_NUM);
  as5x47_set_abi_cb(as5147_abi_signal_handler);
  
  //as5x47_set_abi_pulses(true); // 512 Pulses
  //as5x47_set_direction(true);  // CW
  //as5x47_write_config();       // Apply Configuration
  //as5x47_read_config();
  
  /* Initialize Hardware Timer */
  s_HwTimer = timerBegin(0, 1, true);
  if (s_HwTimer)
    timerStart(s_HwTimer);
#endif
}

void loop()
{
  static int last_time = -1;
  if (last_time == -1 || millis() - last_time > 10)
  {
    bluemix_try_connect();
    //Serial.printf("ANGLE: %x\n", as5x47_read_angle());
    int pcm_count = fifo_capacity(PCM_FIFO_ID) / sizeof(short);
    int count = fifo_capacity(ABI_FIFO_ID) / (sizeof(uint64_t) + sizeof(char));
    if (count > pcm_count)
      count = pcm_count;
    if (count > PUB_UNIT_COUNT)
      count = PUB_UNIT_COUNT;
    if (count > 0)
    {
      fifo_read(PCM_FIFO_ID, (unsigned char*)s_PCMBuffer, count * sizeof(short));
      fifo_read(ABI_FIFO_ID, (unsigned char*)s_ABIBuffer, count * (sizeof(uint64_t) + sizeof(char)));
      bluemix_publish_data((char*)s_ABIBuffer, (char*)s_PCMBuffer, count);
    }
    last_time = millis();
  }

  /* Read & Push Mic Recording Data */
  if (s_MicRecording)
  {
    int pcm_buffer_size = pdm_mic_read_data(s_PCMBuffer, PCM_BUFFER_SIZE, 50);
    int space = fifo_space(PCM_FIFO_ID);
    if (pcm_buffer_size > space)
      pcm_buffer_size = space;
    
    pcm_buffer_size &= ~1;
    if (pcm_buffer_size > 0)
      fifo_write(PCM_FIFO_ID, (unsigned char*)s_PCMBuffer, pcm_buffer_size);
  }
#if 0
  /* Pop PCM & ABI */
  short pcm = 0;
  if (fifo_capacity(PCM_FIFO_ID) >= sizeof(short))
    fifo_read(PCM_FIFO_ID, (unsigned char*)&pcm, sizeof(short));
  
  if (fifo_capacity(ABI_FIFO_ID) >= sizeof(uint64_t) + sizeof(char))
  {
    uint64_t timestamp = 0;
    char abi = 0;

    fifo_read(ABI_FIFO_ID, (unsigned char*)&timestamp, sizeof(uint64_t));
    fifo_read(ABI_FIFO_ID, (unsigned char*)&abi, sizeof(char));

    bluemix_publish_data(timestamp, abi, pcm);
  }
  else
  {
    uint64_t timestamp = 0;
    char abi = 0;
    bluemix_publish_data(timestamp, abi, pcm);
  }
#endif
  bluemix_subscribe_check();
}

void as5147_abi_signal_handler(char abi, char sig)
{
  /* Push <timestamp, abi> to FIFO */
  uint64_t timestamp = timerRead(s_HwTimer);
  if (fifo_space(ABI_FIFO_ID) >= sizeof(uint64_t) + sizeof(char))
  {
    fifo_write(ABI_FIFO_ID, (unsigned char*)&timestamp, sizeof(uint64_t));
    fifo_write(ABI_FIFO_ID, (unsigned char*)&abi, sizeof(char));
  }
  /* Start/Stop Mic Recording */
  if (sig == 2) // I signal
  {
    //Serial.println(String(abi, HEX));
    if (s_MicRecording)
      pdm_mic_record_stop();
    else
      pdm_mic_record_start();
    s_MicRecording = !s_MicRecording;
  }
}

