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

#include <optional>
#include <string>
#include "velox/common/base/Exceptions.h"

namespace facebook::velox::config {
class ConfigBase;
}

namespace facebook::velox::connector::hive {

/// Hive connector configs.
class HiveConfig {
 public:
  enum class InsertExistingPartitionsBehavior {
    kError,
    kOverwrite,
  };

  static std::string insertExistingPartitionsBehaviorString(
      InsertExistingPartitionsBehavior behavior);

  /// Behavior on insert into existing partitions.
  static constexpr const char* kInsertExistingPartitionsBehaviorSession =
      "insert_existing_partitions_behavior";
  static constexpr const char* kInsertExistingPartitionsBehavior =
      "insert-existing-partitions-behavior";

  /// Maximum number of (bucketed) partitions per a single table writer
  /// instance.
  ///
  /// TODO: remove hive_orc_use_column_names since it doesn't exist in presto,
  /// right now this is only used for testing.
  static constexpr const char* kMaxPartitionsPerWriters =
      "max-partitions-per-writers";
  static constexpr const char* kMaxPartitionsPerWritersSession =
      "max_partitions_per_writers";

  /// Whether new data can be inserted into an unpartition table.
  /// Velox currently does not support appending data to existing partitions.
  static constexpr const char* kImmutablePartitions =
      "hive.immutable-partitions";

  /// Virtual addressing is used for AWS S3 and is the default
  /// (path-style-access is false). Path access style is used for some on-prem
  /// systems like Minio.
  static constexpr const char* kS3PathStyleAccess = "hive.s3.path-style-access";

  /// Log granularity of AWS C++ SDK.
  static constexpr const char* kS3LogLevel = "hive.s3.log-level";

  /// Use HTTPS to communicate with the S3 API.
  static constexpr const char* kS3SSLEnabled = "hive.s3.ssl.enabled";

  /// Use the EC2 metadata service to retrieve API credentials.
  static constexpr const char* kS3UseInstanceCredentials =
      "hive.s3.use-instance-credentials";

  /// The S3 storage endpoint server. This can be used to connect to an
  /// S3-compatible storage system instead of AWS.
  static constexpr const char* kS3Endpoint = "hive.s3.endpoint";

  /// Default AWS access key to use.
  static constexpr const char* kS3AwsAccessKey = "hive.s3.aws-access-key";

  /// Default AWS secret key to use.
  static constexpr const char* kS3AwsSecretKey = "hive.s3.aws-secret-key";

  /// IAM role to assume.
  static constexpr const char* kS3IamRole = "hive.s3.iam-role";

  /// Session name associated with the IAM role.
  static constexpr const char* kS3IamRoleSessionName =
      "hive.s3.iam-role-session-name";

  /// Socket connect timeout.
  static constexpr const char* kS3ConnectTimeout = "hive.s3.connect-timeout";

  /// Socket read timeout.
  static constexpr const char* kS3SocketTimeout = "hive.s3.socket-timeout";

  /// Maximum concurrent TCP connections for a single http client.
  static constexpr const char* kS3MaxConnections = "hive.s3.max-connections";

  /// Maximum retry attempts for a single http client.
  static constexpr const char* kS3MaxAttempts = "hive.s3.max-attempts";

  /// Retry mode for a single http client.
  static constexpr const char* kS3RetryMode = "hive.s3.retry-mode";

  /// The GCS storage endpoint server.
  static constexpr const char* kGCSEndpoint = "hive.gcs.endpoint";

  /// The GCS storage scheme, https for default credentials.
  static constexpr const char* kGCSScheme = "hive.gcs.scheme";

  /// The GCS service account configuration JSON key file.
  static constexpr const char* kGCSCredentialsPath =
      "hive.gcs.json-key-file-path";

  /// The GCS maximum retry counter of transient errors.
  static constexpr const char* kGCSMaxRetryCount = "hive.gcs.max-retry-count";

  /// The GCS maximum time allowed to retry transient errors.
  static constexpr const char* kGCSMaxRetryTime = "hive.gcs.max-retry-time";

  /// Maps table field names to file field names using names, not indices.
  // TODO: remove hive_orc_use_column_names since it doesn't exist in presto,
  // right now this is only used for testing.
  static constexpr const char* kOrcUseColumnNames = "hive.orc.use-column-names";
  static constexpr const char* kOrcUseColumnNamesSession =
      "hive_orc_use_column_names";

  /// Maps table field names to file field names using names, not indices.
  static constexpr const char* kParquetUseColumnNames =
      "hive.parquet.use-column-names";
  static constexpr const char* kParquetUseColumnNamesSession =
      "parquet_use_column_names";

  /// Reads the source file column name as lower case.
  static constexpr const char* kFileColumnNamesReadAsLowerCase =
      "file-column-names-read-as-lower-case";
  static constexpr const char* kFileColumnNamesReadAsLowerCaseSession =
      "file_column_names_read_as_lower_case";

