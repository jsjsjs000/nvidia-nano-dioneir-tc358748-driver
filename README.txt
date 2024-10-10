# -------------------- Device Tree - Kernel setup --------------------
code ~/l4t-gcc/Linux_for_Tegra/source/public/hardware/nvidia/platform/t210/porg/kernel-dts/Makefile
# ----------------------------------------
dtbo-$(CONFIG_ARCH_TEGRA_210_SOC) += tegra210-p3448-all-p3449-0000-tc358748.dtbo
# ----------------------------------------

code ~/l4t-gcc/Linux_for_Tegra/source/public/hardware/nvidia/platform/t210/porg/kernel-dts/tegra210-p3448-0000-p3449-0000-b00.dts
# ----------------------------------------
#include "porg-platforms/tegra210-camera-xenics-dual-dione-ir.dtsi"
# ----------------------------------------

# -------------------- Device Tree backup --------------------
backup=~/nvidia-nano-dioneir-tc358748-driver
mkdir -p $backup/dts/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/hardware/nvidia/platform/t210/porg/kernel-dts/tegra210-p3448-all-p3449-0000-camera-dione-ir-dual.dts $backup/dts/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms/tegra210-camera-xenics-dual-dione-ir.dtsi $backup/dts/

# -------------------- Device Tree restore --------------------
backup=~/nvidia-nano-dioneir-tc358748-driver
cp $backup/dts/tegra210-p3448-all-p3449-0000-camera-dione-ir-dual.dts ~/l4t-gcc/Linux_for_Tegra/source/public/hardware/nvidia/platform/t210/porg/kernel-dts/
cp $backup/dts/tegra210-camera-xenics-dual-dione-ir.dtsi              ~/l4t-gcc/Linux_for_Tegra/source/public/hardware/nvidia/platform/t210/porg/kernel-dts/porg-platforms/


# -------------------- Driver - Kernel setup --------------------
code ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/Makefile
# ----------------------------------------
dione-objs := dioneir.o tc358746_calculation.o dioneir_tc358748_i2c.o
obj-$(CONFIG_VIDEO_DIONEIR)	+= dione.o
# ----------------------------------------

code ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/Kconfig
# ----------------------------------------
config VIDEO_DIONEIR
	tristate "Toshiba TC358748 parallel to CSI-2 converter - DioneIR"
	depends on VIDEO_V4L2 && I2C && VIDEO_V4L2_SUBDEV_API
	---help---
	  Support for the Toshiba TC358748 parallel to MIPI CSI-2 bridge.

	  To compile this driver as a module, choose M here: the
	  module will be called tc358748.
# ----------------------------------------

# -------------------- Driver backup --------------------
backup=~/nvidia-nano-dioneir-tc358748-driver
mkdir -p $backup/driver/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/dioneir.c $backup/driver/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/dioneir_tc358748_i2c.c $backup/driver/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/dioneir_tc358748_i2c.h $backup/driver/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/tc358746_regs.h $backup/driver/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/tc358746_calculation.h $backup/driver/

# -------------------- Driver restore --------------------
backup=~/nvidia-nano-dioneir-tc358748-driver
cp $backup/driver/dioneir.c           ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/
cp $backup/driver/dioneir_tc358748_i2c.c           ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/
cp $backup/driver/dioneir_tc358748_i2c.h           ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/
cp $backup/driver/tc358746_regs.h           ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/
cp $backup/driver/tc358746_calculation.h           ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/

# -------------------- Backup patch --------------------
mkdir -p $backup/patch/
cp ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/0001-regmap-add-formats.patch $backup/patch/

# -------------------- Apply patch --------------------
$$$

# -------------------- Compile --------------------
c; l4tm modules && scp build/drivers/media/i2c/dione.ko root@192.168.3.12:/lib/modules/4.9.253-tegra/kernel/drivers/media/i2c/
c; l4tm dtbs && scp build/arch/arm64/boot/dts/tegra210-p3448-0000-p3449-0000-b00.dtb root@192.168.3.12:/boot/ && scp build/arch/arm64/boot/dts/tegra210-p3448-all-p3449-0000-camera-dione-ir-dual.dtbo root@192.168.3.12:/boot/

	# nVidia
depmod

dtsfilename=$(tr -d '\0' < "/proc/device-tree/nvidia,dtsfilename")
dtbfilename=$(basename -s .dts "$dtsfilename")

cp "/boot/$dtbfilename.dtb" "/boot/dtb/kernel_$dtbfilename.dtb"
cp "/boot/$dtbfilename.dtb" "/boot/kernel_$dtbfilename.dtb"

# equivalent of $ sudo /opt/nvidia/jetson-io/jetson-io.py
fdtoverlay -i "/boot/dtb/kernel_$dtbfilename.dtb" -o "/boot/kernel_$dtbfilename-dione-ir.dtb" /boot/tegra210-p3448-all-p3449-0000-camera-dione-ir-dual.dtbo

python3 - << PYTHON_EOF
import sys
sys.path.append('/opt/nvidia/jetson-io')
from Linux.extlinux import add_entry
add_entry('/boot/extlinux/extlinux.conf',
         'Jetson-DioneIR', 'Custom Header Config: <DioneIR>',
         '/boot/kernel_$dtbfilename-dione-ir.dtb',
         True)
PYTHON_EOF

reboot

	# nVidia after reboot
dmesg | grep -i 'dione\|tc358'

DISPLAY=:0.0 gst-launch-1.0 nvarguscamerasrc ! 'video/x-raw(memory:NVMM), width=640, height=480, format=(string)RGBx, framerate=(fraction)60/1' ! nvoverlaysink -e

DISPLAY=:0.0 gst-launch-1.0 nvarguscamerasrc ! 'video/x-raw(memory:NVMM), width=640, height=480, format=(string)RGB, framerate=(fraction)60/1' ! nvoverlaysink -e

--------------------------------------------

apt-get install v4l-utils

v4l2-ctl --list-formats-ext
#> BGRA

v4l2-ctl --all --device /dev/video0

v4l2-ctl --stream-mmap --stream-to=file.raw --stream-count=1
v4l2-ctl --device /dev/video0 --set-fmt-video=width=640,height=480,pixelformat=RGBA --stream-mmap --stream-to=file.raw --stream-count=1
--------------------------------------------

PCO
IP: 192.168.3.12
Gateway: 192.168.3.10
DNS: 10.0.2.10
