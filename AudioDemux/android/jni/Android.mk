LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE:= libffmpeg_jni
LOCAL_SRC_FILES:= $(LOCAL_PATH)/src/ffmpeg_jni.c \
                   $(LOCAL_PATH)/../../src/demuxing_decoding.c
				   
LOCAL_C_INCLUDES:= $(LOCAL_PATH)/include \
                   $(LOCAL_PATH)/../../include \
				   
FFPMEG_LIB:=$(LOCAL_PATH)\lib

#如果是非系统的第三方库，貌似只能用LOCAL_LDFLAGS方式，LOCAL_LDLIBS方式不行
#LOCAL_LDLIBS:= -llog
LOCAL_LDLIBS += -L$(FFPMEG_LIB) -lavcodec-57 \
                 -L$(FFPMEG_LIB) -lavfilter-6 \
				 -L$(FFPMEG_LIB) -lavformat-57 \
				 -L$(FFPMEG_LIB) -lavutil-55 \
				 -L$(FFPMEG_LIB) -lswscale-4 \
				 -L$(FFPMEG_LIB) -lswresample-2
				 
#LOCAL_LDFLAGS += -L$(FFPMEG_LIB) -lavcodec-57 \
#                -L$(FFPMEG_LIB) -lavfilter-6 \
#				 -L$(FFPMEG_LIB) -lavformat-57 \
#				 -L$(FFPMEG_LIB) -lavutil-55 \
#				 -L$(FFPMEG_LIB) -lswscale-4 \
#				 -L$(FFPMEG_LIB) -lswresample-2
							
include $(BUILD_SHARED_LIBRARY)
