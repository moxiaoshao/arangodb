////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2016 ArangoDB GmbH, Cologne, Germany
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
/// @author Jan Christoph Uhde
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_REST_HANDLER_REST_TRANSACTION_HANDLER_H
#define ARANGOD_REST_HANDLER_REST_TRANSACTION_HANDLER_H 1

#include "Basics/ReadWriteLock.h"
#include "RestHandler/RestVocbaseBaseHandler.h"
#include "VocBase/Methods/Transactions.h"

namespace arangodb {

class V8Context;

typedef Result (*executeTransaction_t)(
    v8::Isolate* isolate,
    basics::ReadWriteLock& lock,
    std::atomic<bool>& canceled,
    VPackSlice slice,
    std::string portType,
    VPackBuilder& builder);

class RestTransactionHandler : public arangodb::RestVocbaseBaseHandler {
  V8Context* _v8Context;
  basics::ReadWriteLock _lock;

 public:
  // this allows swapping out the executeTransaction function
  //  during unit tests
  static executeTransaction_t _executeTransactionPtr;

 public:
  RestTransactionHandler(GeneralRequest*, GeneralResponse*);

 public:
  char const* name() const override final { return "RestTransactionHandler"; }
  bool isDirect() const override { return false; }
  RestStatus execute() override;
  bool cancel() override final;

private:
  void returnContext();
};
}

#endif