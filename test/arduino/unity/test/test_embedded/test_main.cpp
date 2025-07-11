#include <Arduino.h>

#include <unity.h>

#include <estd/cstddef.h>
#include "unity/unit-test.h"

#ifdef __AVR__
#include <avr/wdt.h>
#endif

#ifdef LED_BUILTIN
CONSTEXPR static unsigned LED_PIN = LED_BUILTIN;
#endif

//#define DIAGNOSTIC 1


void setup()
{
    Serial.begin(9600);

#ifdef __AVR__
    wdt_disable();
#endif

    // delay was generally recommended by:
    // https://docs.platformio.org/en/stable/plus/unit-testing.html
    // but no longer.  Keeping a minimal one here just for boot/reprogram
    // assist (that was original recommendation)
    delay(2000);

#ifdef __AVR__
    wdt_enable(WDTO_4S);
#endif

    while(!Serial);

#ifdef LED_BUILTIN
    pinMode(LED_PIN, OUTPUT);
#endif

    Serial.println("setup: begin");

    UNITY_BEGIN();

    // DEBT: Consolidate this with the other explicit unity caller

#if DIAGNOSTIC
    Serial.println("setup: phase 1");
#endif
    test_align();
#if DIAGNOSTIC
    Serial.println("setup: phase 2");
#endif
    test_array();
#ifdef __AVR__
    test_avr_pgm();
#endif
    test_bipbuf();
    test_bit();
    test_chrono();
    test_cpp();
    test_cstddef();
    test_expected();
    test_functional();
    test_hash();
    test_limits();
    test_locale();
    test_map();
    test_optional();
    test_ostream();
    test_queue();
    test_ratio();
    test_span();
    test_streambuf();
    test_string();
    test_thread();
    test_tuple();
    test_type_traits();
    test_unordered();
    test_variadic();
    test_variant();

    UNITY_END();

    Serial.println("setup: end");
}


// Just to indicate we're not dead, we blink
void loop()
{
#ifdef LED_BUILTIN
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(500);
#endif

#ifdef __AVR__
    wdt_reset();
#endif
}
