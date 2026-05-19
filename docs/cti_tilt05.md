# Development notes for CTI TILT-05 sensor
Nominal output from the sensor is of the form:

```
$CSTLT,AXN,AYN,AZN,αX,αY,T*CC<CR><LF>
```

or 

```
$CSACC,AX,AY,Az*CC<CR><LF>
```

Usually these come in sequence and can be transmitted up to 100 Hz.

For an update rate of 100 Hz and at message lengths of 31 and 19 characters, these messages take a minimum of 3.47 ms to transmit. This means there *should* be a quiet time of up to 6.5 ms per message at the highest data rate.

These messages are transmitted from power on if the sensor is in continuous transmit mode. It appears that if the sensor is in single message mode, that the sensor transmits the following on power-up.

```
\0
Accelerometer Communication Passed.
```

## Power-up state 
