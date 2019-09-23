# attiny13_eos-UART

ATTiny13 - Cannon EOS Shutter Timer

---

Main goal :
Set Focus and Interval by UART [8N1, 19200, see uart.h] - Android/iOS Phone, Serial USB Terminal + OTG + CH340
Start loop.

Feature (due to memory availability after 1st version :) ) :

J1 [PB1] Jumper Active   = GoTo Loop(Timer) w/ default values = I5, F1  /  Inactive = GoTo CLI

CLI Commands :

f    = Show Focus

       Return : F0/1

f0   = Set No Focus

       Return : F0

f1   = Set Focus before Shutter

       Return : F1

i XX = Set interval to XX second

       Return : IXX

s    = Start Loop

       Return : GO

Unknown commands

       Rerurn : ERR

---

Arduino IDE settings:

   ATtiny13
   1.2MHz internal
   LTO enabled
   BOD 1.8V
   
---

(c) 2019 Petr KLOSKO
 
 UART code and CLI based on 
      ATtiny13/021 -  Simple text CLI (Command Line Interface) via UART.
      by Łukasz Podkalicki <lpodkalicki@gmail.com>
      [https://blog.podkalicki.com/100-projects-on-attiny13/]
