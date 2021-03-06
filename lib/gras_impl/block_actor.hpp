// Copyright (C) by Josh Blum. See LICENSE.txt for licensing information.

#ifndef INCLUDED_LIBGRAS_IMPL_BLOCK_ACTOR_HPP
#define INCLUDED_LIBGRAS_IMPL_BLOCK_ACTOR_HPP

#include <gras_impl/debug.hpp>
#include <gras_impl/bitset.hpp>
#include <gras/gras.hpp>
#include <gras/block.hpp>
#include <gras/top_block.hpp>
#include <gras/thread_pool.hpp>
#include <Apology/Worker.hpp>
#include <gras_impl/token.hpp>
#include <gras_impl/stats.hpp>
#include <gras_impl/messages.hpp>
#include <gras_impl/output_buffer_queues.hpp>
#include <gras_impl/input_buffer_queues.hpp>
#include <gras_impl/interruptible_thread.hpp>
#include <vector>
#include <set>
#include <map>

namespace gras
{

struct BlockActor : Apology::Worker
{
    BlockActor(void);
    ~BlockActor(void);
    Block *block_ptr;
    std::string name; //for debug
    ThreadPool thread_pool;
    Token prio_token;

    //do it here so we can match w/ the handler declarations
    void register_handlers(void)
    {
        this->RegisterHandler(this, &BlockActor::handle_topology);

        this->RegisterHandler(this, &BlockActor::handle_top_alloc);
        this->RegisterHandler(this, &BlockActor::handle_top_active);
        this->RegisterHandler(this, &BlockActor::handle_top_inert);
        this->RegisterHandler(this, &BlockActor::handle_top_token);
        this->RegisterHandler(this, &BlockActor::handle_top_config);
        this->RegisterHandler(this, &BlockActor::handle_top_thread_group);

        this->RegisterHandler(this, &BlockActor::handle_input_tag);
        this->RegisterHandler(this, &BlockActor::handle_input_msg);
        this->RegisterHandler(this, &BlockActor::handle_input_buffer);
        this->RegisterHandler(this, &BlockActor::handle_input_token);
        this->RegisterHandler(this, &BlockActor::handle_input_check);
        this->RegisterHandler(this, &BlockActor::handle_input_alloc);
        this->RegisterHandler(this, &BlockActor::handle_input_update);

        this->RegisterHandler(this, &BlockActor::handle_output_buffer);
        this->RegisterHandler(this, &BlockActor::handle_output_token);
        this->RegisterHandler(this, &BlockActor::handle_output_check);
        this->RegisterHandler(this, &BlockActor::handle_output_hint);
        this->RegisterHandler(this, &BlockActor::handle_output_alloc);
        this->RegisterHandler(this, &BlockActor::handle_output_update);

        this->RegisterHandler(this, &BlockActor::handle_prop_access);
        this->RegisterHandler(this, &BlockActor::handle_self_kick);
        this->RegisterHandler(this, &BlockActor::handle_get_stats);
    }

    //handlers
    void handle_topology(const Apology::WorkerTopologyMessage &, const Theron::Address);

    void handle_top_alloc(const TopAllocMessage &, const Theron::Address);
    void handle_top_active(const TopActiveMessage &, const Theron::Address);
    void handle_top_inert(const TopInertMessage &, const Theron::Address);
    void handle_top_token(const TopTokenMessage &, const Theron::Address);
    void handle_top_config(const GlobalBlockConfig &, const Theron::Address);
    void handle_top_thread_group(const SharedThreadGroup &, const Theron::Address);

    void handle_input_tag(const InputTagMessage &, const Theron::Address);
    void handle_input_msg(const InputMsgMessage &, const Theron::Address);
    void handle_input_buffer(const InputBufferMessage &, const Theron::Address);
    void handle_input_token(const InputTokenMessage &, const Theron::Address);
    void handle_input_check(const InputCheckMessage &, const Theron::Address);
    void handle_input_alloc(const InputAllocMessage &, const Theron::Address);
    void handle_input_update(const InputUpdateMessage &, const Theron::Address);

