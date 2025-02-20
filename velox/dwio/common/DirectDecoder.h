/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#pragma once

#include "velox/dwio/common/DecoderUtil.h"
#include "velox/dwio/common/IntDecoder.h"

namespace facebook::velox::dwio::common {

struct DropValues;

template <bool isSigned>
class DirectDecoder : public IntDecoder<isSigned> {
 public:
  DirectDecoder(
      std::unique_ptr<dwio::common::SeekableInputStream> input,
      bool useVInts,
      uint32_t numBytes,
      bool bigEndian = false)
      : IntDecoder<isSigned>{std::move(input), useVInts, numBytes, bigEndian} {}

  void seekToRowGroup(dwio::common::PositionProvider&) override;

  using IntDecoder<isSigned>::skip;

  void skipPending() final {
    const auto toSkip = this->pendingSkip_;
    this->pendingSkip_ = 0;
    this->skipLongs(toSkip);
  }

  template <typename T>
  void nextValues(T* data, uint64_t numValues, const uint64_t* nulls);

  void next(int64_t* data, uint64_t numValues, const uint64_t* nulls) override {
    nextValues<int64_t>(data, numValues, nulls);
  }

  template <bool hasNulls, typename Visitor>
  void readWithVisitor(
      const uint64_t* nulls,
      Visitor visitor,
      bool useFastPath = true) {
    skipPending();
    if constexpr (!std::is_same_v<typename Visitor::DataType, int128_t>) {
      if (useFastPath &&
          dwio::common::useFastPath<Visitor, hasNulls>(visitor)) {
        fastPath<hasNulls>(nulls, visitor);
        return;
      }
    }

    int32_t current = visitor.start();
    this->template skip<hasNulls>(current, 0, nulls);
    const bool allowNulls = hasNulls && visitor.allowNulls();
    for (;;) {
      bool atEnd = false;
      int32_t toSkip;
      if (hasNulls) {
        if (!allowNulls) {
          toSkip = visitor.checkAndSkipNulls(nulls, current, atEnd);
          if (!Visitor::dense) {
            this->template skip<false>(toSkip, current, nullptr);
          }
          if (atEnd) {
            return;
          }
        } else {
          if (bits::isBitNull(nulls, current)) {
            toSkip = visitor.processNull(atEnd);
            goto skip;
          }
        }
      }
      if constexpr (std::is_same_v<typename Visitor::DataType, float>) {
        toSkip = visitor.process(readFloat(), atEnd);
      } else if constexpr (std::is_same_v<typename Visitor::DataType, double>) {
        toSkip = visitor.process(readDouble(), atEnd);
      } else if constexpr (std::is_same_v<
                               typename Visitor::DataType,
                               int128_t>) {
        if (super::numBytes_ == 12 /* INT96 */) {
          int128_t encoded = super::template readInt<int128_t>();
          int32_t days = encoded & ((1ULL << 32) - 1);
          uint64_t nanos = static_cast<uint64_t>(encoded >> 32);

          auto timestamp = Timestamp::fromDaysAndNanos(days, nanos);
          toSkip =
              visitor.process(*reinterpret_cast<int128_t*>(&timestamp), atEnd);
        } else {
          toSkip = visitor.process(super::template readInt<int128_t>(), atEnd);
        }
      } else {
        toSkip = visitor.process(super::template readInt<int64_t>(), atEnd);
      }

    skip:
      ++current;
      if (toSkip) {
        this->template skip<hasNulls>(toSkip, current, nulls);
        current += toSkip;
      }
      if (atEnd) {
        return;
      }
    }
  }

 private:
  using super = IntDecoder<isSigned>;

  float readFloat() {
    float temp;
    auto buffer = readFixed(sizeof(float), &temp);
    return *reinterpret_cast<const float*>(buffer);
  }

  double readDouble() {
    double temp;
    auto buffer = readFixed(sizeof(double), &temp);
    return *reinterpret_cast<const double*>(buffer);
  }

  // Returns a pointer to the next element of 'size' bytes in the
  // buffer. If the element would straddle buffers, it is copied to
  // *temp and temp is returned.
  const void* readFixed(int32_t size, void* temp) {
    skipPending();
    auto ptr = super::bufferStart_;
    if (ptr && ptr + size <= super::bufferEnd_) {
      super::bufferStart_ += size;
      return ptr;
    }
    readBytes(
        size,
        super::inputStream_.get(),
        temp,
        super::bufferStart_,
        super::bufferEnd_);
    return temp;
  }

