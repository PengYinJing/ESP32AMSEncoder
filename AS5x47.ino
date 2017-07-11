/*
 * AS5x47.ino
 * 
 * Driver for AS5x47 in ESP32
 * 
 * Created by: 
 *       K.C.Y
 * Date:
 *       2017/06/30
 */
 
#include <SPI.h>
#include <driver/mcpwm.h>
#include <soc/mcpwm_reg.h>
#include <soc/mcpwm_struct.h>

#define CAP0_INT_EN BIT(27)  //Capture 0 interrupt bit
#define CAP1_INT_EN BIT(28)  //Capture 1 interrupt bit
#define CAP2_INT_EN BIT(29)  //Capture 2 interrupt bit

static int s_a_io_num = -1;
static int s_b_io_num = -1;
static int s_i_io_num = -1;
static int s_cs_io_num = -1;

static void (*as5x47_abi_detect_cb)(char abi, char sig) = NULL;

struct otp_regs_t {
  uint16_t zposm;
  uint16_t zposl;
  uint16_t settings[2];
  uint16_t red;
} s_otp_regs;

static void IRAM_ATTR isr_handler(void* arg)
{
    uint32_t intr_status = MCPWM0.int_st.val;
    char abi = 0;
    char sig = 3;
    //uint32_t capture_signal = 0;
    if (intr_status & CAP0_INT_EN)
    {
        //capture_signal = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP0);
        sig = 0;
    }
    
    if (intr_status & CAP1_INT_EN)
    {
        //capture_signal = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP1);
        sig = 1;        
    }
    
    if (intr_status & CAP2_INT_EN)
    {
        //capture_signal = mcpwm_capture_signal_get_value(MCPWM_UNIT_0, MCPWM_SELECT_CAP2);
        sig = 2;        
    }
    
    abi = (digitalRead(s_i_io_num) << 2) | (digitalRead(s_b_io_num) << 1) | digitalRead(s_a_io_num);
    
    MCPWM0.int_clr.val = intr_status;
    
    if (as5x47_abi_detect_cb && sig != 3)
      as5x47_abi_detect_cb(abi, sig);
}

static uint16_t make_raw_data(uint16_t data, boolean read)
{
  if (read)
    data |= 0x4000;
  int checksum = 0;
  for (int i = 0; i < 15; i++)
  {
    if (data & (1 << i))
      checksum++;
  }
  if (checksum & 0x01)
    data |= 0x8000;
  return data;
}

static uint16_t write_register(uint16_t address, uint16_t data)
{
  uint16_t command = make_raw_data(address, false);
  /* Write Command */
  digitalWrite(s_cs_io_num, LOW);
  delayMicroseconds(1);
  SPI.transfer16(command);
  delayMicroseconds(1);
  digitalWrite(s_cs_io_num, HIGH);
  delay(10);
  
  data = make_raw_data(data, false);
  /* Write Data */
  digitalWrite(s_cs_io_num, LOW);
  delayMicroseconds(1);
  data = SPI.transfer16(data);
  delayMicroseconds(1);
  digitalWrite(s_cs_io_num, HIGH);
  delay(10);

  data &= 0x3FFF;
  return data;
}

static uint16_t read_register(uint16_t address)
{
  uint16_t command = make_raw_data(address, true);
  /* Write Command */
  digitalWrite(s_cs_io_num, LOW);
  delayMicroseconds(1);
  SPI.transfer16(command);
  delayMicroseconds(1);
  digitalWrite(s_cs_io_num, HIGH);
  delay(10);

  uint16_t data = make_raw_data(0x0, true);
  /* Read Data */
  digitalWrite(s_cs_io_num, LOW);
  delayMicroseconds(1);
  data = SPI.transfer16(data);
  delayMicroseconds(1);
  digitalWrite(s_cs_io_num, HIGH);
  delay(10);

  if (data & 0x4000)
  {
    Serial.printf("Failed reading register (0x%x)\n", address);
    return 0xFFFF;
  }
  data &= 0x3FFF;
  return data;
}

