SCIP_VERSION = 3.1.0
SCIP         = scip-$(SCIP_VERSION)
SCIP_TGZ     = $(SCIP).tgz
SCIP_URL     = http://scip.zib.de/download/release/$(SCIP_TGZ)
SCIP_MD5     = a1a2b08ec8711c478f37a803eb99a716

#
# Interface to top-level prepare Makefile
#
PORTS += $(SCIP)

prepare-scip: $(CONTRIB_DIR)/$(SCIP) include/scip include/blockmemshell include/nlpi include/lpi

$(CONTRIB_DIR)/$(SCIP):clean-scip

#
# Port-specific local rules
#
$(DOWNLOAD_DIR)/$(SCIP_TGZ):
	$(VERBOSE)wget -c -P $(DOWNLOAD_DIR) $(SCIP_URL) && touch $@

$(DOWNLOAD_DIR)/$(SCIP_TGZ).verified: $(DOWNLOAD_DIR)/$(SCIP_TGZ)
	$(VERBOSE)$(HASHVERIFIER) $(DOWNLOAD_DIR)/$(SCIP_TGZ) $(SCIP_MD5) md5
	$(VERBOSE)touch $@

$(CONTRIB_DIR)/$(SCIP): $(DOWNLOAD_DIR)/$(SCIP_TGZ).verified
	$(VERBOSE)tar xfz $(<:.verified=) -C $(CONTRIB_DIR) && touch $@
	$(VERBOSE)patch -d $(CONTRIB_DIR)/$(SCIP) -p1 -i $(CURDIR)/src/lib/scip/clock.patch

SCIP_INCLUDES = scip.h def.h \
					 presolve.h \
					 intervalarith.h \
					 scipshell.h \
					 scipdefplugins.h \
					 disp.h \
					 misc.h \
					 vbc.h

SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/type_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/pub_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/struct_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/branch_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/cons_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/disp_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/heur_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/nodesel_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/presol_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/prop_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/reader_*.h))
SCIP_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/scip/sepa_*.h))

include/scip:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(SCIP_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(SCIP)/src/scip/$$i $@; done

BLOCKMEMSHELL_INCLUDES = memory.h

include/blockmemshell:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(BLOCKMEMSHELL_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(SCIP)/src/blockmemshell/$$i $@; done

NLPI_INCLUDES = nlpi.h \
					 intervalarithext.h \
					 exprinterpret.h \
					 nlpi_ipopt.h \
					 nlpioracle.h
NLPI_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/nlpi/type_*.h))
NLPI_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/nlpi/pub_*.h))
NLPI_INCLUDES+= $(notdir $(wildcard $(CONTRIB_DIR)/$(SCIP)/src/nlpi/struct*.h))


include/nlpi:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(NLPI_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(SCIP)/src/nlpi/$$i $@; done

LPI_INCLUDES = lpi.h type_lpi.h

include/lpi:
	$(VERBOSE)mkdir -p $@
	$(VERBOSE)for i in $(LPI_INCLUDES); do \
		ln -sf ../../$(CONTRIB_DIR)/$(SCIP)/src/lpi/$$i $@; done

clean-scip:
	$(VERBOSE)rm -rf include/scip
	$(VERBOSE)rm -rf include/blockmemshell
	$(VERBOSE)rm -rf include/nlpi
	$(VERBOSE)rm -rf include/lpi
	$(VERBOSE)rm -rf $(CONTRIB_DIR)/$(SCIP)
