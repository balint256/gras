// Copyright (C) by Josh Blum. See LICENSE.txt for licensing information.

#include <gras_impl/block_actor.hpp>
#include <boost/foreach.hpp>

using namespace gras;


void BlockActor::handle_output_buffer(const OutputBufferMessage &message, const Theron::Address)
{
    TimerAccumulate ta(this->stats.total_time_output);
    MESSAGE_TRACER();
    const size_t index = message.index;

    //a buffer has returned from the downstream
    //(all interested consumers have finished with it)
    if GRAS_UNLIKELY(this->block_state == BLOCK_STATE_DONE) return;
    this->output_queues.push(index, message.buffer);

    ta.done();
    this->task_main();
}

void BlockActor::handle_output_token(const OutputTokenMessage &message, const Theron::Address)
{
    TimerAccumulate ta(this->stats.total_time_output);
    MESSAGE_TRACER();
    ASSERT(message.index < this->get_num_outputs());

    //store the token of the downstream consumer
    this->token_pool.insert(message.token);
}

void BlockActor::handle_output_check(const OutputCheckMessage &message, const Theron::Address)
{
    TimerAccumulate ta(this->stats.total_time_output);
    MESSAGE_TRACER();
    const size_t index = message.index;

    //a downstream block has declared itself done, recheck the token
    this->outputs_done.set(index, this->output_tokens[index].unique());
    if (this->outputs_done.all()) //no downstream subscribers?
    {
        this->mark_done();
    }
}

void BlockActor::handle_output_hint(const OutputHintMessage &message, const Theron::Address)
{
    TimerAccumulate ta(this->stats.total_time_output);
    MESSAGE_TRACER();
    const size_t index = message.index;

    //update the buffer allocation hint
    //this->output_allocation_hints.resize(std::max(output_allocation_hints.size(), index+1));

    //remove any old hints with expired token
    //remove any older hints with matching token
    std::vector<OutputHintMessage> hints;
    BOOST_FOREACH(const OutputHintMessage &hint, this->output_allocation_hints[index])
    {
        if (hint.token.expired()) continue;
        if (hint.token.lock() == message.token.lock()) continue;
        hints.push_back(hint);
    }

    //store the new hint as well
    hints.push_back(message);

    this->output_allocation_hints[index] = hints;
}

void BlockActor::handle_output_alloc(const OutputAllocMessage &message, const Theron::Address)
{
    TimerAccumulate ta(this->stats.total_time_output);
    MESSAGE_TRACER();
    const size_t index = message.index;

    //return of a positive downstream allocation
    this->output_queues.set_buffer_queue(index, message.queue);
}

void BlockActor::handle_output_update(const OutputUpdateMessage &message, const Theron::Address)
{
    TimerAccumulate ta(this->stats.total_time_output);
    MESSAGE_TRACER();
    const size_t i = message.index;

    //update buffer queue configuration
    if (i >= this->output_queues.size()) return;
    const size_t reserve_bytes = this->output_configs[i].item_size*this->output_configs[i].reserve_items;
    this->output_queues.set_reserve_bytes(i, reserve_bytes);
}
