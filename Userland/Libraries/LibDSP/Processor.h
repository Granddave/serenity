/*
 * Copyright (c) 2021, kleines Filmröllchen <malu.bertsch@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Noncopyable.h>
#include <AK/StdLibExtras.h>
#include <AK/Types.h>
#include <LibCore/Object.h>
#include <LibDSP/Music.h>
#include <LibDSP/ProcessorParameter.h>
#include <LibDSP/Transport.h>

namespace LibDSP {

// A processor processes notes or audio into notes or audio. Processors are e.g. samplers, synthesizers, effects, arpeggiators etc.
class Processor : public Core::Object {
    C_OBJECT_ABSTRACT(Processor);

public:
    ~Processor() override = default;

    Signal process(Signal const& input_signal)
    {
        VERIFY(input_signal.type() == m_input_type);
        auto processed = process_impl(input_signal);
        VERIFY(processed.type() == m_output_type);
        return processed;
    }
    SignalType input_type() const { return m_input_type; }
    SignalType output_type() const { return m_output_type; }
    Vector<ProcessorParameter&>& parameters() { return m_parameters; }

    template<typename Callback>
    void for_each_parameter(Callback callback)
    {
        for (auto& parameter : m_parameters) {
            if (callback(parameter) == IterationDecision::Break)
                return;
        }
    }

    template<typename T, typename Callback>
    inline void for_each_parameter_of_type(Callback callback) requires IsBaseOf<ProcessorParameter, T>
    {
        for_each_parameter([&](auto& parameter) {
            if (is<T>(parameter))
                return callback(static_cast<T&>(parameter));
            return IterationDecision::Continue;
        });
    }

    template<typename T>
    T* find_parameter_of_type_named(String const& name) requires IsBaseOf<ProcessorParameter, T>
    {
        T* found_parameter = nullptr;
        for_each_parameter_of_type<T>([&](auto& parameter) {
            if (parameter.name() == name) {
                found_parameter = &parameter;
                return IterationDecision::Break;
            }
            return IterationDecision::Continue;
        });

        return found_parameter;
    }

private:
    SignalType const m_input_type;
    SignalType const m_output_type;

protected:
    Processor(NonnullRefPtr<Transport> transport, SignalType input_type, SignalType output_type)
        : m_input_type(input_type)
        , m_output_type(output_type)
        , m_transport(move(transport))
    {
    }
    virtual Signal process_impl(Signal const& input_signal) = 0;

    NonnullRefPtr<Transport> m_transport;
    Vector<ProcessorParameter&> m_parameters;
};

// A common type of processor that changes audio data, i.e. applies an effect to it.
class EffectProcessor : public Processor {
public:
    ~EffectProcessor() override = default;

protected:
    EffectProcessor(NonnullRefPtr<Transport> transport)
        : Processor(transport, SignalType::Sample, SignalType::Sample)
    {
    }
};

// A common type of processor that synthesizes audio from note data.
class SynthesizerProcessor : public Processor {
public:
    ~SynthesizerProcessor() override = default;

protected:
    SynthesizerProcessor(NonnullRefPtr<Transport> transport)
        : Processor(transport, SignalType::Note, SignalType::Sample)
    {
    }
};

}
