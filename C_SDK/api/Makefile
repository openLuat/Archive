## ----------------------------------------------------------- ##
## Don't touch the next line unless you know what you're doing.##
## ----------------------------------------------------------- ##

# Name of the module
LOCAL_NAME := api

# List of submodules which contain code we need to include in the final lib
LOCAL_API_DEPENDS := \

LOCAL_ADD_INCLUDE := \
                    platform/std_inc \
                    platform/OpenAT_inc \
					api/include \

					
# Set this to any non-null string to signal a module which 
# generates a binary (must contain a "main" entry point). 
# If left null, only a library will be generated.
IS_ENTRY_POINT := no

## ------------------------------------ ##
## 	Add your custom flags here          ##
## ------------------------------------ ##
MYCPPFLAGS += -DAM_OPENAT_ROM_BASE=${FLASH_BASE}+${AM_OPENAT_ROM_BASE}
MYCPPFLAGS += -DAM_OPENAT_ROM_SIZE=${AM_OPENAT_ROM_SIZE}

## ------------------------------------- ##
##	List all your sources here           ##
## ------------------------------------- ##
C_SRC := ${notdir ${wildcard src/*.c}}

## ------------------------------------- ##
##  Do Not touch below this line         ##
## ------------------------------------- ##
include ${SOFT_WORKDIR}/platform/compilation/cust_rules.mk