/*
 * AVR Configurable Custom Logic library
 *
 * A library for interfacing with the AVR Configurable Custom Logic.
 * Developed in 2019 by MCUdude. https://github.com/MCUdude/
 * Example by Spence Konde, 2021 https://github.com/SpenceKonde/
 *
 * Example: Oscillate.ino
 *
 * In this example, we show some of the potential behaviors that you can get from the Logic and Event libraries that
 * involve one or more logic blocks oscillating. These demonstrations are likely to be most useful if you are
 * able to put a 'scope probe on PA3 (megaAVR or AVR Dx) or PA4 (tinyAVR).
 * In demos 1~4, we explore the possibilities of an asynchronous logic block oscillating as fast as it can.
 * Then, in demos 5-8, we demonstrate the use of the Filter and Synchronizer options to slow them down and make a
 * potentially practical tool from this concept: an independent prescaled clock output on an event channel.
 * Finally, in demo9a, 9b, and 9d, we demonstrate the potential use case of such configurations: clocking a timer
 *
 * demo9b is by far the most useful, demonstrating the clocking a type B timer from such a clock signal. Requires an
 * AVR Dx or tinyAVR 2-series part (the earlier devices do not support count-on-event for the type B timers).
 *
 * Some tests do not run on all devices due to hardware limitations. Where frequencies of pins are measured, I include
 * my measurements I made during development of the example.
 *
 * See also: Toolbox.md - general listing of a "toolbox" of logic and event combinations
 *           examples/BadExamples - A fever-dream of a sketch containing two equally inadvisable but apparently
 *              functional abuses of these concepts.
 *              In the first, a pair of logic blocks is connected as in demo4, and output on to the pin, conveniently
 *              located near the EXT_CLK pin. It is connected to that with a jumper. System clock prescale set to two
 *              and switched to the "external" clock.
 *              In the second, which is less related to these, the internal HF oscillator, already running
 *              at a third over the manufacturer specifications is used combined with an undocumented PLL setting
 *              in order to clock TCD0 at over 2.5x the rated maximmum speed. The TCD0 PWM output is then put into
 *              a logic block, output on a pin near the EXT_CLK pin, to which it is connected with a jumper. Finally
 *              the MCU's main clock source is switched to this 9.85 MHz"external" clock. AVR-Dx only.
 */

#include <Logic.h>
#include <Event.h>

//#define SHOW_TCD_DEMO


void setup() {
  #if defined(SHOW_TCD_DEMO) && !defined(MEGATINYCORE)
  /* The demonstration of clocking TCD0 from the output of a Logic block
   * requires use of the EXT_CLK pin, which is PA0 on those parts, the same as
   * the non-alternate TX pin for Serial on these parts; the Serial pins must
   * be swapped in this case.
   */
  Serial.swap(1);
  #endif
  Serial.begin(115200);
}


void demo1() {
  /* Async Demo 1: Just how fast is this "asynchronous" stuff?
   *
   * First, let's do the most obvious case - enable a single input, set it as feedback, and tell the logic block to turn on when the input is LOW and off when input is HIGH. If using a 'scope, monitor the output pin, and turn the bandwidth limit to full, not 20M).
   * My oscilloscope is clocking the output of this at an eye-popping 110 MHz!
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::disable;          // No output filter enabled
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0


  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Not Used. Stop Event0 (if it was running).


  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false


  Logic::start();                           // Start the CCL hardware
}

void demo2() {
  /* Async demo 2: Adding a second Logic stage
   *
   * Run it through one of the other Logic blocks...
   * Remember that in::feedback gives the output of the even logic block, not the one it's used with
   * There's no pre-defined input for the output of the odd logic block (sadly).
   * Only feedback for the even one out of the pair, and link for the next highest.
   * This logic block will be set up to just echo it's input, rather than invert it.
   *
   * 'scope shows 55 MHz here (about exactly half)
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::link;                 // Use output of next logic block (Logic1)
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::disable;          // No output filter enabled
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0


  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Not Used. Stop Event0 (if it was running).


  /* Logic1 - CCL LUT1 */
  Logic1.enable = true;                     // Enable logic block 1
  Logic1.input0 = in::feedback;             // use output of even-numbered block, ie, block 0
  Logic1.input1 = in::masked;               // masked
  Logic1.input2 = in::masked;               // masked
  Logic1.output = out::enable;              // enable logic block 1 output pin
  Logic1.filter = filter::disable;          // No output filter enabled
  Logic1.truth = 0x02;                      // Set truth table: Copy, HIGH if input0 HIGH
  Logic1.init();                            // Initialize logic block 1


  Logic::start();                           // Start the CCL hardware

}
void demo3() {
  /* Async Demo 3: Using event channel stage
   *
   * Now, we will take an event channel and point it at the CCL0 generator, set CCL0 as the user of that event
   * This is a useful technique for getting a "feedback" for an odd logic block, as their "feedback" comes from
   * the even block they are associated with.
   *
   * Same as before - 55 MHz
   */
  Logic::stop();                            // Stop the CCL so changes can be made

  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::event_a;              // use event channel A - a virtual feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::disable;          // No output filter enabled
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0

  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Stop Event0 (if it was running)
  Event0.set_generator(gen::ccl0_out);      // Use output of Logic0
  Event0.set_user(user::ccl0_event_a);      // Connect Event0 (carrying Logic0 output) to Logic0 event A input
  Event0.start();                           // Enable Event0

  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false

  Logic::start();                           // Start the CCL hardware

}


