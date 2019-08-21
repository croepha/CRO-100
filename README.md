This is a work-in-progress TIS-100 compatible computer, designed from the 
ground up from scratch, in a way that evokes contemporary ~1975 engineering.
This includes a circuit design for the computer, eventually some associated 
source code for the machine code assembler, and FLASH writing tool.

This is a personal leisure project, and contributions here aren't welcome for 
at this time, but feel free to fork.  I mostly put this on github so that people
on my stream can use it as reference.

# GOAL #

The initial goal was to create the system using components that were available
in 1975 (the year the fictional computer was supposed to be released), however,
that was abandoned because it would have been two or three times more
expensive for me to actually build it that way.  For example a modern one 
megabyte flash on Digikey can be purchased for ~$1.50, but the smallest non-
volatile memory is a 16kb NVSRAM that is $8. In 1975 8Kb EPROMS were the
largest available.  Also the TTL based logic chips have largely been replaced
by CMOS components.  The older components are more expensive, simply because
they just aren't produced in large numbers like the more modern components.


# BACKGROUND #

I had seen Ben Eater's series on YouTube where he goes over the design of his
8-bit computer that he built on breadboards from generic logic ICs.  I found
the series very compelling, and I had to binge watch every episode.  I knew
that I had to build a computer from scratch myself.  However, I wanted to do
something different.  I didn't simply want to copy his design, so I thought it
would be cool to try to build a physical version of the TIS-100.  The TIS-100
is a fictional computer featured in a game by Zachtronics of the same name,

At this point I should say that I am in no way endorsed or affiliated with
Zachtronics, and this is solely a hobby project done for educational and 
recreational purposes...


# CONSTRAINTS #

 - Must execute CRO-100 machine code, (compiled from TIS-100 compatible ASM)
 - Can load in pre-compiled machine code from external serial source
 - NO FPGAs
 - NO micro-controllers or LSI CPUs
 - ROMs/Flashes are OK for microcode or combinational logic
 - Cheaper CMOS components are OK
 - Large RAMs are OK
 - SMT components are OK, because they are much cheaper

The TIS-100 as featured in the game is actually much more than just a
bunch of CPU cores.  It apparently has an integrated assembler, text editor,
debugger and graphical terminal.  I tried hard to actually self host these
things but the TIS-100 architecture is simply too limited for this to be 
possible.  Conceivably if we made significant ISA changes like adding a RAM
module, more nodes, and the ability to page in and out instructions we could
probably have enough processing power to have an integrated text editor and 
assembler.  And if we coupled that with some hardware to power a video display
or simply a serial output for a vt100 or similar terminal we could probably be
really close to having a full TIS-100-like machine similar to the one featured
in the game. However, at that point it's a clear break from the original 
constraints.

So, in order to satisfy these constraints, I think it is acceptable for the
computer to be able to simply read pre-compiled machine code in over some kind
of serial connection.  This serial connection could possibly be fed from a
cassette tape.  But for practical purposes we can just use an Arduino or
Raspberry PI as a cassette tape emulator.

# MORE RESOURCES #

 - Learn about Zachtronics and their games here: http://www.zachtronics.com/
 - Ben Eater's 8-bit breadboard computer: https://www.youtube.com/playlist?list=PLowKtXNTBypGqImE405J2565dvjafglHU
 - I occasionally work on this and other projects live on Twitch: https://www.twitch.tv/croepha
 - I sometimes also announce things on Twitter: https://twitter.com/croepha


# POTENTIAL FURTHER WORK #

 - If I win the lottery, I might be interested in making a version done
 in entirely 1975 era components: through-hole 74LS...., smaller rams, and
 discrete logic gates, instead of ROMs.
 - It might be cool to extend the ISA so that it could more fully self-hosting


# FAQ #

Q: How is this different from Ben Eater's 8-bit computer?

A:
- I am trying to implement a specific ISA, Ben is trying to make a simple computer
- I use CMOS components (cheaper)
- Large flashes instead of ROMs (cheaper)
- 11 bit computer, to support TIS-100 -999...999
- No micro-instructions, one instruction per cycle unless blocking on IO or
one extra cycle when writing to an IO port (for TIS-100 cycle compatibility)
- My clock circuit has 1 555 timer + 1 diode, Ben's has 3 555s and 2 logic ICs
for muxing circuit 
- Multi-core(nodes)
- Blocking IO with neighbor nodes
- Only one general purpose register per node, there is also a backup register
- More complicated program-counter.  With support for relative jumps and
bounds checking, automatic looping 
- Loads program in from tape, not DIP switches

![vid0](https://cdn.discordapp.com/attachments/404399251276169217/613442872108253189/video0.mov)
![img1](https://cdn.discordapp.com/attachments/404399251276169217/612218508113215498/image0.jpg)
![img0](https://cdn.discordapp.com/attachments/404399251276169217/611803958545088523/image0.jpg)

