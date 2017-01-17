
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#ifeq ($(strip $(Module_SDTM_Encryption)),yes)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= encryption-max.cpp

LOCAL_MODULE:= encryption-max

LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder

include $(BUILD_EXECUTABLE)

#endif