  template <bool hasNulls, typename Visitor>
  void fastPath(const uint64_t* nulls, Visitor& visitor) {
    using T = typename Visitor::DataType;
    constexpr bool hasFilter =
        !std::
            is_same_v<typename Visitor::FilterType, velox::common::AlwaysTrue>;
    constexpr bool filterOnly =
        std::is_same_v<typename Visitor::Extract, DropValues>;
    constexpr bool hasHook =
        !std::is_same_v<typename Visitor::HookType, dwio::common::NoHook>;

    int32_t numValues = 0;
    auto rows = visitor.rows();
    auto numRows = visitor.numRows();
    auto numNonNull = numRows;
    auto rowsAsRange = folly::Range<const int32_t*>(rows, numRows);
    auto data = visitor.rawValues(numRows);
    if (hasNulls) {
      int32_t tailSkip = 0;
      raw_vector<int32_t>* innerVector = nullptr;
      auto outerVector = &visitor.outerNonNullRows();
      // In non-DWRF formats, it can be the visitor is not dense but
      // this run of rows is dense.
      if (Visitor::dense || rowsAsRange.back() == rowsAsRange.size() - 1) {
        dwio::common::nonNullRowsFromDense(nulls, numRows, *outerVector);
        numNonNull = outerVector->size();
        if (!numNonNull) {
          visitor.setAllNull(hasFilter ? 0 : numRows);
          return;
        }
      } else {
        innerVector = &visitor.innerNonNullRows();
        auto anyNulls = dwio::common::nonNullRowsFromSparse < hasFilter,
             !hasFilter &&
            !hasHook >
                (nulls,
                 rowsAsRange,
                 *innerVector,
                 *outerVector,
                 (hasFilter || hasHook) ? nullptr : visitor.rawNulls(numRows),
                 tailSkip);
        if (anyNulls) {
          visitor.setHasNulls();
        }
        if (innerVector->empty()) {
          this->template skip<false>(tailSkip, 0, nullptr);
          visitor.setAllNull(hasFilter ? 0 : numRows);
          return;
        }
      }
      if (hasHook && visitor.numValuesBias() > 0) {
        for (auto& row : *outerVector) {
          row += visitor.numValuesBias();
        }
      }
      if (super::useVInts_) {
        if (Visitor::dense) {
          super::bulkRead(numNonNull, data);
        } else {
          super::bulkReadRows(*innerVector, data);
        }
        this->template skip<false>(tailSkip, 0, nullptr);
        auto dataRows = innerVector
            ? folly::Range<const int*>(innerVector->data(), innerVector->size())
            : folly::Range<const int32_t*>(rows, outerVector->size());
        dwio::common::processFixedWidthRun<T, filterOnly, true, Visitor::dense>(
            dataRows,
            0,
            dataRows.size(),
            outerVector->data(),
            data,
            hasFilter ? visitor.outputRows(numRows) : nullptr,
            numValues,
            visitor.filter(),
            visitor.hook());
      } else {
        dwio::common::fixedWidthScan<T, filterOnly, true>(
            innerVector
                ? folly::Range<const int32_t*>(*innerVector)
                : folly::Range<const int32_t*>(rows, outerVector->size()),
            outerVector->data(),
            visitor.rawValues(numRows),
            hasFilter ? visitor.outputRows(numRows) : nullptr,
            numValues,
            *super::inputStream_,
            super::bufferStart_,
            super::bufferEnd_,
            visitor.filter(),
            visitor.hook());
        this->template skip<false>(tailSkip, 0, nullptr);
      }
    } else {
      if (super::useVInts_) {
        if (Visitor::dense) {
          super::bulkRead(numRows, visitor.rawValues(numRows));
        } else {
          super::bulkReadRows(
              folly::Range<const int32_t*>(rows, numRows),
              visitor.rawValues(numRows));
        }
        dwio::common::
            processFixedWidthRun<T, filterOnly, false, Visitor::dense>(
                rowsAsRange,
                0,
                rowsAsRange.size(),
                hasHook ? velox::iota(
                              numRows,
                              visitor.innerNonNullRows(),
                              visitor.numValuesBias())
                        : nullptr,
                visitor.rawValues(numRows),
                hasFilter ? visitor.outputRows(numRows) : nullptr,
                numValues,
                visitor.filter(),
                visitor.hook());
      } else {
        dwio::common::fixedWidthScan<T, filterOnly, false>(
            rowsAsRange,
            hasHook ? velox::iota(
                          numRows,
                          visitor.innerNonNullRows(),
                          visitor.numValuesBias())
                    : nullptr,
            visitor.rawValues(numRows),
            hasFilter ? visitor.outputRows(numRows) : nullptr,
            numValues,
            *super::inputStream_,
            super::bufferStart_,
            super::bufferEnd_,
            visitor.filter(),
            visitor.hook());
      }
    }
    visitor.setNumValues(hasFilter ? numValues : numRows);
  }
};

} // namespace facebook::velox::dwio::common
