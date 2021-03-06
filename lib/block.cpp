// Copyright (C) by Josh Blum. See LICENSE.txt for licensing information.

#include "element_impl.hpp"
#include <gras/block.hpp>
#include <boost/thread/thread.hpp> //sleep
#include <iostream>

using namespace gras;

InputPortConfig::InputPortConfig(void)
{
    item_size = 1;
    reserve_items = 1;
    maximum_items = 0;
    inline_buffer = false;
    preload_items = 0;
}

OutputPortConfig::OutputPortConfig(void)
{
    item_size = 1;
    reserve_items = 1;
    maximum_items = 0;
}

Block::Block(void)
{
    //NOP
}

Block::Block(const std::string &name):
    Element(name)
{
    (*this)->block.reset(new BlockActor());
    (*this)->block->prio_token = Token::make();
    (*this)->thread_pool = (*this)->block->thread_pool; //ref copy of pool
    (*this)->block->name = name; //for debug purposes

    //setup some state variables
    (*this)->block->block_ptr = this;
    (*this)->block->block_state = BlockActor::BLOCK_STATE_INIT;

    //call block methods to init stuff
    this->input_config(0) = InputPortConfig();
    this->output_config(0) = OutputPortConfig();
    this->set_interruptible_work(false);
    this->set_buffer_affinity(-1);
}

Block::~Block(void)
{
    //NOP
}

enum block_cleanup_state_type
{
    BLOCK_CLEANUP_WAIT,
    BLOCK_CLEANUP_WARN,
    BLOCK_CLEANUP_DAMN,
    BLOCK_CLEANUP_DOTS,
};

static void wait_block_cleanup(ElementImpl &self)
{
    const boost::system_time start = boost::get_system_time();
    block_cleanup_state_type state = BLOCK_CLEANUP_WAIT;
    while (self.block->GetNumQueuedMessages())
    {
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        switch (state)
        {
        case BLOCK_CLEANUP_WAIT:
            if (boost::get_system_time() > start + boost::posix_time::seconds(1))
            {
                std::cerr << self.id << ", waiting for you to finish." << std::endl;
                state = BLOCK_CLEANUP_WARN;
            }
            break;

        case BLOCK_CLEANUP_WARN:
            if (boost::get_system_time() > start + boost::posix_time::seconds(2))
            {
                std::cerr << self.id << ", give up the thread context!" << std::endl;
                state = BLOCK_CLEANUP_DAMN;
            }
            break;

        case BLOCK_CLEANUP_DAMN:
            if (boost::get_system_time() > start + boost::posix_time::seconds(3))
            {
                std::cerr << self.id << " FAIL; application will now hang..." << std::endl;
                state = BLOCK_CLEANUP_DOTS;
            }
            break;

        case BLOCK_CLEANUP_DOTS: break;
        }
    }
}

void ElementImpl::block_cleanup(void)
{
    //wait for actor to chew through enqueued messages
    wait_block_cleanup(*this);

    //delete the actor
    this->block.reset();

    //unref actor's framework
    this->thread_pool.reset(); //must be deleted after actor
}

template <typename V>
const typename V::value_type &vector_get_const(const V &v, const size_t index)
{
    if (v.size() <= index)
    {
        return v.back();
    }
    return v[index];
}

template <typename V>
typename V::value_type &vector_get_resize(V &v, const size_t index)
{
    if (v.size() <= index)
    {
        if (v.empty()) v.resize(1);
        v.resize(index+1, v.back());
    }
    return v[index];
}

InputPortConfig &Block::input_config(const size_t which_input)
{
    return vector_get_resize((*this)->block->input_configs, which_input);
}

const InputPortConfig &Block::input_config(const size_t which_input) const
{
    return vector_get_const((*this)->block->input_configs, which_input);
}

OutputPortConfig &Block::output_config(const size_t which_output)
{
    return vector_get_resize((*this)->block->output_configs, which_output);
}

const OutputPortConfig &Block::output_config(const size_t which_output) const
{
    return vector_get_const((*this)->block->output_configs, which_output);
}

void Block::commit_config(void)
{
    Theron::Actor &actor = *((*this)->block);
    for (size_t i = 0; i < (*this)->block->get_num_inputs(); i++)
    {
        InputUpdateMessage message;
        message.index = i;
        actor.GetFramework().Send(message, Theron::Address::Null(), actor.GetAddress());
    }
    for (size_t i = 0; i < (*this)->block->get_num_outputs(); i++)
    {
        OutputUpdateMessage message;
        message.index = i;
        actor.GetFramework().Send(message, Theron::Address::Null(), actor.GetAddress());
    }

}

void Block::notify_active(void)
{
    //NOP
}

void Block::notify_inactive(void)
{
    //NOP
}

void Block::notify_topology(const size_t, const size_t)
{
    return;
}

void Block::set_buffer_affinity(const long affinity)
{
    (*this)->block->buffer_affinity = affinity;
}

void Block::set_interruptible_work(const bool enb)
{
    (*this)->block->interruptible_work = enb;
}
