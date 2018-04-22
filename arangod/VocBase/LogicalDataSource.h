////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2018 ArangoDB GmbH, Cologne, Germany
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
/// @author Andrey Abramov
/// @author Vasiliy Nabatchikov
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD__VOCBASE__LOGICAL_DATA_SOURCE_H
#define ARANGOD__VOCBASE__LOGICAL_DATA_SOURCE_H 1

#include "voc-types.h"
#include "Basics/Result.h"

struct TRI_vocbase_t; // forward declaration

namespace arangodb {

namespace velocypack {

class StringRef; // forward declaration

}

////////////////////////////////////////////////////////////////////////////////
/// @brief a common ancestor to all database objects proving access to documents
///        e.g. LogicalCollection / LoigcalView
////////////////////////////////////////////////////////////////////////////////
class LogicalDataSource {
 public:

  //////////////////////////////////////////////////////////////////////////////
  /// @brief singleton marker identifying the logical data-source category
  ///        each category is identity-compared for equivalence
  ///        e.g. static Category const& LogicalCollection::category()
  ///             static Category const& LogicalView::category()
  //////////////////////////////////////////////////////////////////////////////
  class Category final {
   public:
    Category() {}
    Category(Category const&) = delete;
    Category(Category&&) noexcept = delete;
    Category& operator=(Category const&) = delete;
    Category& operator=(Category&&) noexcept = delete;
    bool operator==(Category const& other) const noexcept { return this == &other; }
    bool operator!=(Category const& other) const noexcept { return this != &other; }
  };

  //////////////////////////////////////////////////////////////////////////////
  /// @brief singleton identifying the underlying implementation type
  ///        each implementation should have its own static instance
  ///        once a type is emplace(...)ed it cannot be removed
  //////////////////////////////////////////////////////////////////////////////
  class Type final {
   public:
    Type(Type&& other) noexcept = default;
    bool operator==(Type const& other) const noexcept { return this == &other; }
    bool operator!=(Type const& other) const noexcept { return this != &other; }
    static Type const& emplace(arangodb::velocypack::StringRef const& name);
    std::string const& name() const noexcept { return _name; }

   private:
    std::string _name; // type name for e.g. log messages

    Type() = default;
    Type(Type const&) = delete;
    Type& operator=(Type const&) = delete;
    Type& operator=(Type&&) noexcept = delete;
  };

  LogicalDataSource(
      Category const& category,
      Type const& type,
      TRI_vocbase_t& vocbase,
      TRI_voc_cid_t id,
      TRI_voc_cid_t planId,
      std::string&& name,
      uint64_t planVersion,
      bool system,
      bool deleted
  ) noexcept
    : _name(std::move(name)),
      _category(category),
      _type(type),
      _vocbase(vocbase),
      _id(id),
      _planId(planId ? planId : id),
      _planVersion(planVersion),
      _deleted(deleted),
      _system(system) {
  }

  LogicalDataSource(LogicalDataSource const& other)
    : _name(other._name),
      _category(other._category),
      _type(other._type),
      _vocbase(other._vocbase),
      _id(other._id),
      _planId(other._planId),
      _planVersion(other._planVersion),
      _deleted(other._deleted),
      _system(other._system) {
  }

  virtual ~LogicalDataSource() = default;

  Category const& category() const noexcept { return _category; }
  bool deleted() const noexcept { return _deleted; }
  virtual arangodb::Result drop() = 0;
  TRI_voc_cid_t const& id() const noexcept { return _id; } // reference required for ShardDistributionReporterTest
  std::string const& name() const noexcept { return _name; }
  TRI_voc_cid_t planId() const noexcept { return _planId; }
  uint64_t planVersion() const noexcept { return _planVersion; }
  virtual Result rename(std::string&& newName, bool doSync) = 0;
  bool system() const noexcept { return _system; }
  Type const& type() const noexcept { return _type; }
  TRI_vocbase_t& vocbase() const noexcept { return _vocbase; }

 protected:
  void deleted(bool deleted) noexcept { _deleted = deleted; }
  void name(std::string&& name) noexcept { _name = std::move(name); }

 private:
  // members ordered by sizeof(decltype(..))
  std::string _name; // data-source name
  Category const& _category; // the category of the logical data-source
  Type const& _type; // the type of the underlying data-source implementation
  TRI_vocbase_t& _vocbase; // the database where the data-source resides
  TRI_voc_cid_t const _id; // local data-source id (current database node)
  TRI_voc_cid_t const _planId; // global data-source id (cluster-wide)
  uint64_t const _planVersion; // Only set if setPlanVersion was called. This only
                           // happens in ClusterInfo when this object is used
                           // to represent a cluster wide collection. This is
                           // then the version in the agency Plan that underpins
                           // the information in this object. Otherwise 0.
  bool _deleted; // data-source marked as deleted
  bool const _system; // this instance represents a system data-source
};

} // arangodb

#endif // ARANGOD__VOCBASE__LOGICAL_DATA_SOURCE_H