  static constexpr const char* kPartitionPathAsLowerCaseSession =
      "partition_path_as_lower_case";

  static constexpr const char* kAllowNullPartitionKeys =
      "allow-null-partition-keys";
  static constexpr const char* kAllowNullPartitionKeysSession =
      "allow_null_partition_keys";

  static constexpr const char* kIgnoreMissingFilesSession =
      "ignore_missing_files";

  /// The max coalesce bytes for a request.
  static constexpr const char* kMaxCoalescedBytes = "max-coalesced-bytes";

  /// The max coalesce distance bytes for combining requests.
  static constexpr const char* kMaxCoalescedDistanceBytes =
      "max-coalesced-distance-bytes";

  /// The number of prefetch rowgroups
  static constexpr const char* kPrefetchRowGroups = "prefetch-rowgroups";

  /// The total size in bytes for a direct coalesce request. Up to 8MB load
  /// quantum size is supported when SSD cache is enabled.
  static constexpr const char* kLoadQuantum = "load-quantum";

  /// Maximum number of entries in the file handle cache.
  static constexpr const char* kNumCacheFileHandles = "num_cached_file_handles";

  /// Enable file handle cache.
  static constexpr const char* kEnableFileHandleCache =
      "file-handle-cache-enabled";

  /// The size in bytes to be fetched with Meta data together, used when the
  /// data after meta data will be used later. Optimization to decrease small IO
  /// request
  static constexpr const char* kFooterEstimatedSize = "footer-estimated-size";

  /// The threshold of file size in bytes when the whole file is fetched with
  /// meta data together. Optimization to decrease the small IO requests
  static constexpr const char* kFilePreloadThreshold = "file-preload-threshold";

  /// Maximum stripe size in orc writer.
  static constexpr const char* kOrcWriterMaxStripeSize =
      "hive.orc.writer.stripe-max-size";
  static constexpr const char* kOrcWriterMaxStripeSizeSession =
      "orc_optimized_writer_max_stripe_size";

  /// Maximum dictionary memory that can be used in orc writer.
  static constexpr const char* kOrcWriterMaxDictionaryMemory =
      "hive.orc.writer.dictionary-max-memory";
  static constexpr const char* kOrcWriterMaxDictionaryMemorySession =
      "orc_optimized_writer_max_dictionary_memory";

  /// Configs to control dictionary encoding.
  static constexpr const char* kOrcWriterIntegerDictionaryEncodingEnabled =
      "hive.orc.writer.integer-dictionary-encoding-enabled";
  static constexpr const char*
      kOrcWriterIntegerDictionaryEncodingEnabledSession =
          "orc_optimized_writer_integer_dictionary_encoding_enabled";
  static constexpr const char* kOrcWriterStringDictionaryEncodingEnabled =
      "hive.orc.writer.string-dictionary-encoding-enabled";
  static constexpr const char*
      kOrcWriterStringDictionaryEncodingEnabledSession =
          "orc_optimized_writer_string_dictionary_encoding_enabled";

  /// Enables historical based stripe size estimation after compression.
  static constexpr const char* kOrcWriterLinearStripeSizeHeuristics =
      "hive.orc.writer.linear-stripe-size-heuristics";
  static constexpr const char* kOrcWriterLinearStripeSizeHeuristicsSession =
      "orc_writer_linear_stripe_size_heuristics";

  /// Minimal number of items in an encoded stream.
  static constexpr const char* kOrcWriterMinCompressionSize =
      "hive.orc.writer.min-compression-size";
  static constexpr const char* kOrcWriterMinCompressionSizeSession =
      "orc_writer_min_compression_size";

  /// The compression level to use with ZLIB and ZSTD.
  static constexpr const char* kOrcWriterCompressionLevel =
      "hive.orc.writer.compression-level";
  static constexpr const char* kOrcWriterCompressionLevelSession =
      "orc_optimized_writer_compression_level";

  /// Config used to create write files. This config is provided to underlying
  /// file system through hive connector and data sink. The config is free form.
  /// The form should be defined by the underlying file system.
  static constexpr const char* kWriteFileCreateConfig =
      "hive.write_file_create_config";

  /// Maximum number of rows for sort writer in one batch of output.
  static constexpr const char* kSortWriterMaxOutputRows =
      "sort-writer-max-output-rows";
  static constexpr const char* kSortWriterMaxOutputRowsSession =
      "sort_writer_max_output_rows";

  /// Maximum bytes for sort writer in one batch of output.
  static constexpr const char* kSortWriterMaxOutputBytes =
      "sort-writer-max-output-bytes";
  static constexpr const char* kSortWriterMaxOutputBytesSession =
      "sort_writer_max_output_bytes";

  /// Sort Writer will exit finish() method after this many milliseconds even if
  /// it has not completed its work yet. Zero means no time limit.
  static constexpr const char* kSortWriterFinishTimeSliceLimitMs =
      "sort-writer_finish_time_slice_limit_ms";
  static constexpr const char* kSortWriterFinishTimeSliceLimitMsSession =
      "sort_writer_finish_time_slice_limit_ms";

