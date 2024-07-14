
/*
 * Copyright (C) 2020-2022  GreenWaves Technologies, ETH Zurich, University of Bologna
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

/* 
 * Authors: Arpan Suravi Prasad, ETH Zurich (prasadar@iis.ee.ethz.ch)
 */
#include "hwpe.hpp"
#include <type_traits>

#include <string>
#include <sstream>
void Hwpe::FsmStartHandler(vp::Block *__this, vp::ClockEvent *event) {// makes sense to move it to task manager
  Hwpe *_this = (Hwpe *)__this;
  ////////////////////////////////////////   TASK - 3  //////////////////////////////////////// 
  // Add a trace message in the FsmStartHandler 
}
void Hwpe::FsmHandler(vp::Block *__this, vp::ClockEvent *event) {
  Hwpe *_this = (Hwpe *)__this;
}
void Hwpe::FsmEndHandler(vp::Block *__this, vp::ClockEvent *event) {// makes sense to move it to task manager
  Hwpe *_this = (Hwpe *)__this;
}

