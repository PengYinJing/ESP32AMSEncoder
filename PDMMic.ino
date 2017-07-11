/*
 * PDMMic.ino
 * 
 * Driver for MEMS microphone in ESP32
 * 
 * Created by: 
 *       K.C.Y
 * Date:
 *       2017/06/30
 */
 
#include <driver/i2s.h>

#define DUMP_I2S_REG_EN   0

#if DUMP_I2S_REG_EN
static void dump_i2s_regs()
{
  int conf_reg = REG_READ(I2S_CONF_REG(0));
  int conf1_reg = REG_READ(I2S_CONF1_REG(0));
  int conf2_reg = REG_READ(I2S_CONF2_REG(0));
  int timing_reg = REG_READ(I2S_TIMING_REG(0));
  int fifo_conf_reg = REG_READ(I2S_FIFO_CONF_REG(0));
  int rxeof_num_reg = REG_READ(I2S_RXEOF_NUM_REG(0));
  int conf_single_data_reg = REG_READ(I2S_CONF_SIGLE_DATA_REG(0));
  int conf_chan_reg = REG_READ(I2S_CONF_CHAN_REG(0));
  int out_link_reg = REG_READ(I2S_OUT_LINK_REG(0));
  int in_link_reg = REG_READ(I2S_IN_LINK_REG(0));
  int out_eof_des_addr_reg = REG_READ(I2S_OUT_EOF_DES_ADDR_REG(0));
  int in_eof_des_addr_reg = REG_READ(I2S_IN_EOF_DES_ADDR_REG(0));
  int out_eof_bfr_des_addr_reg = REG_READ(I2S_OUT_EOF_BFR_DES_ADDR_REG(0));
  int inlink_dscr_reg = REG_READ(I2S_INLINK_DSCR_REG(0));
  int inlink_dscr_bf0_reg = REG_READ(I2S_INLINK_DSCR_BF0_REG(0));
  int inlink_dscr_bf1_reg = REG_READ(I2S_INLINK_DSCR_BF1_REG(0));
  int outlink_dscr_reg = REG_READ(I2S_OUTLINK_DSCR_REG(0));
  int outlink_dscr_bf0_reg = REG_READ(I2S_OUTLINK_DSCR_BF0_REG(0));
  int outlink_dscr_bf1_reg = REG_READ(I2S_OUTLINK_DSCR_BF1_REG(0));
  int lc_conf_reg = REG_READ(I2S_LC_CONF_REG(0));
  int outfifo_push_reg = REG_READ(I2S_OUTFIFO_PUSH_REG(0));
  int infifo_pop_reg = REG_READ(I2S_INFIFO_POP_REG(0));
  int clkm_conf_reg = REG_READ(I2S_CLKM_CONF_REG(0));
  int sample_rate_conf_reg = REG_READ(I2S_SAMPLE_RATE_CONF_REG(0));
  int pdm_conf_reg = REG_READ(I2S_PDM_CONF_REG(0));
  int pdm_freq_conf_reg = REG_READ(I2S_PDM_FREQ_CONF_REG(0));

  Serial.printf("I2S_CONF_REG = 0x%08x\n", conf_reg);
  Serial.printf("I2S_CONF1_REG = 0x%08x\n", conf1_reg);
  Serial.printf("I2S_CONF2_REG = 0x%08x\n", conf2_reg);
  Serial.printf("I2S_TIMING_REG = 0x%08x\n", timing_reg);
  Serial.printf("I2S_FIFO_CONF_REG = 0x%08x\n", fifo_conf_reg);
  Serial.printf("I2S_RXEOF_NUM_REG = 0x%08x\n", rxeof_num_reg);
  Serial.printf("I2S_CONF_SIGLE_DATA_REG = 0x%08x\n", conf_single_data_reg);
  Serial.printf("I2S_CONF_CHAN_REG = 0x%08x\n", conf_chan_reg);
  Serial.printf("I2S_OUT_LINK_REG = 0x%08x\n", out_link_reg);
  Serial.printf("I2S_IN_LINK_REG = 0x%08x\n", in_link_reg);
  Serial.printf("I2S_OUT_EOF_DES_ADDR_REG = 0x%08x\n", out_eof_des_addr_reg);
  Serial.printf("I2S_IN_EOF_DES_ADDR_REG = 0x%08x\n", in_eof_des_addr_reg);
  Serial.printf("I2S_OUT_EOF_BFR_DES_ADDR_REG = 0x%08x\n", out_eof_bfr_des_addr_reg);
  Serial.printf("I2S_INLINK_DSCR_REG = 0x%08x\n", inlink_dscr_reg);
  Serial.printf("I2S_INLINK_DSCR_BF0_REG = 0x%08x\n", inlink_dscr_bf0_reg);
  Serial.printf("I2S_INLINK_DSCR_BF1_REG = 0x%08x\n", inlink_dscr_bf1_reg);
  Serial.printf("I2S_OUTLINK_DSCR_REG = 0x%08x\n", outlink_dscr_reg);
  Serial.printf("I2S_OUTLINK_DSCR_BF0_REG = 0x%08x\n", outlink_dscr_bf0_reg);
  Serial.printf("I2S_OUTLINK_DSCR_BF1_REG = 0x%08x\n", outlink_dscr_bf1_reg);
  Serial.printf("I2S_LC_CONF_REG = 0x%08x\n", lc_conf_reg);
  Serial.printf("I2S_OUTFIFO_PUSH_REG = 0x%08x\n", outfifo_push_reg);
  Serial.printf("I2S_INFIFO_POP_REG = 0x%08x\n", infifo_pop_reg);
  Serial.printf("I2S_CLKM_CONF_REG = 0x%08x\n", clkm_conf_reg);
  Serial.printf("I2S_SAMPLE_RATE_CONF_REG = 0x%08x\n", sample_rate_conf_reg);
  Serial.printf("I2S_PDM_CONF_REG = 0x%08x\n", pdm_conf_reg);
  Serial.printf("I2S_PDM_FREQ_CONF_REG = 0x%08x\n", pdm_freq_conf_reg);
}
#endif

