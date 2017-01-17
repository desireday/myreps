
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

#ifeq ($(strip $(Module_SDTM_Encryptor)),yes)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= encryptor_sdtm.cpp

LOCAL_MODULE:= encryptor_sdtm

LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder

include $(BUILD_EXECUTABLE)

#endif
