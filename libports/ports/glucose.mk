GLUCOSE_VERSION = 3.0
GLUCOSE         = glucose-$(GLUCOSE_VERSION)
GLUCOSE_TGZ     = $(GLUCOSE).tgz
GLUCOSE_URL     = http://www.labri.fr/perso/lsimon/downloads/softwares/$(GLUCOSE_TGZ)
GLUCOSE_MD5     = 96fe40e5f2b85bf2b22b8c88cd0b1be1

#
# Interface to top-level prepare Makefile
#
PORTS += $(GLUCOSE)

prepare-glucose: $(CONTRIB_DIR)/$(GLUCOSE) include/glucose/core include/glucose/mtl include/glucose/utils

$(CONTRIB_DIR)/$(GLUCOSE):clean-glucose

#
# Port-specific local rules
#
$(DOWNLOAD_DIR)/$(GLUCOSE_TGZ):
	$(VERBOSE)wget -c -P $(DOWNLOAD_DIR) $(GLUCOSE_URL) && touch $@

$(DOWNLOAD_DIR)/$(GLUCOSE_TGZ).verified: $(DOWNLOAD_DIR)/$(GLUCOSE_TGZ)
	$(VERBOSE)$(HASHVERIFIER) $(DOWNLOAD_DIR)/$(GLUCOSE_TGZ) $(GLUCOSE_MD5) md5
	$(VERBOSE)touch $@

$(CONTRIB_DIR)/$(GLUCOSE): $(DOWNLOAD_DIR)/$(GLUCOSE_TGZ).verified
	$(VERBOSE)tar xfz $(<:.verified=) -C $(CONTRIB_DIR) && touch $@
	$(VERBOSE)patch -d $(CONTRIB_DIR)/$(GLUCOSE) -p2 -i $(CURDIR)/src/lib/glucose/c++11.patch
	$(VERBOSE)patch -d $(CONTRIB_DIR)/$(GLUCOSE) -p2 -i $(CURDIR)/src/lib/glucose/pow.patch

GLUCOSE_CORE_INCLUDES  = $(notdir $(wildcard $(CONTRIB_DIR)/$(GLUCOSE)/core/*.h))
GLUCOSE_MTL_INCLUDES   = $(notdir $(wildcard $(CONTRIB_DIR)/$(GLUCOSE)/mtl/*.h))
GLUCOSE_UTILS_INCLUDES = $(notdir $(wildcard $(CONTRIB_DIR)/$(GLUCOSE)/utils/*.h))

include/glucose/core:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(GLUCOSE_CORE_INCLUDES); do \
		ln -sf ../../../$(CONTRIB_DIR)/$(GLUCOSE)/core/$$i $@; done

include/glucose/mtl:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(GLUCOSE_MTL_INCLUDES); do \
		ln -sf ../../../$(CONTRIB_DIR)/$(GLUCOSE)/mtl/$$i $@; done

include/glucose/utils:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(GLUCOSE_UTILS_INCLUDES); do \
		ln -sf ../../../$(CONTRIB_DIR)/$(GLUCOSE)/utils/$$i $@; done

clean-glucose:
	$(VERBOSE)rm -rf include/glucose
	$(VERBOSE)rm -rf $(CONTRIB_DIR)/$(GLUCOSE)
