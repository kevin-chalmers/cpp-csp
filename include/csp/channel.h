// Copyright 2019 Kevin Chalmers
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef INCLUDE_CSP_CHANNEL_H_
#define INCLUDE_CSP_CHANNEL_H_

#include <memory>
#include <type_traits>

namespace csp {

/*!
\brief Used to determine is a type is a reference or not in a template.
 */
template<typename T>
using IsNotReference = std::enable_if_t<!std::is_reference_v<T>>;

template<typename T>
class ChannelInternal {
 protected:
    ChannelInternal() = default;
 public:
    ChannelInternal(const ChannelInternal<T>&) = default;

    ChannelInternal(ChannelInternal<T>&&) noexcept = default;

    virtual ~ChannelInternal() = default;

    ChannelInternal<T>& operator=(const ChannelInternal<T>&) = default;

    ChannelInternal<T>& operator=(ChannelInternal<T>&&) noexcept = default;

    virtual void write(T) = 0;

    template<typename T_ = T, typename = IsNotReference<T_>>
    void write(T&& value)
    {
      write(std::move(value));
    }
};

template<typename T>
class ChanIn {
};

template<typename T>
class GuardedChanIn : public ChanIn<T> {
};

template<typename T>
class SharedChanIn : public ChanIn<T> {
};

template<typename T>
class ChanOut {
};

template<typename T>
class SharedChanOut : public ChanOut<T> {
};

template<typename T,
         template<typename> class INPUT_END,
         template<typename> class OUTPUT_END>
class ChanType {
 private:
    INPUT_END<T> input_;
    OUTPUT_END<T> output_;
    std::shared_ptr<ChannelInternal<T>> internal_ = nullptr;
 public:
    inline INPUT_END<T> in() const { return input_; }
    inline OUTPUT_END<T> out() const { return output_; }
};

template<typename T>
using One2OneChan = ChanType<T, GuardedChanIn, ChanOut>;

template<typename T>
using One2AnyChan = ChanType<T, SharedChanIn, ChanOut>;

template<typename T>
using Any2OneChan = ChanType<T, GuardedChanIn, SharedChanOut>;

template<typename T>
using Any2AnyChan = ChanType<T, SharedChanIn, SharedChanOut>;

}

#endif  // INCLUDE_CSP_CHANNEL_H_
