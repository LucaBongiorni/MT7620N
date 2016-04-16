#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=ramips
BOARDNAME:=Ralink RT288x/RT3xxx
SUBTARGETS:=mt7620n
FEATURES:=squashfs gpio
LINUX_VERSION:=3.10.14
KERNEL_PATCHVER:=3.10

include $(INCLUDE_DIR)/target.mk
DEFAULT_PACKAGES += \
	kmod-leds-gpio kmod-gpio-button-hotplug swconfig luci

define Target/Description
	Build firmware images for Ralink RT288x/RT3xxx based boards.
endef

$(eval $(call BuildTarget))
