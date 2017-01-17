
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= justfortest.c

LOCAL_MODULE:= justfortest

LOCAL_SHARED_LIBRARIES := libutils libcutils

include $(BUILD_EXECUTABLE)
