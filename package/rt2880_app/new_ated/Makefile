
include $(TOPDIR)/rules.mk

PKG_NAME:=new_ated
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)


include $(INCLUDE_DIR)/package.mk

define Package/new_ated
  SECTION:=new_ated
  CATEGORY:=rt2880_app
  TITLE:=rt2880_app ated program
endef

define Package/new_ated/description
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

define Package/new_ated/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) ./files/ated $(1)/usr/bin/
endef

$(eval $(call BuildPackage,new_ated))

