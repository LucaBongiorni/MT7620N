
include $(TOPDIR)/rules.mk

PKG_NAME:=zhishi-web
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

define Package/zhishi-web
  SECTION:=zhishi-web
  CATEGORY:=IBeacon
  TITLE:=zhishi-web
  DEPENDS:=+luci +luci-mod-rpc
endef

define Package/zhishi-web/description
	zhishi-web
endef


define Build/Prepare
	echo
endef

define Build/Compile
	echo
endef

define Package/zhishi-web/install
	$(INSTALL_DIR) $(1)/www/cgi-bin
	$(CP) -rf ./files/* $(1)/
	$(INSTALL_BIN) ./files/www/cgi-bin/* $(1)/www/cgi-bin

endef

$(eval $(call BuildPackage,zhishi-web))

