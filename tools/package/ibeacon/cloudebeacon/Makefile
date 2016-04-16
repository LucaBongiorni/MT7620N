
include $(TOPDIR)/rules.mk

PKG_NAME:=CloudeBeacon
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_BUILD_DEPENDS:=libopenssl

include $(INCLUDE_DIR)/package.mk

define Package/CloudeBeacon
  SECTION:=CloudeBeacon
  CATEGORY:=IBeacon
  TITLE:=cloudeBeacon program
  DEPENDS:=+libstdcpp +libm +libpthread +libubox +libuci +libopenssl
endef

define Package/CloudeBeacon/description
 	cloudeBeacon program
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

define Package/CloudeBeacon/install
	$(INSTALL_DIR) $(1)/usr/ibeacon/bin/
	$(INSTALL_DIR) $(1)/usr/ibeacon/conf/
	$(INSTALL_DIR) $(1)/usr/ibeacon/tool/
	$(INSTALL_DIR) $(1)/usr/ibeacon/protfile/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/rc.d/
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cloudebeacon $(1)/usr/ibeacon/bin/
	
	$(INSTALL_DATA) ./files/cbProb.conf $(1)/usr/ibeacon/conf/
	$(INSTALL_BIN)  ./files/libssl.so.1.0.0 $(1)/lib/
	$(INSTALL_BIN)  ./files/libcrypto.so.1.0.0 $(1)/lib/
	$(INSTALL_BIN)  ./files/startup $(1)/etc/init.d/
	$(INSTALL_BIN)  ./files/guardIbeacon.sh $(1)/usr/ibeacon/tool/
	$(LN) $(1)/etc/init.d/startup $(1)/etc/rc.d/S99startup
endef

$(eval $(call BuildPackage,CloudeBeacon,+libopenssl))
