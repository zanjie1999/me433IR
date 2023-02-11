
/**
 * 咩射频 433Mhz射频与红外互转（用于红外学习）
 * 兼使用点灯接入点灯（用于定时控制）使用的是RGB灯的接口，可切换的模式比较多，用小爱训练来对应指令
 * Sparkle
 * 20230207
 * v1.0
 */

#define BLINKER_WIFI
#define BLINKER_MIOT_LIGHT

// 功能开关
#define ME_IR_TO_433
#define ME_433_TO_IR
#define ME_SG90
#define ME_OTA

// 红外配置 库默认的 14in 12out
#define FEEDBACK_LED_IS_ACTIVE_LOW // The LED on my board (D4) is active LOW
#define IR_RECEIVE_PIN          14 // D5
#define IR_SEND_PIN             12 // D6 - D4/pin 2 is internal LED

#define tone(...) void()      // tone() inhibits receive timer
#define noTone(a) void()
#define TONE_PIN                42 // Dummy for examples using it

#include <IRremote.hpp>
#include <RCSwitch.h>
#ifdef ME_OTA
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#endif
#include <Blinker.h>

char auth[] = "key";
char ssid[] = "ssid";
char pswd[] = "pwd";

// 射频模块脚
#define RECEIVE_433_PIN   5
#define SEND_433_PIN  4
#define SG_PIN 13

#define pulseLength 257
#define bitLength 25

// 触发开角度
#define onDeg 180
// 触发关角度
#define offDeg 135
// 等待开角度
#define stay2onDeg 70
// 等待关角度
#define stay2offDeg 100
// 转动等待时间
#define degT 190
// 待机超时ms 控制完超时会关掉输出
#define sgSlpT 10000

#ifdef ME_IR_TO_433
RCSwitch send433 = RCSwitch();
#ifdef ME_433_TO_IR
#endif
RCSwitch recv433 = RCSwitch();
#endif
#ifdef ME_OTA
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#endif

uint8_t colorR, colorG, colorB, colorW;
bool onState = true;
uint8_t wsMode = BLINKER_CMD_MIOT_DAY;

uint32_t getColor() {
    uint32_t color = colorR << 16 | colorG << 8 | colorB;

    return color;
}

#ifdef ME_SG90
// 因为定时器冲突，手动实现这个逻辑
void sgWrite(int deg) {
    // sg90的实际工作范围500-2480 这样初始化才能到180度
    int dms = (deg * 11) + 500;
    digitalWrite(SG_PIN, HIGH);
    delayMicroseconds(dms);
    digitalWrite(SG_PIN, LOW);
    delayMicroseconds(20000 - dms);
}

// 舵机开关灯
void setOnOff(bool isOn) {
    onState = isOn;
    if (isOn) {
        sgWrite(stay2onDeg);
        BUILTIN_SWITCH.print("on");
        BlinkerMIOT.powerState("on");
        BlinkerMIOT.print();
        delay(degT);
        sgWrite(onDeg);
        delay(degT);
        sgWrite(stay2offDeg);
    } else {
        sgWrite(stay2offDeg);
        BUILTIN_SWITCH.print("off");
        BlinkerMIOT.powerState("off");
        BlinkerMIOT.print();
        delay(degT);
        sgWrite(offDeg);
        delay(degT);
        sgWrite(stay2onDeg);
    }
}
#endif

// 小爱开关灯
void miotPowerState(const String & state) {
    BLINKER_LOG("need set power state: ", state);
    #ifdef ME_SG90
    if (state == BLINKER_CMD_ON) {
        setOnOff(true);
    } else if (state == BLINKER_CMD_OFF) {
        setOnOff(false);
    }
    #endif
}

// 小爱设置颜色 可以判断颜色实现功能
void miotColor(int32_t color) {
    BLINKER_LOG("need set color: ", color);

    colorR = color >> 16 & 0xFF;
    colorG = color >>  8 & 0xFF;
    colorB = color       & 0xFF;

    BLINKER_LOG("colorR: ", colorR, ", colorG: ", colorG, ", colorB: ", colorB);

    BlinkerMIOT.color(color);
    BlinkerMIOT.print();
}

