/*******************************************************************************
* Copyright (c) 2018 Anakin Authors All Rights Reserve.
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
*******************************************************************************/

#ifndef SABER_FUNC_IMPL_X86_Z_MAGIC_H
#define SABER_FUNC_IMPL_X86_Z_MAGIC_H

#define CHAIn2(a, b) a b
#define CHAIN2(a, b) CHAIn2(a, b)

#define CONCAt2(a, b) a ## b
#define CONCAT2(a, b) CONCAt2(a, b)

#define STRINGIFy(s) #s
#define STRINGIFY(s) STRINGIFy(s)

#define PRAGMA_MACRo(x) _Pragma(#x)
#define PRAGMA_MACRO(x) PRAGMA_MACRo(x)

#endif

// vim: et ts=4 sw=4 cindent cino^=l0,\:0,N-s