void demo4() {
  /* Async demo 4: Using both second logic block and event stages
   *
   * Finally, we do both of those together
   * And this time I measure 37 MHz - that is to say, it would appear that each of these async stages
   * takes about the same length of time. Which probably shouldn't be particularly surprising.
   *
   * The actual frequency also varies significantly depending on the temperature and other particulars.
   *
   * Obviously, none of these are particularly useful, though you can do funny (if pointless) things like clock
   * the chip off of this (probably prescaled) as an external clock or something. A too-high-frequency signal
   * unmoored from any reference frequency is pretty useless...
   *
   * Next, we'll make some synchronized oscillators which will be more useful!
   *
   */
  Logic::stop();                            // Stop the CCL so changes can be made

  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::link;                 // link - use output of Logic1
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::disable;          // No output filter enabled
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0

  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Stop Event0 (if it was running)
  Event0.set_generator(gen::ccl0_out);      // Use output of Logic0 (virtual-feedback)
  Event0.set_user(user::ccl1_event_a);      // Connect Event0 (carrying Logic0 output) to Logic1 event A input
  Event0.start();                           // Enable Event0

  /* Logic1 - CCL LUT1 */
  Logic1.enable = true;                     // Enable logic block 1
  Logic1.input0 = in::event_a;              // use event channel A
  Logic1.input1 = in::masked;               // masked
  Logic1.input2 = in::masked;               // masked
  Logic1.output = out::enable;              // enable logic block 1 output pin
  Logic1.filter = filter::disable;          // No output filter enabled
  Logic1.truth = 0x02;                      // Set truth table: Copy, HIGH if input0 HIGH
  Logic1.init();                            // Initialize logic block 0

  Logic::start();                           // Start the CCL hardware

}

void demo5() {
  /* Sync demo 1: The synchronizer delays each edge by 2 system clock cycles
   *
   * Same setup as original demo1 - except we enable the synchronizer which delays each transition by 2-3 clocks
   * In practice, we know that the signal will arrive early enough to always be delayed by only 2 clocks.
   * 2 clocks times 2 transitions gives us 4 clock cycles.
   *
   * We will use the default clock source (CLK_PER, which runs at F_CPU) here.
   *
   * Thus, we end up with a signal of F_CPU/4!
   *
   * This might be more useful!
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::sync;             // Synchronizer - 2 clock-cycle delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0


  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Not Used. Stop Event0 (if it was running).


  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false


  Logic::start();                           // Start the CCL hardware
}

void demo6() {
  /* Sync demo 2: The Filter module adds 4 clock cycles per edge, 8 per cycle
   *
   * Same setup as previous - only filter instead of synchronizer. That takes 2 more clocks.
   * on each transition, total of 8. We aren't actually doing any filtering, just using the delay
   *
   * This way, we end up with a signal of F_CPU/8!
   *
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::filter;           // Filter - 4 clock-cycle delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0


  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Not Used. Stop Event0 (if it was running).


  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false


  Logic::start();                           // Start the CCL hardware
}

void demo7() {
  /* Sync demo 3: Passing input through another logic block ADDS it's delay
   *
   * Same idea - only second logic block with a synchronizer user, and linked input.
   *
   * So delays are 8 + 4 = 12, and we end up with of F_CPU/12!
   * Can do the same with filter on both for F_CPU/16....
   * .
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::link;                 // output from Logic1
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin or PA4 (ATtiny))
  Logic0.filter = filter::filter;           // Filter - 4 clock-cycle delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0



  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Not Used. Stop Event0 (if it was running).


  /* Logic1 - CCL LUT1 */
  Logic1.enable = true;                     // Enable logic block 1
  Logic1.input0 = in::feedback;             // feedback (from logic0)
  Logic1.input1 = in::masked;               // masked
  Logic1.input2 = in::masked;               // masked
  Logic1.output = out::enable;              // enable logic block 1 output pin
  Logic1.filter = filter::sync;             // Synchronizer - 2 clock delay each transition
  Logic1.truth = 0x02;                      // Set truth table: Copy, HIGH if input0 HIGH
  Logic1.init();                            // Initialize logic block 1

  Logic::start();                           // Start the CCL hardware
}


