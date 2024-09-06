# Purpose.

Some tools for examining the clock resolutions of various clocks, and setting the TAI offset.

See [A Talk About Time](https://docs.google.com/presentation/d/1Pk2wumkxQWnhFoO3-7KvcZ-Wgj-t7g9u9Qmtj8r29hc/edit?usp=sharing) for the presentation on time this code was written for.

Only works on Linux.
Only tested on Debian 12, amd64.

## Compiling.

### clock_resolutions

```
$ gcc -g -Wall clock_cres.c -o cres -lrt
```

### set_tai_offset

- You need to have libcurl-dev installed.
- On Debian-based systems, you can install it with `sudo apt install libcurl4-gnutls-dev`.

```
$ gcc -g -Wall tai_offset.c -o tai_offset -lrt -lcurl
```


## Usage.

### clock_resolutions

For "reasons", needs to be run as root to see the hardware clock.

All other functions will work as a normal user.

```
$ ./cres
CLOCK_REALTIME:
	precision: 1ns
	value    : 1605430987.000000000
...
```
### tai_offset

Needs to be run as root when setting the TAI offset.

When run with -s, will read the ntp leapseconds file from the [IANA](https://data.iana.org/time-zones/tzdb/leap-seconds.list), add the 9 seconds for leap
seconds between 1958 (TAI epoch) and 1970 (UNIX epoch), and set the offset.

When run with a manual offset, you must calculate the full offset yourself,
including those 9 seconds.


```
$ ./tai_offset 
Usage: ./tai_offset <offset|-r|-s>

$ sudo ./tai_offset -r
TAI offset set successfully to 28.

$ ./tai_offset -s
Current TAI offset is 28.

$ sudo ./tai_offset 999
TAI offset set successfully to 999.

$ ./tai_offset 50000
adjtimex: Operation not permitted
```

## Known Issues.

- Does not account for negative leap seconds. Since there never has been one, and leap seconds are being phased out, this shouldn't be a problem.
- Has limited error checking.
