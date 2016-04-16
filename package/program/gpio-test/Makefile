
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=Gpio-test
PKG_RELEASE:=001

define KernelPackage/$(PKG_NAME)
  SUBMENU:=Other modules
  TITLE:=GPIO test driver
  FILES:=$(PKG_BUILD_DIR)/gpio-test.ko
endef  

define KernelPackage/$(PKG_NAME)/description  
  Kernel module to GPIO test
endef

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
 
$(eval $(call KernelPackage,$(PKG_NAME)))

