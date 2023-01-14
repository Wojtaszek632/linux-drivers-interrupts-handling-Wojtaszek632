# Linux drivers lab4 tasks - "Interrupts handling"

## Implement interrupt handling

### Extend the base driver with interrupt handling functionality:

Based on the materials from the last lecture and Linux documentation extend the driver code with:

* memory space mapping
* accessing device registers
* character device API for controlling the device
* interrupt handling

We'll use [Renode's model](https://github.com/renode/renode-infrastructure/blob/master/src/Emulator/Peripherals/Peripherals/GPIOPort/LiteX_GPIO.cs) of [LiteX GPIO](https://github.com/enjoy-digital/litex/blob/master/litex/soc/cores/gpio.py) controller.
Use the [REPL file](renode/scripts/platform.repl) shared in the repository.
The peripheral is accessible on the system bus at address `0xF000B000`, its interrupt is connected to IRQ3 of the PLIC interrupt controller.

Tasks:

* Update the device tree to provide info about the interrupts
* Implement the logic counting the interrupts caught by the driver
* read function should get the counter
* Use completion for handling the data read (read should wait for the interrupt)
* Implement IOCTL resetting the counter to a given value

  * Remember to guard the increment and reset with spinlock

* Implement an app to test the flow
Hint: you can use `watch` command in Renode to trigger irqs periodically

LiteX GPIO register map:

```
0x0 - GPIO state
0x0c - interrupt status
0x10 - interrupt pending
0x14 - interrupt enable
```

