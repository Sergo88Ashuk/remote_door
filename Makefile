#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := wifi_station

include $(IDF_PATH)/make/project.mk

EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/main/web_app/include
EXTRA_COMPONENT_DIRS += $(PROJECT_PATH)/main/ble_app/include
