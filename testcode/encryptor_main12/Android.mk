
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= encryptor_main1.cpp EncryptorRegistrar.cpp

LOCAL_MODULE:= encryptor_main1

LOCAL_SHARED_LIBRARIES := libbinder libutils libcutils libencryptorservice

include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= encryptor_main2.cpp EncryptorHandler.cpp

LOCAL_MODULE:= encryptor_main2

LOCAL_SHARED_LIBRARIES := libbinder libutils libcutils libencryptorservice

include $(BUILD_EXECUTABLE)
