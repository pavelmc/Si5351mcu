# Arduino Si5351 Library tuned for size and click free. #

This library is tuned for size and MCU operation, it will control only CLK0 and CLK1 outputs (for now).

It's click free, with some provisions, keep readding

## Inspiration ##

This work is based on the previous work of these people:

* [www.etherkit.com](https://www.etherkit.com) the full featured lib, with big code as well.
* [QRP Labs demo libs](http://qrp-labs.com/synth/si5351ademo.html) the smallest and simple ones on the net.
* [DK7IH demo code](https://radiotransmitter.wordpress.com/category/si5351a/) the first click-less code on the wild.

## Click-less ##

The click noise while tunning the chip came from the following actions (stated in the datasheet & app notes):

```
1 - Turn CLKx output off.
2 - Reset the PLLx for the new calculations.
3 - Turn CLKx output on.
```

In my code I follow a strategy of just do that at the start of the freq output and move the PLLs and multisynths freely without reseting the PLLs or multisynths outputs.

In my tests I can move across several Mhz (1 to 10, 28 to 150) without getting off frequency at least with +/- 5 Hz (my instrument tolerance).

## The start sequence is important ##

Yes, in your setup() code segment you must initialize it in the following sequence:

* Apply correction factor (if needed)
* Set sweet spots frequencies to **both** clock outputs
* Force a reset of the PLLs.

In the example code you can see this inside the setup function:

```
    // apply my calculated correction factor
    pll.correction(-1250);

    // set some freqs
    pll.setFreq(0, 25000000);       // CLK0 output
    pll.setFreq(1, 145000000);      // CLK1 output

    // force the first reset
    pll.reset();

```

When I write "sweet spots frequencies" I write about some middle frequencies in your specific case.

For example if you use CLK0 for VFO and CLK1 for BFO for a 40m receiver with 10 Mhz IF and upper injection I will suggest this code segment in your setup:

```
    // apply my calculated correction factor
    pll.correction(-1250);

    // set some freqs
    pll.setFreq(0, 17150000);  // VFO 7.150 Mhz (mid band) + 10.0 Mhz
    pll.setFreq(1, 10000000);  // BFO 10.0 Mhz

    // force the first reset
    pll.reset();

```

## Normal operation ##

After the setup process you can setup any freq to any of the two outputs (CLK0 or CLK1) via the setFreq(CLK, FREQ) procedure as usual.

If you get in trouble (freq not in the exact spot, or other weird things) after moving in steps of more than 10 Mhz or in the lower edge of the coverage (below 3 Mhz) just make a **single** reset() and all will be ok again. This just happened one time to me in the dev process so far, in the present code I don't see this effect any more, but I mentioned it "just in case".

If you found a trouble like this I would like to hear about it to try to fix that, this is beta code and is the first iteration, it may contain some bugs or typos.

If you need to apply/vary the correction factor **after** the setup process (like in a own calibration routine) you will get a click noise on the next setFreq() to apply the changes, so, if you do it repeatedly it will have click noise on it, that perfectly normal.

## Author & contributors ##

The only author is Pavel Milanes, CO7WT, reachable at pavelmc@gmail.com, contributors and sponsors are welcomed.

## Where to download the latest version? ##

Always download the latest version from the [github repository](https://github.com/pavelmc/Si5351mcu/)

## If you like to give thanks... ##

No payment of whatsoever is required to use this code: this is [Free/Libre Software](https://en.wikipedia.org/wiki/Software_Libre), nevertheless donations are very welcomed.

I live in Cuba island and the Internet/Cell is very expensive here (USD $1.50/hour), you can donate anonymously internet time or cell phone air time to me via [Ding Topups](https://www.ding.com/) to keep me connected and developing for the homebrew community.

If you like to do so, please go to Ding, select Cuba, select Cubacell (for phone top up) or Nauta (for Internet time)

* For phone topup use this number (My cell, feel free to call me if you like): +53 538-478-19
* For internet time use this user: co7wt@nauta.com.cu (that's not an email but an user account name)

Thanks before hand.
