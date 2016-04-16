
include $(TOPDIR)/rules.mk

PKG_NAME:=update
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/update
  SECTION:=update
  CATEGORY:=IBeacon
  TITLE:=update ibeacon program
  DEPENDS:=+libstdcpp +libm +libpthread
endef

define Package/update/description
 	ibeacon update program
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

#CROSS_COMPILE=$(CROSS_PATH)bin/mipsel-openwrt-linux-
define Package/update/install
	$(INSTALL_DIR) $(1)/usr/ibeacon/bin/
	$(INSTALL_DIR) $(1)/usr/ibeacon/conf/
	$(INSTALL_DIR) $(1)/usr/ibeacon/tool/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/update $(1)/usr/ibeacon/bin/
	
	$(INSTALL_DATA) ./files/ibeaconUpdate.conf $(1)/usr/ibeacon/conf/
	$(INSTALL_BIN)  ./files/updateIBeacon.sh $(1)/usr/ibeacon/tool/
endef

$(eval $(call BuildPackage,update))
