#include "audio_demux.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <jni.h>
#include <assert.h>

/*
public static class AudioProperty {
	private int mSample = 0;
	private int mChannel = 0;
	private int mEncode = 0;
	private int mDuration = 0;
}*/

jint native_audio_open_demux(JNIEnv* env, jobject thiz, jstring strAudioUrl, jobject oAudioProperty)
{
	const char* url = (*env)->GetStringUTFChars(env, strAudioUrl, 0);
	AudioProperty sAudioProperty = { 0 };
	int sessionId = audio_open_demux(url, &sAudioProperty);

	jclass jclazz = (*env)->GetObjectClass(env, oAudioProperty);
	jfieldID fildIdChannel = (*env)->GetFieldID(env, jclazz, "mChannel", "I");
	jfieldID fildIdSample = (*env)->GetFieldID(env, jclazz, "mSample", "I");
	jfieldID fildIdEncode = (*env)->GetFieldID(env, jclazz, "mEncode", "I");
	jfieldID fildIdDuration = (*env)->GetFieldID(env, jclazz, "mDuration", "J");
	if (fildIdChannel != NULL)
	{
		(*env)->SetIntField(env, oAudioProperty, fildIdChannel, sAudioProperty.nChannel);
	}
	if (fildIdSample != NULL)
	{
		(*env)->SetIntField(env, oAudioProperty, fildIdSample, sAudioProperty.nSample);
	}
	if (fildIdEncode != NULL)
	{
		(*env)->SetIntField(env, oAudioProperty, fildIdEncode, sAudioProperty.nEncode);
	}
	if (fildIdDuration != NULL)
	{
		(*env)->SetLongField(env, oAudioProperty, fildIdDuration, sAudioProperty.nDuration);
	}

	return sessionId;
}

jint native_audio_read_data_decoded(JNIEnv* env, jobject thiz, jint sessionId, jbyteArray buffer, jint size)
{
	AudioOperation* pnAudioOperation = (AudioOperation*)sessionId;
	unsigned char* pBuffer = (*env)->GetByteArrayElements(env, buffer, 0);
	int read = 0;
	read = audio_read_package(pnAudioOperation);
	read = read < size ? read : size;
	if (read > 0)
	{
		memcpy(pBuffer, pnAudioOperation->decoded_data, read);
	}
	(*env)->ReleaseByteArrayElements(env, buffer, pBuffer, 0);
	return read;
}

jint native_audio_seek(JNIEnv* env, jobject thiz, jint sessionId, jint seek)
{

}

jint native_audio_close_demux(JNIEnv* env, jobject thiz, jint sessionId)
{
	audio_close_demux((AudioOperation*)sessionId);
}

/**
* 方法对应表
*/
static JNINativeMethod gMethods[] = {
	{"native_audio_open_demux", "(Ljava/lang/String;Lcom/sy/syplayer/decoder/AudioProperty;)I", (void*)native_audio_open_demux},//绑定
	{"native_audio_read_data_decoded", "(I[BI)I", (void*)native_audio_read_data_decoded },//绑定
	{"native_audio_seek", "(II)I", (void*)native_audio_seek},
	{"native_audio_close_demux", "(I)V", (void*)native_audio_close_demux}
};

/*
* 为某一个类注册本地方法
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
* 为所有类注册本地方法
*/
static int registerNatives(JNIEnv* env) {
	const char* kClassName = "com/sy/syplayer/decoder/SYDecoder";//指定要注册的类
	return registerNativeMethods(env, kClassName, gMethods,
		sizeof(gMethods) / sizeof(gMethods[0]));
}

/*
* System.loadLibrary("lib")时调用
* 如果成功返回JNI版本, 失败返回-1
*/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		return -1;
	}
	assert(env != NULL);

	if (!registerNatives(env)) {//注册
		return -1;
	}
	//成功
	result = JNI_VERSION_1_4;

	return result;
}