// 小爱设置模式
void miotMode(uint8_t mode) {
    BLINKER_LOG("need set mode: ", mode);

    if (mode == BLINKER_CMD_MIOT_DAY) {
        // 日光
    }
    else if (mode == BLINKER_CMD_MIOT_NIGHT) {
        // 夜光
    }
    else if (mode == BLINKER_CMD_MIOT_COLOR) {
        // 彩光
    }
    else if (mode == BLINKER_CMD_MIOT_WARMTH) {
        // 温馨
    }
    else if (mode == BLINKER_CMD_MIOT_TV) {
        // 电视
    }
    else if (mode == BLINKER_CMD_MIOT_READING) {
        // 阅读
    }
    else if (mode == BLINKER_CMD_MIOT_COMPUTER) {
        // 电脑
    }

    wsMode = mode;

    BlinkerMIOT.mode(mode);
    BlinkerMIOT.print();
}

// 小爱设置亮度
void miotBright(const String & bright) {
    BLINKER_LOG("need set brightness: ", bright);

    colorW = bright.toInt();

    BLINKER_LOG("now set brightness: ", colorW);

    BlinkerMIOT.brightness(colorW);
    BlinkerMIOT.print();
}

// 小爱设置色温
void miotColoTemp(int32_t colorTemp) {
    BLINKER_LOG("need set colorTemperature: ", colorTemp);

    BlinkerMIOT.colorTemp(colorTemp);
    BlinkerMIOT.print();
}

// 小爱查状态
void miotQuery(int32_t queryCode) {
    BLINKER_LOG("MIOT Query codes: ", queryCode);

    switch (queryCode)
    {
        case BLINKER_CMD_QUERY_POWERSTATE_NUMBER :
            BLINKER_LOG("MIOT Query Power State");
            BlinkerMIOT.powerState(onState ? "on" : "off");
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_COLOR_NUMBER :
            BLINKER_LOG("MIOT Query Color");
            BlinkerMIOT.color(0);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_MODE_NUMBER :
            BLINKER_LOG("MIOT Query Mode");
            BlinkerMIOT.mode(0);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_COLORTEMP_NUMBER :
            BLINKER_LOG("MIOT Query ColorTemperature");
            BlinkerMIOT.colorTemp(1000);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_BRIGHTNESS_NUMBER :
            BLINKER_LOG("MIOT Query Brightness");
            BlinkerMIOT.brightness(1);
            BlinkerMIOT.print();
            break;
        case BLINKER_CMD_QUERY_ALL_NUMBER :
            BLINKER_LOG("MIOT Query All");
        default :
            BlinkerMIOT.powerState(onState ? "on" : "off");
            BlinkerMIOT.color(0);
            BlinkerMIOT.mode(0);
            BlinkerMIOT.colorTemp(1000);
            BlinkerMIOT.brightness(1);
            BlinkerMIOT.print();
            break;
    }
}

void dataRead(const String & data) {
    BLINKER_LOG("Blinker readString: ", data);

    // Blinker.vibrate();
    
    uint32_t BlinkerTime = millis();
    
    Blinker.print("millis", BlinkerTime);
}

String summary() {
    String data = "online, switch: " + STRING_format(onState ? "on" : "off");
    return data;
}

// 按键定时器调用
#ifdef ME_SG90
// BUILTIN_SWITCH
void setOnOffBtn(const String & state) {
    setOnOff(state == "on");
//    Blinker.vibrate();
}

BlinkerSlider degSlider("ran-deg");
void setDeg(int32_t value) {
    sgWrite(value);
    Blinker.vibrate();
}
#endif
#ifdef ME_IR_TO_433
// 风扇开关
BlinkerButton fanOnBtm("fan-on");
void fanOn(const String & state) {
    send433.send(13990914, bitLength);
    delay(500);
    if (recv433.available()) recv433.resetAvailable();
}

