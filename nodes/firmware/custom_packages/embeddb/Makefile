PKG_NAME=embeddb
PKG_URL=https://github.com/ubco-db/EmbedDB.git
PKG_VERSION=efad0b8e8e2061e278350eb82a6a1f85628ae659 # v 1.0
PKG_LICENSE=BSD-3-Clause

include $(RIOTBASE)/pkg/pkg.mk

all:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/lib/Distribution -f $(CURDIR)/Makefile.embeddb
