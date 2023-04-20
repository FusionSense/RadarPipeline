# Data File Structure

## adc_data

    1. Stores one frame of raw adc data in .bin format.
    2. type float 16
    3. 786,432 total samples per .bin file
        --> 512 * 64 * 4 * 3 * 2
        --> (#Fast_time * #Slow_time * #RX * #TX * #IQ)

## rdm_data

    1. Stores one frame of reshaped adc data in .bin format.
    2. type float 8
    3. 393,216 total samples per .bin file
        --> 512 * 64 * 4 * 3
        --> (#Fast_time * #Slow_time * #RX * #TX) the IQ samples have been combined