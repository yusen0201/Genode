SQLITE_VERSION = 3.8.4.3
SQLITE         = sqlite-amalgamation-3080403
SQLITE_ZIP     = $(SQLITE).zip
SQLITE_URL     = https://sqlite.org/2014/$(SQLITE_ZIP)
SQLITE_MD5     = fc3cfdb642a14f4c69f6cdb082ec2bbb

#
# Interface to top-level prepare Makefile
#
PORTS += $(SQLITE)

prepare-sqlite3: $(CONTRIB_DIR)/$(SQLITE) include/sqlite3

$(CONTRIB_DIR)/$(SQLITE):clean-sqlite

#
# Port-specific local rules
#
$(DOWNLOAD_DIR)/$(SQLITE_ZIP):
	$(VERBOSE)wget -c -P $(DOWNLOAD_DIR) $(SQLITE_URL) && touch $@

$(DOWNLOAD_DIR)/$(SQLITE_ZIP).verified: $(DOWNLOAD_DIR)/$(SQLITE_ZIP)
	$(VERBOSE)$(HASHVERIFIER) $(DOWNLOAD_DIR)/$(SQLITE_ZIP) $(SQLITE_MD5) md5
	$(VERBOSE)touch $@

$(CONTRIB_DIR)/$(SQLITE): $(DOWNLOAD_DIR)/$(SQLITE_ZIP).verified
	$(VERBOSE)unzip $(<:.verified=) -d $(CONTRIB_DIR) && touch $@
	$(VERBOSE)patch -d $(CONTRIB_DIR)/$(SQLITE) -p1 -i $(CURDIR)/src/lib/sqlite/min_file_descriptor.patch

SQLITE_INCLUDES = sqlite3.h sqlite3ext.h

include/sqlite3:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(SQLITE_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(SQLITE)/$$i $@; done

clean-sqlite:
	$(VERBOSE)rm -rf include/sqlite3
	$(VERBOSE)rm -rf $(CONTRIB_DIR)/$(SQLITE)
