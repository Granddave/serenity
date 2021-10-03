/*
 * Copyright (c) 2021, kleines Filmröllchen <malu.bertsch@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "WidgetWithLabel.h"
#include <LibDSP/ProcessorParameter.h>
#include <LibGUI/Label.h>
#include <LibGUI/Slider.h>
#include <LibGfx/Orientation.h>

class ProcessorParameterSlider : public GUI::Slider
    , public WidgetWithLabel {
    C_OBJECT(ProcessorParameterSlider);

public:
    ProcessorParameterSlider(Orientation, LibDSP::ProcessorRangeParameter&, RefPtr<GUI::Label>, double label_value_scale = 1.0);

protected:
    LibDSP::ProcessorRangeParameter& m_parameter;
};
