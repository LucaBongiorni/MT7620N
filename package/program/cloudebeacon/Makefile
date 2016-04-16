
include $(TOPDIR)/rules.mk

PKG_NAME:=CloudBeacon
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_BUILD_DEPENDS:=libopenssl

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=$(PKG_NAME)
  CATEGORY:=IBeacon
  TITLE:=cloudbeacon program
  DEPENDS:=+libstdcpp +libm +libpthread +libubox +libuci +libopenssl
endef

define Package/$(PKG_NAME)/description
 	cloudBeacon program
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

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/ibeacon/bin/
	$(INSTALL_DIR) $(1)/usr/ibeacon/conf/
	$(INSTALL_DIR) $(1)/usr/ibeacon/tool/
	$(INSTALL_DIR) $(1)/usr/ibeacon/protfile/
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/cloudbeacon $(1)/usr/ibeacon/bin/
	
	$(INSTALL_DATA) ./files/cbProb.conf $(1)/usr/ibeacon/conf/
	$(INSTALL_DATA) ./files/ScanFormats.xml $(1)/usr/ibeacon/conf/
	$(INSTALL_BIN)  ./files/guardIbeacon.sh $(1)/usr/ibeacon/tool/
	$(INSTALL_BIN)  ./files/getSerials.sh $(1)/usr/ibeacon/tool/
	$(INSTALL_BIN)  ./files/updateCB.sh $(1)/usr/ibeacon/tool/
	$(INSTALL_DATA) ./files/firewareType $(1)/usr/ibeacon/conf/
endef

$(eval $(call BuildPackage,$(PKG_NAME),+libopenssl))
