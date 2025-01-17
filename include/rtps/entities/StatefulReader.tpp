/*
The MIT License
Copyright (c) 2019 Lehrstuhl Informatik 11 - RWTH Aachen University
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE

This file is part of embeddedRTPS.

Author: i11 - Embedded Software, RWTH Aachen University
*/

#include "lwip/tcpip.h"
#include "rtps/entities/StatefulReader.h"
#include "rtps/messages/MessageFactory.h"
#include "rtps/utils/Lock.h"

#define SFR_VERBOSE 0

#if SFR_VERBOSE
#include "rtps/utils/printutils.h"
#endif

using rtps::StatefulReaderT;

template <class NetworkDriver>
StatefulReaderT<NetworkDriver>::~StatefulReaderT() {
  // if(sys_mutex_valid(&m_mutex)){
  sys_mutex_free(&m_mutex);
  //}
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::init(const TopicData &attributes,
                                          NetworkDriver &driver) {
  m_attributes = attributes;
  m_transport = &driver;
  m_packetInfo.srcPort = attributes.unicastLocator.port;
  sys_mutex_new(&m_mutex);
  m_is_initialized_ = true;
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::newChange(
    const ReaderCacheChange &cacheChange) {
  if (m_callback == nullptr) {
    return;
  }
  Lock lock{m_mutex};
  for (auto &proxy : m_proxies) {
    if (proxy.remoteWriterGuid == cacheChange.writerGuid) {
      //if (proxy.expectedSN == cacheChange.sn) {
        m_callback(m_callee, cacheChange);
        ++proxy.expectedSN;
        return;
      //}
    }
  }
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::registerCallback(ddsReaderCallback_fp cb,
                                                      void *callee) {
  if (cb != nullptr) {
    m_callback = cb;
    m_callee = callee; // It's okay if this is null
  } else {
#if SLR_VERBOSE
    printf("StatefulReader[%s]: Passed callback is nullptr\n",
           &m_attributes.topicName[0]);
#endif
  }
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::addNewMatchedWriter(
    const WriterProxy &newProxy) {
#if SFR_VERBOSE
  printf("StatefulReader[%s]: New writer added with id: ",
         &this->m_attributes.topicName[0]);
  printGuid(newProxy.remoteWriterGuid);
  printf("\n");
#endif
  return m_proxies.add(newProxy);
}

template <class NetworkDriver>
void StatefulReaderT<NetworkDriver>::removeWriter(const Guid &guid) {
  auto isElementToRemove = [&](const WriterProxy &proxy) {
    return proxy.remoteWriterGuid == guid;
  };
  auto thunk = [](void *arg, const WriterProxy &value) {
    return (*static_cast<decltype(isElementToRemove) *>(arg))(value);
  };

  m_proxies.remove(thunk, &isElementToRemove);
}

template <class NetworkDriver>
bool StatefulReaderT<NetworkDriver>::onNewHeartbeat(
    const SubmessageHeartbeat &msg, const GuidPrefix_t &sourceGuidPrefix) {
  Lock lock(m_mutex);
  PacketInfo info;
  info.srcPort = m_packetInfo.srcPort;
  WriterProxy *writer = nullptr;
  // Search for writer
  for (WriterProxy &proxy : m_proxies) {
    if (proxy.remoteWriterGuid.prefix == sourceGuidPrefix &&
        proxy.remoteWriterGuid.entityId == msg.writerId) {
      writer = &proxy;
      break;
    }
  }

  if (writer == nullptr) {
#if SFR_VERBOSE
    printf("StatefulReader[%s]: Ignore heartbeat. Couldn't find a matching "
           "writer with id: ",
           &this->m_attributes.topicName[0]);
    printEntityId(msg.writerId);
    printf("\n");
#endif
    return false;
  }

  if (msg.count.value <= writer->hbCount.value) {
#if SFR_VERBOSE
    printf("StatefulReader[%s]: Ignore heartbeat. Count too low.\n",
           &this->m_attributes.topicName[0]);
#endif
    return false;
  }

  writer->hbCount.value = msg.count.value;
  info.destAddr = writer->remoteLocator.getIp4Address();
  info.destPort = writer->remoteLocator.port;
  rtps::MessageFactory::addHeader(info.buffer,
                                  m_attributes.endpointGuid.prefix);
  rtps::MessageFactory::addSubMessageDestination(info.buffer);
  rtps::MessageFactory::addAckNack(info.buffer, msg.writerId, msg.readerId,
                                   writer->getMissing(msg.firstSN, msg.lastSN),
                                   writer->getNextAckNackCount());

#if SFR_VERBOSE
  printf("StatefulReader[%s]: Sending acknack.\n",
         &this->m_attributes.topicName[0]);
#endif
  m_transport->sendPacket(info);
  return true;
}

#undef SFR_VERBOSE
