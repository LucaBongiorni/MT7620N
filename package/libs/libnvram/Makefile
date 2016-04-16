include $(TOPDIR)/rules.mk

PKG_NAME:=libnvram
PKG_RELEASE:=1
PKG_VERSION:=1.1.0

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/libnvram
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Libraries needs by ated
  DEPENDS:=+libz
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		libnvram.a libnvram.so.$(PKG_VERSION)
endef


define Package/libnvram/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnvram.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libnvram.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libnvram))
