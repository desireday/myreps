
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= IEncryptorRegistrar.cpp IEncryptorHandler.cpp

LOCAL_SHARED_LIBRARIES := libutils libcutils libbinder

LOCAL_MODULE:= libencryptorservice

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
