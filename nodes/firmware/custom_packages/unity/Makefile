PKG_NAME=unity
PKG_URL=https://github.com/ThrowTheSwitch/Unity.git
PKG_VERSION=cbcd08fa7de711053a3deec6339ee89cad5d2697 # v 2.6.1
PKG_LICENSE=MIT

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/src -f $(CURDIR)/Makefile.unity