void demo8() {
  /* Sync demo 4: Using other logic block as clock MULTIPLIES the delays
   *
   * Two logic blocks.
   * Logic0 is doing as it was in demo7. But in logic block 2, we connect it to input2
   * and set it to use input 2 as the clock. Then we use an event channel to get feedback for Logic1
   * as input0, and have this one also oscillate.
   *
   * With the filter on LUT1 clocked from the F_CPU/8 from LUT0, we we are generating an output of F_CPU/64
   *
   * What does this mean?
   * It means that with a few LUTs and into a prescaled clock on either a pin or an event channel at a wide
   * variety of fractions of the system clock speed. See the table:
   * Each frequency is listed only for the least-demanding combination of hardware that can do it:
   *
   * Divisor           Min logic blocks:
   * /4, /8,           one
   * /12, /16          two
   * /32, /64          two and one event channel   - 8*4 and 8*8
   * /20, /24          three,  no event channel    - L0 + L1 + L2
   * /48, /96, /128    three,  no event channel    - (L0 + L1) * L2
   * /256 /512         three, one event channel    - 8*8*4 and 8*8*8
   * /28               four, no event channel      - L0 + L1 + L2 + L3
   * /144 /192         four, no event channel      - (L0 + L1) * (L2 + L3)
   * /384, /768, /1024 four, one event channel     - (L0 + L1) * L2 * L3
   * /2048, /4096      four, two event channels    - 8*8*8*4 and 8*8*8*8
   *
   * Next, we will demonstrate how to use this to clock a timer.
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::link;                 // input2 (used as clock) from Logic1
  Logic0.clocksource = clocksource::in2;     // Use input 2 as clock instead of the default
  Logic0.output = out::enable;              // Enable logic block 0 output pin PA3 or PA4 (ATtiny))
  Logic0.filter = filter::filter;           // Filter - 4 clock delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0



  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Stop Event0 (if it was running)
  Event0.set_generator(gen::ccl1_out);      // Use output of Logic1
  Event0.set_user(user::ccl1_event_a);      // Connect Event0 (carrying Logic1 output) to Logic1 event A input
  Event0.start();                           // Enable Event0


  /* Logic1 - CCL LUT1 */
  Logic1.enable = true;                     // Enable logic block 1
  Logic1.input0 = in::event_a;              // use event A, which is coming from output of Logic1 (ie, feedback for an odd block)
  Logic1.input1 = in::masked;               // masked
  Logic1.input2 = in::masked;               // masked
  Logic1.output = out::enable;              // enable logic block 1 output pin
  Logic1.filter = filter::filter;           // Filter - 4 clock delay each transition
  Logic1.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic1.init();                            // Initialize logic block 1



  Logic::start();                           // Start the CCL hardware
}



