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
obj-$(CONFIG_VIDEO_TC358748)	+= tc358748.o
# ----------------------------------------

code ~/l4t-gcc/Linux_for_Tegra/source/public/kernel/kernel-4.9/drivers/media/i2c/Kconfig
# ----------------------------------------
config VIDEO_TC358748
	tristate "Toshiba TC358748 parallel to CSI-2 converter"
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
