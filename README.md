# water-filter-sensor
Indicator light to show the water fill level within a ceramic water filter

## Concept
We use a [ceramic water filter](https://en.wikipedia.org/wiki/Ceramic_water_filter) at home for our drinking water and ice-making. Normally we try to keep it full, so there's always about 2L available at any time. However to be sure we haven't over-filled, we have to remove the top section to see how much is left in the bottom. Obviously, this minor effort is totally unacceptable, so I built a sensor system to tell us how full the tank is.

PHOTO OF THE FINISHED THING HERE

## Overview
The water filter now sits on top of a mounting frame, under which there is a HX-711 compatible weight sensor. Alongside the base of the filter, an RGB LED displays different colours to indicate fill level or warning states.

DIAGRAM OF THE SYSTEM HERE

## Materials required
Electronics

* 1 [Arduino nano](https://www.amazon.de/gp/product/B01C9J7NGS/ref=ppx_yo_dt_b_asin_title_o02__o00_s00?ie=UTF8&psc=1)
* 1 HX-711 compatible [load sensor](https://www.amazon.de/gp/product/B075KKH416/ref=ppx_yo_dt_b_asin_title_o03__o00_s00?ie=UTF8&psc=1)
* 1 [RGB LED](https://www.conrad.de/de/makerfactory-led-modul-vma307-passend-fuer-arduino-boards-arduino-arduino-uno-fayaduino-freeduino-seeeduino-see-1612767.html)
* [Ribbon cable](https://www.amazon.de/gp/product/B076CLY8NH/ref=oh_aui_detailpage_o00_s00?ie=UTF8&psc=1)
* [Jumper pin connectors](https://www.amazon.de/gp/product/B01MRSUEHD/ref=oh_aui_detailpage_o01_s00?ie=UTF8&psc=1)
* 2 [mini breadboards](https://www.amazon.de/gp/product/B01M9CHKO4/ref=oh_aui_detailpage_o01_s00?ie=UTF8&psc=1)

Other:

  * Soldering iron
  * 3rd hand
  * Small screws
  * Wooden boards
  * Drill and assorted bits
  * Various small pliers, screwdriver, common tools

## Code
See the full Arduino C++ code in [water-filter-sensor.ino](water-filter-sensor.ino). 

Dependencies:
 * [movingAvg](https://github.com/JChristensen/movingAvg)
 * [Q2HX711](https://github.com/queuetue/Q2-HX711-Arduino-Library)

## Circuit

![Circuit diagram](docs/water-filter_bb.jpg "Circuit diagram")

This can also be viewed in the [Fritzing](http://fritzing.org/) software using the [water-filter.fzz](water-filter.fzz) file. 
