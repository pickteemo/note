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

#include "./general_channel_message.h"

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "./general_message.h"
#include "./screen.h"

namespace {
constexpr int ReaderWriterOffset = 4;

}  // namespace

const char* GeneralChannelMessage::errCode2Str(
    GeneralChannelMessage::ErrorCode errCode) {
  const char* ret;
  switch (errCode) {
    case GeneralChannelMessage::ErrorCode::NewSubClassFailed:
      ret = "Cannot Create Parser Object";
      break;

    case GeneralChannelMessage::ErrorCode::CreateNodeFailed:
      ret = "Cannot Create Cyber Node";
      break;

    case GeneralChannelMessage::ErrorCode::CreateReaderFailed:
      ret = "Cannot Create Cyber Reader";
      break;

    case GeneralChannelMessage::ErrorCode::MessageTypeIsEmpty:
      ret = "Message Type is Empty";
      break;

    case GeneralChannelMessage::ErrorCode::ChannelNameOrNodeNameIsEmpty:
      ret = "Channel Name or Node Name is Empty";
      break;

    case GeneralChannelMessage::ErrorCode::NoCloseChannel:
      ret = "No Close Channel";
      break;

    default:
      ret = "Unknown Error Code";
  }
  return ret;
}

bool GeneralChannelMessage::isErrorCode(void* ptr) {
  GeneralChannelMessage::ErrorCode err =
      (GeneralChannelMessage::ErrorCode)(reinterpret_cast<intptr_t>(ptr));
  switch (err) {
    case ErrorCode::NewSubClassFailed:
    case ErrorCode::CreateNodeFailed:
    case ErrorCode::CreateReaderFailed:
    case ErrorCode::MessageTypeIsEmpty:
    case ErrorCode::ChannelNameOrNodeNameIsEmpty:
    case ErrorCode::NoCloseChannel:
      return true;

    default: {}
  }
  return false;
}

double GeneralChannelMessage::frame_ratio(void) {
  if (!is_enabled() || !has_message_come()) {
    return 0.0;
  }
  auto time_now = system_clock::to_time_t(system_clock::now());
  auto interval = time_now - time_last_calc_;
  if (interval > 1) {
    int old = frame_counter_;
    while (!frame_counter_.compare_exchange_strong(old, 0)) {
    }
    if (old == 0) {
      return 0.0;
    }
    auto curMsgTime = msg_time_;
    auto deltaTime = curMsgTime - last_time_;
    frame_ratio_ = old / deltaTime;
    last_time_ = curMsgTime;
    time_last_calc_ = time_now;
  }
  return frame_ratio_;
}

GeneralChannelMessage* GeneralChannelMessage::OpenChannel(
    const std::string& channelName) {
  if (channelName.empty() || node_name_.empty()) {
    return castErrorCode2Ptr(ErrorCode::ChannelNameOrNodeNameIsEmpty);
  }
  auto callBack = [this](const std::shared_ptr<std::string>& rawMsg) {
    updateRawMessage(*rawMsg);
  };

  //  auto callbackLcm = [this](const lcm::ReceiveBuffer* rbuf,
  //                            const std::string& chan) {
  //    std::string str = "";
  //    str.swap(*static_cast<std::string*>(rbuf->data));
  //    updateRawMessage(str);
  //  };

  LcmAdapter::Instance()->AddSubscriber(channelName,
                                        &GeneralChannelMessage::handler, this);
  //  channel_reader_ =
  //      channel_node_->CreateReader<apollo::cyber::message::RawMessage>(
  //          channelName, callBack);
  //  if (channel_reader_ == nullptr) {
  //    return castErrorCode2Ptr(ErrorCode::CreateReaderFailed);
  //  }
  return this;
}

int GeneralChannelMessage::Render(const Screen* s, int key) {
  switch (key) {
    case 'b':
    case 'B':
      current_state_ = State::ShowDebugString;
      break;

    case 'i':
    case 'I':
      current_state_ = State::ShowInfo;
      break;

    default: {}
  }

  clear();

  int line_no = 0;

  s->SetCurrentColor(Screen::WHITE_BLACK);
  s->AddStr(0, line_no++, "ChannelName: ");
  s->AddStr(readers_.back().c_str());

  s->AddStr(0, line_no++, "MessageType: ");
  s->AddStr(message_type().c_str());

  if (is_enabled()) {
    switch (current_state_) {
      case State::ShowDebugString:
        RenderDebugString(s, key, line_no);
        break;
      case State::ShowInfo:
        RenderInfo(s, key, line_no);
        break;
    }
  } else {
    s->AddStr(0, line_no++, "Channel has been closed");
  }
  s->ClearCurrentColor();

  return line_no;
}