void as5x47_init(int a_io_num, int b_io_num, int i_io_num, int cs_io_num)
{
  s_a_io_num = a_io_num;
  s_b_io_num = b_io_num;
  s_i_io_num = i_io_num;
  s_cs_io_num = cs_io_num;

  /* Initialize SPI */
  pinMode(s_cs_io_num, OUTPUT);
  digitalWrite(s_cs_io_num, HIGH);
  SPI.begin();
  SPI.setFrequency(1000000);
  SPI.setBitOrder(SPI_MSBFIRST);
  SPI.setDataMode(SPI_MODE1);

  /* Initialize OTP Reg Variables */
  as5x47_read_config();
  
  /* Initialize MCPWM */
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0, s_a_io_num);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_1, s_b_io_num);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_2, s_i_io_num);
  gpio_pulldown_en((gpio_num_t)a_io_num);
  gpio_pulldown_en((gpio_num_t)b_io_num);
  gpio_pulldown_en((gpio_num_t)i_io_num);

  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP0, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP1, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
  mcpwm_capture_enable(MCPWM_UNIT_0, MCPWM_SELECT_CAP2, MCPWM_POS_EDGE, 0);  //capture signal on rising edge, prescale = 0 i.e. 800,000,000 counts is equal to one second
  MCPWM0.int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;  //Enable interrupt on  CAP0, CAP1 and CAP2 signal
  mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);  //Set ISR Handler
}

void as5x47_set_abi_cb(void (*abi_detect_cb)(char, char))
{
  as5x47_abi_detect_cb = abi_detect_cb;
}

void as5x47_set_zero_position(uint16_t zpos)
{
  s_otp_regs.zposm = (zpos >> 6) & 0x00FF;
  s_otp_regs.zposl &= ~0x003F;
  s_otp_regs.zposl |= zpos & 0x3F;
  //write_register(0x0016, s_otp_regs.zposm);
  //write_register(0x0017, s_otp_regs.zposl);
}

void as5x47_set_direction(boolean cw)
{
  if (cw)
    s_otp_regs.settings[0] &= ~0x0004;
  else
    s_otp_regs.settings[0] |= 0x0004;
  //write_register(0x0018, s_otp_regs.settings[0]);
}

void as5x47_set_dynamic_angle_compensation(boolean on)
{
  if (on)
    s_otp_regs.settings[0] &= ~0x0010;
  else
    s_otp_regs.settings[0] |= 0x0010;
  //write_register(0x0018, s_otp_regs.settings[0]);
}

void as5x47_set_abi_pulses(boolean pulse512)
{
  if (pulse512)
    s_otp_regs.settings[1] &= ~0x0020;
  else
    s_otp_regs.settings[1] |= 0x0020;
  //write_register(0x0019, s_otp_regs.settings[1]);
}

void as5x47_set_index_width(boolean lsb3)
{
  if (lsb3)
    s_otp_regs.settings[0] &= ~0x0001;
  else
    s_otp_regs.settings[0] |= 0x0001;
  //write_register(0x0018, s_otp_regs.settings[0]);
}

void as5x47_set_uvw_pole_pairs(uint16_t uvw_pp) // 1, 2, 3, 4, 5, 6, 7
{
  if (uvw_pp < 1 || uvw_pp > 7)
    return;
  uvw_pp--;
  s_otp_regs.settings[1] &= 0x0007;
  s_otp_regs.settings[1] |= uvw_pp;
  //write_register(0x0019, s_otp_regs.settings[1]);
}

void as5x47_set_hysteresis(uint16_t hys) // 0, 1, 2, 3
{
  if (hys != 0 && hys != 1 && hys != 2 && hys != 3)
    return;
  s_otp_regs.settings[1] &= ~0x0018;
  s_otp_regs.settings[1] |= (3 - hys) << 3;
  //write_register(0x0019, s_otp_regs.settings[1]);
}

void as5x47_set_compensation_error(uint16_t comp_error_status) // 0 : Disable, 1 : Comp_Low, 2 : Comp_High, 3 : Comp_High & Comp_Low 
{
  if (comp_error_status != 0 && comp_error_status != 1 && comp_error_status != 2 && comp_error_status != 3)
    return;
  s_otp_regs.zposl &= ~0x00C0;
  s_otp_regs.zposl |= comp_error_status << 6;
  //write_register(0x0017, s_otp_regs.zposl);
}

