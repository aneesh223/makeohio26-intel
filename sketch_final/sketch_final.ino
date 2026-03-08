#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMP_PIN  27
#define TDS_PIN   28
#define LED_RED   6
#define LED_GREEN 12
#define LED_BLUE  10

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

unsigned long lastBlink = 0;
bool ledState = false;
int blinkInterval = 0;

void updateOnboardLED() {
    if (blinkInterval == 0) {
        digitalWrite(LED_BUILTIN, HIGH);
        return;
    }
    unsigned long now = millis();
    if (now - lastBlink >= blinkInterval) {
        lastBlink = now;
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW);
    }
}

void setLED(bool red, bool green, bool blue) {
    digitalWrite(LED_RED,   red   ? HIGH : LOW);
    digitalWrite(LED_GREEN, green ? HIGH : LOW);
    digitalWrite(LED_BLUE,  blue  ? HIGH : LOW);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    pinMode(LED_RED,     OUTPUT);
    pinMode(LED_GREEN,   OUTPUT);
    pinMode(LED_BLUE,    OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    setLED(false, false, false);

    analogReadResolution(12);
    sensors.begin();
    Serial.println("System ready. Commencing LIVE monitoring...");
}

void loop() {
    sensors.requestTemperatures();
    float tempC = sensors.getTempCByIndex(0);
    bool failSafe = false;
    if (tempC == -127) {
        tempC = 25.0;
        failSafe = true;
    }

    int tdsRaw = analogRead(TDS_PIN);
    float tdsVoltage = (tdsRaw / 4095.0) * 3.3;

    float tdsRawPPM = (133.42 * pow(tdsVoltage, 3)
                     - 255.86 * pow(tdsVoltage, 2)
                     + 857.39 * tdsVoltage) * 0.5;
    if (tdsRawPPM < 0) tdsRawPPM = 0;

    float compensationCoeff = 1.0 + 0.02 * (tempC - 25.0);
    float compVoltage = tdsVoltage / compensationCoeff;
    float tdsComp = (133.42 * pow(compVoltage, 3)
                   - 255.86 * pow(compVoltage, 2)
                   + 857.39 * compVoltage) * 0.5;
    if (tdsComp < 0) tdsComp = 0;

    String routingDecision;
    if (tdsComp > 500.0 || tempC > 35.0) {
        routingDecision = "RED (Discard / Brine Management)";
        setLED(true, false, false);
        blinkInterval = 150;
    } else if (tdsComp >= 1.0) {
        routingDecision = "GREEN (Moderate Filtration)";
        setLED(false, true, false);
        blinkInterval = 600;
    } else {
        routingDecision = "BLUE (Direct UPW Reuse)";
        setLED(false, false, true);
        blinkInterval = 0;
    }

    Serial.println("\n════════════════════════════════════════");
    Serial.println("           LIVE WATER ANALYSIS          ");
    Serial.println("════════════════════════════════════════");
    Serial.print("Temp:          "); Serial.print(tempC, 1);      Serial.println(" °C");
    if (failSafe) Serial.println("WARN: Temp sensor disconnected! Defaulting to 25.0 °C");
    Serial.print("TDS (raw):     "); Serial.print(tdsRawPPM, 1);  Serial.println(" ppm");
    Serial.print("TDS (@25°C):   "); Serial.print(tdsComp, 1);    Serial.println(" ppm");
    Serial.println("────────────────────────────────────────");
    Serial.print("ACTION: "); Serial.println(routingDecision);
    Serial.println("════════════════════════════════════════");

    updateOnboardLED();
    delay(100);
}