void demo9a() {
  /* Using prescaled clocks 1: TCA0 (if you want to try this and don't have a Dx or tiny 1-series to play with)
   *
   * TCA0 is not a timer that really cries out for prescaling the clock going into it... it already HAS a prescaler that works fine and doesn't eat CCL
   * But if you're on a megaavr 0-series this is your only option - so we'll do it for fun. Start with what we had last time - but let's change the
   * filter to a synchronizer on one of the blocks, just to make it different from normal PWM in some way (64 is default TCA0 prescaler, while 32 isn't
   * an option. And we'll add an event channel to bring carrying the output of Logic0. Then we'll reconfigure TCA0 to have it generate, uh. 8-bit PWM
   * on pin A2 at the twice frequency it normally would...
   * But it will be doing that without using it's own prescaler!
   * How exciting...
   *     ...
   *           ... Actually, this causes it's prescaler to not effect the TCA's clock rate.... but it does still effect that of a TCB using it!
   * So depending on how much you need which timer prescaled by, this might be just as good as getting a prescaler for a TCB (below, Dx/2-series only).
   */

  Logic::stop();                            // Stop the CCL so changes can be made

  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::link;                 // input2 (used as clock) from Logic1
  Logic0.clocksource = clocksource::in2;    // Use input 2 as clock instead of the default
  Logic0.output = out::enable;              // Enable logic block 0 output pin PA3 or PA4 (ATtiny))
  Logic0.filter = filter::filter;           // Filter - 4 clock delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0



  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Stop Event0 (if it was running)
  Event0.set_generator(gen::ccl1_out);      // Use output of Logic1
  Event0.set_user(user::ccl1_event_a);      // Connect Event0 (carrying Logic1 output) to Logic1 event A input
  Event0.start();                           // Enable Event0


  /* Logic1 - CCL LUT1 */
  Logic1.enable = true;                     // Enable logic block 1
  Logic1.input0 = in::event_a;              // use event A, which is coming from output of Logic1 (ie, feedback for an odd block)
  Logic1.input1 = in::masked;               // masked
  Logic1.input2 = in::masked;               // masked
  Logic1.output = out::enable;              // enable logic block 1 output pin
  Logic1.filter = filter::sync;             // Filter - 4 clock delay each transition
  Logic1.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic1.init();                            // Initialize logic block 1

  /* Event1 - EVSYS CHANNEL1 */
  Event1.stop();                            // Stop Event1 (if it was running)
  Event1.set_generator(gen::ccl0_out);      // Use output of Logic0
  Event1.set_user(user::tca0_cnta);         // Connect Event1 (carrying Logic0 output) to TCA0 event a
  Event1.start();                           // Enable Event1

  Logic::start();                           // Start the CCL hardware

  /* TCA0 - Type A timer */
  TCA0.SPLIT.CTRLA &= ~TCA_SPLIT_ENABLE_bm; // disable and reset
  TCA0.SPLIT.CTRLESET = TCA_SPLIT_CMD_RESET_gc | TCA_SPLIT_CMDEN_BOTH_gc;

  // Set single-slope PWM mode, on CMP2
  TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;

  // set PORTMUX so that the TCA waveform output appears on PORTA
  PORTMUX.TCAROUTEA = 0;

  // Count on positive edge, cnta event input
  TCA0.SINGLE.EVCTRL = TCA_SINGLE_EVACTA_CNT_POSEDGE_gc | TCA_SINGLE_CNTAEI_bm;

  // counting to 254 is the same thing that the timers are configured to do by default (at least on DxCore and megaTinyCore)
  // so we're going to be expecting a 25% duty cycle
  TCA0.SINGLE.PER = 254;
  TCA0.SINGLE.CMP2 = 63;

  // And turn it back on!
  TCA0.SINGLE.CTRLA |= TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;


}

