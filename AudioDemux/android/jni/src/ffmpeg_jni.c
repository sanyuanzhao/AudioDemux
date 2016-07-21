#include "demux.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <assert.h>

jint native_start_decode(JNIEnv* env, jobject thiz, jstring strAudioUrl, jstring strAudioDest)
{
	const char* audioUrl = (*env)->GetStringUTFChars(env, strAudioUrl, JNI_FALSE);
	const char* audioDest = (*env)->GetStringUTFChars(env, strAudioDest, JNI_FALSE);
	const char* args[4] = { "", "hao.mp3", "/sdcard/hao.yuv", "hao.pcm" };
	
	args[1] = audioUrl;
	args[3] = audioDest;
	int ret = demux(4, args);
	(*env)->ReleaseStringUTFChars(env, strAudioUrl, (const char *)audioUrl);
	(*env)->ReleaseStringUTFChars(env, strAudioDest, (const char *)audioDest);
	return ret;
}

void native_stop_decode(JNIEnv* env, jobject thiz) 
{

}

/**
* ������Ӧ��
*/
static JNINativeMethod gMethods[] = {
	{ "native_start_decode", "(Ljava/lang/String;Ljava/lang/String;)I", (void*)native_start_decode},//��
	{ "native_stop_decode", "()V", (void*)native_stop_decode},//��
};

/*
* Ϊĳһ����ע�᱾�ط���
*/
static int registerNativeMethods(JNIEnv* env
	, const char* className
	, JNINativeMethod* gMethods, int numMethods) {
	jclass clazz;
	clazz = (*env)->FindClass(env, className);
	if (clazz == NULL) {
		return JNI_FALSE;
	}
	if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
		return JNI_FALSE;
	}

	return JNI_TRUE;
}


/*
* Ϊ������ע�᱾�ط���
*/
static int registerNatives(JNIEnv* env) {
	const char* kClassName = "com/sy/syplayer/decoder/SYDecoder";//ָ��Ҫע�����
	return registerNativeMethods(env, kClassName, gMethods,
		sizeof(gMethods) / sizeof(gMethods[0]));
}

/*
* System.loadLibrary("lib")ʱ����
* ����ɹ�����JNI�汾, ʧ�ܷ���-1
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		return -1;
	}
	assert(env != NULL);

	if (!registerNatives(env)) {//ע��
		return -1;
	}
	//�ɹ�
	result = JNI_VERSION_1_4;

	return result;
}
