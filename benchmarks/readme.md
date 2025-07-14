Integration tests and benchmarks are incomplete. See individual unit tests and benchmarks.

## Managing CPU scaling

Disable CPU scaling to decrease noise in benchmark measurements.

```shell
# check options
cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_governors

# set to performance (disable)
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
  echo performance | sudo tee $cpu > /dev/null
done

# set to powersave (enable)
for cpu in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do
  echo powersave | sudo tee $cpu > /dev/null
done

# or
sudo cpupower frequency-set -g performance
```