void demo9b() {
  /* Divided Clocks: TCB0 - prescale it without dedicating the prescaler of a TCA to it!
   *
   * The type B timers are every AVR developers dream as utility timers. There are only two issues with them -
   * the first being that they're awful for PWM (but you have TCAs for that) and they dont have an independent
   * prescaler - they can either use a TCA prescaler, CLK_PER, or CLK_PER/2. Sometimes you really need to time
   * something that's a bit too long for CLK_PER/2, but you've got the TCA's way down at /64 or /256 for PWM
   * and you don't want to throw away all that accuracy. Or maybe you're writing a library, or code you expect
   * to be copy-pasted all over by people who don't understand it, who might be mixing it with other libraries?
   * (though, to be faaiiir, the official cores basically fall over like a house of cards the moment you turn off the prescal)
   *
   * The AVR Dx and tinyAVR 2-series added the ability to clock on event, so you can now easily use the clock
   * generated as in the other examples to clock a TCB!
   *
   * We will use PWM frequency to demonstrate the change, though. Mostly because I have
   */

  Logic::stop();                            // Stop the CCL so changes can be made

  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.clocksource = clocksource::clk_per;// Use CLK_PER (default) as clock source once more.
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::filter;           // Filter - 4 clock-cycle delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0


  /* Event0 - EVSYS CHANNEL0 */
  Event0.set_generator(gen::ccl0_out);      // Use output of Logic0
  Event0.set_user(user::tcb0_cnt);          // Connect Event0 (carrying Logic0 output) to TCB0 count
  Event0.start();                           // Enable Event0


  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false

  /* Event1 - EVSYS CHANNEL1 */
  Event1.stop();                            // Not Used. Stop Event1 (if it was running).

  Logic::start();                           // Start the CCL hardware


  /* TCB0 - Timer/Counter Type B */
  TCB0.CTRLA = 0;
  TCB0.CTRLA = TCB_CLKSEL_DIV2_gc | TCB_ENABLE_bm;  // Switch to clk_per clock & enable
  Serial.println("Before (CLK_PER/2): ~47 kHz");    // 24 / 2 = 12 MHz, 12 MHz / 255 count/cycle = ~47 kHz
  analogWrite(PIN_PF4, 128);                         // Output some pwm to demo change in frequency
  delay(5000);
  TCB0.CTRLA = 0;                                   // Disable TCB0
  TCB0.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm; // Switch to event clock & enable
  Serial.println("After: ~11.7 kHz");               // 24 / 8 = 3 MHz, 3 MHz / 255 count/cycle = ~11.7 kHz

  /* To be clear: The PWM mode is just being used to give numbers, and something that looks good on a 'scope screen.
   * the typical use case for playing with the timer when you want a prescaler between the "8-bit analogWrite-like-PWM" prescalers and the
   * /4, /8, /12, and /16 range is timing that you need to be more precise than you can get with micros. Alternately, if you're taking over your
   * only TCA for 16-bit PWM, you might instead need to slow down the clock on the TCBs to get some plain 8-bit PWM down at a more tractable
   * frequency.
   *
   * One thing to remember is that you can have as many users as you want on a channel - you only need to make the desired clock rate once
   * and you can use it on everything.
   */


}

