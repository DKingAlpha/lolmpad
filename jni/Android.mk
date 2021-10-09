LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := lolmpad
LOCAL_SRC_FILES := lolmpad.cpp padhelper.cpp screenhelper.cpp

include $(BUILD_EXECUTABLE)