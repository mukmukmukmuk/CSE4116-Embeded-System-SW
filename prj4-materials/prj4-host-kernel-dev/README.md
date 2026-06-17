# Project#4 - Task 1. Enabling mkfs.ext4 on Cosmos+ OpenSSD Platform
*Sogang University – Embedded Systems Software (CSE4116, Prof. Youngjae Kim)*

---

Since Linux kernel versions later than 4.x (roughly 2019 onward), creating an ext4 file system on the Cosmos+ OpenSSD device fails by default. Your task is to analyze this behavior, modify the kernel, rebuild it, and make `mkfs.ext4` work correctly.

To avoid breaking the lab environment, you will **build and install a new kernel** *alongside* the existing one and select it from GRUB during boot.

---

## Lab Policy (Important)

- **DO NOT modify or remove the existing lab PC's kernel (e.g., v5.15.139)**
- Build and install a **new custom kernel only (see below)** 
- If anything goes wrong, you can always boot back into the original kernel

## Step-by-Step Instructions

### 0. Install Required Packages for Kernel Development (One-Time)

```bash
sudo apt update
sudo apt install -y build-essential libncurses-dev bison flex libssl-dev libelf-dev dwarves git gcc-9 g++-9 zstd
````

### 1. Download Linux Kernel v5.15.190 Source and Setup the Environment (One-Time)

This lab uses **Linux kernel version 5.15.190** as the reference version.
All students should follow the steps below using this kernel source to ensure a consistent development and evaluation environment.

```bash
cd ~
wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.15.190.tar.xz
tar xf linux-5.15.190.tar.xz
cd linux-5.15.190
cp /boot/config-$(uname -r) .config
scripts/config --disable X86_X32
scripts/config --enable MODULES
scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
scripts/config --set-str LOCALVERSION "-openssd"
unset ARCH
unset CROSS_COMPILE
make ARCH=x86_64 CC=gcc-9 olddefconfig
````

* `cp /boot/config-$(uname -r) .config` initializes the kernel configuration using the current system kernel settings, ensuring that required drivers and subsystems remain enabled.

* `scripts/config --set-str LOCALVERSION "-openssd"` assigns a custom local version suffix, resulting in the kernel name 5.15.190-openssd. This makes the lab kernel clearly distinguishable from the original system kernel in GRUB and simplifies recovery if needed.

### 2. Shrink the Kernel Config with modprobed-db (One-Time)

A full build takes 1–1.5 hours. `modprobed-db` keeps only the drivers this PC actually uses, cutting the build to minutes.

```bash
cd ~
git clone https://github.com/graysky2/modprobed-db.git
cd modprobed-db && make && sudo make install
modprobed-db init
modprobed-db store
cd ~/linux-5.15.190
make LSMOD=$HOME/.config/modprobed.db ARCH=x86_64 CC=gcc-9 localmodconfig
```

> Press **Enter** through any prompts.

### 3. Re-enable the OpenSSD Drivers (One-Time, REQUIRED)

`localmodconfig` drops drivers not loaded right now — including the OpenSSD's USB-serial (UART) and NVMe. Re-enable them, or your new kernel will boot but won't see the board.

```bash
# Run in ~/linux-5.15.190 . Do NOT `make clean` first.
scripts/config --module CONFIG_USB_SERIAL
scripts/config --module CONFIG_USB_SERIAL_GENERIC
scripts/config --module CONFIG_USB_SERIAL_FTDI_SIO
scripts/config --module CONFIG_USB_SERIAL_CP210X
scripts/config --enable CONFIG_NVME_CORE
scripts/config --module CONFIG_BLK_DEV_NVME
make ARCH=x86_64 CC=gcc-9 olddefconfig
```

### 4. Build and Install the New Kernel

```bash
sudo make ARCH=x86_64 CC=gcc-9 -j"$(nproc)"
sudo make ARCH=x86_64 CC=gcc-9 modules_install
sudo make ARCH=x86_64 CC=gcc-9 install
sudo update-grub
````

This step will:
- Build and install the new kernel in /boot
- Add a new GRUB menu entry
- Keep the original kernel unchanged

With the trimmed config from Steps 2–3, this build is much smaller than a full kernel build. Still, allow sufficient time for the first build to complete.

### 5. Reboot and Select the Lab Kernel

```bash
sudo reboot
````

During boot:
- **Choose Advanced options for Ubuntu**
- Select:
  > Ubuntu, with Linux 5.15.190-openssd

### 6. Verify Kernel Version After Boot

```bash
uname -r
````

Expected output:

> 5.15.190-openssd

### 7. Modify Kernel Source Files

> Before starting any kernel modification, **read `PATCH_GUIDE.md` carefully**.
> You must initialize Git and manage your changes according to the workflow described in that document.

At this stage, **inspect and modify the Linux kernel source as needed** to enable successful execution of `mkfs.ext4` on the OpenSSD device. The necessary changes require careful analysis of the kernel behavior and its interaction with the storage device.

After modifications, repeat Step 4 to Step 6, building and verifying the new kernel.

---

## Recovery (If Something Goes Wrong)

If the system fails to boot or behaves unexpectedly:

1. Reboot the system
2. Enter GRUB
3. Select the original kernel (e.g., 5.15.0-XXX-generic)

The system will return to its original safe state.

---

## (Optional) Rebuilding Only the NVMe Driver 

After completing the above lab Kernel setup and determining that no further changes beyond the NVMe driver are necessary, it is not required to rebuild the entire kernel. In this case, the NVMe kernel driver module can be rebuilt and reloaded directly.

### 1. Identify the OpenSSD PCIe Address

Check the PCIe address of the OpenSSD device:

```bash
lspci | grep Xilinx
````

Example:

> 01:00.0 Non-Volatile memory controller: Xilinx Corporation Device 7028

### 2. Modify and Build the NVMe Driver

```bash
cd ~/linux-5.15.190/drivers/nvme/host
# Modify source files as needed
make -C ~/linux-5.15.190 M=$(pwd) modules
````

### 3. Install and Reload the NVMe Module

```bash
sudo cp nvme.ko /lib/modules/5.15.190-openssd/kernel/drivers/nvme/host/
sudo depmod -a
sudo modprobe -r nvme
sudo modprobe nvme
````

### 4. Rebind the OpenSSD Device (If Necessary)

```bash
echo "0000:01:00.0" | sudo tee /sys/bus/pci/drivers/nvme/unbind
echo "nvme" | sudo tee /sys/bus/pci/devices/0000:01:00.0/driver_override
echo "0000:01:00.0" | sudo tee /sys/bus/pci/drivers_probe
````

Verify the driver binding:

> sudo lspci -vvv -s 01:00.0
> ...
> Kernel driver in use: nvme

Notes
* **This method applies only when kernel core changes are not required**

---

