
include $(TOPDIR)/rules.mk

PKG_NAME:=ated
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/ated
  SECTION:=ated
  CATEGORY:=rt2880_app
  TITLE:=rt2880_app ated program
endef

define Package/ated/description
 	rt2880_app ated program
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
define Package/ated/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ated $(1)/usr/bin/ated
endef

$(eval $(call BuildPackage,ated))

