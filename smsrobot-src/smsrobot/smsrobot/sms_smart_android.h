/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef LIBROSE_SMS_SMART_H_INCLUDED
#define LIBROSE_SMS_SMART_H_INCLUDED

#include "game_config.hpp"

#ifdef ANDROID

#include "sdk/android/src/jni/native_handle_impl.h"
#include "sdk/android/src/jni/surfacetexturehelper_jni.h"
#include "rtc_base/thread_checker.h"
#include "rtc_base/timestampaligner.h"
#include "media/base/videoadapter.h"
#include "rtc_base/timeutils.h"
#include "modules/utility/include/helpers_android.h"
#include "modules/utility/include/jvm_android.h"
#include "rtc_base/scoped_ref_ptr.h"
#include "modules/video_capture/video_capture_impl.h"

class tsms_smart
{
public:
  // Wraps the Java specific parts of the AudioManager into one helper class.
  // Stores method IDs for all supported methods at construction and then
  // allows calls like JavaAudioManager::Close() while hiding the Java/JNI
  // parts that are associated with this call.
  class JavaSmsSmart {
   public:
    JavaSmsSmart(webrtc::NativeRegistration* native_registration, std::unique_ptr<webrtc::GlobalRef> audio_manager);
    ~JavaSmsSmart();

	void getSimInfo(std::vector<tsim_info>& result);
    bool Send(int slot, const std::string& target_phone, const std::string& message);

   private:
    std::unique_ptr<webrtc::GlobalRef> audio_manager_;
	jmethodID getSimInfo_;
    jmethodID Send_;
  };

  explicit tsms_smart();
  virtual ~tsms_smart();

  // Implementation of VideoCaptureImpl.
  void get_siminfo(std::vector<tsim_info>& result);
  bool send(int slot, const std::string& target_phone, const std::string& message);

private:
	bool InitCaptureAndroidJNI();
	void UninitCaptureAndroidJNI();

private:
  // Stores thread ID in the constructor.
  // We can then use ThreadChecker::CalledOnValidThread() to ensure that
  // other methods are called from the same thread.
  rtc::ThreadChecker thread_checker_;

  // Calls AttachCurrentThread() if this thread is not attached at construction.
  // Also ensures that DetachCurrentThread() is called at destruction.
  std::unique_ptr<webrtc::AttachCurrentThreadIfNeeded> attach_thread_if_needed_;

  // Contains factory method for creating the Java object.
  std::unique_ptr<webrtc::NativeRegistration> j_native_registration_;

  // Wraps the Java specific parts of the AudioManager.
  std::unique_ptr<JavaSmsSmart> j_sms_smart_;

  // Set to true by Init() and false by Close().
  bool initialized_;
};

#else

class tsms_smart
{
public:
	void get_siminfo(std::vector<tsim_info>& result)
	{
		result.clear();
		result.push_back(tsim_info(0, "Chinese Mobile", "13868997634"));
		result.push_back(tsim_info(1, "Chinese Unicom", "13818230661"));
	}

	bool send(int slot, const std::string& target_phone, const std::string& message) { return true; }
};

#endif

#endif  // LIBROSE_SMS_SMART_H_INCLUDED
