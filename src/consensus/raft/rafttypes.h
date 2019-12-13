// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the Apache 2.0 License.
#pragma once

#include "ds/ringbuffer_types.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <msgpack.hpp>

namespace raft
{
  using Index = int64_t;
  using Term = uint64_t;
  using NodeId = uint64_t;
  using Node2NodeMsg = uint64_t;

  static constexpr NodeId NoNode = std::numeric_limits<NodeId>::max();

  struct Config
  {
    size_t request_timeout;
    size_t election_timeout;
    MSGPACK_DEFINE(request_timeout, election_timeout);
  };

  template <typename S>
  class Store
  {
  public:
    virtual ~Store() {}
    virtual S deserialise(
      const std::vector<uint8_t>& data,
      bool public_only = false,
      Term* term = nullptr) = 0;
    virtual void compact(Index v) = 0;
    virtual void rollback(Index v) = 0;
  };

  template <typename T, typename S>
  class Adaptor : public Store<S>
  {
  private:
    std::weak_ptr<T> x;

  public:
    Adaptor(std::shared_ptr<T> x) : x(x) {}

    S deserialise(
      const std::vector<uint8_t>& data,
      bool public_only = false,
      Term* term = nullptr)
    {
      auto p = x.lock();
      if (p)
        return p->deserialise(data, public_only, term);

      return S::FAILED;
    }

    void compact(Index v)
    {
      auto p = x.lock();
      if (p)
        p->compact(v);
    }

    void rollback(Index v)
    {
      auto p = x.lock();
      if (p)
        p->rollback(v);
    }
  };

  enum RaftMsgType : Node2NodeMsg
  {
    raft_append_entries = 0,
    raft_append_entries_response,
    raft_request_vote,
    raft_request_vote_response,
  };

#pragma pack(push, 1)
  struct RaftHeader
  {
    RaftMsgType msg;
    NodeId from_node;
  };

  struct AppendEntries : RaftHeader
  {
    Index idx;
    Term term;
    Index prev_idx;
    Term prev_term;
    Index leader_commit_idx;
    Term term_of_idx;
  };

  struct AppendEntriesResponse : RaftHeader
  {
    Term term;
    Index last_log_idx;
    bool success;
  };

  struct RequestVote : RaftHeader
  {
    Term term;
    Index last_verified_idx;
    Term last_verified_term;
  };

  struct RequestVoteResponse : RaftHeader
  {
    Term term;
    bool vote_granted;
  };
#pragma pack(pop)
}