    void handle_output_buffer(const OutputBufferMessage &, const Theron::Address);
    void handle_output_token(const OutputTokenMessage &, const Theron::Address);
    void handle_output_check(const OutputCheckMessage &, const Theron::Address);
    void handle_output_hint(const OutputHintMessage &, const Theron::Address);
    void handle_output_alloc(const OutputAllocMessage &, const Theron::Address);
    void handle_output_update(const OutputUpdateMessage &, const Theron::Address);

    void handle_prop_access(const PropAccessMessage &, const Theron::Address);
    void handle_self_kick(const SelfKickMessage &, const Theron::Address);
    void handle_get_stats(const GetStatsMessage &, const Theron::Address);

    //helpers
    void mark_done(void);
    void task_main(void);
    void input_fail(const size_t index);
    void output_fail(const size_t index);
    void sort_tags(const size_t index);
    void trim_tags(const size_t index);
    void trim_msgs(const size_t index);
    void produce(const size_t index, const size_t items);
    void consume(const size_t index, const size_t items);
    void produce_buffer(const size_t index, const SBuffer &buffer);
    void task_kicker(void);
    void update_input_avail(const size_t index);
    bool is_input_done(const size_t index);
    bool is_work_allowed(void);

    //per port properties
    std::vector<InputPortConfig> input_configs;
    std::vector<OutputPortConfig> output_configs;

    //work buffers for the new work interface
    Block::InputItems input_items;
    Block::OutputItems output_items;

    //track the subscriber counts
    std::vector<Token> input_tokens;
    std::vector<Token> output_tokens;
    BitSet inputs_done;
    BitSet outputs_done;
    std::set<Token> token_pool;

    //buffer queues and ready conditions
    InputBufferQueues input_queues;
    OutputBufferQueues output_queues;
    std::vector<bool> produce_outputs;
    BitSet inputs_available;

    //tag and msg tracking
    std::vector<bool> input_tags_changed;
    std::vector<std::vector<Tag> > input_tags;
    std::vector<size_t> num_input_msgs_read;
    std::vector<std::vector<PMCC> > input_msgs;

    //interruptible thread stuff
    bool interruptible_work;
    SharedThreadGroup thread_group;
    boost::shared_ptr<InterruptibleThread> interruptible_thread;

    //work helpers
    inline void task_work(void)
    {
        block_ptr->work(this->input_items, this->output_items);
    }

    //is the fg running?
    enum
    {
        BLOCK_STATE_INIT,
        BLOCK_STATE_LIVE,
        BLOCK_STATE_DONE,
    } block_state;
    long buffer_affinity;

    std::vector<std::vector<OutputHintMessage> > output_allocation_hints;

    //property stuff
    std::map<std::string, PropertyRegistrySptr> getter_registry;
    std::map<std::string, PropertyRegistrySptr> setter_registry;

    BlockStats stats;
};

//-------------- common functions from this BlockActor class ---------//

GRAS_FORCE_INLINE void BlockActor::task_kicker(void)
{
    if (this->is_work_allowed()) this->Send(SelfKickMessage(), this->GetAddress());
}

GRAS_FORCE_INLINE void BlockActor::update_input_avail(const size_t i)
{
    const bool has_input_bufs = not this->input_queues.empty(i) and this->input_queues.ready(i);
    const bool has_input_msgs = not this->input_msgs[i].empty();
    this->inputs_available.set(i, has_input_bufs or has_input_msgs);
    this->input_queues.update_has_msg(i, has_input_msgs);
}

GRAS_FORCE_INLINE bool BlockActor::is_input_done(const size_t i)
{
    return this->inputs_done[i] and not this->inputs_available[i];
}

GRAS_FORCE_INLINE bool BlockActor::is_work_allowed(void)
{
    return (
        this->prio_token.unique() and
        this->block_state == BLOCK_STATE_LIVE and
        this->inputs_available.any() and
        this->input_queues.all_ready() and
        this->output_queues.all_ready()
    );
}

} //namespace gras

#endif /*INCLUDED_LIBGRAS_IMPL_BLOCK_ACTOR_HPP*/