void as5x47_set_pwm_out(uint16_t mode) // 0 : OFF, 1 : PWM->W, 2 : PWM->I
{
  if (mode != 0 && mode != 1 && mode != 2)
    return;
  if (mode == 0)
    s_otp_regs.settings[0] &= ~0x0080;
  else
  {
    s_otp_regs.settings[0] |= 0x0080;
    if (mode == 1)
      s_otp_regs.settings[0] &= ~0x0008;
    else
      s_otp_regs.settings[0] |= 0x0008;
  }
  //write_register(0x0018, s_otp_regs.settings[0]);
}

uint16_t as5x47_read_magnitude()
{
  return read_register(0x3FFD);
}

uint16_t as5x47_read_angle()
{
  if (s_otp_regs.settings[0] & 0x0040)
    return read_register(0x3FFE);
  return read_register(0x3FFF);
}

void as5x47_read_config()
{
  uint16_t reg;
  reg = read_register(0x0016);
  if (reg == 0xFFFF)
    reg = 0;
  
  s_otp_regs.zposm =reg;

  reg = read_register(0x0017);
  if (reg == 0xFFFF)
    reg = 0;
  s_otp_regs.zposl = reg;

  reg = read_register(0x0018);
  if (reg == 0xFFFF)
    reg = 0;
  s_otp_regs.settings[0] = reg;

  reg = read_register(0x0019);
  if (reg == 0xFFFF)
    reg = 0;
  s_otp_regs.settings[1] = reg;

  reg = read_register(0x001A);
  if (reg == 0xFFFF)
    reg = 0;
  s_otp_regs.red = reg;

  Serial.printf("ZPOSM: %x\n", s_otp_regs.zposm);
  Serial.printf("ZPOSL: %x\n", s_otp_regs.zposl);
  Serial.printf("SETTINGS1: %x\n", s_otp_regs.settings[0]);
  Serial.printf("SETTINGS2: %x\n", s_otp_regs.settings[1]);
}

void as5x47_write_config()
{
  Serial.println("as5x47_write_config");
  write_register(0x0016, s_otp_regs.zposm);
  write_register(0x0017, s_otp_regs.zposl);
  write_register(0x0018, s_otp_regs.settings[0]);
  write_register(0x0019, s_otp_regs.settings[1]);
}

boolean as5x47_burn_config()
{
  /* Write Custom Settings */
  write_register(0x0016, s_otp_regs.zposm);
  write_register(0x0017, s_otp_regs.zposl);
  write_register(0x0018, s_otp_regs.settings[0]);
  write_register(0x0019, s_otp_regs.settings[1]);
  
  /* Read Custom Settings */
  uint16_t zposm = read_register(0x0016);
  uint16_t zposl = read_register(0x0017);
  uint16_t settings0 = read_register(0x0018);
  uint16_t settings1 = read_register(0x0019);  
  if (s_otp_regs.zposm != zposm || s_otp_regs.zposl != zposl || s_otp_regs.settings[0] != settings0 || s_otp_regs.settings[1] != settings1)
  {
    as5x47_read_config();
    return false;
  }
  
  /* Enable Programming */
  write_register(0x0003, 0x0001); // PROGEN = 1
  write_register(0x0003, 0x0008); // PROGOTP = 1
  while (read_register(0x0003) != 0x0001)
    delay(100);
  
  /* Clear Non-volatile memory */
  write_register(0x0016, 0x0000);
  write_register(0x0017, 0x0000);
  write_register(0x0018, 0x0000);
  write_register(0x0019, 0x0000);

  /* Verify & Refresh */
  write_register(0x0003, 0x0040); // PROGVER = 1
  write_register(0x0003, 0x0004); // OTPREF = 1

  /* Read Custom Settings */
  zposm = read_register(0x0016);
  zposl = read_register(0x0017);
  settings0 = read_register(0x0018);
  settings1 = read_register(0x0019);
  if (s_otp_regs.zposm != zposm || s_otp_regs.zposl != zposl || s_otp_regs.settings[0] != settings0 || s_otp_regs.settings[1] != settings1)
  {
    /* Guard band test fail */
    as5x47_read_config();
    return false;
  }
  
  /* New Power on cycle */
  zposm = read_register(0x0016);
  zposl = read_register(0x0017);
  settings0 = read_register(0x0018);
  settings1 = read_register(0x0019);
  if (s_otp_regs.zposm != zposm || s_otp_regs.zposl != zposl || s_otp_regs.settings[0] != settings0 || s_otp_regs.settings[1] != settings1)
  {
    /* Wrong programming */
    as5x47_read_config();
    return false;
  }

  return true;
}

