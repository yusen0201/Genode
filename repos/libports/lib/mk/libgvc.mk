GRAPHVIZ_DIR = $(call select_from_ports,graphviz)/src/lib
LIBS    += libc expat
INC_DIR += $(GRAPHVIZ_DIR)/gvc
INC_DIR += $(GRAPHVIZ_DIR)/common
INC_DIR += $(GRAPHVIZ_DIR)/fdpgen
INC_DIR += $(GRAPHVIZ_DIR)/pathplan
INC_DIR += $(GRAPHVIZ_DIR)/cgraph
INC_DIR += $(GRAPHVIZ_DIR)/cdt
INC_DIR += $(GRAPHVIZ_DIR)/xdot
INC_DIR += $(GRAPHVIZ_DIR)/../
INC_DIR += $(REP_DIR)/src/lib/graphviz
SRC_C   = gvrender.c gvlayout.c gvdevice.c gvloadimage.c \
			 gvcontext.c gvjobs.c gvevent.c gvplugin.c gvconfig.c \
			 gvtextlayout.c gvusershape.c gvbuffstderr.c gvc.c \
			 memory.c textspan.c emit.c \
			 input.c xdot.c globals.c labels.c utils.c \
			 shapes.c geom.c arrows.c colxlate.c \
			 taper.c htmltable.c splines.c intset.c pointset.c \
			 htmlparse.c htmllex.c ns.c psusershape.c timing.c \
			 ellipse.c util.c 

CC_OPT += -DHAVE_CONFIG_H

vpath %.c $(GRAPHVIZ_DIR)/gvc
vpath %.c $(GRAPHVIZ_DIR)/common
vpath xdot.c $(GRAPHVIZ_DIR)/xdot
vpath util.c $(GRAPHVIZ_DIR)/pathplan

#SHARED_LIB = yes