void demo9d() {
  /* Using prescaled clocks 1: TCD0 pre-prescaler
   *
   * Finally, though we need a jumper to do so (between PA3 and PA0) we can also use this output as the "external clock". We'll leave the /8 prescale
   *
   * Now you may be thinking "Why on earth would I ever want to do this?! TCD0 HAS a prescaler!"
   * Yes. Yes it does. And it's options are 1, 2, 4, and 8. And then that can be further divided by 1, 4, or 32 for count, and 1, 2, 4, or 8 for delay.
   * Meaning that if you're using the delay event as shown in the TCDThirdPWM example to get another channel, the highest prescale you can get is /64,
   * and it's an 8-bit timer. At the higher end of the clock speed, those frequencies are getting faster than you would like to PWM a power MOSFET gate
   * directly (you don't want much over 1 kHz for that, at least with a beefy MOSFET - high frequency PWM of large loads generally needs a gate driver).
   *
   * For this, you need a jumper - as short as you can make it - between PA0 and PA3, because we can't pipe events directly in as clock sources for TCD0.
   * We can, however use an "external" clock by connecting the output of the LUT, PA3 to the CLK_IN pin, PA0. Frequencies are high enough that you want
   * it short - at least if you want the first few demos to work with the jumper in place.
   * I used a piece of female pin header with the two middle pins yanked out, and 4-hole piece of strip-board.
   *
   * This is not a guide to configuration of the the Type D timer for Arduino users. That is a subject for an entirely different document. The TCD0
   * configuration steps are not commented extensively. Refer to the datasheet for more detailed information; it may be the most complicated
   * peripheral of the AVR architecture.
   *
   */

  Logic::stop();                            // Stop the CCL so changes can be made

  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::feedback;             // feedback
  Logic0.input1 = in::masked;               // masked
  Logic0.input2 = in::masked;               // masked
  Logic0.output = out::enable;              // Enable logic block 0 output pin on PA3 (non-tiny) or PA4 (tiny)
  Logic0.filter = filter::filter;           // Filter - 4 clock-cycle delay each transition
  Logic0.truth = 0x01;                      // Set truth table: Invert, HIGH if input0 LOW
  Logic0.init();                            // Initialize logic block 0


  /* Event0 - EVSYS CHANNEL0 */
  Event0.set_generator(gen::ccl0_out);      // Use output of Logic0
  Event0.set_user(user::tcb0_cnt);          // Connect Event0 (carrying Logic0 output) to TCB0 count
  Event0.start();                           // Enable Event0


  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false

  /* Event1 - EVSYS CHANNEL1 */
  Event1.stop();                            // Not Used. Stop Event1 (if it was running).

  Logic::start();                           // Start the CCL hardware


  /* TCD0 - Timer/Counter Type D */
  TCD0.CTRLA = 0;                           // Stop the timer, clear CTRLA.
  // This puts it into the state we describe in the TCDThirdPWM example.
  TCD0.CTRLA = (TCD_CNTPRES_DIV4_gc | TCD_SYNCPRES_DIV8_gc | TCD_CLKSEL_OSCHF_gc);
  // In Arduino-land, usually OSCHF = CLK_PER unless external crystal or clock
  // is used or the "safe operating area" forces one to use lower voltage, but
  // that's not a thing on the AVR Dx-series.
  while (!(TCD0.STATUS & TCD_ENRDY_bm));     // Wait until ENRDY (Enable Ready)
  TCD0.CTRLA |= TCD_ENABLE_bm;              // Re-enable TCD0

  // Start some PWM so we can see the change in frequency 1.46 kHz is pushing it
  // if controlling a big beefy power MOSFET (like, say, an MCU90N02)
  Serial.println("Before: ~1.46 kHz @ 24 MHz OSCHF/CLK_PER");
  analogWrite(PIN_PA6, 128);                 // Output 50% duty cycle on PIN_PA6
  delay(5000);


  /* TCD0 - Timer/Counter Type D */
  TCD0.CTRLA &= ~TCD_ENABLE_bm;             // Stop the timer
  TCD0.CTRLA &= ~TCD_CLKSEL_gm;             // Clear the clksel bits (though they weren't set by default)
  TCD0.CTRLA |= TCD_CLKSEL_EXTCLK_gc;       // Use external clock
  while (!(TCD0.STATUS & TCD_ENRDY_bm));     // Wait until ENRDY (Enable Ready)
  TCD0.CTRLA |= TCD_ENABLE_bm;              // Re-enable TCD0
  Serial.println("After: ~183 Hz");

  // Obviously, 183 Hz is too slow for almost any purpose... however, that's /32 and /64 prescaling!
  // If we went back to a sync prescale of /2 instead of /8 within the TCD0 peripheral,
  // we would have 750 Hz output, with option to double it  or halve it by changing
  // sync prescale, all while keeping the period of the "real" PWM channels and that of the third
  // "fake" PWM made from PROGEV equal to each other - which is just what I wanted.

}

