#include "controller.h"
#include <ps5Controller.h>
#include <esp_rom_crc.h>

extern "C" {
#include <stack/bt_types.h>
#include <stack/l2c_api.h>
#include <osi/allocator.h>
extern uint16_t l2cap_interrupt_channel;  // owned by the ps5 library
}

struct Input {
  uint8_t l2, r2;
  int8_t lx;
  bool l1, r1, l3, r3, left, right, cross;
};
static Input in;

static volatile uint8_t ledStage;  // 0: link not ready, 1: lightbar reset pending, 2: accepting colors
static int32_t sentColor;

// The library only parses the basic report 0x01, but the gamepad switches to the
// extended report 0x31 (different layout) after the first LED packet, so both are parsed here.
static void onPacket() {
  const uint8_t* p = ps5.LatestPacket() + 10;  // p[0] = report id
  const uint8_t* btn;
  Input next;
  if (p[0] == 0x01) {
    next.lx = p[1] - 128; btn = p + 5; next.l2 = p[8]; next.r2 = p[9];
  } else if (p[0] == 0x31) {
    next.lx = p[2] - 128; next.l2 = p[6]; next.r2 = p[7]; btn = p + 9;
  } else {
    return;
  }
  uint8_t dpad = btn[0] & 0x0F;
  next.right = dpad == 2;
  next.left  = dpad == 6;
  next.cross = btn[0] & 0x20;
  next.l1 = btn[1] & 0x01;
  next.r1 = btn[1] & 0x02;
  next.l3 = btn[1] & 0x40;
  next.r3 = btn[1] & 0x80;
  in = next;
}

// The library's setLed() sends a PS4-format packet the DualSense ignores, so the
// real output report 0x31 is built here and sent on the interrupt channel.
static bool sendLightbar(bool reset, uint8_t r, uint8_t g, uint8_t b) {
  static uint8_t seq;
  uint8_t out[79] = {0xA2, 0x31, uint8_t(seq++ << 4), 0x10};  // HID header, report id, sequence, tag
  if (reset) {
    out[42] = 0x02; out[45] = 0x02;  // take the lightbar away from the firmware's startup animation
  } else {
    out[5] = 0x04; out[48] = r; out[49] = g; out[50] = b;  // valid_flag1: lightbar colour
  }
  uint32_t crc = esp_rom_crc32_le(0, out, sizeof(out) - 4);
  memcpy(out + sizeof(out) - 4, &crc, 4);

  BT_HDR* buf = (BT_HDR*)osi_malloc(BT_DEFAULT_BUFFER_SIZE);
  if (!buf) return false;
  buf->length = sizeof(out);
  buf->offset = L2CAP_MIN_OFFSET;
  memcpy((uint8_t*)(buf + 1) + buf->offset, out, sizeof(out));
  return L2CA_DataWrite(l2cap_interrupt_channel, buf) != L2CAP_DW_FAILED;
}

void controllerBegin() {
  if (esp_rom_crc32_le(0, (const uint8_t*)"123456789", 9) != 0xCBF43926) {
    Serial.println("CRC32 self-check failed: the gamepad will reject lightbar packets");
  }
  ps5.attach(onPacket);
  ps5.attachOnConnect([] { in = {}; ledStage = 1; });
  ps5.begin(CONTROLLER_MAC);
}

bool controllerConnected() { return ps5.isConnected(); }

uint8_t controllerThrottle() { return in.r2; }
uint8_t controllerReverse()  { return in.l2; }
bool    controllerBrake()    { return in.r1; }
bool    controllerFrontBrake() { return in.l1; }
int8_t  controllerSteer()    { return in.lx; }

bool controllerSticksPressed() { return in.l3 && in.r3; }
bool controllerLeft()          { return in.left; }
bool controllerRight()         { return in.right; }
bool controllerCross()         { return in.cross; }

void controllerSetLed(uint8_t r, uint8_t g, uint8_t b) {
  int32_t color = r << 16 | g << 8 | b;
  bool reset = ledStage == 1;
  if (ledStage == 0 || (!reset && color == sentColor)) return;
  if (!sendLightbar(reset, r, g, b)) return;  // retried on the next call
  sentColor = reset ? -1 : color;
  ledStage = 2;
}
