
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=DIY_Firewall
PKG_USER_NAME:=ctrlFirewall
PKG_RELEASE:=0.0.001

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Other modules
  TITLE:=DIY_Firewall driver
  FILES:=$(PKG_BUILD_DIR)/DiyFirewall.ko
  #AUTOLOAD:=$(call AutoLoad,98,DiyFirewall)
  DEPENDS:=+libc
endef  

define KernelPackage/$(PKG_NAME)/description  
  Kernel module to DIY_Firewall  
endef

define Package/$(PKG_USER_NAME)
  SECTION:=ctrlFirewall
  CATEGORY:=IBeacon
  TITLE:=controll kernel program diyFirewall
  DEPENDS:=+kmod-DIY_Firewall
endef

define Package/$(PKG_USER_NAME)/description
	The user program to controll kernel program diyFirewall.
endef


PKG_KCONFIG := CONFIG_DIY_Firewall=m

define Build/Prepare
	$(call Build/Prepare/Default)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	$(CP) ./src/tool.* $(PKG_BUILD_DIR)/ctrlFirewall/
endef  
  
define Build/Compile 
    $(MAKE) -C "$(LINUX_DIR)" \
    	ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		M="$(PKG_BUILD_DIR)" \
		CONFIG_DIY_FIREWALL=m \
		SUBDIRS="$(PKG_BUILD_DIR)" \
		$(foreach c,$(PKG_KCONFIG),$(if $(CONFIG_$c),CONFIG_$(c)=y)) \
		modules

	$(MAKE) -C $(PKG_BUILD_DIR)/ctrlFirewall \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef  

define Package/$(PKG_USER_NAME)/install
	$(INSTALL_DIR) $(1)/usr/ibeacon/bin
	$(INSTALL_DIR) $(1)/usr/ibeacon/conf
	$(INSTALL_DIR) $(1)/usr/ibeacon/tool

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ctrlFirewall/ctrlFirewall $(1)/usr/ibeacon/bin/
	$(INSTALL_CONF) ./files/*.conf $(1)/usr/ibeacon/conf/
	$(INSTALL_CONF) ./files/index.html $(1)/usr/ibeacon/conf/
	$(INSTALL_BIN)  ./files/guardIbeacon.sh $(1)/usr/ibeacon/tool/
endef
 
$(eval $(call KernelPackage,DIY_Firewall))
$(eval $(call BuildPackage,ctrlFirewall))