void demo10() {
  /* Another route to scaled clocks on event channel: TCD+PLL+CCL
   *
   * The idea of this is straightforward - less "weird" than the other approach, really.
   * Though it uses a precious resource:
   *       Your type D timer.
   *
   * The Logic blocks can take the Waveform Output of the timers as inputs, and output it unchanged.
   *
   * All you need to do is run the timer at a given speed....Using the PLL helps
   * in terms of getting the resolution you need to divide it down.
   * The dither mode could also help, if the consumer of the clock cycles can handle the jitter.
   *
   */
  Logic::stop();                            // Stop the CCL so changes can be made


  /* CLKCTRL - Clock Controller */
  // Enable the PLL -
  _PROTECTED_WRITE(CLKCTRL_PLLCTRLA, CLKCTRL_MULFAC_2x_gc);


  /* TCD0 - Timer/Counter Type D */
  TCD0.CTRLA = 0;                           // Stop the timer, clear CTRLA.
  // Turn off all prescaling
  TCD0.CTRLA = (TCD_CNTPRES_DIV1_gc | TCD_SYNCPRES_DIV1_gc | TCD_CLKSEL_PLL_gc);
  uint8_t period = 18;                      // 48 MHz / 18 = 2.66667 MHz
  TCD0.CMPBSET = (period / 2) - 1;              // Switch the output halfway through a cycle
  TCD0.CMPBCLR = period - 1;                  // 0 is a count, so we subtract 1 from each
  while (!(TCD0.STATUS & TCD_ENRDY_bm));     // Wait until ENRDY (Enable Ready)
  TCD0.CTRLA |= TCD_ENABLE_bm;              // Re-enable TCD0



  /* Logic0 - CCL LUT0 */
  Logic0.enable = true;                     // Enable logic block 0
  Logic0.input0 = in::masked;               // masked - this would be WOA, we want WOB
  Logic0.input1 = in::tcd;                  // TCD0 WOB
  Logic0.input2 = in::masked;               // masked
  Logic0.clocksource = clocksource::clk_per;// Nothing clock-dependent here.
  Logic0.output = out::enable;              // Enable logic block 0 output pin PA3 or PA4 (ATtiny))
  Logic0.filter = filter::disable;          // No need for filter
  Logic0.truth = 0x04;                      // Set truth table: Copy, HIGH if input1 HIGH
  Logic0.init();                            // Initialize logic block 0



  /* Event0 - EVSYS CHANNEL0 */
  Event0.stop();                            // Stop Event0 (if it was running)
  Event0.set_generator(gen::ccl0_out);      // Use output of Logic1
  Event0.set_user(user::tcb0_cnt);          // Connect Event0 (carrying Logic0 output) to TCB0 COUNT
  Event0.start();                           // Enable Event0


  /* Logic1 - CCL LUT1 */
  Logic1.enable = false;                    // Not using Logic1
  Logic1.init();                            // Initialize logic block 1 to apply the enable=false

  /* Event1 - EVSYS CHANNEL1 */
  Event1.stop();                            // Not Used. Stop Event1 (if it was running).


  /* TCB0 - Timer/Counter Type B */
  analogWrite(PIN_PF4, 128);                         // Output some pwm to demo frequency
  TCB0.CTRLA = 0;                                   // Disable TCB0
  TCB0.CTRLA = TCB_CLKSEL_EVENT_gc | TCB_ENABLE_bm; // Switch to event clock & enable
  Serial.println("After: ~10.4 kHz");               // 24*2 / 18 = 2.667 MHz, 2.667 MHzz / 255 count/cycle = ~10.4 kHz




  Logic::start();                           // Start the CCL hardware
}




void loop() {
  demo1(); // Async - timer feeding back on itself reaching mindboggling speeds.
  Serial.println("Async: Single logic block, feedback - 100-110 MHz");
  delay(20000);
  Serial.println("Async: out -> second logic block -> link - 50-55 MHz");
  demo2();
  delay(20000);
  Serial.println("Async: out -> event -> input - 50-55 MHz");
  demo3();
  delay(20000);
  Serial.println("Async: out -> event -> second lut -> link - 33-37 MHz");
  demo4();
  delay(20000);
  Serial.println("Clock dividing: Introducing synchronizer - 2 clocks/edge");
  demo5();
  delay(20000);
  Serial.println("Clock dividing: Filter - 4 clocks/edge");
  demo6();
  delay(20000);
  Serial.println("Clock dividing: One logic block into another");
  demo7();
  delay(20000);
  Serial.println("Clock dividing: One logic block clocked from another");
  demo8();
  delay(20000);
  Serial.println("Divided clocks: TCA0 - n");
  demo9a();
  delay(20000);
  TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;

  #ifdef TCB_CLKSEL2_bm // Only parts with the third CLKSEL bit have event clock
  Serial.println("Divided clocks: TCB gets independent prescaler! Dx/2-series only");
  demo9b();
  delay(10000);
  digitalWrite(PIN_PF4, 0);
  pinMode(PIN_PF4, INPUT);
  #endif

  #if defined(SHOW_TCD_DEMO) && defined(TCD0)
  Serial.println("Divided clocks: TCD pre-prescaler. Dx/1-series only");
  demo9d();
  delay(10000);
  digitalWrite(PIN_PA6, 0);
  pinMode(PIN_PA6, INPUT);
  #endif

  #if defined(TCD0) && defined(TCB_CLKSEL2_bm)
  Serial.println("Clock dividing: TCD+PLL->CCL->Event->TCB0 ");
  demo10();
  delay(10000);
  #endif

}
