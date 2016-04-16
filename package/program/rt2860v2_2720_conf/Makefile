
USE_SRC=$(shell ls ./src/ 2>/dev/null >/dev/null && echo 1)

include $(TOPDIR)/rules.mk

PKG_NAME:=WiFi-rt2860V2
PKG_RELEASE:=2720

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=ralink
  CATEGORY:=IBeacon
  TITLE:=Ralink APSoC WiFi SoftAP driver
  DEPENDS:=@TARGET_ramips +wireless-tools +maccalc
endef

define Package/$(PKG_NAME)/description
	Ralink APSoC WiFi driver for DreamBox
endef

#ifneq ($(USE_SRC),)
#define Build/Prepare
#	mkdir -p $(PKG_BUILD_DIR)
#	$(CP) ./src/* $(PKG_BUILD_DIR)/
#	$(Build/Patch)
#	$(if $(QUILT),touch $(PKG_BUILD_DIR)/.quilt_used)
#endef

#define Build/Compile
#	cd $(LINUX_DIR); \
#		ARCH=mips CC=$(TARGET_CC) \
#		$(MAKE) M=$(PKG_BUILD_DIR)/rt2860v2_ap/ V=1 modules
#endef

#else

define Build/Prepare
	echo
endef

define Build/Compile
	echo
endef

#endif

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/modules/$(LINUX_VERSION) $(1)/etc/Wireless/RT2860/
	$(CP) -rf ./files/* $(1)/
	$(CP) modules/* $(1)/lib/modules/$(LINUX_VERSION)
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

