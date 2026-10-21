CHIL Library for instruments and dataloggers (`chillib`) for use with STM32L433 and SAMD21 microcontroller projects.

**Drivers**:

* CTi TILT-05 inclinometer ([datasheet](https://ctisensors.com/Documents/TILT-05-Datasheet.pdf)).
* Keller 7L(HP)D pressure sensors ([webpage](https://keller-pressure.com/en/products/pressure-transmitters/oem-pressure-transmitters/series-7ld)).
* LSM303AGR e-Compass ([webpage](https://www.st.com/en/mems-and-sensors/lsm303agr.html).
* TMP117 temperature sensor ([datasheet](https://www.ti.com/lit/ds/symlink/tmp117.pdf)).

**CHIL instrumentation library**:

* `chil_instrument.h` instrument status, battery voltage and electrical conductivity sensing.
* `packets.h` definitions of 2026 CHIL data protocol blocks and functions to compile blocks.
