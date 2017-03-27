////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2017 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Daniel H. Larkin
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGO_ROCKSDB_ROCKSDB_TYPES_H
#define ARANGO_ROCKSDB_ROCKSDB_TYPES_H 1

#include "Basics/Common.h"
#include "Basics/Result.h"

#include <rocksdb/status.h>

namespace arangodb {
namespace rocksdb {

enum StatusHint { none, document, collection, view, index, database };

arangodb::Result convertStatus(::rocksdb::Status const&,
                               StatusHint hint = StatusHint::none);

}  // namespace rocksdb
}  // namespace arangodb

#endif
