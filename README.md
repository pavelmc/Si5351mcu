# Arduino Si5351 Library tuned for size and click noise free. #

This library is tuned for size on the Arduino (MCU) platform, it will control CLK0, CLK1 and CLK2 outputs for the Si5351A (the version with just 3 clocks out).

But there is not such thing as free lunch: please read the *Two of three* section below; and sure it's click free with some provisions, keep reading.

## Inspiration ##

This work is based on the previous work of these great people:

* [Etherkit/NT7S:](https://github.com/etherkit/Si5351Arduino) The mainstream full featured lib, with big code as well.
* [QRP Labs demo code:](http://qrp-labs.com/synth/si5351ademo.html) The smallest and simple ones on the net.
* [DK7IH demo code:](https://radiotransmitter.wordpress.com/category/si5351a/) The first clickless noise code on the wild.

## Features ##

This are so far the implemented features (Any particular wish? use the Issues tab for that):

* Custom XTAL passing on init (default is 27.0MHz)
* You are able to pass a correction to the xtal while running (as for your own calibration procedure)
* You have a fast way to power off all outputs of the Chip.
* You can enable/disable any output at any time.
* You can only have 2 of the 3 outputs running at any moment, see "Two of three" below.
* It's free of click noise while you move on frequency.
* **NEW!**  Power control on each output independently (see setPower(clk, level) on the lib header, initial default is to lowest level: 2mA)

## Click noise free ##

The click noise while tunning the chip came from the following actions (stated in the datasheet & app notes):

```
1 - Turn CLKx output off.
2 - Reset the PLLx for the new calculations.
3 - Turn CLKx output on.
```

In my code I follow a strategy of just do that at the start of the freq output and move the PLLs and multisynths freely without reseting the PLLs or multisynths outputs.

In my tests I can move across several Mhz (1 to 10, 28 to 150) without getting off frequency at least with +/- 5 Hz (my instrument tolerance).

## The start sequence is important ##

Yes, in your setup code segment you must initialize it in the following sequence:

* Apply correction factor (if needed)
* (Optional) power off all outputs
* Set sweet spots frequencies to **both** clock outputs
* Force a reset of the PLLs.

Here you have an example code of what I mean ("Si" is the lib instance): 

```
setup() {
    (... my great code here ...)

    //////////////////////////////////
    // Si5351 functions
    //////////////////////////////////

    // apply my calculated correction factor
    Si.correction(-1250);

    // Optional, power off all outputs until needed
    Si.off();

    // set some sweet spot freqs
    Si.setFreq(0, 25000000);       // CLK0 output
    Si.setFreq(1, 145000000);      // CLK1 output

    // force the first reset
    Si.reset();

    (... other great code here ...)
}

```

When I write "sweet spots frequencies" I write about some middle frequencies in your specific case.

For example if you use CLK0 for VFO and CLK1 for BFO for a 40m receiver with 10 Mhz IF and upper injection I will suggest this code segment in your setup:

```
    // set some sweet spot freqs
    Si.setFreq(0, 17150000);  // VFO 7.150 Mhz (mid band) + 10.0 Mhz
    Si.setFreq(1, 10000000);  // BFO 10.0 Mhz

    // force the first reset
    Si.reset();

```

If you need to apply/vary the correction factor **after** the setup process you will get a click noise on the next setFreq() to apply the changes.

## Two of three ##

Yes, there is a tittle catch here with CLK1 and CLK2: both share PLL_B and as we use math to produce an integer division you can only use one of them at a time.

Note: _In practice you can, but the other will move from the frequency you set, which is odd, so I made them mutually exclusive._

This are the valid combinations for independent clocks output.

* CLK0 and CLK1
* CLK0 and CLK2

Again: You can't use CLK1 and CLK2 at the same time, as soon as you set one of them the other will shut off. That's why you get two of three and one of them must be always CLK0.

## Author & contributors ##

The only author is Pavel Milanes, CO7WT, a cuban amateur radio operator; reachable at pavelmc@gmail.com, Until now I have no contributors or sponsors.

## Where to download the latest version? ##

Always download the latest version from the [github repository](https://github.com/pavelmc/Si5351mcu/)

See ChangeLog.md on this repository to know what are the latest changes.

## If you like to give thanks... ##

No payment of whatsoever is required to use this code: this is [Free/Libre Software](https://en.wikipedia.org/wiki/Software_Libre), nevertheless donations are very welcomed.

I live in Cuba island and the Internet/Cell is very expensive here (USD $1.50/hour), you can donate anonymously internet time or cell phone air time to me via [Ding Topups](https://www.ding.com/) to keep me connected and developing for the homebrew community.

If you like to do so, please go to Ding, select Cuba, select Cubacell (for phone top up) or Nauta (for Internet time)

* For phone topup use this number (My cell, feel free to call me if you like): +53 538-478-19
* For internet time use this user: co7wt@nauta.com.cu (that's not an email but an user account name)

Thanks!
