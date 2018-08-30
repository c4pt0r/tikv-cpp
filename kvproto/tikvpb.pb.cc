// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: tikvpb.proto

#include "tikvpb.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)

namespace tikvpb {
}  // namespace tikvpb
namespace protobuf_tikvpb_2eproto {
void InitDefaults() {
}

const ::google::protobuf::uint32 TableStruct::offsets[1] = {};
static const ::google::protobuf::internal::MigrationSchema* schemas = NULL;
static const ::google::protobuf::Message* const* file_default_instances = NULL;

void protobuf_AssignDescriptors() {
  AddDescriptors();
  AssignDescriptors(
      "tikvpb.proto", schemas, file_default_instances, TableStruct::offsets,
      NULL, NULL, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\014tikvpb.proto\022\006tikvpb\032\021coprocessor.prot"
      "o\032\rkvrpcpb.proto\032\023raft_serverpb.proto\032\024g"
      "ogoproto/gogo.proto2\262\017\n\004Tikv\0224\n\005KvGet\022\023."
      "kvrpcpb.GetRequest\032\024.kvrpcpb.GetResponse"
      "\"\000\0227\n\006KvScan\022\024.kvrpcpb.ScanRequest\032\025.kvr"
      "pcpb.ScanResponse\"\000\022C\n\nKvPrewrite\022\030.kvrp"
      "cpb.PrewriteRequest\032\031.kvrpcpb.PrewriteRe"
      "sponse\"\000\022=\n\010KvCommit\022\026.kvrpcpb.CommitReq"
      "uest\032\027.kvrpcpb.CommitResponse\"\000\022=\n\010KvImp"
      "ort\022\026.kvrpcpb.ImportRequest\032\027.kvrpcpb.Im"
      "portResponse\"\000\022@\n\tKvCleanup\022\027.kvrpcpb.Cl"
      "eanupRequest\032\030.kvrpcpb.CleanupResponse\"\000"
      "\022C\n\nKvBatchGet\022\030.kvrpcpb.BatchGetRequest"
      "\032\031.kvrpcpb.BatchGetResponse\"\000\022R\n\017KvBatch"
      "Rollback\022\035.kvrpcpb.BatchRollbackRequest\032"
      "\036.kvrpcpb.BatchRollbackResponse\"\000\022C\n\nKvS"
      "canLock\022\030.kvrpcpb.ScanLockRequest\032\031.kvrp"
      "cpb.ScanLockResponse\"\000\022L\n\rKvResolveLock\022"
      "\033.kvrpcpb.ResolveLockRequest\032\034.kvrpcpb.R"
      "esolveLockResponse\"\000\0221\n\004KvGC\022\022.kvrpcpb.G"
      "CRequest\032\023.kvrpcpb.GCResponse\"\000\022L\n\rKvDel"
      "eteRange\022\033.kvrpcpb.DeleteRangeRequest\032\034."
      "kvrpcpb.DeleteRangeResponse\"\000\022;\n\006RawGet\022"
      "\026.kvrpcpb.RawGetRequest\032\027.kvrpcpb.RawGet"
      "Response\"\000\022J\n\013RawBatchGet\022\033.kvrpcpb.RawB"
      "atchGetRequest\032\034.kvrpcpb.RawBatchGetResp"
      "onse\"\000\022;\n\006RawPut\022\026.kvrpcpb.RawPutRequest"
      "\032\027.kvrpcpb.RawPutResponse\"\000\022J\n\013RawBatchP"
      "ut\022\033.kvrpcpb.RawBatchPutRequest\032\034.kvrpcp"
      "b.RawBatchPutResponse\"\000\022D\n\tRawDelete\022\031.k"
      "vrpcpb.RawDeleteRequest\032\032.kvrpcpb.RawDel"
      "eteResponse\"\000\022S\n\016RawBatchDelete\022\036.kvrpcp"
      "b.RawBatchDeleteRequest\032\037.kvrpcpb.RawBat"
      "chDeleteResponse\"\000\022>\n\007RawScan\022\027.kvrpcpb."
      "RawScanRequest\032\030.kvrpcpb.RawScanResponse"
      "\"\000\022S\n\016RawDeleteRange\022\036.kvrpcpb.RawDelete"
      "RangeRequest\032\037.kvrpcpb.RawDeleteRangeRes"
      "ponse\"\000\022M\n\014RawBatchScan\022\034.kvrpcpb.RawBat"
      "chScanRequest\032\035.kvrpcpb.RawBatchScanResp"
      "onse\"\000\022<\n\013Coprocessor\022\024.coprocessor.Requ"
      "est\032\025.coprocessor.Response\"\000\022D\n\021Coproces"
      "sorStream\022\024.coprocessor.Request\032\025.coproc"
      "essor.Response\"\0000\001\022;\n\004Raft\022\032.raft_server"
      "pb.RaftMessage\032\023.raft_serverpb.Done\"\000(\001\022"
      "A\n\010Snapshot\022\034.raft_serverpb.SnapshotChun"
      "k\032\023.raft_serverpb.Done\"\000(\001\022J\n\013SplitRegio"
      "n\022\033.kvrpcpb.SplitRegionRequest\032\034.kvrpcpb"
      ".SplitRegionResponse\"\000\022M\n\014MvccGetByKey\022\034"
      ".kvrpcpb.MvccGetByKeyRequest\032\035.kvrpcpb.M"
      "vccGetByKeyResponse\"\000\022Y\n\020MvccGetByStartT"
      "s\022 .kvrpcpb.MvccGetByStartTsRequest\032!.kv"
      "rpcpb.MvccGetByStartTsResponse\"\000B&\n\030com."
      "pingcap.tikv.kvproto\340\342\036\001\310\342\036\001\320\342\036\001b\006proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 2120);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "tikvpb.proto", &protobuf_RegisterTypes);
  ::protobuf_coprocessor_2eproto::AddDescriptors();
  ::protobuf_kvrpcpb_2eproto::AddDescriptors();
  ::protobuf_raft_5fserverpb_2eproto::AddDescriptors();
}

void AddDescriptors() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_tikvpb_2eproto
namespace tikvpb {

// @@protoc_insertion_point(namespace_scope)
}  // namespace tikvpb
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)