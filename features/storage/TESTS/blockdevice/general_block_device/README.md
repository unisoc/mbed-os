# Getting started with the Mbed OS block device test

Mbed OS block device test is used to test existing and new block devices.

You can find more information about the Mbed OS block device and other related pieces of the Mbed OS storage stack [in the storage overview](https://os.mbed.com/docs/latest/reference/storage.html).

**Table of contents:**

1. [Hardware requirements](#hardware-requirements)
2. [Usage](#usage)
   - [Compile the test](#compile-the-test)
   - [Run the test](#run-the-test)
3. [Changing the block device](#changing-the-block-device)
4. [Tested configurations](#tested-configurations)

## Hardware requirements

This test uses a block device as storage. This can be either an external
block device (one of SPI flash, DataFlash or an SD card) or simulated on a
heap block device on boards with enough RAM.

## Usage

#### Compile the test

Invoke `mbed test`, and specify the name of your platform and your favorite
toolchain (`GCC_ARM`, `ARM`, `IAR`). For example, for the ARM Compiler 5:

```
mbed test -m K64F -t ARM -n mbed-os-features-storage-tests-blockdevice-general_block_device --compile
```

#### Run the example

Use `mbed test` again:

```
mbed test -m K64F -t ARM -n mbed-os-features-storage-tests-blockdevice-general_block_device --run
```
   
#### Troubleshooting

If you have problems, you can review the [documentation](https://os.mbed.com/docs/latest/tutorials/debugging.html)
for suggestions on what could be wrong and how to fix it.

## Changing the block device

In Mbed OS, a C++ classes that inherits from the [BlockDevice](https://os.mbed.com/docs/latest/reference/storage.html#block-devices)
interface represents each block device. 
This test uses the default block device received by the function get_default_instance(), it is defined in [SystemStorage.cpp](https://github.com/ARMmbed/mbed-os/blob/master/features/storage/system_storage/SystemStorage.cpp#L35-L77) as MBED_WEAK, in case we would like to test a new block device we will have to override it.

first add the new block device cpp and header files to [blockdevice folder](https://github.com/ARMmbed/mbed-os/tree/master/features/storage/blockdevice), then implement get_default_instance() inside the test main.cpp.

for example, in order to test the HeapBlockDevice add:

``` diff
+#define TEST_BLOCK_SIZE 128
+#define TEST_BLOCK_DEVICE_SIZE 32*TEST_BLOCK_SIZE

+BlockDevice *BlockDevice::get_default_instance()
+{
+    utest_printf("NEW test block device!!!\n");
+    static HeapBlockDevice default_bd(TEST_BLOCK_DEVICE_SIZE, TEST_BLOCK_SIZE);
+    return &default_bd;
+}
```

Now you can recompile and run.

## Tested configurations

- K64F + SD
- K64F + Heap
- K82F + SPIF
- K82F + Heap