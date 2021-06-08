/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#ifndef TOOLS_CVT_MONITOR_GENERAL_CHANNEL_MESSAGE_H_
#define TOOLS_CVT_MONITOR_GENERAL_CHANNEL_MESSAGE_H_

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

#include "general_message_base.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "modules/common/adapters/lcm_adapter.hpp"
#include "modules/common/data_struct/proto/control/control_cmd.pb.h"
#include "modules/common/data_struct/proto/drivers/camera/mobileye.pb.h"
#include "modules/common/data_struct/proto/localization/gps_imu_info.pb.h"
#include "modules/common/data_struct/proto/map/hwp_trajectory.pb.h"
#include "modules/common/data_struct/proto/monitor/system_status.pb.h"
#include "modules/common/data_struct/proto/perception/perception_obstacle.pb.h"
#include "modules/common/data_struct/proto/planning/trajectory.pb.h"
#include "modules/common/data_struct/proto/prediction/prediction_obstacle.pb.h"
#include "modules/common/data_struct/proto/vehicle/chassis.pb.h"

template <typename M0>
using CallbackFunc = std::function<void(const std::shared_ptr<M0>&)>;

class GeneralMessage;

using namespace std::chrono;

class GeneralChannelMessage : public GeneralMessageBase {
 public:
  enum class ErrorCode {
    NewSubClassFailed = -1,
    CreateNodeFailed = -2,
    CreateReaderFailed = -3,
    MessageTypeIsEmpty = -4,
    ChannelNameOrNodeNameIsEmpty = -5,
    NoCloseChannel = -6
  };

  static const char* errCode2Str(ErrorCode errCode);
  static bool isErrorCode(void* ptr);

  static ErrorCode castPtr2ErrorCode(void* ptr) {
    assert(isErrorCode(ptr));
    return static_cast<ErrorCode>(reinterpret_cast<intptr_t>(ptr));
  }
  static GeneralChannelMessage* castErrorCode2Ptr(ErrorCode errCode) {
    return reinterpret_cast<GeneralChannelMessage*>(
        static_cast<intptr_t>(errCode));
  }

  ~GeneralChannelMessage() {
    //    channel_node_.reset();
    //    channel_reader_.reset();
    //    channel_message_.reset();
    if (raw_msg_class_) {
      delete raw_msg_class_;
      raw_msg_class_ = nullptr;
    }
  }

  std::string GetChannelName(void) const {
    return channel_name_;
    // return channel_reader_->GetChannelName();
  }

  void set_message_type(const std::string& msgTypeName) {
    message_type_ = msgTypeName;
  }
  const std::string& message_type(void) const {
    return message_type_;
  }

  bool is_enabled(void) const {
    return true;
    //    return channel_reader_ != nullptr;
  }
  bool has_message_come(void) const {
    return has_message_come_;
  }

  double frame_ratio(void) override;

  const std::string& NodeName(void) const {
    return node_name_;
  }

  void add_reader(const std::string& reader) {
    DoAdd(readers_, reader);
  }
  void del_reader(const std::string& reader) {
    DoDelete(readers_, reader);
  }

  void add_writer(const std::string& writer) {
    DoAdd(writers_, writer);
  }
  void del_writer(const std::string& writer) {
    DoDelete(writers_, writer);
    if (!writers_.size()) {
      set_has_message_come(false);
    }
  }

  int Render(const Screen* s, int key) override;

  void CloseChannel(void) {
    return;
    //    if (channel_reader_ != nullptr) {
    //      channel_reader_.reset();
    //    }

    //    if (channel_node_ != nullptr) {
    //      channel_node_.reset();
    //    }
  }
  std::string channel_name_;

 private:
  explicit GeneralChannelMessage(const std::string& nodeName,
                                 RenderableMessage* parent = nullptr)
      : GeneralMessageBase(parent),
        current_state_(State::ShowDebugString),
        has_message_come_(false),
        message_type_(),
        frame_counter_(0),
        last_time_(system_clock::to_time_t(system_clock::now())),
        msg_time_(system_clock::to_time_t(system_clock::now())),
        node_name_(nodeName),
        readers_(),
        writers_(),
        channel_message_(""),
        channel_reader_(nullptr),
        inner_lock_(),
        raw_msg_class_(nullptr) {}

  GeneralChannelMessage(const GeneralChannelMessage&) = delete;
  GeneralChannelMessage& operator=(const GeneralChannelMessage&) = delete;

  static void DoDelete(std::vector<std::string>& vec, const std::string& str) {
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
      if (*iter == str) {
        vec.erase(iter);
        break;
      }
    }
  }

  static void DoAdd(std::vector<std::string>& vec, const std::string& str) {
    for (auto iter = vec.begin(); iter != vec.end(); ++iter) {
      if (*iter == str) {
        return;
      }
    }

    vec.emplace_back(str);
  }

  void handler(const lcm::ReceiveBuffer* rbuf, const std::string& channel) {
    if (length != 0) {
      free(datavoid);
    }
    std::lock_guard<std::mutex> g(inner_lock_);
    length = rbuf->data_size;
    datavoid = malloc(length);
    memcpy(datavoid, rbuf->data, length);
    // datavoid = rbuf->data;
    char* data = static_cast<char*>(datavoid);
    std::string str(data);
    updateRawMessage(str);
  }

  void updateRawMessage(const std::string& rawMsg) {
    set_has_message_come(true);
    msg_time_ = system_clock::to_time_t(system_clock::now());
    ++frame_counter_;
    // std::lock_guard<std::mutex> _g(inner_lock_);
    channel_message_ = rawMsg;
  }

  std::shared_ptr<std::string> CopyMsgPtr(void) const {
    decltype(channel_message_) channelMsg;
    {
      std::lock_guard<std::mutex> g(inner_lock_);
      channelMsg = channel_message_;
    }
    return std::make_shared<std::string>(channelMsg);
  }

  std::shared_ptr<void*> CopyBufferPtr(int& _length) const {
    _length = length;
    std::lock_guard<std::mutex> g(inner_lock_);
    void* p_buffer;
    p_buffer = malloc(length);
    memcpy(p_buffer, datavoid, length);
    return std::make_shared<void*>(p_buffer);
  }

  GeneralChannelMessage* OpenChannel(const std::string& channelName);

  void RenderDebugString(const Screen* s, int key, int& line_no);
  void RenderInfo(const Screen* s, int key, int& line_no);

  void set_has_message_come(bool b) {
    has_message_come_ = b;
  }

  enum class State { ShowDebugString, ShowInfo } current_state_;

  bool has_message_come_;
  std::string message_type_;
  std::atomic<int> frame_counter_;
  std::time_t last_time_;
  std::time_t msg_time_;
  std::time_t time_last_calc_ = system_clock::to_time_t(system_clock::now());

  std::string node_name_;

  std::vector<std::string> readers_;
  std::vector<std::string> writers_;

  std::string channel_message_;
  std::shared_ptr<std::string> channel_reader_;
  mutable std::mutex inner_lock_;

  google::protobuf::Message* raw_msg_class_;

  friend class CyberTopologyMessage;
  friend class GeneralMessage;

  void* datavoid;
  int length = 0;
};  // GeneralChannelMessage

#endif  // TOOLS_CVT_MONITOR_GENERAL_CHANNEL_MESSAGE_H_
