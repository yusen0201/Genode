SOPLEX_VERSION = 2.0.0
SOPLEX         = soplex-$(SOPLEX_VERSION)
SOPLEX_TGZ     = $(SOPLEX).tgz
SOPLEX_URL     = http://soplex.zib.de/download/release/$(SOPLEX_TGZ)
SOPLEX_MD5     = dd7098b9c24bb980cc2e90f396730f25

#
# Interface to top-level prepare Makefile
#
PORTS += $(SOPLEX)

prepare-soplex: $(CONTRIB_DIR)/$(SOPLEX) include/soplex

$(CONTRIB_DIR)/$(SOPLEX):clean-soplex

#
# Port-specific local rules
#
$(DOWNLOAD_DIR)/$(SOPLEX_TGZ):
	$(VERBOSE)wget -c -P $(DOWNLOAD_DIR) $(SOPLEX_URL) && touch $@

$(DOWNLOAD_DIR)/$(SOPLEX_TGZ).verified: $(DOWNLOAD_DIR)/$(SOPLEX_TGZ)
	$(VERBOSE)$(HASHVERIFIER) $(DOWNLOAD_DIR)/$(SOPLEX_TGZ) $(SOPLEX_MD5) md5
	$(VERBOSE)touch $@

$(CONTRIB_DIR)/$(SOPLEX): $(DOWNLOAD_DIR)/$(SOPLEX_TGZ).verified
	$(VERBOSE)tar xfz $(<:.verified=) -C $(CONTRIB_DIR) && touch $@

SOPLEX_INCLUDES = soplex.h

include/soplex:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(SOPLEX_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(SOPLEX)/src/$$i $@; done

clean-soplex:
	$(VERBOSE)rm -rf include/soplex
	$(VERBOSE)rm -rf $(CONTRIB_DIR)/$(SOPLEX)
