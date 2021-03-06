/*
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include "CellularStateMachine.h"
#include "CellularDevice.h"

namespace mbed {

CellularStateMachine::CellularStateMachine(CellularDevice &device, events::EventQueue &queue) :
        _cellularDevice(device), _queue(queue)
{

}

CellularStateMachine::~CellularStateMachine()
{
}

void CellularStateMachine::stop()
{
}

void CellularStateMachine::set_sim_pin(const char *sim_pin)
{
}

void CellularStateMachine::set_plmn(const char *plmn)
{
}

nsapi_error_t CellularStateMachine::run_to_state(CellularStateMachine::CellularState state)
{
    return NSAPI_ERROR_OK;
}

bool CellularStateMachine::get_current_status(CellularStateMachine::CellularState &current_state, CellularStateMachine::CellularState &target_state)
{
    return true;
}

nsapi_error_t CellularStateMachine::start_dispatch()
{
    return NSAPI_ERROR_OK;
}

void CellularStateMachine::set_cellular_callback(mbed::Callback<void(nsapi_event_t, intptr_t)> status_cb)
{
}

}
