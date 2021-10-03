/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2019-2020, William McPherson <willmcpherson2@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "KnobsWidget.h"
#include "MainWidget.h"
#include "ProcessorParameterWidget/Slider.h"
#include "TrackManager.h"
#include <LibDSP/ProcessorParameter.h>
#include <LibGUI/BoxLayout.h>
#include <LibGUI/Label.h>
#include <LibGUI/Slider.h>
#include <LibGfx/Orientation.h>

KnobsWidget::KnobsWidget(TrackManager& track_manager, MainWidget& main_widget)
    : m_track_manager(track_manager)
    , m_main_widget(main_widget)
{
    set_layout<GUI::VerticalBoxLayout>();
    set_fill_with_background_color(true);

    m_labels_container = add<GUI::Widget>();
    m_labels_container->set_layout<GUI::HorizontalBoxLayout>();
    m_labels_container->set_fixed_height(45);

    m_values_container = add<GUI::Widget>();
    m_values_container->set_layout<GUI::HorizontalBoxLayout>();
    m_values_container->set_fixed_height(10);

    m_knobs_container = add<GUI::Widget>();
    m_knobs_container->set_layout<GUI::HorizontalBoxLayout>();

    // FIXME: Implement vertical flipping in GUI::Slider, not here.

    auto& master_parameter = *m_track_manager.current_track().master()->find_parameter_of_type_named<LibDSP::ProcessorRangeParameter>("Master");
    m_volume_value = m_values_container->add<GUI::Label>(String::formatted("{:.1f}", (1 - static_cast<double>(master_parameter.value())) * 100.0));
    m_volume_label = m_labels_container->add<GUI::Label>("Volume");
    m_volume_knob = m_knobs_container->add<ProcessorParameterSlider>(Orientation::Vertical, master_parameter, m_volume_value, 100.0);

    auto& pan_parameter = *m_track_manager.current_track().master()->find_parameter_of_type_named<LibDSP::ProcessorRangeParameter>("Pan");
    m_pan_value = m_values_container->add<GUI::Label>(String::formatted("{:.1f}", static_cast<double>(pan_parameter.value())));
    m_pan_label = m_labels_container->add<GUI::Label>("Pan");
    m_pan_knob = m_knobs_container->add<ProcessorParameterSlider>(Orientation::Vertical, pan_parameter, m_pan_value);

    m_octave_knob = m_knobs_container->add<GUI::VerticalSlider>();
    m_octave_knob->set_tooltip("Z: octave down, X: octave up");
    m_octave_knob->set_range(octave_min - 1, octave_max - 1);
    m_octave_knob->set_value((octave_max - 1) - (m_track_manager.octave() - 1));
    m_octave_knob->set_step(1);
    m_octave_value = m_values_container->add<GUI::Label>(String::number(m_track_manager.octave()));
    m_octave_label = m_labels_container->add<GUI::Label>("Octave");
    m_octave_knob->on_change = [this](int value) {
        int new_octave = octave_max - value;
        m_main_widget.set_octave_and_ensure_note_change(new_octave);
        VERIFY(new_octave == m_track_manager.octave());
        m_octave_value->set_text(String::number(new_octave));
    };

    for (auto& raw_parameter : m_track_manager.current_track().synth()->parameters()) {
        // The synth has range and enum parameters
        switch (raw_parameter.type()) {
        case LibDSP::ParameterType::Range: {
            auto& parameter = static_cast<LibDSP::ProcessorRangeParameter&>(raw_parameter);
            m_synth_values.append(m_values_container->add<GUI::Label>(String::number(static_cast<double>(parameter.value()))));
            auto& parameter_knob_value = m_synth_values.last();
            m_synth_labels.append(m_labels_container->add<GUI::Label>(String::formatted("Synth: {}", parameter.name())));
            m_synth_knobs.append(m_knobs_container->add<ProcessorParameterSlider>(Orientation::Vertical, parameter, parameter_knob_value));
            break;
        }
        case LibDSP::ParameterType::Enum: {
            // FIXME: We shouldn't do that, but we know the synth and it is nice
            auto& parameter = static_cast<LibDSP::ProcessorEnumParameter<LibDSP::Synthesizers::Waveform>&>(raw_parameter);
            // The value is empty for enum parameters
            m_synth_values.append(m_values_container->add<GUI::Label>(String::empty()));
            m_synth_labels.append(m_labels_container->add<GUI::Label>(String::formatted("Synth: {}", parameter.name())));
            auto enum_strings = Vector<String> { "Sine", "Triangle", "Square", "Saw", "Noise" };
            m_synth_knobs.append(m_knobs_container->add<ProcessorParameterDropdown<LibDSP::Synthesizers::Waveform>>(parameter, move(enum_strings)));
            m_synth_waveform = static_cast<ProcessorParameterDropdown<LibDSP::Synthesizers::Waveform>&>(m_synth_knobs.last());
            break;
        }
        default:
            VERIFY_NOT_REACHED();
        }
    }

    for (auto& raw_parameter : m_track_manager.current_track().delay()->parameters()) {
        // FIXME: We shouldn't do that, but we know the effect and it's nice.
        auto& parameter = static_cast<LibDSP::ProcessorRangeParameter&>(raw_parameter);
        m_delay_values.append(m_values_container->add<GUI::Label>(String::number(static_cast<double>(parameter.value()))));
        auto& parameter_knob_value = m_delay_values.last();
        m_delay_labels.append(m_labels_container->add<GUI::Label>(String::formatted("Delay: {}", parameter.name())));
        m_delay_knobs.append(m_knobs_container->add<ProcessorParameterSlider>(Orientation::Vertical, parameter, parameter_knob_value));
    }
}

KnobsWidget::~KnobsWidget()
{
}

void KnobsWidget::cycle_waveform()
{
    m_synth_waveform->set_selected_index((m_synth_waveform->selected_index() + 1) % m_synth_waveform->model()->row_count());
}

void KnobsWidget::update_knobs()
{
    m_octave_knob->set_value(octave_max - m_track_manager.octave(), GUI::AllowCallback::No);
}
