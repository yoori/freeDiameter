# The app_radgw extension
PROJECT("RADIUS/Diameter extensible gateway application for freeDiameter" C)


# Overwrite the debug level for the extension code if configured
OPTION(DEBUG_LEVEL_APP_RADGW "Overwrite debug level for the extension app_radgw if defined (valid values: FULL, ANNOYING, CALL...)" OFF)
IF (DEBUG_LEVEL_APP_RADGW)
     ADD_DEFINITIONS(-DTRACE_LEVEL=${DEBUG_LEVEL_APP_RADGW})
ENDIF (DEBUG_LEVEL_APP_RADGW)
IF (NOT CMAKE_BUILD_TYPE MATCHES "Debug")
  MARK_AS_ADVANCED(DEBUG_LEVEL_APP_RADGW)
ENDIF (NOT CMAKE_BUILD_TYPE MATCHES "Debug")


########### Utility library #############
# utility libray for the extension and its plugins
# See rgw_common.h for detail

SET( RG_COMMON_SRC
	radius.c
	md5.c
	rgw_msg_codes.c
	rgw_msg_attrtype.c
)
SET( RG_COMMON_HEADER
	rgw_common.h
	radius.h
	md5.h
	hostap_compat.h
)
ADD_LIBRARY(rgw_common ${RG_COMMON_SRC})



########### Main app_radgw extension #############

# Parser files
BISON_FILE(rgw_conf.y)
FLEX_FILE(rgw_conf.l)
SET_SOURCE_FILES_PROPERTIES(lex.rgw_conf.c rgw_conf.tab.c PROPERTIES COMPILE_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}")

# List of source files
SET( RGW_DEFAULT_SRC
	rgw_main.c
	lex.rgw_conf.c
	rgw_conf.tab.c
	rgw_conf.tab.h
	rgw_clients.c
	rgw_plugins.c
	rgw_msg.c
	rgw_servers.c
	rgw_worker.c
)

# Compile these files as a freeDiameter extension.
FD_ADD_EXTENSION(app_radgw ${RGW_DEFAULT_SRC} ${RG_COMMON_HEADER})
TARGET_LINK_LIBRARIES(app_radgw rgw_common)



########### RADIUS/Diameter translation agent plugins (support for RADIUS protocol) ############
# Use the macro RGWX_ADD_PLUGIN(name files...) to create a plugin.
# It is equivalent to add_library with the appropriate parameters
# and naming conventions (.rgwx : Radius GateWay eXtension)
MACRO(RGWX_ADD_PLUGIN PLGNAME)
  ADD_LIBRARY(${PLGNAME} MODULE ${ARGN})
  SET_TARGET_PROPERTIES(${PLGNAME} PROPERTIES PREFIX "" )
  SET_TARGET_PROPERTIES(${PLGNAME} PROPERTIES SUFFIX ".rgwx" )
  TARGET_LINK_LIBRARIES(${PLGNAME} rgw_common)
  INSTALL(TARGETS ${PLGNAME}
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-radius-gateway)
ENDMACRO(RGWX_ADD_PLUGIN)


### Debug 
# Example of plugin:
OPTION(BUILD_RGWX_SAMPLE "Build sample plugin? (for developers only)" OFF)
 	IF (BUILD_RGWX_SAMPLE)
 	   RGWX_ADD_PLUGIN(sample ${RG_COMMON_HEADER} rgwx_sample.c)
 	ENDIF (BUILD_RGWX_SAMPLE)

# A plugin for debug: dumps RADIUS and Diameter messages state at the time the plugin is called.
OPTION(BUILD_RGWX_DEBUG "Build debug plugin? (display status of RADIUS and Diameter messages)" ON)
 	IF (BUILD_RGWX_DEBUG)
 	   RGWX_ADD_PLUGIN(debug ${RG_COMMON_HEADER} rgwx_debug.c)
 	ENDIF (BUILD_RGWX_DEBUG)


### Authentication, Authorization messages translation.
OPTION(BUILD_RGWX_AUTH "Build Authentication & Authorization RADIUS translation plugin? (RFC2865, RFC3579)" ON)
	IF (BUILD_RGWX_AUTH)
 	   RGWX_ADD_PLUGIN(auth ${RG_COMMON_HEADER} rgwx_auth.c)
	ENDIF (BUILD_RGWX_AUTH)

### Accounting messages translation.
OPTION(BUILD_RGWX_ACCT "Build Accounting RADIUS translation plugin? (RFC2866)" ON)
	IF (BUILD_RGWX_ACCT)
 	   RGWX_ADD_PLUGIN(acct ${RG_COMMON_HEADER} rgwx_acct.c)
	ENDIF (BUILD_RGWX_ACCT)


### Generic plugin to handle some attributes (either delete them or simply echo them in the answer) 
OPTION(BUILD_RGWX_ECHODROP "Build 'echo/drop' plugin? (drop specific RADIUS attributes or echo them in RADIUS answer)" ON)
  	IF (BUILD_RGWX_ECHODROP)
  	   BISON_FILE(rgwx_echodrop.y)
  	   FLEX_FILE(rgwx_echodrop.l)
  	   SET_SOURCE_FILES_PROPERTIES(lex.rgwx_echodrop.c rgwx_echodrop.tab.c PROPERTIES COMPILE_FLAGS "-I ${CMAKE_CURRENT_SOURCE_DIR}")
 	   RGWX_ADD_PLUGIN(echodrop ${RG_COMMON_HEADER} rgwx_echodrop.h rgwx_echodrop.c lex.rgwx_echodrop.c rgwx_echodrop.tab.c rgwx_echodrop.tab.h )
  	ENDIF (BUILD_RGWX_ECHODROP)
	
	
####
## INSTALL section ##

INSTALL(TARGETS app_radgw
	LIBRARY DESTINATION ${INSTALL_EXTENSIONS_SUFFIX}
	COMPONENT freeDiameter-radius-gateway)

# Note that all compiled plugins are added by the RGWX_ADD_PLUGIN macro in the component freeDiameter-radius-gateway as well.