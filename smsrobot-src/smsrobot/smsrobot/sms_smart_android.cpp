/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifdef ANDROID

#include <utility>
#include <android/log.h>

#include "sms_smart_android.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/refcount.h"
#include "rtc_base/scoped_ref_ptr.h"

#define TAG "VideoCaptureAndroid"
#define ALOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, TAG, __VA_ARGS__)
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define ALOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

// VideoCaptureAndroid::JavaVideoCapturer implementation
tsms_smart::JavaSmsSmart::JavaSmsSmart(webrtc::NativeRegistration* native_reg, std::unique_ptr<webrtc::GlobalRef> audio_manager)
    : audio_manager_(std::move(audio_manager))
	, Send_(native_reg->GetMethodId("Send", "(ILjava/lang/String;Ljava/lang/String;)Z"))
	, getSimInfo_(native_reg->GetMethodId("getSimInfo", "()Ljava/util/List;"))
		
{
	ALOGD("JavaSmsSmart::ctor%s", webrtc::GetThreadInfo().c_str());
}

tsms_smart::JavaSmsSmart::~JavaSmsSmart() 
{
	ALOGD("JavaSmsSmart::dtor%s", webrtc::GetThreadInfo().c_str());
}

void tsms_smart::JavaSmsSmart::getSimInfo(std::vector<tsim_info>& result)
{
	ALOGD("JavaSmsSmart::getSimInfo%s", webrtc::GetThreadInfo().c_str());

	JNIEnv* jni = audio_manager_->jni();
	const jobject j_object = audio_manager_->j_object();

	jobject infos = jni->CallObjectMethod(j_object, getSimInfo_);
	jmethodID mid = webrtc::GetMethodID(jni, webrtc_jni::GetObjectClass(jni, infos), "size", "()I");
	int count = jni->CallIntMethod(infos, mid);

	for (int at = 0; at < count; at ++) {
		result.push_back(tsim_info());
		tsim_info& info = result.back();

		mid = webrtc::GetMethodID(jni, webrtc_jni::GetObjectClass(jni, infos), "get", "(I)Ljava/lang/Object;");
		jobject jinfo = jni->CallObjectMethod(infos, mid, at);

		mid = webrtc::GetMethodID(jni, webrtc_jni::GetObjectClass(jni, jinfo), "getLevel22", "()Z");
		jboolean jlevel22 = jni->CallBooleanMethod(jinfo, mid);
		info.level22 = jlevel22? true: false;

		mid = webrtc::GetMethodID(jni, webrtc_jni::GetObjectClass(jni, jinfo), "getSlot", "()I");
		jint jslot = jni->CallIntMethod(jinfo, mid);
		info.slot = jslot;

		mid = webrtc::GetMethodID(jni, webrtc_jni::GetObjectClass(jni, jinfo), "getCarrier", "()Ljava/lang/String;");
		// if String in java is null, CallObjectMethod will return nullptr.
		jstring carrier_jstr = static_cast<jstring>(jni->CallObjectMethod(jinfo, mid));
		
		const char* carrier_cstr = nullptr;
		if (carrier_jstr) {
			// webrtc_jni::JavaToStdString use unicode output, it requrie utf-8 output.
			carrier_cstr = jni->GetStringUTFChars(carrier_jstr, nullptr);
		}
		info.carrier = carrier_cstr? carrier_cstr: "";

		mid = webrtc::GetMethodID(jni, webrtc_jni::GetObjectClass(jni, jinfo), "getNumber", "()Ljava/lang/String;");
		jstring number_jstr = static_cast<jstring>(jni->CallObjectMethod(jinfo, mid));

		const char* number_cstr = nullptr;
		if (number_jstr) {
			number_cstr = jni->GetStringUTFChars(number_jstr, nullptr);
		}
		info.number = number_cstr? number_cstr: "";
	}

}

bool tsms_smart::JavaSmsSmart::Send(int slot, const std::string& target_phone, const std::string& message) 
{
	ALOGD("JavaSmsSmart::Send%s, audio_manager_.get(): %p", webrtc::GetThreadInfo().c_str(), audio_manager_.get());

	JNIEnv* jni = webrtc_jni::AttachCurrentThreadIfNeeded();

	jstring jtarget_phone = webrtc_jni::JavaStringFromStdString(jni, target_phone);
	jstring jmessage = webrtc_jni::JavaStringFromStdString(jni, message);

	bool result = audio_manager_->CallBooleanMethod(Send_, slot, jtarget_phone, jmessage);

	// (*jni)->DeleteLocalRef(jni, jtarget_phone);
	// (*jni)->DeleteLocalRef(jni, jmessage);
	return result;
}

tsms_smart::tsms_smart()
    : initialized_(false)
{
}

tsms_smart::~tsms_smart() 
{
}

void tsms_smart::get_siminfo(std::vector<tsim_info>& result)
{
	result.clear();

	InitCaptureAndroidJNI();

	if (j_sms_smart_.get()) {
		j_sms_smart_->getSimInfo(result);
	}
	UninitCaptureAndroidJNI();
}

bool tsms_smart::send(int slot, const std::string& target_phone, const std::string& message) 
{
	InitCaptureAndroidJNI();

	bool result = false;
	if (j_sms_smart_.get()) {
		result = j_sms_smart_->Send(slot, target_phone, message);
	}
	UninitCaptureAndroidJNI();

	return result;
}

bool tsms_smart::InitCaptureAndroidJNI()
{
	attach_thread_if_needed_.reset(new webrtc::AttachCurrentThreadIfNeeded); 
	std::unique_ptr<webrtc::JNIEnvironment> j_environment_ = webrtc::JVM::GetInstance()->environment();

	RTC_CHECK(j_environment_);

	JNINativeMethod native_methods[] = {
	};

	j_native_registration_ = j_environment_->RegisterNatives(
		"org/libsdl/app/SmsSmart", native_methods, 
		// arraysize(native_methods));
		0);

	j_sms_smart_.reset(new JavaSmsSmart(
      j_native_registration_.get(),
      j_native_registration_->NewObject(
          "<init>", "(J)V",
          webrtc::PointerTojlong(this))));

	RTC_DCHECK(!initialized_);

	initialized_ = true;
	return true;
}

void tsms_smart::UninitCaptureAndroidJNI()
{
	if (j_sms_smart_.get()) {
		j_sms_smart_.reset();
	}
	if (j_native_registration_.get()) {
		j_native_registration_.reset();
	}
	attach_thread_if_needed_.reset(nullptr);
	initialized_ = false;
}

#endif