void pdm_mic_init(int clk_io_num, int data_io_num, int samplerate)
{
  i2s_config_t i2s_cfg;
  i2s_cfg.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
  i2s_cfg.sample_rate = samplerate; //75000 : bad // 8000, 44100, 48000 : very good
  i2s_cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  i2s_cfg.channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT;
  i2s_cfg.communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_PCM | I2S_COMM_FORMAT_PCM_SHORT);
  i2s_cfg.dma_buf_count = 16;
  i2s_cfg.dma_buf_len = 64;
  i2s_cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;

  i2s_pin_config_t pin_cfg = {
    .bck_io_num = -1,
    .ws_io_num = clk_io_num,
    .data_out_num = -1,
    .data_in_num = data_io_num
  };
  i2s_driver_install(I2S_NUM_0, &i2s_cfg, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_cfg);
  i2s_stop(I2S_NUM_0);

#if DUMP_I2S_REG_EN
  dump_i2s_regs();
#endif
}

void pdm_mic_record_start()
{
  //Serial.println("pdm_mic_record_start()");
  i2s_start(I2S_NUM_0);
}

void pdm_mic_record_stop()
{
  //Serial.println("pdm_mic_record_stop()");
  i2s_stop(I2S_NUM_0);
}

int pdm_mic_read_data(char* sample, int max_size, int delay_ms)
{
  TickType_t delay_tick = delay_ms / portTICK_PERIOD_MS;
  return i2s_read_bytes(I2S_NUM_0, sample, max_size, delay_tick);
}

#if 0


void publish_pcm_to_waston(char* data, int size)
{
  //StaticJsonBuffer<JSON_OBJECT_SIZE(2048)> jsonBuffer;  //  allow for a few extra json fields that actually being used at the moment
 
}
#endif