void GeneralChannelMessage::RenderInfo(const Screen* s, int key, int& line_no) {
  page_item_count_ = s->Height() - line_no;
  pages_ = static_cast<int>(readers_.size() + writers_.size() + line_no) /
               page_item_count_ +
           1;
  SplitPages(key);

  bool hasReader = true;
  std::vector<std::string>* vec = &readers_;

  auto iter = vec->cbegin();
  unsigned int y = page_index_ * page_item_count_;
  if (y < vec->size()) {
    for (unsigned i = 0; i < y; ++i) {
      ++iter;
    }
  } else {
    y -= static_cast<unsigned int>(vec->size());
    vec = &writers_;
    iter = vec->cbegin();
    while (y) {
      ++iter;
      --y;
    }

    hasReader = false;
  }

  if (hasReader) {
    s->AddStr(0, line_no++, "Readers:");
    for (; iter != vec->cend(); ++iter) {
      s->AddStr(ReaderWriterOffset, line_no++, iter->c_str());
    }

    ++line_no;
    vec = &writers_;
    iter = vec->cbegin();
  }

  s->AddStr(0, line_no++, "Writers:");
  for (; iter != vec->cend(); ++iter) {
    s->AddStr(ReaderWriterOffset, line_no++, iter->c_str());
  }
}

void GeneralChannelMessage::RenderDebugString(const Screen* s, int key,
                                              int& line_no) {
  if (has_message_come()) {
    if (raw_msg_class_ == nullptr) {
      auto descriptor = google::protobuf::DescriptorPool::generated_pool()
                            ->FindMessageTypeByName(message_type_);
      if (descriptor != nullptr) {
        auto prototype =
            google::protobuf::MessageFactory::generated_factory()->GetPrototype(
                descriptor);
        if (prototype != nullptr) {
          google::protobuf::Message* message = prototype->New();
          raw_msg_class_ = message;
        }
      }
    }

    if (raw_msg_class_ == nullptr) {
      s->AddStr(0, line_no++, "Cannot Generate Message by Message Type");
    } else {
      s->AddStr(0, line_no++, "FrameRatio: ");

      std::ostringstream outStr;
      outStr << std::fixed << std::setprecision(FrameRatio_Precision)
             << frame_ratio();
      s->AddStr(outStr.str().c_str());

      std::shared_ptr<std::string> channelMsg = CopyMsgPtr();

      if (channelMsg->size()) {
        s->AddStr(0, line_no++, "RawMessage Size: ");
        outStr.str("");
        outStr << channelMsg->size() << " Bytes";
        //        if (channelMsg->size() >= kGB) {
        //          outStr << " (" <<
        //          static_cast<float>(channelMsg->message.size()) / kGB
        //                 << " GB)";
        //        } else if (channelMsg->message.size() >= kMB) {
        //          outStr << " (" <<
        //          static_cast<float>(channelMsg->message.size()) / kMB
        //                 << " MB)";
        //        } else if (channelMsg->message.size() >= kKB) {
        //          outStr << " (" <<
        //          static_cast<float>(channelMsg->message.size()) / kKB
        //                 << " KB)";
        //        }
        s->AddStr(outStr.str().c_str());
        int length = 0;
        auto msg_buffer = CopyBufferPtr(length);
        if (raw_msg_class_->ParseFromArray(*msg_buffer,
                                           static_cast<int>(length))) {
          int lcount = lineCount(*raw_msg_class_, s->Width());
          page_item_count_ = s->Height() - line_no;
          pages_ = lcount / page_item_count_ + 1;
          SplitPages(key);
          int jumpLines = page_index_ * page_item_count_;
          jumpLines <<= 2;
          jumpLines /= 5;
          GeneralMessageBase::PrintMessage(this, *raw_msg_class_, jumpLines, s,
                                           line_no, 0);
        } else {
          s->AddStr(0, line_no++, "Cannot parse the raw message");
        }
      } else {
        s->AddStr(0, line_no++, "The size of this raw Message is Zero");
      }
    }
  } else {
    s->AddStr(0, line_no++, "No Message Came");
  }
}
