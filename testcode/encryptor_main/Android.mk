
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= encryptor_main.cpp

LOCAL_MODULE:= encryptor_main

LOCAL_SHARED_LIBRARIES := libutils libcutils

include $(BUILD_EXECUTABLE)
