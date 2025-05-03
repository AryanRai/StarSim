#include <Arduino.h>
#include "ParsecCore.h"
#include "IPlatform.h"
#include <TeensyThreads.h>  // optional if not using native FreeRTOS
#include <string>

// -- Platform Layer --
class TeensyPlatform : public IPlatform {
public:
    uint64_t getMillis() override {
        return millis();
    }

    void log(const std::string& msg) override {
        Serial.println(("[TEENSY] " + msg).c_str());
    }
};

// -- Globals --
TeensyPlatform platform;
ParsecCore engine(&platform);

// -- RTOS Task --
void engineTask(void* pvParameters) {
    while (true) {
        engine.tick();
        vTaskDelay(100 / portTICK_PERIOD_MS);  // 100ms delay
    }
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("Starting ParsecCore...");

    // Create task (stack size, priority, etc.)
    xTaskCreate(
        engineTask,      // Function to call
        "EngineTask",    // Name
        2048,            // Stack size in words
        nullptr,         // Parameter
        1,               // Priority
        nullptr          // Task handle
    );
}

void loop() {
    // Optional: other code or sleep
    delay(1000);
}