// 风速减
BlinkerButton fanMinBtm("fan-min");
void fanMin(const String & state) {
    send433.send(13990926, bitLength);
    delay(500);
    if (recv433.available()) recv433.resetAvailable();
}

// 风速加
BlinkerButton fanAddBtm("fan-add");
void fanAdd(const String & state) {
    send433.send(13990924, bitLength);
    delay(500);
    if (recv433.available()) recv433.resetAvailable();
}

// 风扇摇头
BlinkerButton fanShaBtm("fan-sha");
void fanSha(const String & state) {
    send433.send(13990916, bitLength);
    delay(500);
    if (recv433.available()) recv433.resetAvailable();
}
#endif

void setup() {
    Serial.begin(115200);    
    BLINKER_DEBUG.stream(Serial);
    BLINKER_DEBUG.debugAll();

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);
    Blinker.attachSummary(summary);

    #ifdef ME_SG90
    pinMode(SG_PIN, OUTPUT);
    sgWrite(stay2offDeg);

    BUILTIN_SWITCH.attach(setOnOffBtn);
    degSlider.attach(setDeg);
    #endif

    #ifdef ME_IR_TO_433
    send433.enableTransmit(SEND_433_PIN);
    send433.setPulseLength(pulseLength);
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    fanOnBtm.attach(fanOn);
    fanMinBtm.attach(fanMin);
    fanAddBtm.attach(fanAdd);
    fanShaBtm.attach(fanSha);
    #endif

    #ifdef ME_433_TO_IR
    recv433.enableReceive(RECEIVE_433_PIN);
    IrSender.begin();
    // not esp32
//    IrSender.enableIROut(38);
    #endif

    BlinkerMIOT.attachPowerState(miotPowerState);
    BlinkerMIOT.attachColor(miotColor);
    BlinkerMIOT.attachMode(miotMode);
    BlinkerMIOT.attachBrightness(miotBright);
    BlinkerMIOT.attachColorTemperature(miotColoTemp);
    BlinkerMIOT.attachQuery(miotQuery);

//    wifi_set_sleep_type(LIGHT_SLEEP_T);

    #ifdef ME_OTA
    httpUpdater.setup(&httpServer);
    httpServer.begin();
    #endif
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    Blinker.run();
    #ifdef ME_OTA
    httpServer.handleClient();
    #endif
    #ifdef ME_433_TO_IR
    if (recv433.available()) {
        digitalWrite(LED_BUILTIN, LOW);
//        Serial.print("Decimal: ");
//        Serial.print(recv433.getReceivedValue());
//        Serial.print(" Bit: ");
//        Serial.print(recv433.getReceivedBitlength());
//        Serial.print(" PulseLength: ");
//        Serial.print(recv433.getReceivedDelay());
//        Serial.print(" Protocol: ");
//        Serial.println(recv433.getReceivedProtocol());
        
        Blinker.print("Decimal", recv433.getReceivedValue());
        Blinker.print("Bit", recv433.getReceivedBitlength());
        Blinker.print("PulseLength", recv433.getReceivedDelay());
        Blinker.print("Protocol", recv433.getReceivedProtocol());
        IrSender.sendNECRaw(recv433.getReceivedValue());
        recv433.resetAvailable();
        delay(500);
        if (IrReceiver.decode()) IrReceiver.resume();
        digitalWrite(LED_BUILTIN, HIGH);
    }
    #endif
    #ifdef ME_IR_TO_433
    if (IrReceiver.decode()) {
        IrReceiver.printIRResultShort(&Serial);
        if (IrReceiver.decodedIRData.protocol == 0) {
            IrReceiver.resume();
        } else {
            digitalWrite(LED_BUILTIN, LOW);
            Blinker.print("Protocol", IrReceiver.decodedIRData.protocol);
            Blinker.print("RawData", String(IrReceiver.decodedIRData.decodedRawData));
            send433.send(IrReceiver.decodedIRData.decodedRawData, bitLength);
            IrReceiver.resume();
            delay(500);
            if (recv433.available()) recv433.resetAvailable();
            digitalWrite(LED_BUILTIN, HIGH);
        }
    }
    #endif
}