  static constexpr const char* kS3UseProxyFromEnv =
      "hive.s3.use-proxy-from-env";

  // The unit for reading timestamps from files.
  static constexpr const char* kReadTimestampUnit =
      "hive.reader.timestamp-unit";
  static constexpr const char* kReadTimestampUnitSession =
      "hive.reader.timestamp_unit";

  static constexpr const char* kCacheNoRetention = "cache.no_retention";
  static constexpr const char* kCacheNoRetentionSession = "cache.no_retention";

  InsertExistingPartitionsBehavior insertExistingPartitionsBehavior(
      const config::ConfigBase* session) const;

  uint32_t maxPartitionsPerWriters(const config::ConfigBase* session) const;

  bool immutablePartitions() const;

  bool s3UseVirtualAddressing() const;

  std::string s3GetLogLevel() const;

  bool s3UseSSL() const;

  bool s3UseInstanceCredentials() const;

  std::string s3Endpoint() const;

  std::optional<std::string> s3AccessKey() const;

  std::optional<std::string> s3SecretKey() const;

  std::optional<std::string> s3IAMRole() const;

  std::string s3IAMRoleSessionName() const;

  std::optional<std::string> s3ConnectTimeout() const;

  std::optional<std::string> s3SocketTimeout() const;

  std::optional<uint32_t> s3MaxConnections() const;

  std::optional<int32_t> s3MaxAttempts() const;

  std::optional<std::string> s3RetryMode() const;

  std::string gcsEndpoint() const;

  std::string gcsScheme() const;

  std::string gcsCredentialsPath() const;

  std::optional<int> gcsMaxRetryCount() const;

  std::optional<std::string> gcsMaxRetryTime() const;

  bool isOrcUseColumnNames(const config::ConfigBase* session) const;

  bool isParquetUseColumnNames(const config::ConfigBase* session) const;

  bool isFileColumnNamesReadAsLowerCase(
      const config::ConfigBase* session) const;

  bool isPartitionPathAsLowerCase(const config::ConfigBase* session) const;

  bool allowNullPartitionKeys(const config::ConfigBase* session) const;

  bool ignoreMissingFiles(const config::ConfigBase* session) const;

  int64_t maxCoalescedBytes() const;

  int32_t maxCoalescedDistanceBytes() const;

  int32_t prefetchRowGroups() const;

  int32_t loadQuantum() const;

  int32_t numCacheFileHandles() const;

  bool isFileHandleCacheEnabled() const;

  uint64_t fileWriterFlushThresholdBytes() const;

  uint64_t orcWriterMaxStripeSize(const config::ConfigBase* session) const;

  uint64_t orcWriterMaxDictionaryMemory(
      const config::ConfigBase* session) const;

  bool isOrcWriterIntegerDictionaryEncodingEnabled(
      const config::ConfigBase* session) const;

  bool isOrcWriterStringDictionaryEncodingEnabled(
      const config::ConfigBase* session) const;

  bool orcWriterLinearStripeSizeHeuristics(
      const config::ConfigBase* session) const;

  uint64_t orcWriterMinCompressionSize(const config::ConfigBase* session) const;

  std::optional<uint8_t> orcWriterCompressionLevel(
      const config::ConfigBase* session) const;

  uint8_t orcWriterZLIBCompressionLevel(
      const config::ConfigBase* session) const;

  uint8_t orcWriterZSTDCompressionLevel(
      const config::ConfigBase* session) const;

  std::string writeFileCreateConfig() const;

  uint32_t sortWriterMaxOutputRows(const config::ConfigBase* session) const;

  uint64_t sortWriterMaxOutputBytes(const config::ConfigBase* session) const;

  uint64_t sortWriterFinishTimeSliceLimitMs(
      const config::ConfigBase* session) const;

  uint64_t footerEstimatedSize() const;

  uint64_t filePreloadThreshold() const;

  bool s3UseProxyFromEnv() const;

  // Returns the timestamp unit used when reading timestamps from files.
  uint8_t readTimestampUnit(const config::ConfigBase* session) const;

  /// Returns true to evict out a query scanned data out of in-memory cache
  /// right after the access, and also skip staging to the ssd cache. This helps
  /// to prevent the cache space pollution from the one-time table scan by large
  /// batch query when mixed running with interactive query which has high data
  /// locality.
  bool cacheNoRetention(const config::ConfigBase* session) const;

  HiveConfig(std::shared_ptr<const config::ConfigBase> config) {
    VELOX_CHECK_NOT_NULL(
        config, "Config is null for HiveConfig initialization");
    config_ = std::move(config);
    // TODO: add sanity check
  }

  const std::shared_ptr<const config::ConfigBase>& config() const {
    return config_;
  }

 private:
  std::shared_ptr<const config::ConfigBase> config_;
};

} // namespace facebook::velox::connector::hive
