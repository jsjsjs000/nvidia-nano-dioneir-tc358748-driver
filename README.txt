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


	# from Dione IR documentation
DISPLAY=:0.0 gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,formAt=BGRA,width=640,height=480 ! videoconvert ! video/x-raw,format=NV12 ! nvvidconv ! nvoverlaysink sync=false

	# Lattice Crosslink FPGA as IMX219 - works
DISPLAY=:0.0 gst-launch-1.0 nvarguscamerasrc ! 'video/x-raw(memory:NVMM), width=3280, height=2464, format=(string)NV12, framerate=(fraction)20/1' ! nvoverlaysink -e
DISPLAY=:0.0 gst-launch-1.0 nvarguscamerasrc ! 'video/x-raw(memory:NVMM), width=640, height=480, format=(string)NV12, framerate=(fraction)120/1' ! nvoverlaysink -e

--------------------------------------------

apt-get install v4l-utils

v4l2-ctl --list-formats-ext
#> BGRA

v4l2-ctl --all --device /dev/video0

v4l2-compliance -d /dev/video0

v4l2-ctl -V -d /dev/video0

v4l2-ctl --stream-mmap --stream-to=file.raw --stream-count=1
v4l2-ctl --device /dev/video0 --set-fmt-video=width=640,height=480,pixelformat=RGBA --stream-mmap --stream-to=file.raw --stream-count=1


v4l2-ctl --set-fmt-video=width=3280,height=2464,pixelformat=AR24 --stream-mmap --stream-count=1 -d /dev/video0 --stream-to=image.raw
https://github.com/xenicsir/dione_mipi_tegra/tree/main/jetpack

--------------------------------------------

PCO
IP: 192.168.3.12
Gateway: 192.168.3.10
DNS: 10.0.2.10






-------------------- log IMX477 - works ok ------------------------
$$$$

-------------------- log crosslink imx219 imitation - not works ------------------------
Setting pipeline to PAUSED ...
Pipeline is live and does not need PREROLL ...
Setting pipeline to PLAYING ...
New clock: GstSystemClock
GST_ARGUS: Creating output stream
CONSUMER: Waiting until producer is connected...
GST_ARGUS: Available Sensor modes :
GST_ARGUS: 3264 x 2464 FR = 21,000000 fps Duration = 47619048 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 3264 x 1848 FR = 28,000001 fps Duration = 35714284 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1920 x 1080 FR = 29,999999 fps Duration = 33333334 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1640 x 1232 FR = 29,999999 fps Duration = 33333334 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1280 x 720 FR = 59,999999 fps Duration = 16666667 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1280 x 720 FR = 120,000005 fps Duration = 8333333 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: Running with following settings:
   Camera index = 0 
   Camera mode  = 0 
   Output Stream W = 3264 H = 2464 
   seconds to Run    = 0 
   Frame Rate = 21,000000 
GST_ARGUS: Setup Complete, Starting captures for 0 seconds
GST_ARGUS: Starting repeat capture requests.
CONSUMER: Producer has connected; continuing.
nvbuf_utils: dmabuf_fd -1 mapped entry NOT found
nvbuf_utils: Can not get HW buffer from FD... Exiting...
CONSUMER: ERROR OCCURRED
ERROR: from element /GstPipeline:pipeline0/GstNvArgusCameraSrc:nvarguscamerasrc0: CANCELLED
Additional debug info:
Argus Error Status
EOS on shutdown enabled -- waiting for EOS after Error
Waiting for EOS...

-------------------- log crosslink imx219 imitation - not works ------------------------
Setting pipeline to PAUSED ...
Pipeline is live and does not need PREROLL ...
Setting pipeline to PLAYING ...
New clock: GstSystemClock
GST_ARGUS: Creating output stream
CONSUMER: Waiting until producer is connected...
GST_ARGUS: Available Sensor modes :
GST_ARGUS: 3264 x 2464 FR = 21,000000 fps Duration = 47619048 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 3264 x 1848 FR = 28,000001 fps Duration = 35714284 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1920 x 1080 FR = 29,999999 fps Duration = 33333334 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1640 x 1232 FR = 29,999999 fps Duration = 33333334 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1280 x 720 FR = 59,999999 fps Duration = 16666667 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: 1280 x 720 FR = 120,000005 fps Duration = 8333333 ; Analog Gain range min 1,000000, max 10,625000; Exposure Range min 13000, max 683709000;
GST_ARGUS: Running with following settings:
   Camera index = 0 
   Camera mode  = 0 
   Output Stream W = 3264 H = 2464 
   seconds to Run    = 0 
   Frame Rate = 21,000000 
GST_ARGUS: Setup Complete, Starting captures for 0 seconds
GST_ARGUS: Starting repeat capture requests.
CONSUMER: Producer has connected; continuing.
^C

handling interrupt.
Interrupt: Stopping pipeline ...
EOS on shutdown enabled -- Forcing EOS on the pipeline
Waiting for EOS...
Got EOS from element "pipeline0".
EOS received - stopping pipeline...
Execution ended after 0:10:20.270086638
Setting pipeline to PAUSED ...
Setting pipeline to READY ...
GST_ARGUS: Cleaning up
CONSUMER: Done Success
GST_ARGUS: Done Success
Setting pipeline to NULL ...
Freeing pipeline ...
