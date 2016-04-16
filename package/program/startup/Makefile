
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=Startup
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

define Package/$(PKG_NAME)
  SECTION:=$(PKG_NAME)
  CATEGORY:=IBeacon
  TITLE:=startup program
endef

define Package/$(PKG_NAME)/description
  startup program
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)"\
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/rc.d/
	
	$(INSTALL_BIN)  ./files/startup $(1)/etc/init.d/
	$(LN) $(1)/etc/init.d/startup $(1)/etc/rc.d/S99startup
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
