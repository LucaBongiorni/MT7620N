#
# Copyright (C) 2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define Profile/ZHISHI_Proprietary_Driver
	NAME:=Zhishi_Proprietary_Driver
	PACKAGES:=\
		ralink-wifi -kmod-rt2800-soc \
		kmod-usb-core kmod-usb2 kmod-usb-ohci \
		kmod-ledtrig-usbdev
endef

define Profile/ZHISHI_Proprietary_Driver/Description
	Zhishi Proprietary Wifi Driver
endef
$(eval $(call Profile,ZHISHI_Proprietary_Driver))

define Profile/ZHISHI
	NAME:=Zhishi_OpenSource_Driver
	PACKAGES:=\
		-ralink-wifi kmod-rt2800-soc \
		kmod-usb-core kmod-usb2 kmod-usb-ohci \
		kmod-ledtrig-usbdev
endef

define Profile/ZHISHI/Description
	Zhishi OpenSource Wifi Driver
endef
$(eval $(call Profile,ZHISHI))
